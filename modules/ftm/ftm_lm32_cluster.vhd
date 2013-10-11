library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.wb_irq_pkg.all;

entity ftm_lm32_cluster is
generic(g_cores         : natural := 3
        g_ram_per_core  : natural := 16384;
        g_msi_per_core  : natural := 4;
        g_profile       : string := "medium_icache_debug";
        g_bridge_con    : t_sdb_bridge);
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

-- generate device/address map for cluster
-- accepts a device <multi> it will place <instances> times followed by a list of devices to be placed after that
function f_create_cluster_layout(multi : t_sdb_device; instances : natural; singles : t_sdb_record_array)
    return t_sdb_record_array
  is
   variable border         : natural := (instances + singles'length)-1;      
   variable result         : t_sdb_record_array;
   variable offset_aux     : natural;   
   variable offset_multi   : t_wishbone_address;
   variable offset_single_start : t_wishbone_address;
  begin
   --calculate aligned offset for a multi instance   
   offset_aux_multi := (2**to_integer(unsigned(f_hot_to_bin(multi.sdb_component.addr_last)))));   
   offset_multi := t_wishbone_address(to_unsigned(offset_aux_multi, t_wishbone_address'length));
   --calculate aligned offset at the end of multis/for all singles 
   offset_single_start_aux := (2**to_integer(unsigned(f_hot_to_bin(offset_aux*instances)))));
   offset_single_start := t_wishbone_address(to_unsigned(offset_single_start_aux, t_wishbone_address'length));
   --calculate invidiual aligned single offsets from their component address range
   

   for i in 0 to instances-1 loop               -- ascii to string
      result(i) := character'val(to_integer(unsigned(sdb_record(159-i*8 downto 152-i*8))));
   end loop;
   return result;
  end;

   component ftm_lm32 is
   generic(g_size          : natural := 16384;                 -- size of the dpram
           g_bridge_sdb    : t_sdb_bridge;                     -- record for the superior bridge
           g_profile       : string := "medium_icache_debug";  -- lm32 profile
           g_init_file     : string := "";                     -- memory init file - binary for lm32
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
   end ftm_lm32;

   constant c_ext_msi = g_msi_per_core -1; -- lm32 irq sources are not masters of irq crossbar to reduce fan out

   -- lm32 crossbar. this is the main crossbar of the FTM, and it's BIG ...
   constant c_lm32_slaves   : natural := g_cores+3; -- an irq queue per lm32 + eca + shared mem + ext interface out
   constant c_lm32_masters  : natural := g_cores+1; -- lm32's
   

   constant c_lm32_layout   : t_sdb_record_array(c_lm32_slaves-1 downto 0) :=
   (0 => f_sdb_embed_device(f_xwb_dpram(g_ram_per_core),        x"00000000"),
    1 => f_sdb_embed_bridge(c_wrcore_bridge_sdb,                x"80000000"));
   constant c_lm32_sdb_address : t_wishbone_address := x"FFFFFF00";
 	
   signal lm32_cbar_masterport_in   : t_wishbone_master_in_array  (c_lm32_slaves-1 downto 0);
   signal lm32_cbar_masterport_out  : t_wishbone_master_out_array (c_lm32_slaves-1 downto 0);
	signal lm32_cbar_slaveport_in    : t_wishbone_slave_in_array   (c_lm32_masters-1 downto 0);
   signal lm32_cbar_slaveport_out   : t_wishbone_slave_out_array  (c_lm32_masters-1 downto 0);


   -- irq crossbar
   constant c_irq_slaves   : natural := g_cores*c_ext_msi;  -- all but one irq queue per lm32 are connected here
   constant c_irq_masters  : natural := 3;                  -- eca, interlock, others

   signal irq_cbar_masterport_in    : t_wishbone_master_in_array  (c_irq_slaves-1 downto 0);
   signal irq_cbar_masterport_out   : t_wishbone_master_out_array (c_irq_slaves-1 downto 0);
	signal irq_cbar_slaveport_in     : t_wishbone_slave_in_array   (c_irq_masters-1 downto 0);
   signal irq_cbar_slaveport_out    : t_wishbone_slave_out_array  (c_irq_masters-1 downto 0);

   constant c_lm32_bridge_sdb  : t_sdb_bridge       :=
   f_xwb_bridge_layout_sdb(true, c_per_layout, c_per_sdb_address);

   -- ram crossbar   

 
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
 
  
