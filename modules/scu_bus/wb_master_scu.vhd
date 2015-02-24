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

entity wb_master_scu is
  generic(
    g_slave_powerup : natural);
  port(
    clk_i           : in  std_logic;
    rstn_i          : in  std_logic;
    slave_i         : in  t_wishbone_slave_in;
    slave_o         : out t_wishbone_slave_out;
    
    scub_clk_o      : out std_logic;
    scub_rstn_o     : out std_logic;
    scub_stb_o      : out std_logic; -- DS
    scub_ack_i      : in  std_logic; -- Dtack
    scub_data_o     : out std_logic_vector(15 downto 0);
    scub_data_i     : in  std_logic_vector(15 downto 0);
    scub_data_en_o  : out std_logic;
    scub_data_dir_o : out std_logic; -- '1' = from SCU ... not(RDnWR)
    scub_addr_o     : out std_logic_vector(15 downto 0);
    scub_addr_i     : in  std_logic_vector(15 downto 0);
    scub_addr_en_o  : out std_logic;
    scub_addr_dir_o : out std_logic; -- '1' = from SCU
    scub_sel_o      : out std_logic_vector(11 downto 0);
    scub_srq_i      : in  std_logic_vector(11 downto 0));
end wb_master_scu;

architecture rtl of wb_master_scu is

  type t_state is (S_RESET, S_IDLE, S_HALF_OUT, S_HALF_IN);
  signal r_state  : t_state := S_RESET;
  
  -- Queued SCU operation
  signal s_scu_stb : std_logic;
  signal r_scu_stb : std_logic; -- Full
  signal r_sel     : std_logic_vector(15 downto 0);
  signal r_adr_hi  : std_logic_vector(15 downto 0);
  signal r_adr_lo  : std_logic_vector(15 downto 0);
  signal r_dat_hi  : std_logic_vector(15 downto 0);
  signal r_dat_lo  : std_logic_vector(15 downto 0);
  
  -- SCU ack path
  signal r_ack_i     : std_logic;
  signal r_dat_i     : std_logic_vector(15 downto 0);
  signal r_dat_demux : std_logic_vector(15 downto 0);
  
  -- Is an incoming Wishbone request processed? (cyc+stb+!stall)
  signal s_accept_stb : std_logic;
  signal s_scu_picks  : std_logic;
  signal s_stall      : std_logic;
  
  -- Presence of devices
  signal r_presence : std_logic_vector(15 downto 0);
  
  -- At most 128 cycles inflight
  signal r_queued : unsigned(7 downto 0); 
  
  -- Report a bad access (non-existant slave)
  signal r_noslave_err : std_logic;
  
  -- Connect the SDB bridge ROM
  signal r_rom_ack : std_logic;
  signal r_rom_dat : std_logic;
  signal s_rom_dat : std_logic_vector(31 downto 0);
  
  -- Stay in reset for slow slaves to power-up
  signal r_u_there_yet : unsigned(f_ceil_log2(g_slave_powerup) downto 0) := (others => '0');
  
  -- The SDB ROM
  component scu_sdb_rom is
    port(
      clk_i        : in  std_logic;
      s_presence_i : in  std_logic_vector(11 downto 0);
      s_adr_i      : in  std_logic_vector( 9 downto 0);
      s_dat_o      : out std_logic_vector(31 downto 0));
  end component;
  
begin

  -- Keep the bus synchronous to wishbone
  scub_clk_o  <= clk_i;
  
  -- Slave output lines
  slave_o.rty <= '0';
  slave_o.int <= '0';
  slave_o.stall <= s_stall;

  -- Write buffer is not full?
  s_stall      <= '1' when r_queued = 127 else r_scu_stb;
  s_accept_stb <= (slave_i.cyc and slave_i.stb and not s_stall);
  
  queue_limit : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      r_queued <= (others => '0');
    elsif rising_edge(clk_i) then
      if r_state = S_HALF_IN or r_rom_ack = '1' or r_noslave_err = '1' then
        if s_accept_stb = '1' then
          -- No change; both increased and decreased
        else
          r_queued <= r_queued - 1;
        end if;
      else
        if s_accept_stb = '1' then
          r_queued <= r_queued + 1;
        else
          -- No change
        end if;
      end if;
    end if;
  end process;
  
  rom : scu_sdb_rom
    port map(
      clk_i        => clk_i,
      s_presence_i => r_presence(11 downto 0),
      s_adr_i      => slave_i.adr(9 downto 0),
      s_dat_o      => s_rom_dat);
  
  -- Only push SCUBUS operation if the slave is present
  -- ... but always forward to any already selected slave
  -- ... otherwise the error could come in mixed in with queued acks/errs
  s_scu_picks <= 
    '1' when unsigned(r_sel) /= 0 else
    r_presence(to_integer(unsigned(slave_i.adr(28 downto 25))));
  
  wb_in : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      r_scu_stb     <= '0';
      r_noslave_err <= '0';
      r_rom_ack     <= '0';
      r_adr_hi <= (others => '-');
      r_adr_lo <= (others => '-');
      r_dat_hi <= (others => '-');
      r_dat_lo <= (others => '-');
    elsif rising_edge(clk_i) then
      r_noslave_err <= '0';
      r_rom_ack     <= '0';
      
      if s_accept_stb = '1' then
        if s_scu_picks = '1' then
          -- Keep this in sync with s_scu_stb!
          r_scu_stb <= '1';
        elsif slave_i.adr(28 downto 25) = "1111" then
          -- The SDB ROM
          r_rom_ack <= '1';
        else -- Slave not present
          r_noslave_err <= '1';
        end if;
        r_adr_hi(15) <= '0'; -- TIMING MESSAGE !!!
        r_adr_hi(14) <= slave_i.we;
        r_adr_hi(13 downto 10) <= slave_i.sel;
        r_adr_hi(9) <= '0'; -- FIXME: increase to 64MB !!!
        r_adr_hi( 8 downto 0) <= slave_i.adr(24 downto 16);
        r_adr_lo(15 downto 0) <= slave_i.adr(15 downto  0);
        r_dat_hi(15 downto 0) <= slave_i.dat(31 downto 16);
        r_dat_lo(15 downto 0) <= slave_i.dat(15 downto  0);
      end if;
      if r_ack_i = '0' and r_state = S_HALF_OUT then
        r_scu_stb <= '0'; -- Successfully sent
      end if;
    end if;
  end process;

  -- This should be the same as r_scu_stb!
  s_scu_stb <= r_scu_stb or (s_accept_stb and s_scu_picks);
  
  fsm : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      scub_rstn_o   <= '0';
      r_state       <= S_RESET;
      r_presence    <= (others => '0');
      r_u_there_yet <= (others => '0'); 
    elsif rising_edge(clk_i) then
      scub_rstn_o <= '1';
      case r_state is
        when S_RESET =>
          -- just awesome.
          r_u_there_yet <= r_u_there_yet + 1;
          if r_u_there_yet = g_slave_powerup then
            r_state <= S_IDLE;
          end if;
          
          r_presence <= (others => '0');
          r_presence(scub_srq_i'range) <= scub_srq_i;
          scub_rstn_o <= '0';
        when S_IDLE =>
          if r_ack_i = '1' then -- prior ack?
            r_state <= S_HALF_IN;
          elsif s_scu_stb = '1' then
            r_state <= S_HALF_OUT;
          end if;
        when S_HALF_OUT =>
          if r_ack_i = '1' then -- was there a simultaneous ack?
            r_state <= S_HALF_IN;
          else -- we just sent both
            r_state <= S_IDLE;
          end if;
        when S_HALF_IN =>
          -- On this cycle, r_ack_i is the error status
          if s_scu_stb = '1' then
            r_state <= S_HALF_OUT;
          else
            r_state <= S_IDLE;
          end if;
      end case;
    end if;
  end process;
  
  scu_in : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      r_ack_i <= '0';
      r_dat_i <= (others => '-');
    elsif rising_edge(clk_i) then
      r_ack_i <= scub_ack_i;
      r_dat_i <= scub_data_i;
    end if;
  end process;
  
  -- Mux data from either SDB ROM or SCU bus
  slave_o.dat <= 
    s_rom_dat when r_rom_dat = '1' else
    (r_dat_demux & r_dat_i);
  
  wb_out : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      r_dat_demux <= (others => '-');
      r_rom_dat   <= '0';
      slave_o.ack <= '0';
      slave_o.err <= '0';
    elsif rising_edge(clk_i) then
      r_dat_demux <= r_dat_i;
      r_rom_dat   <= r_rom_ack;
      -- ACK/ERR is decided based on ack pattern. 11 = ack & 10 = error
      if r_state = S_HALF_IN then
        slave_o.ack <= r_ack_i;
        slave_o.err <= not r_ack_i;
      else
        -- Report error for missing slaves
        -- Note that this is safe because
        --   A: the SCUbus must be idle for these slaves to be active => no queued acks/errs
        --   B: the SDB ack and noslave_err BOTH respond after exactly 2 cycles
        slave_o.err <= r_noslave_err;
        slave_o.ack <= r_rom_ack;
      end if;
    end if;
  end process;
  
  scub_sel_o  <= r_sel(11 downto 0);
  scu_out : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      r_sel <= (others => '0');
      scub_data_en_o  <= '0';
      scub_addr_en_o  <= '0';
      scub_data_dir_o <= '1';
      scub_addr_dir_o <= '1';
      scub_stb_o      <= '0';
      scub_data_o     <= (others => '-');
      scub_addr_o     <= (others => '-');
    elsif falling_edge(clk_i) then
      -- Enable the bus drivers all the time
      scub_data_en_o <= '1';
      scub_addr_en_o <= '1';
      
      -- If previous cycle was an ack, we take data FROM bus
      if r_state = S_HALF_IN then
        scub_data_dir_o <= '0'; -- from slave
        scub_addr_dir_o <= '0';
      else
        scub_data_dir_o <= not r_ack_i;
        scub_addr_dir_o <= not r_ack_i;
      end if;
      
      if slave_i.cyc = '0' then
        -- Release slave
        r_sel <= (others => '0');
      elsif s_scu_stb = '1' and unsigned(r_sel) = 0 then
        -- Raise select line, if not done yet
        r_sel(to_integer(unsigned(slave_i.adr(28 downto 25)))) <= '1';
      end if;
      
      case r_state is
        when S_RESET =>
          scub_stb_o  <= '0';
          scub_data_o <= (others => '-');
          scub_addr_o <= (others => '-');
        when S_IDLE =>
          scub_stb_o  <= s_scu_stb and not r_ack_i; --- guard vs. prior ack
          scub_data_o <= r_dat_lo;
          scub_addr_o <= r_adr_lo;
        when S_HALF_OUT =>
          scub_stb_o  <= not r_ack_i; -- simultaneous ack?
          scub_data_o <= r_dat_hi;
          scub_addr_o <= r_adr_hi;
        when S_HALF_IN =>
          scub_stb_o  <= s_scu_stb; -- can strobe blindly
          scub_data_o <= (others => '-');
          scub_addr_o <= (others => '-');
      end case;
    end if;
  end process;

end rtl;
