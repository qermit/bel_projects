-------------------------------------------------------------------------------
-- Title      : Scalable Control Unit Bus Interface -- WB version
-- Project    : SCU
-------------------------------------------------------------------------------
-- File       : wb_master_scu.vhd
-- Author     : Stefan Rauch and Wesley Terpstra
-- Company    : GSI
-- Created    : 2015-02-23
-- Last update: 2015-02-23
-- Platform   : FPGA-generics
-- Standard   : VHDL '93
-------------------------------------------------------------------------------
-- Description:
--
-- Master Bus Interface for the SCU Bus
-------------------------------------------------------------------------------
-- Revisions  :
-- Date        Version  Author          Description
-- 2015-02-23  1.0      -               Created
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.wishbone_pkg.all;

entity wb_slave_scu is
  port(
    clk_i           : in  std_logic;
    rstn_i          : in  std_logic;
    master_i        : in  t_wishbone_master_in;
    master_o        : out t_wishbone_master_out;
    
    scub_clk_i      : in  std_logic;
    scub_rstn_i     : in  std_logic;
    scub_stb_i      : in  std_logic; -- DS
    scub_ack_o      : out std_logic; -- Dtack
    scub_data_o     : out std_logic_vector(15 downto 0);
    scub_data_i     : in  std_logic_vector(15 downto 0);
    scub_data_en_o  : out std_logic;
    scub_data_dir_o : out std_logic; -- '1' = from SCU... not(A_Ext_Data_RD)
    scub_addr_o     : out std_logic_vector(15 downto 0);
    scub_addr_i     : in  std_logic_vector(15 downto 0);
    scub_addr_en_o  : out std_logic;
    scub_addr_dir_o : out std_logic; -- '1' = from SCU
    scub_sel_i      : in  std_logic;
    scub_srq_o      : out std_logic);
end wb_slave_scu;

architecture rtl of wb_slave_scu is

  type t_state is (S_IDLE, S_HALF_OUT, S_HALF_IN);
  signal r_state       : t_state;
  
  signal s_master_o    : t_wishbone_master_out;
  signal s_master_i    : t_wishbone_master_in;
  
  signal s_slave_ready : std_logic;
  signal r_slave_stall : std_logic;
  signal r_dat_again   : std_logic;
  
  signal r_sel_i       : std_logic;
  signal r_stb_i       : std_logic;
  signal r_dat_i       : std_logic_vector(15 downto 0);
  signal r_adr_i       : std_logic_vector(15 downto 0);
  signal r_dat_demux   : std_logic_vector(15 downto 0);
  signal r_adr_demux   : std_logic_vector(15 downto 0);

begin

  sys2scu : xwb_clock_crossing
  generic map(
    g_size => 128)
  port map(
    slave_clk_i    => scub_clk_i,
    slave_rst_n_i  => scub_rstn_i,
    slave_i        => s_master_o,
    slave_o        => s_master_i,
    master_clk_i   => clk_i,
    master_rst_n_i => rstn_i,
    master_i       => master_i,
    master_o       => master_o,
    slave_ready_o  => s_slave_ready,
    slave_stall_i  => r_slave_stall);

  scu_in : process(scub_clk_i, scub_rstn_i) is
  begin
    if scub_rstn_i = '0' then
      r_sel_i <= '0';
      r_stb_i <= '0';
      r_dat_i <= (others => '-');
      r_adr_i <= (others => '-');
    elsif rising_edge(scub_clk_i) then
      r_sel_i <= scub_sel_i;
      r_stb_i <= scub_stb_i;
      r_dat_i <= scub_data_i;
      r_adr_i <= scub_addr_i;
    end if;
  end process;
  
  wb_in : process(scub_clk_i, scub_rstn_i) is
  begin
    if scub_rstn_i = '0' then
      r_slave_stall <= '0';
    elsif rising_edge(scub_clk_i) then
      if s_slave_ready = '1' then
        r_slave_stall <= '1';
      end if;
      if r_state = S_HALF_OUT then
        r_slave_stall <= '0'; -- Successfully sent
      end if;
    end if;
  end process;

  fsm : process(scub_clk_i, scub_rstn_i) is
  begin
    if scub_rstn_i = '0' then
      r_state <= S_IDLE;
    elsif rising_edge(scub_clk_i) then
      case r_state is
        when S_IDLE =>
          if (r_sel_i and r_stb_i) = '1' then -- prior stb?
            r_state <= S_HALF_IN;
          elsif (r_slave_stall or s_slave_ready) = '1' then
            r_state <= S_HALF_OUT;
          end if;
        when S_HALF_OUT =>
          -- simultaneous stb is irrelevant: we win.
          r_state <= S_IDLE;
        when S_HALF_IN =>
          -- r_sel_i and r_stb_i MUST be high here
          -- r_dat_i has HIGH word
          if (r_slave_stall or s_slave_ready) = '1' then
            r_state <= S_HALF_OUT;
          else
            r_state <= S_IDLE;
          end if;
      end case;
    end if;
  end process;
  
  -- Note: s_master_i.stall = '0' ALWAYS
  -- The SCUBUS master would never ever overflow our 128 entry FIFO
  -- He promised.
  wb_out : process(scub_clk_i, scub_rstn_i) is
  begin
    if scub_rstn_i = '0' then
      s_master_o.cyc <= '0';
      s_master_o.stb <= '0';
      s_master_o.we  <= '-';
      s_master_o.sel <= (others => '-');
      r_adr_demux    <= (others => '0');
      r_dat_demux    <= (others => '-');
    elsif rising_edge(scub_clk_i) then
      if r_sel_i = '0' then
        s_master_o.cyc <= '0';
      end if;
      s_master_o.stb <= '0';
      
      if r_state = S_HALF_IN then
        s_master_o.cyc <= '1';
        s_master_o.stb <= '1';
        s_master_o.we  <= r_adr_i(14);
        s_master_o.sel <= r_adr_i(13 downto 10);
        
        r_adr_demux <= (others => '0');
        r_adr_demux(8 downto 0) <= r_adr_i(8 downto 0); -- !!! FIXME: add bit 9
        r_dat_demux <= r_dat_i;
      end if;
    end if;
  end process;
  s_master_o.adr <= r_adr_demux & r_adr_i;
  s_master_o.dat <= r_dat_demux & r_dat_i;
  
  scub_addr_dir_o <= '1';
  scub_addr_o <= (others => '0');

  scub_srq_o <= '1'; -- !!! presence detect; don't pull high always
  
  scu_out : process(scub_clk_i, scub_rstn_i) is
  begin
    if scub_rstn_i = '0' then
      scub_data_en_o  <= '0';
      scub_addr_en_o  <= '0';
      scub_data_dir_o <= '1';
      scub_ack_o      <= '0';
      scub_data_o     <= (others => '-');
    elsif falling_edge(scub_clk_i) then
      -- Enable the bus drivers all the time
      scub_data_en_o <= '1';
      scub_addr_en_o <= '1';
      
      -- If previous cycle was an ack, we drive data TO bus
      if r_state = S_HALF_OUT then
        scub_data_dir_o <= '0'; -- from slave
        r_dat_again <= '1';
      else
        scub_data_dir_o <= not r_dat_again;
        r_dat_again <= '0';
      end if;
      
      case r_state is
        when S_IDLE =>
          scub_ack_o  <= not (r_sel_i and r_stb_i) -- don't interrupt in-progress stb
                         and (r_slave_stall or s_slave_ready);
          scub_data_o <= s_master_i.dat(15 downto 0);
        when S_HALF_OUT =>
          scub_ack_o  <= s_master_i.ack;
          scub_data_o <= s_master_i.dat(31 downto 16);
        when S_HALF_IN =>
          scub_ack_o  <= r_slave_stall or s_slave_ready;
          scub_data_o <= (others => '-');
      end case;
    end if;
  end process;
  
end rtl;
