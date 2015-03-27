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
  port
  (
    -- Clock and reset signals
    clk_i        : in  std_logic;
    rst_n_i      : in  std_logic;

    -- Period and maks register inputs, synchronous to clk_i
    hperr_i      : in  std_logic_vector(31 downto 0);
    maskr_i      : in  std_logic_vector(31 downto 0);

    -- Data output to SERDES, synchronous to clk_i
    serdes_dat_o : out std_logic_vector(7 downto 0)
  );
end entity serdes_clk_gen;


architecture arch of serdes_clk_gen is

  --============================================================================
  -- Signal declarations
  --============================================================================


  -- TODO: remove these effing init values



  signal cnt     : unsigned(31 downto 0) := (others => '0');
  signal sub     : unsigned(32 downto 0);
  signal cnt_sat : std_logic;
  signal mask    : std_logic_vector( 7 downto 0) := (others => '0');
  signal outp    : std_logic_vector( 7 downto 0) := (others => '0');
  signal outp_d0 : std_logic;

  signal msk     : unsigned(15 downto 0);
  signal shmsk   : unsigned(15 downto 0);

--==============================================================================
--  architecture begin
--==============================================================================
begin

  -- Count cycles to bit-flip, subtracting 8 (the num. of bits of the SERDES) on
  -- each cycle
  sub <= ('0' & cnt) - 8;

  p_cnt : process (clk_i, rst_n_i)
  begin
    if rst_n_i = '0' then
      cnt <= (others => '0');
    elsif rising_edge(clk_i) then
      if sub(sub'high) = '1' then
        cnt <= sub(cnt'range) + unsigned(hperr_i);
      else
        cnt <= sub(cnt'range);
      end if;
    end if;
  end process p_cnt;

  -- Saturated barrel shifter, shifts the mask by the number of bits indicated by
  -- cnt, or 16 bits if counter saturated from the point of view of the shifter (shift
  -- value == cnt > 15).
  --
  -- The lower 8 bits of the mask are presented to the XOR chain below prior to
  -- outputting to the SERDES.
  msk   <= unsigned(maskr_i(15 downto 0));
  shmsk <= shift_right(msk, to_integer(cnt(3 downto 0)));
  mask  <= std_logic_vector(shmsk(7 downto 0)) when (cnt < 15) else
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
