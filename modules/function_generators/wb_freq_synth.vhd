--==============================================================================
-- CERN (BE-CO-HT)
-- Generic Wishbone-interface frequency synthesizer
--==============================================================================
--
-- author: Theodor Stana (t.stana@gsi.de)
--
-- date of creation: 2015-03-16
--
-- version: 1.0
--
-- description:
--
-- dependencies:
--
-- references:
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
--    2015-03-16   Theodor Stana     File created
--==============================================================================
-- TODO: -
--==============================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity wb_freq_synth is
  port
  (
    -- Reference clock domain signals
    clk_ref_i   : in  std_logic;
    rst_ref_n_i : in  std_logic;
    dat_o       : out std_logic_vector(7 downto 0)
  );
end entity wb_freq_synth;


architecture arch of wb_freq_synth is

  --============================================================================
  -- Type declarations
  --============================================================================
  type t_shifter is array(3 downto 0) of std_logic_vector(15 downto 0);

  --============================================================================
  -- Constant declarations
  --============================================================================

  --============================================================================
  -- Component declarations
  --============================================================================

  --============================================================================
  -- Signal declarations
  --============================================================================






  -- TODO: remove these effing init values




  signal cnt       : signed(31 downto 0) := (others => '0');
  signal cnt_sat   : std_logic;
  signal maskr     : std_logic_vector(31 downto 0);
  signal shifter   : t_shifter;
  signal perr      : std_logic_vector(31 downto 0);
  signal mask      : std_logic_vector( 7 downto 0) := (others => '0');
  signal outp      : std_logic_vector( 7 downto 0) := (others => '0');
  signal outp_d0   : std_logic;

--==============================================================================
--  architecture begin
--==============================================================================
begin

  perr <= std_logic_vector(to_signed(8,32));
  maskr <= x"00008888";

  -- Count cycles to bit-flip
  p_cnt : process (clk_ref_i)
    variable c : signed(31 downto 0);
  begin
    if rising_edge(clk_ref_i) then
      if (rst_ref_n_i = '0') then
        c   := (others => '0');
        cnt <= (others => '0');
      else
        c := c - 8;
        if (c < 0) then
          c := c + signed(perr);
        end if;
        cnt <= c;
      end if;
    end if;
  end process p_cnt;

  -- Saturated barrel shifter, shifts the mask by the number of bits indicated by
  -- cnt, or 16 bits if counter saturated from the point of view of the shifter (shift
  -- value == cnt > 15).
  cnt_sat <= '1' when ((cnt(31 downto 4)) /= (cnt(31 downto 4)'range => '0')) else
             '0';

  gen_shifter_lvl_0 : for j in 15 downto 15-((2**3)-1) generate
    shifter(3)(j) <= '0' when (cnt(3) = '1') else maskr(j);
  end generate gen_shifter_lvl_0;
  gen_shifter_lvl_bit : for j in 15-((2**3)-1)-1 downto 0 generate
    shifter(3)(j) <= maskr(j+2**3) when (cnt(3) = '1') else maskr(j);
  end generate gen_shifter_lvl_bit;

  gen_shifter : for i in 2 downto 0 generate
    gen_shifter_lvl_0 : for j in 15 downto 15-((2**i)-1) generate
      shifter(i)(j) <= '0' when (cnt(i) = '1') else shifter(i+1)(j);
    end generate gen_shifter_lvl_0;
    gen_shifter_lvl_bit : for j in 15-((2**i)-1)-1 downto 0 generate
      shifter(i)(j) <= shifter(i+1)(j+2**i) when (cnt(i) = '1') else shifter(i+1)(j);
    end generate gen_shifter_lvl_bit;
  end generate gen_shifter;

  mask <= shifter(0)(7 downto 0) when (cnt_sat = '0') else
          (others => '0');

  -- Output bit-flip based on mask and value of output on prev. cycle
  outp(7) <= outp_d0 xor mask(7);
  gen_outp_bits : for i in 6 downto 0 generate
    outp(i) <= outp(i+1) xor mask(i);
  end generate gen_outp_bits;

  p_outp_delay : process (clk_ref_i)
  begin
    if rising_edge(clk_ref_i) then
      if (rst_ref_n_i = '0') then
        outp_d0 <= '0';
      else
        outp_d0 <= outp(0);
      end if;
    end if;
  end process p_outp_delay;

  -- Register the output to the port
  p_outp_reg : process (clk_ref_i)
  begin
    if rising_edge(clk_ref_i) then
      if (rst_ref_n_i = '0') then
        dat_o <= (others => '0');
      else
        dat_o <= outp;
      end if;
    end if;
  end process p_outp_reg;

end architecture arch;
--==============================================================================
--  architecture end
--==============================================================================
