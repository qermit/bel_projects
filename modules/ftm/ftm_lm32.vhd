library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.wb_irq_pkg.all;

entity ftm_lm32 is
generic(g_cpu_id        : t_wishbone_data := x"CAFEBABE";
        g_size          : natural := 16384;                 -- size of the dpram
        g_bridge_sdb    : t_sdb_bridge;                     -- record for the superior bridge
        g_profile       : string := "medium_icache_debug";  -- lm32 profile
        g_init_file     : string := "";          -- memory init file - binary for lm32
        g_addr_ext_bits : natural := 1;                     -- address extension bits (starting from MSB)
        g_msi_queues    : natural := 3);                    -- number of msi queues connected to the lm32
port(
clk_sys_i      : in  std_logic;  -- system clock 
rst_n_i        : in  std_logic;  -- reset, active low 
rst_lm32_n_i   : in  std_logic;  -- reset, active low

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

architecture rtl of ftm_lm32 is 
    
    constant c_adr_ext_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"0000000000000007",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"10040084",
    version       => x"00000001",
    date          => x"20131009",
    name          => "ADDR_EXTENSION     ")));
  
   constant c_cpu_id_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"0000000000000003",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"10040085",
    version       => x"00000001",
    date          => x"20131009",
    name          => "CPU_ID_ROM         "))); 

   -- crossbar layout
   constant c_lm32_slaves        : natural := 5;
   constant c_lm32_masters       : natural := 2;
   constant c_lm32_layout        : t_sdb_record_array(c_lm32_slaves-1 downto 0) :=
   (0 => f_sdb_embed_device(f_xwb_dpram(g_size),   x"00000000"),
    1 => f_sdb_embed_device(c_irq_ctrl_sdb,        x"7FFFFE00"),
    2 => f_sdb_embed_device(c_cpu_id_sdb,          x"7FFFFFF4"),
    3 => f_sdb_embed_device(c_adr_ext_sdb,         x"7FFFFFF8"),  
    4 => f_sdb_embed_bridge(g_bridge_sdb,          x"80000000"));
   constant c_lm32_sdb_address    : t_wishbone_address := x"7FFFFC00";
 
   --signals
   signal lm32_idwb_master_in    : t_wishbone_master_in_array(c_lm32_masters-1 downto 0);
   signal lm32_idwb_master_out   : t_wishbone_master_out_array(c_lm32_masters-1 downto 0);
   signal lm32_cb_master_in      : t_wishbone_master_in_array(c_lm32_slaves-1 downto 0);
   signal lm32_cb_master_out     : t_wishbone_master_out_array(c_lm32_slaves-1 downto 0);
   signal s_irq : std_logic_vector(31 downto 0);
   signal r_addr_ext             : std_logic_vector(g_addr_ext_bits-1 downto 0);
   signal rst_lm32_n             : std_logic;

begin
--------------------------------------------------------------------------------
-- Crossbar
-------------------------------------------------------------------------------- 
   LM32_CON : xwb_sdb_crossbar
   generic map(
      g_num_masters => c_lm32_masters,
      g_num_slaves  => c_lm32_slaves,
      g_registered  => true,
      g_wraparound  => true,
      g_layout      => c_lm32_layout,
      g_sdb_addr    => c_lm32_sdb_address)
   port map(
      clk_sys_i     => clk_sys_i,
      rst_n_i       => rst_n_i,
      -- Master connections (INTERCON is a slave)
      slave_i       => lm32_idwb_master_out,
      slave_o       => lm32_idwb_master_in,
      -- Slave connections (INTERCON is a master)
      master_i      => lm32_cb_master_in,
      master_o      => lm32_cb_master_out);

--------------------------------------------------------------------------------
-- Master 0 & 1 - LM32
--------------------------------------------------------------------------------  
   LM32_CORE : xwb_lm32
   generic map(g_profile => g_profile)
   port map(
      clk_sys_i   => clk_sys_i,
      rst_n_i     => rst_lm32_n,
      irq_i       => s_irq,
      dwb_o       => lm32_idwb_master_out(0),
      dwb_i       => lm32_idwb_master_in(0),
      iwb_o       => lm32_idwb_master_out(1),
      iwb_i       => lm32_idwb_master_in(1));

rst_lm32_n <= rst_n_i and rst_lm32_n_i;

--------------------------------------------------------------------------------
-- Slave 0 - DPRAM A side
--------------------------------------------------------------------------------
   DPRAM : xwb_dpram
   generic map(
      g_size                  => g_size,
      g_init_file             => g_init_file,
      g_must_have_init_file   => false,
      g_slave1_interface_mode => PIPELINED,
      g_slave2_interface_mode => PIPELINED,
      g_slave1_granularity    => BYTE,
      g_slave2_granularity    => BYTE)  
   port map(
      clk_sys_i   => clk_sys_i,
      rst_n_i     => rst_n_i,
      slave1_i    => lm32_cb_master_out(0),
      slave1_o    => lm32_cb_master_in(0),
      slave2_i    => ram_slave_i,
      slave2_o    => ram_slave_o);

--------------------------------------------------------------------------------
-- Slave 1 - MSI-IRQ
--------------------------------------------------------------------------------
   MSI_IRQ: wb_irq_slave 
   GENERIC MAP( g_queues  => g_msi_queues,
                g_depth   => 8)
   PORT MAP (
      clk_i         => clk_sys_i,
      rst_n_i       => rst_n_i,  
           
      irq_slave_o   => irq_slaves_o, 
      irq_slave_i   => irq_slaves_i,
      irq_o         => s_irq(g_msi_queues-1 downto 0),
           
      ctrl_slave_o  => lm32_cb_master_in(1),
      ctrl_slave_i  => lm32_cb_master_out(1));

   s_irq(31 downto g_msi_queues) <= (others => '0');


--------------------------------------------------------------------------------
-- Slave 2 - ROM ID 
--------------------------------------------------------------------------------
-- physical address extension (yes, it's dirty, but the best way for now)   
   rom_id : process(clk_sys_i)
   begin
    if rising_edge(clk_sys_i) then
      -- This is an easy solution for a device that never stalls:
      lm32_cb_master_in(2).ack <= lm32_cb_master_out(2).cyc and lm32_cb_master_out(2).stb;
    end if;
  end process;   

lm32_cb_master_in(2).dat <= g_cpu_id;

--------------------------------------------------------------------------------
-- Slave 3 - External LM32 Master interface 
--------------------------------------------------------------------------------
   -- physical address extension (yes, it's dirty, but the best way for now)   
   addr_ext : process(clk_sys_i)
   begin
    if rising_edge(clk_sys_i) then
      -- This is an easy solution for a device that never stalls:
      lm32_cb_master_in(3).ack <= lm32_cb_master_out(4).cyc and lm32_cb_master_out(4).stb;
      lm32_cb_master_in(3).dat <= (others => '0');
      
      if rst_n_i = '0' then
        r_addr_ext <= (others => '0');
      else
        -- Detect a write to the register byte
        if lm32_cb_master_out(4).cyc = '1' and lm32_cb_master_out(4).stb = '1' and
           lm32_cb_master_out(4).we = '1' and lm32_cb_master_out(4).sel(0) = '1' then
          case(to_integer(unsigned(lm32_cb_master_out(4).adr(2 downto 2)))) is
            when 0 => r_addr_ext <= lm32_cb_master_out(4).dat(31 downto 32-g_addr_ext_bits);
            when others => null;
          end case;
        end if;
        
        case to_integer(unsigned(lm32_cb_master_out(4).adr(4 downto 2))) is
          when 0 => lm32_cb_master_in(3).dat(31 downto 32-g_addr_ext_bits) <= r_addr_ext ;
          when 1 => lm32_cb_master_in(3).dat <= (others => '0');--std_logic_vector(to_unsigned(2**32 - 2**(32-g_addr_ext_bits), 32));
          when others => null;
        end case;
      end if;
    end if;
  end process;   


   lm32_master_o.cyc       <= lm32_cb_master_out(4).cyc;
   lm32_master_o.stb       <= lm32_cb_master_out(4).stb;
   lm32_master_o.we        <= lm32_cb_master_out(4).we;
   lm32_master_o.sel       <= lm32_cb_master_out(4).sel;
   lm32_master_o.dat       <= lm32_cb_master_out(4).dat;
   lm32_master_o.adr(31-g_addr_ext_bits downto 0)  <= lm32_cb_master_out(4).adr(31-g_addr_ext_bits downto 0);
   lm32_master_o.adr(31 downto 32-g_addr_ext_bits) <= r_addr_ext;
   
   lm32_cb_master_in(4)   <= lm32_master_i;

end rtl;
  
