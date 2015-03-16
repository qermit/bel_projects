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
  type t_state is (
    IDLE,
    SHIFT
  );

  --============================================================================
  -- Constant declarations
  --============================================================================

  --============================================================================
  -- Component declarations
  --============================================================================

  --============================================================================
  -- Signal declarations
  --============================================================================
  signal cnth, cntl  : unsigned(31 downto 0);

  signal hir, lor    : std_logic_vector(31 downto 0);
  signal fhir, flor  : std_logic_vector(31 downto 0);

  signal obit        : std_logic;
  signal shr         : std_logic_vector(7 downto 0);

  



  signal bitcnt      : unsigned( 2 downto 0);
  signal state       : t_state;




--==============================================================================
--  architecture begin
--==============================================================================
begin

  hir <= std_logic_vector(to_unsigned(3, 32));
  lor <= std_logic_vector(to_unsigned(3, 32));

  --============================================================================
  -- Counters and output shift register
  --============================================================================
  p_counters : process(clk_ref_i)
  begin
    if rising_edge(clk_ref_i) then
      if (rst_ref_n_i = '0') then
        cnth <= (others => '0');
        cntl <= (others => '0');
        obit <= '0';
      elsif (obit = '1') then
        cnth <= cnth + 1;
        if (cnth = unsigned(hir)) then
          cnth <= (others => '0');
          obit <= '0';
        end if;
      elsif (obit = '0') then
        cntl <= cntl + 1;
        if (cntl = unsigned(lor)) then
          cntl <= (others => '0');
          obit <= '1';
        end if;
      end if;
    end if;
  end process p_counters;

  p_shift_reg : process (clk_ref_i)
  begin
    if rising_edge(clk_ref_i) then
      if (rst_ref_n_i = '0') then
        shr <= (others => '0');
      else
        shr <= obit & shr(7 downto 1);
      end if;
    end if;
  end process p_shift_reg;

  p_data_out : process(clk_ref_i)
  begin
    if rising_edge(clk_ref_i) then
      if (rst_ref_n_i = '0') then
        dat_o <= (others => '0');
      else
        dat_o <= shr;
      end if;
    end if;
  end process p_data_out;

end architecture arch;
--==============================================================================
--  architecture end
--==============================================================================
