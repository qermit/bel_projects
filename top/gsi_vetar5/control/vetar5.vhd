library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;

entity vetar5 is
  port(
    clk_20m_vcxo_i    : in std_logic;  -- 20MHz VCXO clock
    clk_125m_pllref_i : in std_logic;  -- 125 MHz PLL reference
    clk_125m_local_i  : in std_logic;  -- local clk from 125Mhz oszillator
    
    -----------------------------------------------------------------------
    -- VME bus
    -----------------------------------------------------------------------
    vme_as_n_i          : in    std_logic;
    vme_rst_n_i         : in    std_logic;
    vme_write_n_i       : in    std_logic;
    vme_am_i            : in    std_logic_vector(5 downto 0);
    vme_ds_n_i          : in    std_logic_vector(1 downto 0);
    vme_ga_i            : in    std_logic_vector(3 downto 0);
    vme_ga_extended_i   : in    std_logic_vector(3 downto 0);
    vme_addr_data_b     : inout std_logic_vector(31 downto 0);
    vme_iackin_n_i      : in    std_logic;
    vme_iackout_n_o     : out   std_logic;
    vme_iack_n_i        : in    std_logic;
    vme_irq_n_o         : out   std_logic_vector(6 downto 0);
    vme_berr_o          : out   std_logic;
    vme_dtack_oe_o      : out   std_logic;
    vme_buffer_latch_o  : out   std_logic_vector(3 downto 0);
    vme_data_oe_ab_o    : out   std_logic;
    vme_data_oe_ba_o    : out   std_logic;
    vme_addr_oe_ab_o    : out   std_logic;
    vme_addr_oe_ba_o    : out   std_logic;
  
    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    dac_sclk       : out std_logic;
    dac_din        : out std_logic;
    ndac_cs        : out std_logic_vector(2 downto 1);
    
    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
    rom_data        : inout std_logic;
    
    -----------------------------------------------------------------------
    -- display
    -----------------------------------------------------------------------
    di              : out std_logic_vector(6 downto 0);
    ai              : in  std_logic_vector(1 downto 0);
    dout_LCD        : in  std_logic;
    wrdis           : out std_logic := '0';
    dres            : out std_logic := '1';
    
    -----------------------------------------------------------------------
    -- io
    -----------------------------------------------------------------------
    fpga_res        : in std_logic;
    nres            : in std_logic;
    pbs2            : in std_logic;
    hpw             : inout std_logic_vector(15 downto 0) := (others => 'Z'); -- logic analyzer
    ant             : inout std_logic_vector(26 downto 1) := (others => 'Z'); -- trigger bus
    
    -----------------------------------------------------------------------
    -- pexaria5db1/2
    -----------------------------------------------------------------------
    p1              : inout std_logic := 'Z'; -- HPWX0 logic analyzer: 3.3V
    n1              : inout std_logic := 'Z'; -- HPWX1
    p2              : inout std_logic := 'Z'; -- HPWX2
    n2              : inout std_logic := 'Z'; -- HPWX3
    p3              : inout std_logic := 'Z'; -- HPWX4
    n3              : inout std_logic := 'Z'; -- HPWX5
    p4              : inout std_logic := 'Z'; -- HPWX6
    n4              : inout std_logic := 'Z'; -- HPWX7
    p5              : out   std_logic := 'Z'; -- LED1 1-6: 3.3V (red)   1|Z=off, 0=on
    n5              : out   std_logic := 'Z'; -- LED2           (blue)
    p6              : out   std_logic := 'Z'; -- LED3           (green)
    n6              : out   std_logic := 'Z'; -- LED4           (white)
    p7              : out   std_logic := 'Z'; -- LED5           (red)
    n7              : out   std_logic := 'Z'; -- LED6           (blue)
    p8              : out   std_logic := 'Z'; -- LED7 7-8: 2.5V (green)
    n8              : out   std_logic := 'Z'; -- LED8           (white)
    
    p9              : out   std_logic := 'Z'; -- TERMEN1 = terminate TTLIO1, 1=x, 0|Z=x (Q2 BSH103 -- G pin)
    n9              : out   std_logic := 'Z'; -- TERMEN2 = terminate TTLIO2, 1=x, 0|Z=x
    p10             : out   std_logic := 'Z'; -- TERMEN3 = terminate TTLIO3, 1=x, 0|Z=x
    n10             : out   std_logic := 'Z'; -- TTLEN1  = TTLIO1 output enable, 0=enable, 1|Z=disable
    p11             : out   std_logic := 'Z'; -- n/c
    n11             : out   std_logic := 'Z'; -- TTLEN3  = TTLIO2 output enable, 0=enable, 1|Z=disable
    p12             : out   std_logic := 'Z'; -- n/c
    n12             : out   std_logic := 'Z'; -- n/c
    p13             : out   std_logic := 'Z'; -- n/c
    n13             : out   std_logic := 'Z'; -- n/c
    p14             : out   std_logic := 'Z'; -- n/c
    n14             : out   std_logic := 'Z'; -- TTLEN5  = TTLIO3 output enable, 0=enable, 1|Z=disable
    p15             : out   std_logic := 'Z'; -- n/c
    n15             : inout std_logic := 'Z'; -- ROM_DATA
    p16             : out   std_logic := 'Z'; -- FPLED5  = TTLIO3 (red)  0=on, Z=off
    n16             : out   std_logic := 'Z'; -- FPLED6           (blue)
    
    p17             : in    std_logic;        -- N_LVDS_1 / SYnIN
    n17             : in    std_logic;        -- P_LVDS_1 / SYpIN
    p18             : in    std_logic;        -- N_LVDS_2 / TRnIN
    n18             : in    std_logic;        -- P_LVDS_2 / TRpIN
    p19             : out   std_logic;        -- N_LVDS_3 / CK200n
--    n19             : out   std_logic;        -- P_LVDS_3 / CK200p
    p21             : in    std_logic;        -- N_LVDS_6  = TTLIO1 in
    n21             : in    std_logic;        -- P_LVDS_6
    p22             : in    std_logic;        -- N_LVDS_8  = TTLIO2 in
    n22             : in    std_logic;        -- P_PVDS_8
    p23             : in    std_logic;        -- N_LVDS_10 = TTLIO3 in
    n23             : in    std_logic;        -- P_LVDS_10
    p24             : out   std_logic;        -- N_LVDS_4 / SYnOU
--    n24             : out   std_logic;        -- P_LVDS_4 / SYpOU
    p25             : out   std_logic;        -- N_LVDS_5  = TTLIO1 out
    n25             : out   std_logic;        -- P_LVDS_5
    p26             : out   std_logic := 'Z'; -- FPLED3    = TTLIO2 (red)  0=on, Z=off
    n26             : out   std_logic := 'Z'; -- FPLED4             (blue)
    p27             : out   std_logic;        -- N_LVDS_7  = TTLIO2 out
    n27             : out   std_logic;        -- P_LVDS_7
    p28             : out   std_logic;        -- N_LVDS_9  = TTLIO3 out
    n28             : out   std_logic;        -- P_LVDS_9
    p29             : out   std_logic := 'Z'; -- FPLED1    = TTLIO1 (red)  0=on, Z=off
    n29             : out   std_logic := 'Z'; -- FPLED2             (blue)
    p30             : out   std_logic := 'Z'; -- n/c
    n30             : out   std_logic := 'Z'; -- n/c
    
    -----------------------------------------------------------------------
    -- connector cpld
    -----------------------------------------------------------------------
    con             : out std_logic_vector(5 downto 1);
    
    -----------------------------------------------------------------------
    -- usb
    -----------------------------------------------------------------------
    slrd            : out   std_logic;
    slwr            : out   std_logic;
    fd              : inout std_logic_vector(7 downto 0) := (others => 'Z');
    pa              : inout std_logic_vector(7 downto 0) := (others => 'Z');
    ctl             : in    std_logic_vector(2 downto 0);
    uclk            : in    std_logic;
    ures            : out   std_logic;
    
    -----------------------------------------------------------------------
    -- leds onboard
    -----------------------------------------------------------------------
    led             : out std_logic_vector(8 downto 1) := (others => '1');
    
    -----------------------------------------------------------------------
    -- leds SFPs
    -----------------------------------------------------------------------
    ledsfpr          : out std_logic_vector(4 downto 1);
    ledsfpg          : out std_logic_vector(4 downto 1);

    sfp234_ref_clk_i    : in  std_logic;

    -----------------------------------------------------------------------
    -- SFP 
    -----------------------------------------------------------------------
    
    sfp4_tx_disable_o : out std_logic := '0';
    sfp4_tx_fault     : in std_logic;
    sfp4_los          : in std_logic;
    
    sfp4_txp_o        : out std_logic;
    sfp4_rxp_i        : in  std_logic;
    
    sfp4_mod0         : in    std_logic; -- grounded by module
    sfp4_mod1         : inout std_logic; -- SCL
    sfp4_mod2         : inout std_logic); -- SDA
    
end vetar5;

architecture rtl of vetar5 is

  signal led_link_up  : std_logic;
  signal led_link_act : std_logic;
  signal led_track    : std_logic;
  signal led_pps      : std_logic;
  
  signal s_hex_vn1_i  : std_logic_vector(3 downto 0);
  signal s_hex_vn2_i  : std_logic_vector(3 downto 0);
  
  signal gpio_o       : std_logic_vector(7 downto 0);
  signal lvds_p_i     : std_logic_vector(4 downto 0);
  signal lvds_n_i     : std_logic_vector(4 downto 0);
  signal lvds_i_led   : std_logic_vector(4 downto 0);
  signal lvds_p_o     : std_logic_vector(2 downto 0);
  signal lvds_n_o     : std_logic_vector(2 downto 0);
  signal lvds_o_led   : std_logic_vector(2 downto 0);
  signal lvds_oen     : std_logic_vector(2 downto 0);

  constant c_family  : string := "Arria V"; 
  constant c_project : string := "vetar5";
  constant c_initf   : string := c_project & ".mif";
  -- projectname is standard to ensure a stub mif that prevents unwanted scanning of the bus 
  -- multiple init files for n processors are to be seperated by semicolon ';'

begin

  main : monster
    generic map(
      g_family          => c_family,
      g_project         => c_project,
      g_flash_bits      => 25,
      g_gpio_out        => 2, -- 2x User LED
      --g_lvds_in         => 2, -- 2x IN 10 pin boxed header
      --g_lvds_out        => 2, -- 2x OUT 10 pin boxed header
      g_lvds_inout      => 12, -- 10x addon board(s) + 2 base board
      g_lvds_invert     => true,
      g_en_pmc_ctrl     => true,
      g_en_vme          => true,
      g_en_usb          => true,
      g_en_lcd          => true,
      g_lm32_init_files => c_initf
    )  
    port map(
      core_clk_20m_vcxo_i    => clk_20m_vcxo_i,
      core_clk_125m_pllref_i => clk_125m_pllref_i,
      core_clk_125m_sfpref_i => sfp234_ref_clk_i,
      core_clk_125m_local_i  => clk_125m_local_i,
      core_rstn_i            => pbs2,
      core_clk_butis_o       => p19,
      core_clk_butis_t0_o    => p24,
      wr_onewire_io          => rom_data,
      wr_sfp_sda_io          => sfp4_mod2,
      wr_sfp_scl_io          => sfp4_mod1,
      wr_sfp_det_i           => sfp4_mod0,
      wr_sfp_tx_o            => sfp4_txp_o,
      wr_sfp_rx_i            => sfp4_rxp_i,
      wr_dac_sclk_o          => dac_sclk,
      wr_dac_din_o           => dac_din,
      wr_ndac_cs_o           => ndac_cs,
      gpio_o                 => gpio_o,
      lvds_p_i               => lvds_p_i,
      lvds_n_i               => lvds_n_i,
      lvds_i_led_o           => lvds_i_led,
      lvds_p_o               => lvds_p_o,
      lvds_n_o               => lvds_n_o,
      lvds_o_led_o           => lvds_o_led,
      lvds_oen_o             => lvds_oen,
      led_link_up_o          => led_link_up,
      led_link_act_o         => led_link_act,
      led_track_o            => led_track,
      led_pps_o              => led_pps,
      vme_as_n_i             => vme_as_n_i,
      vme_rst_n_i            => vme_rst_n_i,
      vme_write_n_i          => vme_write_n_i,
      vme_am_i               => vme_am_i,
      vme_ds_n_i             => vme_ds_n_i,
      vme_ga_i               => vme_ga_i,
      vme_addr_data_b        => vme_addr_data_b,
      vme_iack_n_i           => vme_iack_n_i,
      vme_iackin_n_i         => vme_iackin_n_i,
      vme_iackout_n_o        => vme_iackout_n_o,
      vme_irq_n_o            => vme_irq_n_o,
      vme_berr_o             => vme_berr_o,
      vme_dtack_oe_o         => vme_dtack_oe_o,
      vme_buffer_latch_o     => vme_buffer_latch_o,
      vme_data_oe_ab_o       => vme_data_oe_ab_o,
      vme_data_oe_ba_o       => vme_data_oe_ba_o,
      vme_addr_oe_ab_o       => vme_addr_oe_ab_o,
      vme_addr_oe_ba_o       => vme_addr_oe_ba_o,
      usb_rstn_o             => ures,
      usb_ebcyc_i            => pa(3),
      usb_speed_i            => pa(0),
      usb_shift_i            => pa(1),
      usb_readyn_io          => pa(7),
      usb_fifoadr_o          => pa(5 downto 4),
      usb_sloen_o            => pa(2),
      usb_fulln_i            => ctl(1),
      usb_emptyn_i           => ctl(2),
      usb_slrdn_o            => slrd,
      usb_slwrn_o            => slwr,
      usb_pktendn_o          => pa(6),
      usb_fd_io              => fd,
      pmc_ctrl_hs_i          => s_hsw_fpga,
      pmc_pb_i               => s_pbs_fpga,
      pmc_ctrl_hs_cpld_i     => con(4 downto 1),
      pmc_pb_cpld_i          => con(5),
      pmc_clk_oe_o           => s_wr_ext_in,
      pmc_log_oe_o           => s_log_oe,
      pmc_log_out_o          => s_log_out,
      pmc_log_in_i           => s_log_in,
      lcd_scp_o              => di(3),
      lcd_lp_o               => di(1),
      lcd_flm_o              => di(2),
      lcd_in_o               => di(0));

  -- SFP1-3 are not mounted
  sfp4_tx_disable_o <= '0';

  -- Link LEDs
  wrdis <= '0';
  dres  <= '1';
  di(5) <= '0' when (not led_link_up)                   = '1' else 'Z'; -- red
  di(6) <= '0' when (    led_link_up and not led_track) = '1' else 'Z'; -- blue
  di(4) <= '0' when (    led_link_up and     led_track) = '1' else 'Z'; -- green

  led(1) <= not (led_link_act and led_link_up); -- red   = traffic/no-link
  led(2) <= not led_link_up;                    -- blue  = link
  led(3) <= not led_track;                      -- green = timing valid
  led(4) <= not led_pps;                        -- white = PPS
  
  ledsfpg(3 downto 1) <= (others => '1');
  ledsfpr(3 downto 1) <= (others => '1');
  ledsfpg(4) <= not led_link_up;
  ledsfpr(4) <= not led_link_act;
  
  -- Wires to CPLD, currently unused
  con <= (others => 'Z');
  
end rtl;
