library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;

entity vetar5 is
  port(
--    clk_20m_vcxo_i    : in std_logic;  -- 20MHz VCXO clock
--    clk_125m_pllref_i : in std_logic;  -- 125 MHz PLL reference
--    clk_125m_local_i  : in std_logic;  -- local clk from 125Mhz oszillator


-- clocks, resets
--clk_125m_wrpll_c_n_i : in std_logic_vector(1 downto 0);
clk_125m_wrpll_i : in std_logic_vector(1 downto 0);

--clk_lvttl_n_i : in std_logic;
clk_lvttl_i : in std_logic;

--clk_osc_i(n) : in std_logic_vector(1 downto 0);
clk_osc_i : in std_logic;

clk_20m_vcxo_i : in std_logic;

    
    -----------------------------------------------------------------------
    -- VME bus
    -----------------------------------------------------------------------
--    vme_as_n_i          : in    std_logic;
--    vme_rst_n_i         : in    std_logic;
--    vme_write_n_i       : in    std_logic;
--    vme_am_i            : in    std_logic_vector(5 downto 0);
--    vme_ds_n_i          : in    std_logic_vector(1 downto 0);
--    vme_ga_i            : in    std_logic_vector(3 downto 0);
--    vme_ga_extended_i   : in    std_logic_vector(3 downto 0);
--    vme_addr_data_b     : inout std_logic_vector(31 downto 0);
--    vme_iackin_n_i      : in    std_logic;
--    vme_iackout_n_o     : out   std_logic;
--    vme_iack_n_i        : in    std_logic;
--    vme_irq_n_o         : out   std_logic_vector(6 downto 0);
--    vme_berr_o          : out   std_logic;
--    vme_dtack_oe_o      : out   std_logic;


vmeb_as_n_i : in std_logic;
vmeb_sysrst_n_i : in std_logic;
vmeb_write_n_i : in std_logic;	 
vmeb_am_i : in std_logic_vector(5 downto 0);
vmeb_ds_n_i : in std_logic_vector(1 downto 0);
vmeb_ga_n_i : in std_logic_vector(4 downto 0);
vmeb_gap_n_i : in std_logic;
vmeb_ad_io : inout std_logic_vector(31 downto 0);
vmeb_iackin_n_i : in std_logic;
vmeb_iackout_n_o : out std_logic;
vmeb_iack_n_i : in std_logic;
vmeb_irq_n_o : out std_logic_vector(7 downto 1);
vmeb_berr_n_o : out std_logic;
vmeb_dtack_n_o : out std_logic;

vmeb_retry_n_o : out std_logic;

vme_hsa_i : in std_logic_vector(7 downto 0); -- GA address from HEX switches
	 
----------------------------------------------------------
-- VME buffers control
----------------------------------------------------------
--    vme_buffer_latch_o  : out   std_logic_vector(3 downto 0);
--    vme_data_oe_ab_o    : out   std_logic;
--    vme_data_oe_ba_o    : out   std_logic;
--    vme_addr_oe_ab_o    : out   std_logic;
--    vme_addr_oe_ba_o    : out   std_logic;


laiv_o : out std_logic;
caiv_o : out std_logic;
oaiv_o : out std_logic;
lavi_o : out std_logic;
cavi_o : out std_logic;
oavi_o : out std_logic;

ldiv_o : out std_logic;
cdiv_o : out std_logic;
odiv_o : out std_logic;
ldvi_o : out std_logic;
cdvi_o : out std_logic;
odvi_o : out std_logic;
  
  
    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
--    dac_sclk       : out std_logic;
--    dac_din        : out std_logic;
--    ndac_cs        : out std_logic_vector(2 downto 1);

wr_dac_sclk_o : out std_logic;
wr_dac_din_o : out std_logic;
wr_dac_cs_n_o : out std_logic_vector(2 downto 1);
    
    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
--    rom_data        : inout std_logic;
rom_data_io : inout std_logic;
    -----------------------------------------------------------------------
    -- display
    -----------------------------------------------------------------------
--    di              : out std_logic_vector(6 downto 0);
--    ai              : in  std_logic_vector(1 downto 0);
--    dout_LCD        : in  std_logic;
--    wrdis           : out std_logic := '0';
--    dres            : out std_logic := '1';
	 
dis_rst_o : out std_logic;
dis_di_o : out std_logic_vector(6 downto 0);
dis_ai_i : in std_logic_vector(1 downto 0);
dis_do_o : out std_logic;
dis_wr_o : out std_logic;
	 
    
    -----------------------------------------------------------------------
    -- io
    -----------------------------------------------------------------------
--    fpga_res        : in std_logic;
--    nres            : in std_logic;
--    pbs2            : in std_logic;
--    hpw             : inout std_logic_vector(15 downto 0) := (others => 'Z'); -- logic analyzer
--    ant             : inout std_logic_vector(26 downto 1) := (others => 'Z'); -- trigger bus

nres_i : in std_logic;
fpga_res_i : in std_logic;

pbs_f_i : in std_logic;
	 
dip_sel_i : in std_logic_vector(2 downto 1);

hswf_i : in std_logic_vector(4 downto 1);

   -----------------------------------------------------------------------
	-- LOGIC ANALZER
   -----------------------------------------------------------------------
	 
hpwck_io : inout std_logic;
hpw_io : inout std_logic_vector(15 downto 0);
    
    
    -----------------------------------------------------------------------
    -- CPLD
    -----------------------------------------------------------------------
--    con             : out std_logic_vector(5 downto 1);

con_io : inout std_logic_vector(5 downto 1);
    
    -----------------------------------------------------------------------
    -- usb
    -----------------------------------------------------------------------
--    slrd            : out   std_logic;
--    slwr            : out   std_logic;
--    fd              : inout std_logic_vector(7 downto 0) := (others => 'Z');
--    pa              : inout std_logic_vector(7 downto 0) := (others => 'Z');
--    ctl             : in    std_logic_vector(2 downto 0);
--    uclk            : in    std_logic;
--    ures            : out   std_logic;


slrd_o : out std_logic;
slwr_o : out std_logic;
fd_io : inout std_logic_vector(7 downto 0);
pa_io : inout std_logic_vector(7 downto 0);
ctl_i : in std_logic_vector(2 downto 0);
uclk_i : in std_logic;
ures_o : out std_logic;
ifclk_i : in std_logic;
    
    -----------------------------------------------------------------------
    -- leds onboard
    -----------------------------------------------------------------------
--    led             : out std_logic_vector(8 downto 1) := (others => '1');

led_status_o : out std_logic_vector(6 downto 1);
led_user_o : out std_logic_vector(8 downto 1);
    

    -----------------------------------------------------------------------
    -- SFP 
    -----------------------------------------------------------------------
    
--    sfp4_tx_disable_o : out std_logic := '0';
--    sfp4_tx_fault     : in std_logic;
--    sfp4_los          : in std_logic;

sfp_los_i : in std_logic;
sfp_tx_fault_i : in std_logic;
sfp_tx_dis_o : out std_logic;
    
--sfp_rxd_i(n) : in std_logic;
sfp_rxd_i : in std_logic;
--sfp_txd_o(n) : out std_logic;
sfp_txd_o : out std_logic;
    
--    sfp4_mod0         : in    std_logic; -- grounded by module
--    sfp4_mod1         : inout std_logic; -- SCL
--    sfp4_mod2         : inout std_logic); -- SDA

sfp_mod0_i : in std_logic;
sfp_mod1_io : inout std_logic;
sfp_mod2_io : inout std_logic;

----------------------------------------------------------
-- PG1 mezzanine connector
----------------------------------------------------------

-- LOG1 L, BANK 8D
-- receiver channels
pg1_7_1_n_o : out std_logic_vector(7 downto 1);
pg1_7_1_p_o : out std_logic_vector(7 downto 1);

-- transmitter channels
pg1_15_9_n_o : out std_logic_vector(15 downto 9);
pg1_15_9_p_o : out std_logic_vector(15 downto 9);

-- LOG1 J, BANK 7D
-- high speed differential IOs
-- receiver channels
pg1_24_17_n_i : in std_logic_vector(24 downto 17);
pg1_24_17_p_i : in std_logic_vector(24 downto 17);

-- transmitter channels
pg1_31_15_n_o : out std_logic_vector(31 downto 25);
pg1_31_15_p_o : out std_logic_vector(31 downto 25);


pg1_card_present_n_i : in std_logic;
pg1_power_good_f_i : in std_logic;
pg1_power_run_f_o : out std_logic;

pg1_rom_data_io : inout std_logic;


----------------------------------------------------------
-- PG2 mezzanine connector
----------------------------------------------------------

-- LOG1 B, BANK 3D
-- receiver channels
pg2_7_1_n_o : out std_logic_vector(7 downto 1);
pg2_7_1_p_o : out std_logic_vector(7 downto 1);

-- transmitter channels
pg2_15_9_n_o : out std_logic_vector(15 downto 9);
pg2_15_9_p_o : out std_logic_vector(15 downto 9);

-- LOG1 I, BANK 7C
-- high speed differential IOs

-- receiver channels
pg2_24_17_n_i : in std_logic_vector(24 downto 17);
pg2_24_17_p_i : in std_logic_vector(24 downto 17);

-- transmitter channels
pg2_31_15_n_o : out std_logic_vector(31 downto 25);
pg2_31_15_p_o : out std_logic_vector(31 downto 25);

pg2_card_present_n_i : in std_logic;
pg2_power_good_f_i : in std_logic;
pg2_power_run_f_o : out std_logic;

pg2_rom_data_io : inout std_logic;

----------------------------------------------------------
-- carrier lemo IOs
----------------------------------------------------------

lvtio_in_n_i : in std_logic_vector(2 downto 1);
lvtio_in_p_i : in std_logic_vector(2 downto 1);

lvtio_out_n_o : out std_logic_vector(2 downto 1);
lvtio_out_p_o : out std_logic_vector(2 downto 1);

lvtio_term_en_o : out std_logic_vector(2 downto 1);
lvtio_led_dir_o : out std_logic_vector(2 downto 1);
lvtio_led_act_o : out std_logic_vector(2 downto 1);

lvtio_oe_n_o : out std_logic_vector(2 downto 1);
lvttl_in_clk_en_n_o : out std_logic

);    
end vetar5;

architecture rtl of vetar5 is

  signal led_link_up  : std_logic;
  signal led_link_act : std_logic;
  signal led_track    : std_logic;
  signal led_pps      : std_logic;
  
  signal s_hex_vn1_i  : std_logic_vector(3 downto 0);
  signal s_hex_vn2_i  : std_logic_vector(3 downto 0);
  
  signal gpio_o       : std_logic_vector(1 downto 0);
  
  signal lvds_in_p    : std_logic_vector(11 downto 0);
  signal lvds_in_n    : std_logic_vector(11 downto 0);
  signal lvds_i_led   : std_logic_vector(11 downto 0);
  signal lvds_out_p   : std_logic_vector(11 downto 0);
  signal lvds_out_n   : std_logic_vector(11 downto 0);
  signal lvds_o_led   : std_logic_vector(11 downto 0);
  signal lvds_oen     : std_logic_vector(11 downto 0);
  
  signal vme_ga_internal	: std_logic_vector(5 downto 0);
  
  signal vme_buffer_latch : std_logic_vector(3 downto 0);
  signal vme_data_oe_ab : std_logic;
  signal vme_data_oe_ba : std_logic;
  signal vme_addr_oe_ab : std_logic;
  signal vme_addr_oe_ba : std_logic;
  

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
      core_clk_125m_pllref_i => clk_125m_wrpll_i(1),
      core_clk_125m_sfpref_i => clk_125m_wrpll_i(0),
      core_clk_125m_local_i  => clk_osc_i,
      core_rstn_i            => pbs_f_i,
      core_clk_butis_o       => open,
      core_clk_butis_t0_o    => open,
      
		wr_onewire_io          => rom_data_io,
      
		wr_sfp_sda_io          => sfp_mod2_io,
      wr_sfp_scl_io          => sfp_mod1_io,
      wr_sfp_det_i           => sfp_mod0_i,
      wr_sfp_tx_o            => sfp_txd_o,
      wr_sfp_rx_i            => sfp_rxd_i,
      wr_dac_sclk_o          => wr_dac_sclk_o,
      wr_dac_din_o           => wr_dac_din_o,
      wr_ndac_cs_o           => wr_dac_cs_n_o,
      
		gpio_o                 => gpio_o,
      
		lvds_p_i               => lvds_in_p,
      lvds_n_i               => lvds_in_n,
      lvds_i_led_o           => lvds_i_led,
      lvds_p_o               => lvds_out_p,
      lvds_n_o               => lvds_out_n,
      lvds_o_led_o           => lvds_o_led,
      lvds_oen_o             => lvds_oen,
      
		led_link_up_o          => led_link_up,
      led_link_act_o         => led_link_act,
      led_track_o            => led_track,
      led_pps_o              => led_pps,
		
      vme_as_n_i             => vmeb_as_n_i,
      vme_rst_n_i            => vmeb_sysrst_n_i,
      vme_write_n_i          => vmeb_write_n_i,
      vme_am_i               => vmeb_am_i,
      vme_ds_n_i             => vmeb_ds_n_i,
      vme_ga_i               => vme_ga_internal(3 downto 0),
      vme_addr_data_b        => vmeb_ad_io,
      vme_iack_n_i           => vmeb_iack_n_i,
      vme_iackin_n_i         => vmeb_iackin_n_i,
      vme_iackout_n_o        => vmeb_iackout_n_o,
      vme_irq_n_o            => vmeb_irq_n_o,
      vme_berr_o             => vmeb_berr_n_o,
      vme_dtack_oe_o         => vmeb_dtack_n_o,
		
      vme_buffer_latch_o     => vme_buffer_latch,
      vme_data_oe_ab_o       => vme_data_oe_ab,
      vme_data_oe_ba_o       => vme_data_oe_ba,
      vme_addr_oe_ab_o       => vme_addr_oe_ab,
      vme_addr_oe_ba_o       => vme_addr_oe_ba,
      
		  usb_rstn_o             => ures_o,
      usb_ebcyc_i            => pa_io(3),
      usb_speed_i            => pa_io(0),
      usb_shift_i            => pa_io(1),
      usb_readyn_io          => pa_io(7),
      usb_fifoadr_o          => pa_io(5 downto 4),
      usb_sloen_o            => pa_io(2),
      usb_fulln_i            => ctl_i(1),
      usb_emptyn_i           => ctl_i(2),
      usb_slrdn_o            => slrd_o,
      usb_slwrn_o            => slwr_o,
      usb_pktendn_o          => pa_io(6),
      usb_fd_io              => fd_io,
      
      lcd_scp_o              => dis_di_o(3),
      lcd_lp_o               => dis_di_o(1),
      lcd_flm_o              => dis_di_o(2),
      lcd_in_o               => dis_di_o(0)
      );

  -- SFP
  sfp_tx_dis_o <= '0';
  
  -- select VME Geagraphicall Address source with DIP SW
  vme_ga_internal <= vmeb_gap_n_i & vmeb_ga_n_i when dip_sel_i(1)= '1' else vme_hsa_i(5 downto 0);

  -- Link LEDs
  dis_wr_o <= '0';
  dis_rst_o  <= '1';
  dis_di_o(5) <= '0' when (not led_link_up)                   = '1' else 'Z'; -- red
  dis_di_o(6) <= '0' when (    led_link_up and not led_track) = '1' else 'Z'; -- blue
  dis_di_o(4) <= '0' when (    led_link_up and     led_track) = '1' else 'Z'; -- green

  led_status_o(1) <= not (led_link_act and led_link_up); -- red   = traffic/no-link
  led_status_o(2) <= not led_link_up;                    -- blue  = link
  led_status_o(3) <= not led_track;                      -- green = timing valid
  led_status_o(4) <= not led_pps;                        -- white = PPS
  
  
  -- Wires to CPLD, currently only used as inputs
  con_io <= (others => 'Z');
  
  
-- mezzanine control signals  
pg1_power_run_f_o <= '1';
pg1_rom_data_io   <= 'Z';

pg2_power_run_f_o <= '1';
pg2_rom_data_io   <= 'Z';


-- VME buffer control

--    vme_buffer_latch_o  : out   std_logic_vector(3 downto 0);
--    vme_data_oe_ab_o    : out   std_logic;
--    vme_data_oe_ba_o    : out   std_logic;
--    vme_addr_oe_ab_o    : out   std_logic;
--    vme_addr_oe_ba_o    : out   std_logic;


laiv_o <= vme_buffer_latch(1);
caiv_o <= '0';
oaiv_o <= vme_addr_oe_ba;

lavi_o <= vme_buffer_latch(0);
cavi_o <= '0';
oavi_o <= vme_addr_oe_ab;

ldiv_o <= vme_buffer_latch(3);
cdiv_o <= '0';
odiv_o <= vme_data_oe_ba;

ldvi_o <= vme_buffer_latch(2);
cdvi_o <= '0';
odvi_o <= vme_data_oe_ab;

-- mezzanine ios
---------------------------------------------------------
-- Mezzanine PG1
-- control outputs (LEDS, OE, TERM en)
pg1_7_1_p_o(5 downto 1)  <= lvds_oen(4 downto 0);
pg1_7_1_n_o(5 downto 1)  <= lvds_oen(4 downto 0);

pg1_7_1_p_o(7 downto 6)  <= (others => '0'); -- not used on mezzanine MZNN_VME_A_REVA
pg1_7_1_n_o(7 downto 6)  <= (others => '0'); -- not used on mezzanine MZNN_VME_A_REVA

pg1_15_9_p_o(13 downto 9)  <= lvds_i_led(4 downto 0);
pg1_15_9_n_o(13 downto 9)  <= lvds_o_led(4 downto 0);

pg1_15_9_p_o(15 downto 14) <= (others => '0'); -- not used on mezzanine MZNN_VME_A_REVA
pg1_15_9_n_o(15 downto 14) <= (others => '0'); -- not used on mezzanine MZNN_VME_A_REVA

-- differential IOs
lvds_in_n(4 downto 0) <= pg1_24_17_n_i(21 downto 17);
lvds_in_p(4 downto 0) <= pg1_24_17_p_i(21 downto 17);

-- pg1_24_17_n_i(24 downto 22); -- not used on mezzanine MZNN_VME_A_REVA
-- pg1_24_17_p_i(24 downto 22); -- not used on mezzanine MZNN_VME_A_REVA

pg1_31_15_n_o(29 downto 25) <= lvds_out_n(4 downto 0);
pg1_31_15_p_o(29 downto 25) <= lvds_out_p(4 downto 0);

pg1_31_15_p_o(31 downto 30) <= (others => '0'); -- not used on mezzanine MZNN_VME_A_REVA
pg1_31_15_n_o(31 downto 30) <= (others => '0'); -- not used on mezzanine MZNN_VME_A_REVA

---------------------------------------------------------
-- Mezzanine PG2
-- control outputs (LEDS, OE, TERM en)
pg2_7_1_p_o(5 downto 1)  <= lvds_oen(9 downto 5);
pg2_7_1_n_o(5 downto 1)  <= lvds_oen(9 downto 5);

pg2_7_1_p_o(7 downto 6)  <= (others => '0'); -- not used on mezzanine MZNN_VME_A_REVA
pg2_7_1_n_o(7 downto 6)  <= (others => '0'); -- not used on mezzanine MZNN_VME_A_REVA

pg2_15_9_p_o(13 downto 9)  <= lvds_i_led(9 downto 5);
pg2_15_9_n_o(13 downto 9)  <= lvds_o_led(9 downto 5);

pg2_15_9_p_o(15 downto 14) <= (others => '0'); -- not used on mezzanine MZNN_VME_A_REVA
pg2_15_9_n_o(15 downto 14) <= (others => '0'); -- not used on mezzanine MZNN_VME_A_REVA

-- differential IOs
lvds_in_n(9 downto 5) <= pg2_24_17_n_i(21 downto 17);
lvds_in_p(9 downto 5) <= pg2_24_17_p_i(21 downto 17);

-- pg2_24_17_n_i(24 downto 22); -- not used on mezzanine MZNN_VME_A_REVA
-- pg2_24_17_p_i(24 downto 22); -- not used on mezzanine MZNN_VME_A_REVA

pg2_31_15_n_o(29 downto 25) <= lvds_out_n(9 downto 5);
pg2_31_15_p_o(29 downto 25) <= lvds_out_p(9 downto 5);

pg2_31_15_p_o(31 downto 30) <= (others => '0'); -- not used on mezzanine MZNN_VME_A_REVA
pg2_31_15_n_o(31 downto 30) <= (others => '0'); -- not used on mezzanine MZNN_VME_A_REVA


---------------------------------------------------
-- on carrier LEMO IOs

lvds_in_n(11 downto 10) <= lvtio_in_n_i;
lvds_in_p(11 downto 10) <= lvtio_in_p_i;

lvtio_out_n_o <= lvds_out_n(11 downto 10);
lvtio_out_p_o <= lvds_out_p(11 downto 10);

-- output enable (oe) is active low, termination enable is active hi
lvtio_term_en_o <= lvds_oen(11 downto 10); 
lvtio_led_dir_o <= not lvds_oen(11 downto 10);
lvtio_led_act_o <= lvds_i_led(11 downto 10);

-- output enable (oe) is active low
lvtio_oe_n_o <= lvds_oen(11 downto 10);

lvttl_in_clk_en_n_o <= '1'; -- input buffers for clock input disabled

  
end rtl;
