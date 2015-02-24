library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

library work;
use work.gencores_pkg.all;
use work.wishbone_pkg.all;
use work.genram_pkg.all;
use work.scu_bus_pkg.all;
use work.aux_functions_pkg.all;
use work.adc_pkg.all;
use work.dac714_pkg.all;
use work.fg_quad_pkg.all;
use work.altera_flash_pkg.all;
use work.addac_sys_clk_local_clk_switch_pkg.all;

entity scu_addac is
  generic(
    g_cid_group: integer := 38
    );
  port (
    -------------------------------------------------------------------------------------------------------------------
--    CLK_FPGA: in std_logic;
    
    --------- Parallel SCU-Bus-Signale --------------------------------------------------------------------------------
    A_A:                  inout std_logic_vector(15 downto 0);  -- SCU-Adressbus
    A_nADR_EN:            out   std_logic := '0';               -- '0' => externe Adresstreiber des Slaves aktiv
    A_nADR_FROM_SCUB:     out   std_logic := '0';               -- '0' => externe Adresstreiber-Richtung: SCU-Bus nach Slave
    A_D:                  inout std_logic_vector(15 downto 0);  -- SCU-Datenbus
    A_nDS:                in    std_logic;                      -- Data-Strobe vom Master gertieben
    A_RnW:                in    std_logic;                      -- Schreib/Lese-Signal vom Master getrieben, '0' => lesen
    A_nSel_Ext_Data_Drv:  out   std_logic;                      -- '0' => externe Datentreiber des Slaves aktiv
    A_Ext_Data_RD:        out   std_logic;                      -- '0' => externe Datentreiber-Richtung: SCU-Bus nach
                                                                -- Slave (besser default 0, oder Treiber A/B tauschen)
                                                                -- SCU-Bus nach Slave (besser default 0, oder Treiber A/B tauschen)
    A_nDtack:             out   std_logic;                      -- Data-Acknowlege null aktiv, '0' => aktiviert externen
                                                                -- Opendrain-Treiber
    A_nSRQ:               out   std_logic;                      -- Service-Request null aktiv, '0' => aktiviert externen
                                                                -- Opendrain-Treiber
    A_nBoardSel:          in    std_logic;                      -- '0' => Master aktiviert diesen Slave
    A_nEvent_Str:         in    std_logic;                      -- '0' => Master sigalisiert Timing-Zyklus
    A_SysClock:           in    std_logic;                      -- Clock vom Master getrieben.
    A_Spare0:             in    std_logic;                      -- vom Master getrieben
    A_Spare1:             in    std_logic;                      -- vom Master getrieben
    A_nReset:             in    std_logic;                      -- Reset (aktiv '0'), vom Master getrieben

    ------------ ADC Signals ------------------------------------------------------------------------------------------
    ADC_DB:           inout   std_logic_vector(15 downto 0);
    ADC_CONVST_A:     buffer  std_logic;
    ADC_CONVST_B:     buffer  std_logic;
    nADC_CS:          buffer  std_logic;
    nADC_RD_SCLK:     buffer  std_logic;
    ADC_BUSY:         in      std_logic;
    ADC_RESET:        buffer  std_logic;
    ADC_OS:           buffer  std_logic_vector(2 downto 0);
    nADC_PAR_SER_SEL: buffer  std_logic := '0';
    ADC_Range:        buffer  std_logic;
    ADC_FRSTDATA:     in      std_logic;
    EXT_TRIG_ADC:     in      std_logic;
    ------------ ADC Diagnostic ---------------------------------------------------------------------------------------
    A_ADC_DAC_SEL: in std_logic_vector(3 downto 0);

    ------------ DAC Signals ------------------------------------------------------------------------------------------
    DAC1_SDI:         buffer  std_logic;      -- is connected to DAC1-SDI
    DAC1_SDO:         buffer  std_logic;
    nDAC1_CLK:        buffer  std_logic;      -- spi-clock of DAC1
    nDAC1_CLR:        buffer  std_logic;      -- '0' set DAC1 to zero (pulse width min 200 ns)
    nDAC1_A0:         buffer  std_logic;      -- '0' enable shift of internal shift register of DAC1
    nDAC1_A1:         buffer  std_logic;      -- '0' copy shift register to output latch of DAC1
    DAC2_SDI:         buffer  std_logic;      -- is connected to DAC2-SDI
    DAC2_SDO:         buffer  std_logic;      
    nDAC2_CLK:        buffer  std_logic;      -- spi-clock of DAC2
    nDAC2_CLR:        buffer  std_logic;      -- '0' set DAC2 to zero (pulse width min 200 ns)
    nDAC2_A0:         buffer  std_logic;      -- '0' enable shift of internal shift register of DAC2
    nDAC2_A1:         buffer  std_logic;      -- '0' copy shift register to output latch of DAC2
    EXT_TRIG_DAC:     in      std_logic;
    A_NLED_TRIG_DAC:  out     std_logic;
    
    ------------ IO-Port-Signale --------------------------------------------------------------------------------------
    a_io_7_0_tx:        out   std_logic;                    -- '1' = external io(7..0)-buffer set to output.
    a_io_15_8_tx:       out   std_logic;                    -- '1' = external io(15..8)-buffer set to output
    a_io_23_16_tx:      out   std_logic;                    -- '1' = external io(23..16)-buffer set to output
    a_io_31_24_tx:      out   std_logic;                    -- '1' = external io(31..24)-buffer set to output
    a_ext_io_7_0_dis:   out   std_logic;                    -- '1' = disable external io(7..0)-buffer.
    a_ext_io_15_8_dis:  out   std_logic;                    -- '1' = disable external io(15..8)-buffer.
    a_ext_io_23_16_dis: out   std_logic;                    -- '1' = disable external io(23..16)-buffer.
    a_ext_io_31_24_dis: out   std_logic;                    -- '1' = disable external io(31..24)-buffer.
    a_io:               inout std_logic_vector(31 downto 0);-- select and set direction only in 8-bit partitions
    
    ------------ Logic analyser Signals -------------------------------------------------------------------------------
    A_SEL:            in    std_logic_vector(3 downto 0);   -- use to select sources for the logic analyser ports
    A_TA:             out   std_logic_vector(15 downto 0);  -- test port a
    A_CLK_TA:         out   std_logic;
    A_TB:             inout std_logic_vector(15 downto 0);  -- test port b
    A_CLK_TB:         out   std_logic;
    TP:               out   std_logic_vector(2 downto 1);   -- test points
    
    A_nState_LED:     out   std_logic_vector(2 downto 0);   --..LED(2) = R/W, ..LED(1) = Dtack, ..LED(0) = Sel
    A_nLED:           out   std_logic_vector(15 downto 0);
    A_NLED_TRIG_ADC:  out   std_logic;
    
    HW_REV:           in    std_logic_vector(3 downto 0);
    A_MODE_SEL:       in    std_logic_vector(1 downto 0);
    A_OneWire:        inout std_logic;
    A_OneWire_EEPROM: inout std_logic;
    
    NDIFF_IN_EN: buffer std_logic -- enables diff driver for ADC channels 3-8
    
    
    );
end entity;



architecture scu_addac_arch of scu_addac is

  signal s_A_A : std_logic_vector(15 downto 0);
  signal s_A_D : std_logic_vector(15 downto 0);
  signal s_A_dir : std_logic;
  signal s_D_dir : std_logic;
  
  component addac_local_clk_to_12p5_mhz
    port(
      inclk0:   in    std_logic;
      c0:       out   std_logic;
      c1:       out   std_logic;
      locked:   out   std_logic
      );
  end component;
  
  signal s_clk  : std_logic;
  signal s_rstn : std_logic;
  signal s_clk_flash : std_logic;
  
  constant c_layout : t_sdb_record_array(1 downto 0) :=
    (0 => f_sdb_embed_device(f_xwb_dpram(256),       x"01000000"),
     1 => f_sdb_embed_device(f_wb_spi_flash_sdb(24), x"00000000"));

  constant c_sdb_address : t_wishbone_address := x"01fff000";
  constant c_bridge_sdb  : t_sdb_bridge       :=
    f_xwb_bridge_layout_sdb(true, c_layout, c_sdb_address);
  
  constant c_num_slaves  : natural := 2;
  constant c_num_masters : natural := 1;
  
  signal cbar_slave_i  : t_wishbone_slave_in_array (c_num_masters-1 downto 0);
  signal cbar_slave_o  : t_wishbone_slave_out_array(c_num_masters-1 downto 0);
  signal cbar_master_i : t_wishbone_master_in_array (c_num_slaves-1 downto 0);
  signal cbar_master_o : t_wishbone_master_out_array(c_num_slaves-1 downto 0);

begin

  pll : addac_local_clk_to_12p5_mhz
    port map(
      inclk0 => A_SysClock,
      c0     => s_clk,
      c1     => s_clk_flash,
      locked => s_rstn);
  
  bar : xwb_sdb_crossbar
    generic map(
      g_num_masters => c_num_masters,
      g_num_slaves  => c_num_slaves,
      g_registered  => true,
      g_wraparound  => true,
      g_layout      => c_layout,
      g_sdb_addr    => c_sdb_address)
    port map(
      clk_sys_i => s_clk,
      rst_n_i   => s_rstn,
      slave_i   => cbar_slave_i,
      slave_o   => cbar_slave_o,
      master_i  => cbar_master_i,
      master_o  => cbar_master_o);
      
  slave : wb_slave_scu
    port map(
      clk_i                  => s_clk,
      rstn_i                 => s_rstn,
      master_i               => cbar_slave_o(0),
      master_o               => cbar_slave_i(0),
      scub_clk_i             => A_SysClock,
      scub_rstn_i            => A_nReset,
      scub_stb_i             => "not"(A_nDS),
      "not"(scub_ack_o)      => A_nDtack,
      scub_data_o            => s_A_D,
      scub_data_i            => A_D,
      "not"(scub_data_en_o)  => A_nSel_Ext_Data_Drv,
      scub_data_dir_o        => s_D_dir,
      scub_addr_o            => s_A_A,
      scub_addr_i            => A_A,
      "not"(scub_addr_en_o)  => A_nADR_EN,
      scub_addr_dir_o        => s_A_dir,
      scub_sel_i             => "not"(A_nBoardSel),
      "not"(scub_srq_o)      => A_nSRQ);

  A_Ext_Data_RD    <= not s_D_dir;
  A_nADR_FROM_SCUB <= not s_A_dir;
  A_A <= s_A_A when s_A_dir='0' else (others => 'Z');
  A_D <= s_A_D when s_D_dir='0' else (others => 'Z');
  
  ram : xwb_dpram
    generic map(
      g_size                     => 256,
      g_init_file                => "",
      g_slave1_interface_mode => PIPELINED,
      g_slave1_granularity    => BYTE)
    port map(
      clk_sys_i => s_clk,
      rst_n_i   => s_rstn,
      slave1_i  => cbar_master_o(0),
      slave1_o  => cbar_master_i(0),
      slave2_i  => cc_dummy_master_out,
      slave2_o  => open);
  
  flash : flash_top
    generic map(
      g_family                 => "Arria II GX",
      g_port_width             => 1,   -- single-lane SPI bus
      g_addr_width             => 24,
      g_dummy_time             => 8,   -- 8 cycles between address and data
      g_input_latch_edge       => '0', -- 30ns at 50MHz (10+20) after falling edge sets up SPI output
      g_output_latch_edge      => '1', -- falling edge to meet SPI setup times
      g_input_to_output_cycles => 2)   -- delayed to work-around unconstrained design
    port map(
      clk_i     => s_clk_flash,
      rstn_i    => s_rstn,
      slave_i   => cbar_master_o(1),
      slave_o   => cbar_master_i(1),
      clk_ext_i => A_SysClock,
      clk_out_i => A_SysClock,
      clk_in_i  => A_SysClock);

end architecture;
