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

ext_irq_slave_o    : out t_wishbone_slave_out; 
ext_irq_slave_i    : in  t_wishbone_slave_in;
         
ext_lm32_master_o   : out t_wishbone_master_out; 
ext_ext_ext_lm32_master_i : in  t_wishbone_master_in;  

ext_ram_slave_o    : out t_wishbone_slave_out;                            
ext_ram_slave_i    : in  t_wishbone_slave_in

);
end ftm_lm32_cluster;

architecture rtl of ftm_lm32_cluster is 

constant null_sdbs : t_sdb_record_array(0 downto 1) := (others=>(others => '0'));

function f_string_fix_len
   ( s : string;
   ret_len : natural := 10;
   fill_char_c : character := '0' )
   return string is
      variable ret_v : string (1 to ret_len_c);
      constant pad_len_c : integer := ret_len_c - arg_str'length ;
      variable pad_v : string (1 to abs(pad_len_c));
   begin
      if pad_len_c < 1 then
      ret_v := arg_str(ret_v'range);
      else
      pad_v := (others => fill_char_c);
      ret_v := pad_v & arg_str;
      end if;
   return ret_v;
end f_string_fix_len;

function f_sdb_create_array(g_enum_dev_id    : boolean := false;
                            g_dev_id_offs    : natural := 0;
                            g_enum_dev_name  : boolean := false;
                            g_dev_name_offs  : natural := 0;       
                            device           : t_sdb_device; 
                            instances        : natural := 1)
    return t_sdb_record_array
is
   variable result   : t_sdb_record_array(instances-1 downto 0);  
   variable i,j, pos : natural;
   variable dev      : t_sdb_device;
   variable serial_no : string(1 to 3); 
   begin
      for i in 0 to instances-1 loop
         dev := device;         
         if(g_enum_dev_id) then         
            dev.sdb_component.product.device_id :=  
            std_logic_vector( unsigned(dev.sdb_component.product.device_id) 
                              + to_unsigned(i+g_dev_id_offs, dev.sdb_component.product.device_id'length));        
         end if;
         if(g_enum_dev_name) then         
         -- find end of name
            for j in dev.sdb_component.product.name'length downto 1 loop
               if(dev.sdb_component.product.name(j) /= ' ') then
                   report "Found non space " & dev.sdb_component.product.name(j) & "@" & integer'image(j)
                  severity note;                   
                  pos := j;                  
                  exit;
               end if;
            end loop;
         -- convert i+g_dev_name_offs to string
            serial_no := f_string_fix_len(integer'image(i+g_dev_name_offs), serial_no'length);
         -- check if space is sufficient
            assert (serial_no'length <= dev.sdb_component.product.name'length - pos)
            report "Not enough space in namestring of sdb_device " & dev.sdb_component.product.name & " to add serial number " & serial_no & 
                  ". Space available " & integer'image(dev.sdb_component.product.name'length-pos) & ", required " & integer'image(serial_no'length+1)    
            severity Failure;            
         -- insert
            dev.sdb_component.product.name(pos+1) := '_'; 
            for j in 1 to serial_no'length loop
               dev.sdb_component.product.name(pos+1+j) := serial_no(j);
            end loop;
               
         end if;
         result(i) := f_sdb_embed_device(dev, (others=>'1'));
      end loop;
   return result;
  end;

function f_sdb_join_arrays(a : t_sdb_record_array; b : t_sdb_record_array)
    return t_sdb_record_array
is
   variable result   : t_sdb_record_array(a'length+b'length-1 downto 0);  
   variable i : natural;
   begin
      for i in 0 to a'left loop
         result(i) := a(i);
      end loop;
      for i in 0 to b'left loop
         result(i+a'length) := b(i);
      end loop;
   return result;
  end;


function f_sdb_extract_base_addr(sdb_record : t_sdb_record)
   return std_logic_vector
  is
  begin
   return sdb_record(447 downto 384);
  end;

function f_sdb_extract_end_addr(sdb_record : t_sdb_record)
   return std_logic_vector
  is
  begin
   return sdb_record(383 downto 320);
  end;


function f_align_addr_offset(offs : unsigned; this_rng : unsigned; prev_rng : unsigned)
    return unsigned
  is
   variable this_pow, prev_pow   : natural;  
   variable start, env, result : unsigned(63 downto 0) := (others => '0');
   
  begin
      
   start(offs'left downto 0) := offs;   
   --calculate address envelopes (next power of 2) for previous and this component and choose the larger one
   this_pow := f_hot_to_bin(std_logic_vector(this_rng)); 
   prev_pow := f_hot_to_bin(std_logic_vector(prev_rng));    
   -- no max(). thank you very much, std_numeric :-/   
   if(this_pow >= prev_pow) then
      env(this_pow) := '1';
   else
      env(prev_pow) := '1';
   end if;
   --round up to the next multiple of the envelope...
   if(prev_rng /= 0) then   
      result := start + env - (start mod env);
   else
      result := start;   --...except for first element, result is start. 
   end if;
   return result;
  end;


 -- generates aligned address map for an sdb_record_array, accepts optional start offset 
function f_sdb_automap_array(sdb_array : t_sdb_record_array; start_offset : t_wishbone_address := (others => '0'))
    return t_sdb_record_array
  is
   variable this_rng    : unsigned(63 downto 0) := (others => '0');   
   variable prev_rng    : unsigned(63 downto 0) := (others => '0');   
   variable prev_offs   : unsigned(63 downto 0) := (others => '0');   
   variable this_offs   : unsigned(63 downto 0) := (others => '0');   
   variable device      : t_sdb_device;
   variable bridge      : t_sdb_bridge;  
   variable sdb_type    : std_logic_vector(7 downto 0);
   variable i           : natural;
   variable result      : t_sdb_record_array(sdb_array'length-1 downto 0); -- last 

  begin
   
   prev_offs(start_offset'left downto 0) := unsigned(start_offset);
   --traverse the array   
   for i in 0 to sdb_array'length-1 loop
      -- find the fitting extraction function by evaling the type byte. 
      -- could also use the component, but it's safer to use Wes' embed and extract functions.      
      sdb_type := sdb_array(i)(7 downto 0);
      case sdb_type is
         --device         
         when x"01"  => device      := f_sdb_extract_device(sdb_array(i));
                        this_rng    := unsigned(device.sdb_component.addr_last) - unsigned(device.sdb_component.addr_first);                      
                        this_offs   := f_get_aligned_offset(prev_offs, this_rng, prev_rng);
                        result(i)   := f_sdb_embed_device(device, std_logic_vector(this_offs(31 downto 0)));
         --bridge
         when x"02"  => bridge      := f_sdb_extract_bridge(sdb_array(i));
                        this_rng    := unsigned(bridge.sdb_component.addr_last) - unsigned(bridge.sdb_component.addr_first);
                        this_offs   := f_get_aligned_offset(prev_offs, this_rng, prev_rng);
                        result(i)   := f_sdb_embed_bridge(bridge, std_logic_vector(this_offs(31 downto 0)) );
         --other
         when others => result(i) := sdb_array(i);   
      end case;
      -- doesnt hurt because this_* doesnt change if its not a device or bridge
      prev_rng    := this_rng;
      prev_offs   := this_offs;
   end loop;

   return result;
  end;


  -- find place for sdb rom on crossbar and return address
  function f_sdb_create_rom_addr(sdb_array : t_sdb_record_array)
    return t_wishbone_address
  is
   constant rom_bytes            : natural := (2**f_ceil_log2(sdb_array'length + 1)) * (c_sdb_device_length / 8);
   variable result               : t_wishbone_address  := (others => '0');
   variable this_base, this_end  : unsigned(63 downto 0)          := (others => '0');    
   variable prev_base, prev_end  : unsigned(63 downto 0)          := (others => '0');
   variable rom_base             : unsigned(63 downto 0)          := (others => '0');
   variable sdb_type             : std_logic_vector(7 downto 0);     
   begin
   --traverse the array   
   for i in 0 to sdb_array'length-1 loop     
      sdb_type := sdb_array(i)(7 downto 0);
      if(sdb_type = x"01" or sdb_type = x"02") then
         -- get         
         this_base := unsigned(f_sdb_extract_base_addr(sdb_array(i)));
         this_end  := unsigned(f_sdb_extract_end_addr(sdb_array(i)));
         if(unsigned(result) = 0) then
            rom_base := f_get_aligned_offset(prev_base, to_unsigned(rom_bytes-1, 64), (prev_end-prev_base));
            if(rom_base + to_unsigned(rom_bytes, 64) <= this_base) then
               result := std_logic_vector(rom_base(t_wishbone_address'left downto 0));
            end if;   
         end if;
         prev_base := this_base;
         prev_end  := this_end;      
      end if;
   end loop;   
   -- if there was no gap to fit the sdb rom, place it at the end   
   if(unsigned(result) = 0) then
         result := std_logic_vector(f_get_aligned_offset(this_base, to_unsigned(rom_bytes-1, 64), this_end-this_base)(t_wishbone_address'left downto 0));   
   end if;      
   return result;
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
   ext_lm32_master_o  : out t_wishbone_master_out; 
   ext_ext_ext_lm32_master_i: in  t_wishbone_master_in;  
   -- wb msi interfaces
   irq_slaves_o   : out t_wishbone_slave_out_array(g_msi_queues-1 downto 0);  
   irq_slaves_i   : in  t_wishbone_slave_in_array(g_msi_queues-1 downto 0);
   -- port B of the LM32s DPRAM 
   ext_ram_slave_o    : out t_wishbone_slave_out;                           
   ext_ram_slave_i    : in  t_wishbone_slave_in

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
   constant c_lm32_layout   : t_sdb_record_array(c_lm32_slaves-1 downto 0) := 
   f_sdb_automap_array(f_sdb_join_arrays(f_sdb_create_array(  device            => c_irq_ep_sdb, 
                                                               instances         => g_cores,
                                                               g_enum_dev_id     => true,
                                                               g_dev_id_offs     => 0,
                                                               g_enum_dev_name   => true,
                                                               g_dev_name_offs   => 0), c_local_periphery),  x"00000000");
   
   constant c_lm32_sdb_address   : t_wishbone_address := f_sdb_create_rom_addr(c_lm32_layout);
 	
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
   
   ------------------------------------------------------------------------------
   -- there is no 'reverse' generic. this is awkward: since the master if(s) of
   -- the IRQ crossbar are slaves to the outside  world , all this might need 
   -- to be done in the top file as well so a possible higher level IRQ crossbar
   -- can insert us as a bridge.   
   constant c_irq_layout   : t_sdb_record_array(c_irq_slaves-1 downto 0) := 
   f_sdb_automap_array(f_sdb_create_array(device            => c_irq_ep_sdb, 
                                          instances         => c_irq_slaves,
                                          g_enum_dev_id     => true,
                                          g_dev_id_offs     => g_cores,
                                          g_enum_dev_name   => true,
                                          g_dev_name_offs   => g_cores),  x"00000000");
   
   constant c_irq_sdb_address       : t_wishbone_address := f_sdb_create_rom_addr(c_irq_layout);

   signal irq_cbar_masterport_in    : t_wishbone_master_in_array  (c_irq_slaves-1 downto 0);
   signal irq_cbar_masterport_out   : t_wishbone_master_out_array (c_irq_slaves-1 downto 0);
	signal irq_cbar_slaveport_in     : t_wishbone_slave_in_array   (c_irq_masters-1 downto 0);
   signal irq_cbar_slaveport_out    : t_wishbone_slave_out_array  (c_irq_masters-1 downto 0);

   --**************************************************************************--
   -- RAM CROSSBAR
   ------------------------------------------------------------------------------
   constant c_ram_slaves   : natural := g_cores;  
   constant c_ram_masters  : natural := 1;       
   
   ------------------------------------------------------------------------------
   -- there is no 'reverse' generic. this is awkward: since the master of the  
   -- RAM crossbar is a slave to the outside  world (top crossbar), all this 
   -- needs to be done in the top file as well so the top crossbar can insert us
   -- as a bridge.          
   constant c_ram_layout   : t_sdb_record_array(c_irq_slaves-1 downto 0) := 
   f_sdb_automap_array(f_sdb_create_array(device            => f_xwb_dpram(g_ram_per_core), 
                                          instances         => g_cores,
                                          g_enum_dev_id     => true,
                                          g_dev_id_offs     => g_cores,
                                          g_enum_dev_name   => true,
                                          g_dev_name_offs   => g_cores),  x"00000000");
   
   constant c_ram_sdb_address       : t_wishbone_address := f_sdb_create_rom_addr(c_ram_layout);
   ------------------------------------------------------------------------------

   signal ram_cbar_masterport_in    : t_wishbone_master_in_array  (c_ram_slaves-1 downto 0);
   signal ram_cbar_masterport_out   : t_wishbone_master_out_array (c_ram_slaves-1 downto 0);
	signal ram_cbar_slaveport_in     : t_wishbone_slave_in_array   (c_ram_masters-1 downto 0);
   signal ram_cbar_slaveport_out    : t_wishbone_slave_out_array  (c_ram_masters-1 downto 0);

 
 ----------------------------------------------------------------------------------
  
   G1: for I in 0 to g_cores-1 generate
    
      --instantiate an ftm-lm32 (LM32 core with its own DPRAM and 4..n msi queues)
      LM32 : ftm_lm32
      generic map(g_size         => g_ram_per_core,
                  g_bridge_sdb   => c_lm32_bridge_sdb,
                  g_profile      => g_profile,
                  g_init_file    => g_init_files(I),
                  g_msi_queues   => g_msi_per_core);
      port map(clk_sys_i         => clk_sys_i,
               rst_n_i           => r_rst_n(I),
               ext_lm32_master_o     => lm32_cbar_slaveport_in  (I),
               ext_ext_ext_lm32_master_i   => lm32_cbar_slaveport_out (I), 
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
               ext_ram_slave_o       => ram_cbar_masterport_in(I),                      
               ext_ram_slave_i       => ram_cbar_masterport_out(I));
      
      end generate;  
  
   LM32_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_lm32_masters,
     g_num_slaves  => c_lm32_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_lm32_layout,
     g_sdb_addr    => c_lm32_sdb_address)
   port map(
     clk_sys_i     => clk_sys,
     rst_n_i       => rstn_sys,
     -- Master connections (INTERCON is a slave)
     slave_i       => lm32_cbar_slaveport_i,
     slave_o       => lm32_cbar_slaveport_o,
     -- Slave connections (INTERCON is a master)
     master_i      => lm32_cbar_masterport_in,
     master_o      => lm32_cbar_masterport_out);

   -- last slave on the lm32 crossbar is the connection to the periphery crossbar
   ext_lm32_master_o                         => lm32_cbar_masterport_out(c_lm32_slaves-1);
   lm32_cbar_masterport_in(c_lm32_slaves-1)  => ext_lm32_master_i;  

   IRQ_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_irq_masters,
     g_num_slaves  => c_irq_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_irq_layout,
     g_sdb_addr    => c_irq_sdb_address)
   port map(
     clk_sys_i     => clk_sys,
     rst_n_i       => rstn_sys,
     -- Master connections (INTERCON is a slave)
     slave_i       => irq_cbar_slaveport_in,
     slave_o       => irq_cbar_slaveport_out,
     -- Slave connections (INTERCON is a master)
     master_i      => irq_cbar_masterport_in,
     master_o      => irq_cbar_masterport_out);

   ext_irq_slave_o            => irq_cbar_masterport_out(0);
   irq_cbar_slaveport_in(0)   => ext_irq_slave_i;

   RAM_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_ram_masters,
     g_num_slaves  => c_ram_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_ram_layout,
     g_sdb_addr    => c_ram_sdb_address)
   port map(
     clk_sys_i     => clk_sys,
     rst_n_i       => rstn_sys,
        -- Master connections (INTERCON is a slave)
     slave_i       => ram_cbar_slaveport_in,
     slave_o       => ram_cbar_slaveport_out,
     -- Slave connections (INTERCON is a master)
     master_i      => ram_cbar_masterport_in,
     master_o      => ram_cbar_masterport_out);

     ext_ram_slave_o          <= ram_cbar_slaveport_in(0);                           
     ram_cbar_slaveport_in(0) <= ext_ram_slave_i; 
 
  
