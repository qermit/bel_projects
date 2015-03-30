--==============================================================================
-- GSI Helmholz center for Heavy Ion Research GmbH
-- SERDES clock generator
--==============================================================================
--
-- author: Theodor Stana (t.stana@gsi.de)
--
-- date of creation: 2015-03-24
--
-- version: 1.0
--
-- description:
--    This module implements a clock generator via a SERDES interface. It drives
--    the data input of a SERDES transceiver with the necessary bit pattern in
--    order to generate a clock at the SERDES data rate.
--
--==============================================================================
-- GNU LESSER GENERAL PUBLIC LICENSE
--==============================================================================
-- This source file is free software; you can redistribute it and/or modify it
-- under the terms of the GNU Lesser General Public License as published by the
-- Free Software Foundation; either version 2.1 of the License, or (at your
-- option) any later version. This source is distributed in the hope that it
-- will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
-- of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
-- See the GNU Lesser General Public License for more details. You should have
-- received a copy of the GNU Lesser General Public License along with this
-- source; if not, download it from http://www.gnu.org/licenses/lgpl-2.1.html
--==============================================================================
-- last changes:
--    2015-03-24   Theodor Stana     File created
--==============================================================================
-- TODO: -
--==============================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity serdes_clk_gen is
  generic
  (
    g_serdes_num_bits : natural
  );
  port
  (
    -- Clock and reset signals
    clk_i        : in  std_logic;
    rst_n_i      : in  std_logic;

    -- Period and maks register inputs, synchronous to clk_i
    per_i        : in  std_logic_vector(31 downto 0);
    frac_i       : in  std_logic_vector(31 downto 0);
    mask_i       : in  std_logic_vector(31 downto 0);

    -- Data output to SERDES, synchronous to clk_i
    serdes_dat_o : out std_logic_vector(7 downto 0)
  );
end entity serdes_clk_gen;


architecture arch of serdes_clk_gen is

  --============================================================================
  -- Signal declarations
  --============================================================================

  signal percnt  : unsigned(31 downto 0);
  signal persub  : unsigned(32 downto 0);
  signal fraccnt : unsigned(31 downto 0);
  signal fracadd : unsigned(32 downto 0);

  signal msk     : unsigned(15 downto 0);
  signal shmsk   : unsigned(15 downto 0);
  signal mask    : std_logic_vector( 7 downto 0);

  signal outp    : std_logic_vector( 7 downto 0);
  signal outp_d0 : std_logic;

--==============================================================================
--  architecture begin
--==============================================================================
begin

  -- Count cycles to bit-flip, subtracting the num. of bits of the SERDES on
  -- every cycle
  persub  <= (('0' & percnt) - g_serdes_num_bits) when (fracadd(fracadd'high) = '0') else
             (('0' & percnt) - (g_serdes_num_bits-1));
  fracadd <= ('0' & fraccnt) + unsigned(frac_i);

  p_counters : process (clk_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      percnt  <= (others => '0');
      fraccnt <= (others => '0');
    elsif rising_edge(clk_i) then
      if (persub(persub'high) = '1') then
        percnt  <= persub(percnt'range) + unsigned(per_i);
        fraccnt <= fracadd(fraccnt'range);
      else
        percnt <= persub(percnt'range);
      end if;
    end if;
  end process p_counters;

  -- Saturated barrel shifter, shifts the mask by the number of bits indicated by
  -- percnt, or fully if the counter is saturated from the point of view of the
  -- shifter (shift value > SERDES number of bits).
  --
  -- The lower bits of the bit mask are presented to the XOR chain below prior to
  -- outputting to the SERDES.
  msk   <= unsigned(mask_i(15 downto 0)) when fracadd(fracadd'high) = '0' else
           unsigned(mask_i(15 downto 8) & ('0' & mask_i(6 downto 0)));
  shmsk <= shift_right(msk, to_integer(percnt(3 downto 0)));
  mask  <= std_logic_vector(shmsk(7 downto 0)) when (percnt < 2*g_serdes_num_bits) else
           (others => '0');

  -- Output bit-flip based on mask and value of output on prev. cycle
  outp(7) <= outp_d0 xor mask(7);
  gen_outp_bits : for i in 6 downto 0 generate
    outp(i) <= outp(i+1) xor mask(i);
  end generate gen_outp_bits;

  p_outp_delay : process (clk_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      outp_d0 <= '0';
    elsif rising_edge(clk_i) then
      outp_d0 <= outp(0);
    end if;
  end process p_outp_delay;

  -- Register the output to the port
  p_outp_reg : process (clk_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      serdes_dat_o <= (others => '0');
    elsif rising_edge(clk_i) then
      serdes_dat_o <= outp;
    end if;
  end process p_outp_reg;

end architecture arch;
--==============================================================================
--  architecture end
--==============================================================================
