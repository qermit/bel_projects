library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.wb_irq_pkg.all;

entity ftm_lm32_cluster is
generic(g_cores         : natural := 3;
        g_ram_per_core  : natural := 32768/4;
        g_msi_per_core  : natural := 4;
        g_profile       : string := "medium_icache_debug";  
        g_bridge_sdb    : t_sdb_bridge                      -- periphery crossbar         
   );
port(
clk_sys_i      : in  std_logic;
rst_n_i        : in  std_logic;

irq_slave_o    : out t_wishbone_slave_out; 
irq_slave_i    : in  t_wishbone_slave_in;
         
lm32_master_o   : out t_wishbone_master_out; 
lm32_master_i   : in  t_wishbone_master_in;  

ram_slave_o    : out t_wishbone_slave_out;                            
ram_slave_i    : in  t_wishbone_slave_in

);
end ftm_lm32_cluster;

architecture rtl of ftm_lm32_cluster is 

constant null_sdbs : t_sdb_record_array(0 downto 1) := (others=>(others => '0'));

function f_create_cluster(multi : t_sdb_device; instances : natural; singles : t_sdb_record_array)
    return t_sdb_record_array
is
   variable result   : t_sdb_record_array(singles'length+instances-1 downto 0);  
   variable i : natural;
   begin
      for i in 0 to instances-1 loop
         result(i) := f_sdb_embed_device(multi, x"00000000");
      end loop;
      for i in 0 to singles'left loop
         result(i+instances) := singles(i);
      end loop;
   return result;
  end;



function f_get_aligned_offset(offs : std_logic_vector; this_rng : std_logic_vector; prev_rng : std_logic_vector)
    return std_logic_vector
  is
   variable result   : std_logic_vector(63 downto 0);  
   variable start, env, env_prev, env_this, aux : natural;
   
  begin
      
   start := to_integer(unsigned(offs));   
   --calculate address envelopes for previous and this component and choose the larger one
   env_prev := (2**f_hot_to_bin(prev_rng));   
   env_this := (2**f_hot_to_bin(this_rng));
   if(env_this >= env_prev) then
      env := env_this;
   else
      env := env_prev;
   end if;

   --round up to the next multiple of the envelope...
   if(unsigned(prev_rng) /= 0) then   
      aux := start + env - (start mod env);
   else
      aux := 0;   --...except for offset 0, result is also 0. 
   end if;
   result := std_logic_vector(to_unsigned(aux, result'length));
   
   --report "o " & f_bits2string(offs) & " rt " & f_bits2string(this_rng) & " rp " & f_bits2string(prev_rng) & " res " & f_bits2string(result)
   --severity Note;   

   return result;
  end;


constant dummy_product : t_sdb_product := (  vendor_id => (others=>'0'),
                                             device_id => (others=>'0'),
                                             version => (others=>'0'),
                                             date => (others=>'0'),   
                                             name => (others=>'0'));
  
constant dummy_comp : t_sdb_component := (   addr_first  => (others=>'0'),
                                             addr_last   => (others=>'0'),
                                             product     => dummy_product);



-- regenerates aligned addresses for an sdb_record_array + a dummy component for the sdb rom 
function f_create_meta_layout(sdb_array : t_sdb_record_array)
    return t_sdb_record_array
  is
   variable prev_rng,tmp_rng    : std_logic_vector(63 downto 0) := (others => '0');   
   variable prev_offs   : std_logic_vector(63 downto 0) := (others => '0');   
   variable this_offs   : std_logic_vector(63 downto 0) := (others => '0');   
   variable device      : t_sdb_device;
   variable bridge      : t_sdb_bridge;  
   variable sdb_type    : std_logic_vector(7 downto 0);
   variable i           : natural;
   variable result      : t_sdb_record_array(sdb_array'length downto 0); -- last 
   variable rom_comp    : t_sdb_component := dummy_comp;
   variable rom_bytes   : natural := (2**f_ceil_log2(sdb_array'length + 1)) * (c_sdb_device_length / 8);
  begin
  
   --traverse the array   
   for i in 0 to sdb_array'length-1 loop
      -- find the fitting extraction function by evaling the type byte      
      sdb_type := sdb_array(i)(7 downto 0);
      case sdb_type is
         --device         
         when x"01"  => device      := f_sdb_extract_device(sdb_array(i));
                        this_offs   := f_get_aligned_offset(prev_offs, device.sdb_component.addr_last, prev_rng);
                        result(i)   := f_sdb_embed_device(device, this_offs(31 downto 0));
                        tmp_rng    := device.sdb_component.addr_last;
         --bridge
         when x"02"  => bridge      := f_sdb_extract_bridge(sdb_array(i));
                        this_offs   := f_get_aligned_offset(prev_offs, bridge.sdb_component.addr_last, prev_rng);
                        result(i)   := f_sdb_embed_bridge(bridge, this_offs(31 downto 0));
                        tmp_rng    := bridge.sdb_component.addr_last;
         --other
         when others => result(i) := sdb_array(i);   
      end case;
        
      report "### " & integer'image(i) & "/" & integer'image(sdb_array'length-1) & " to " & f_bits2string(this_offs) & " po " & f_bits2string(prev_offs) & " rt " & f_bits2string(tmp_rng)
         severity Note;
      if(unsigned(this_offs) - (unsigned(prev_offs) + unsigned(prev_rng)) >= rom_bytes-1) and (unsigned(rom_comp.addr_last) = 0) then
                   
         rom_comp.addr_last := f_get_aligned_offset(prev_offs, std_logic_vector(to_unsigned(rom_bytes-1, 64)), prev_rng);
         --report "jetzt " & f_bits2string(rom_comp.addr_last) severity Note;      
      end if;
      prev_rng  := tmp_rng;
      prev_offs := this_offs;
   end loop;
   if(unsigned(rom_comp.addr_last) = 0) then
         report "ist immer noch null" severity Note; 
         rom_comp.addr_last := f_get_aligned_offset(prev_offs, std_logic_vector(to_unsigned(rom_bytes-1, 64)), prev_rng);   
   end if;
   result(result'left) := (others => '0');   
   result(result'left)(447 downto 8) := f_sdb_embed_component(rom_comp, (others => '0'));
   return result;
  end;

  -- returns layout sdb_record_array from crossbar meta layout
  function f_get_layout(sdb_array : t_sdb_record_array)
    return t_sdb_record_array
  is
  begin
   return sdb_array(sdb_array'left-1 downto 0);
  end;

  -- returns sdb rom address from crossbar meta layout
  function f_get_sdb_address(sdb_array : t_sdb_record_array)
    return t_wishbone_address
  is
   variable comp      : t_sdb_component;  
   begin
      comp := f_sdb_extract_component(sdb_array(sdb_array'left)(447 downto 8));
      return comp.addr_last(t_wishbone_address'left downto 0);
  end;   



   component ftm_lm32 is
   generic(g_size          : natural := 16384;                 -- size of the dpram
           --g_bridge_sdb    : t_sdb_bridge;                     -- record for the superior bridge
           g_profile       : string := "medium_icache_debug";  -- lm32 profile
           g_init_file     : string := "msidemo.bin";                     -- memory init file - binary for lm32
           g_addr_ext_bits : natural := 1;                     -- address extension bits (starting from MSB)
           g_msi_queues    : natural := 4);                    -- number of msi queues connected to the lm32
   port(
   clk_sys_i      : in  std_logic;  -- system clock 
   rst_n_i        : in  std_logic;  -- reset, active low 

   -- wb master interface of the lm32
   lm32_master_o  : out t_wishbone_master_out; 
   lm32_master_i  : in  t_wishbone_master_in;  
   -- wb msi interfaces
   irq_slaves_o   : out t_wishbone_slave_out_array(g_msi_queues-1 downto 0);  
   irq_slaves_i   : in  t_wishbone_slave_in_array(g_msi_queues-1 downto 0);
   -- port B of the LM32s DPRAM 
   ram_slave_o    : out t_wishbone_slave_out;                           
   ram_slave_i    : in  t_wishbone_slave_in

   );
   end component;

   
   --**************************************************************************--
   -- dummy periphery crossbar for testing
   ------------------------------------------------------------------------------
   constant c_per_slaves   : natural := 2;
   constant c_per_masters  : natural := 2;
   constant c_per_layout   : t_sdb_record_array(c_per_slaves-1 downto 0) :=
   (0 => f_sdb_embed_device(f_xwb_dpram(4096/4),   x"00000000"),
    1 => f_sdb_embed_device(f_xwb_dpram(16384/4),   x"00000000"));

  constant c_per_sdb_address : t_wishbone_address := x"000F0000";
  constant c_per_bridge_sdb  : t_sdb_bridge       :=
    f_xwb_bridge_layout_sdb(true, c_per_layout, c_per_sdb_address);	
   ------------------------------------------------------------------------------

   --**************************************************************************--
   -- LM32 CROSSBAR. this is the main crossbar of the FTM, and it's BIG ...
   ------------------------------------------------------------------------------
   constant c_local_periphery : t_sdb_record_array(2 downto 0) :=
   (  0 => f_sdb_embed_device(c_eca_sdb,                    x"00000000"),
      1 => f_sdb_embed_device(c_eca_evt_sdb,                x"00000000"),
      2 => f_sdb_embed_bridge(c_per_bridge_sdb,             x"00000000"));
   
   constant c_lm32_slaves   : natural := g_cores+c_local_periphery'length; -- an irq queue per lm32 + eca + ext interface out
   constant c_lm32_masters  : natural := g_cores; -- lm32's
   constant c_lm32_meta     : t_sdb_record_array(c_lm32_slaves downto 0) :=
   f_create_meta_layout(f_create_cluster(c_irq_ep_sdb, g_cores, c_local_periphery)); 
   constant c_lm32_layout        : t_sdb_record_array(c_lm32_slaves-1 downto 0) := f_get_layout(c_lm32_meta);
   constant c_lm32_sdb_address   : t_wishbone_address := f_get_sdb_address(c_lm32_meta);
 	
   signal lm32_cbar_masterport_in   : t_wishbone_master_in_array  (c_lm32_slaves-1 downto 0);
   signal lm32_cbar_masterport_out  : t_wishbone_master_out_array (c_lm32_slaves-1 downto 0);
	signal lm32_cbar_slaveport_in    : t_wishbone_slave_in_array   (c_lm32_masters-1 downto 0);
   signal lm32_cbar_slaveport_out   : t_wishbone_slave_out_array  (c_lm32_masters-1 downto 0);
   ------------------------------------------------------------------------------
   
   --**************************************************************************--
   -- IRQ CROSSBAR
   ------------------------------------------------------------------------------
   constant c_ext_msi      : natural := g_msi_per_core -1;  -- lm32 irq sources are not masters of irq crossbar to reduce fan out
   constant c_irq_slaves   : natural := g_cores*c_ext_msi;  -- all but one irq queue per lm32 are connected here
   constant c_irq_masters  : natural := 2;                  -- eca action queues, interlocks
   constant c_irq_layout   : t_sdb_record_array(c_irq_slaves-1 downto 0) :=
   f_align_records(f_create_cluster(c_irq_ep_sdb, c_ext_msi));
   constant c_lm32_sdb_address : t_wishbone_address := x"FFFFF00";

   null_sdbs

   signal irq_cbar_masterport_in    : t_wishbone_master_in_array  (c_irq_slaves-1 downto 0);
   signal irq_cbar_masterport_out   : t_wishbone_master_out_array (c_irq_slaves-1 downto 0);
	signal irq_cbar_slaveport_in     : t_wishbone_slave_in_array   (c_irq_masters-1 downto 0);
   signal irq_cbar_slaveport_out    : t_wishbone_slave_out_array  (c_irq_masters-1 downto 0);

  

 
 ----------------------------------------------------------------------------------
  
   G1: for I in 0 to g_cores-1 generate
    
      --instantiate an ftm-lm32 (LM32 core with its own DPRAM and 4-n msi queues)
      LM32 : ftm_lm32
      generic map(g_size         => g_ram_per_core,
                  g_bridge_sdb   => c_lm32_bridge_sdb,
                  g_profile      => g_profile,
                  g_init_file    => g_init_files(I),
                  g_msi_queues   => g_msi_per_core);
      port map(clk_sys_i         => clk_sys_i,
               rst_n_i           => r_rst_n(I),
               lm32_master_o     => lm32_cbar_slaveport_in  (I),
               lm32_master_i     => lm32_cbar_slaveport_out (I), 
               --highest prio irq from eca               
               irq_slaves_o(0)   => irq_cbar_masterport_in  (I*c_ext_msi+0),
               irq_slaves_i(0)   => irq_cbar_masterport_out (I*c_ext_msi+0),
               --second prio irq from other LM32s               
               irq_slaves_o(1)   => lm32_cbar_masterport_in (I),
               irq_slaves_i(1)   => lm32_cbar_masterport_in (I),
               --third prio irq from interlocks               
               irq_slaves_o(2)   => irq_cbar_masterport_in  (I*c_ext_msi+1),
               irq_slaves_i(2)   => irq_cbar_masterport_out (I*c_ext_msi+1),
               --fourth to xth prio irq from all others               
               irq_slaves_o(g_msi_per_core-1-1 downto 3) => irq_cbar_masterport_in  ((I+1)*c_ext_msi-1 downto I*c_ext_msi+2),
               irq_slaves_i(g_msi_per_core-1-1 downto 3) => irq_cbar_masterport_out ((I+1)*c_ext_msi-1 downto I*c_ext_msi+2),
               --RAM & FTM periphery crossbar 
               ram_slave_o       => ram_cbar_masterport_in(I),                      
               ram_slave_i       => ram_cbar_masterport_out(I));
      
      end generate;  
  
   LM32_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_per_masters,
     g_num_slaves  => c_per_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_per_layout,
     g_sdb_addr    => c_per_sdb_address)
   port map(
     clk_sys_i     => clk_sys,
     rst_n_i       => rstn_sys,
     -- Master connections (INTERCON is a slave)
     slave_i       => per_cbar_slave_i,
     slave_o       => per_cbar_slave_o,
     -- Slave connections (INTERCON is a master)
     master_i      => per_cbar_master_i,
     master_o      => per_cbar_master_o);

   IRQ_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_per_masters,
     g_num_slaves  => c_per_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_per_layout,
     g_sdb_addr    => c_per_sdb_address)
   port map(
     clk_sys_i     => clk_sys,
     rst_n_i       => rstn_sys,
     -- Master connections (INTERCON is a slave)
     slave_i       => per_cbar_slave_i,
     slave_o       => per_cbar_slave_o,
     -- Slave connections (INTERCON is a master)
     master_i      => per_cbar_master_i,
     master_o      => per_cbar_master_o);

   RAM_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_per_masters,
     g_num_slaves  => c_per_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_per_layout,
     g_sdb_addr    => c_per_sdb_address)
   port map(
     clk_sys_i     => clk_sys,
     rst_n_i       => rstn_sys,
     -- Master connections (INTERCON is a slave)
     slave_i       => per_cbar_slave_i,
     slave_o       => per_cbar_slave_o,
     -- Slave connections (INTERCON is a master)
     master_i      => per_cbar_master_i,
     master_o      => per_cbar_master_o);
 
  
