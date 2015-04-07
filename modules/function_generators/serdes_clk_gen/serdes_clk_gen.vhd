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

use work.genram_pkg.all;


entity serdes_clk_gen is
  generic
  (
    g_num_serdes_bits       : natural;
    g_with_frac_counter     : boolean := false;
    g_selectable_duty_cycle : boolean := false
  );
  port
  (
    -- Clock and reset signals
    clk_i        : in  std_logic;
    rst_n_i      : in  std_logic;

    -- Inputs from registers, synchronous to clk_i
    per_i        : in  std_logic_vector(31 downto 0);
    frac_i       : in  std_logic_vector(31 downto 0);
    mask_i       : in  std_logic_vector(31 downto 0);
    ph_shift_i   : in  std_logic_vector(31 downto 0);

    -- Data output to SERDES, synchronous to clk_i
    serdes_dat_o : out std_logic_vector(7 downto 0)
  );
end entity serdes_clk_gen;


architecture arch of serdes_clk_gen is

  --============================================================================
  -- Signal declarations
  --============================================================================
  signal per_count_1  : unsigned(31 downto 0);
  signal per_add_1    : unsigned(33 downto 0);
  signal frac_count_1 : unsigned(31 downto 0);
  signal frac_add_1   : unsigned(32 downto 0);
  signal frac_carry_1 : std_logic;

  signal msk_1        : unsigned(31 downto 0);
  signal shmks_1      : unsigned(31 downto 0);
  signal mask_1       : std_logic_vector(g_num_serdes_bits-1 downto 0);

  signal outp_1       : std_logic_vector(g_num_serdes_bits-1 downto 0);
  signal outp_1_d0    : std_logic;

  signal per_count_2  : unsigned(31 downto 0);
  signal per_add_2    : unsigned(33 downto 0);
  signal frac_count_2 : unsigned(31 downto 0);
  signal frac_add_2   : unsigned(32 downto 0);
  signal frac_carry_2 : std_logic;

  signal msk_2        : unsigned(31 downto 0);
  signal shmks_2      : unsigned(31 downto 0);
  signal mask_2       : std_logic_vector(g_num_serdes_bits-1 downto 0);

  signal outp_2       : std_logic_vector(g_num_serdes_bits-1 downto 0);
  signal outp_2_d0    : std_logic;

--==============================================================================
--  architecture begin
--==============================================================================
begin

--------------------------------------------------------------------------------
gen_frac_y : if (g_with_frac_counter = true) generate

  per_add_1  <= ('0' & per_count_1 & '1') +
                ('1' & unsigned(to_signed(-g_num_serdes_bits, 32)) & frac_carry_1);
  frac_add_1 <= ('0' & frac_count_1) + ('0' & unsigned(frac_i));

  p_counters : process (clk_i, rst_n_i)
  begin
    if rising_edge(clk_i) then
      if (rst_n_i = '0') then
        per_count_1  <= (others => '0');
        frac_count_1 <= (others => '0');
        frac_carry_1 <= '0';
      elsif (per_add_1(per_add_1'high) = '1') then
        per_count_1  <= per_add_1(32 downto 1) + unsigned(per_i);
        frac_count_1 <= frac_add_1(frac_count_1'range);
        frac_carry_1 <= frac_add_1(frac_add_1'high);
      else
        per_count_1  <= per_add_1(32 downto 1);
        frac_carry_1 <= '0';
      end if;
    end if;
  end process p_counters;

  gen_secondary_counters : if (g_selectable_duty_cycle = true) generate
    per_add_2  <= ('0' & per_count_2 & '1') +
                  ('1' & unsigned(to_signed(-g_num_serdes_bits, 32)) & frac_carry_2);
    frac_add_2 <= ('0' & frac_count_2) + ('0' & unsigned(frac_i));

    p_secondary_counters : process (clk_i, rst_n_i)
    begin
      if rising_edge(clk_i) then
        if (rst_n_i = '0') then
          per_count_2  <= unsigned(ph_shift_i);
          frac_count_2 <= (others => '0');
          frac_carry_2 <= '0';
        elsif (per_add_2(per_add_2'high) = '1') then
          per_count_2  <= per_add_2(32 downto 1) + unsigned(per_i);
          frac_count_2 <= frac_add_2(frac_count_2'range);
          frac_carry_2 <= frac_add_2(frac_add_2'high);
        else
          per_count_2  <= per_add_2(32 downto 1);
          frac_carry_2 <= '0';
        end if;
      end if;
    end process p_secondary_counters;

  end generate gen_secondary_counters;

end generate gen_frac_y;
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
gen_frac_n : if (g_with_frac_counter = false) generate

  frac_carry_1 <= '0';

  per_add_1 <= ('0' & per_count_1 & '1') +
               ('1' & unsigned(to_signed(-g_num_serdes_bits, 32)) & frac_carry_1);

  p_counter : process (clk_i, rst_n_i)
  begin
    if rising_edge(clk_i) then
      if (rst_n_i = '0') then
        per_count_1  <= (others => '0');
      elsif (per_add_1(per_add_1'high) = '1') then
        per_count_1 <= per_add_1(32 downto 1) + unsigned(per_i);
      else
        per_count_1 <= per_add_1(32 downto 1);
      end if;
    end if;
  end process p_counter;

  gen_secondary_counter : if (g_selectable_duty_cycle = true) generate

    frac_carry_2 <= '0';

    per_add_2 <= ('0' & per_count_2 & '1') +
                 ('1' & unsigned(to_signed(-g_num_serdes_bits, 32)) & frac_carry_2);

    p_secondary_counter : process (clk_i, rst_n_i)
    begin
      if rising_edge(clk_i) then
        if (rst_n_i = '0') then
          per_count_2 <= unsigned(ph_shift_i);
        elsif (per_add_2(per_add_2'high) = '1') then
          per_count_2 <= per_add_2(32 downto 1) + unsigned(per_i);
        else
          per_count_2 <= per_add_2(32 downto 1);
        end if;
      end if;
    end process p_secondary_counter;

  end generate gen_secondary_counter;

end generate gen_frac_n;
--------------------------------------------------------------------------------

  -- Saturated barrel shifter, shifts the mask_1 by the number of bits indicated by
  -- per_count_1, or fully if the counter is saturated from the point of view of the
  -- shifter (shift value > SERDES number of bits).
  --
  -- The lower bits of the bit mask_1 are presented to the XOR chain below prior to
  -- outputting to the SERDES.
  msk_1   <= unsigned(mask_i(31 downto 0)) when (frac_carry_1 = '0') else
             unsigned(mask_i(31 downto g_num_serdes_bits) & ('0' & mask_i(g_num_serdes_bits-1 downto 1)));
  shmks_1 <= shift_right(msk_1, to_integer(per_count_1(f_log2_size(2*g_num_serdes_bits)-1 downto 0)))
               when (per_count_1 < 2*g_num_serdes_bits) else
             (others => '0');
  mask_1  <= std_logic_vector(shmks_1(g_num_serdes_bits-1 downto 0));

  -- Output bit-flip based on mask_1 and value of output on prev. cycle
  outp_1(g_num_serdes_bits-1) <= outp_1_d0 xor mask_1(g_num_serdes_bits-1);
  gen_outp_bits : for i in g_num_serdes_bits-2 downto 0 generate
    outp_1(i) <= outp_1(i+1) xor mask_1(i);
  end generate gen_outp_bits;

  p_outp_delay : process (clk_i, rst_n_i)
  begin
    if rising_edge(clk_i) then
      if (rst_n_i = '0') then
        outp_1_d0 <= '0';
      else
        outp_1_d0 <= outp_1(0);
      end if;
    end if;
  end process p_outp_delay;

gen_secondary_outp_logic : if (g_selectable_duty_cycle = true) generate

  msk_2   <= unsigned(mask_i(31 downto 0)) when (frac_carry_2 = '0') else
             unsigned(mask_i(31 downto g_num_serdes_bits) & ('0' & mask_i(g_num_serdes_bits-1 downto 1)));
  shmks_2 <= shift_right(msk_2, to_integer(per_count_2(f_log2_size(2*g_num_serdes_bits)-1 downto 0)))
               when (per_count_2 < 2*g_num_serdes_bits) else
             (others => '0');
  mask_2  <= std_logic_vector(shmks_2(g_num_serdes_bits-1 downto 0));

  outp_2(g_num_serdes_bits-1) <= outp_2_d0 xor mask_2(g_num_serdes_bits-1);
  gen_secondary_outp_bits : for i in g_num_serdes_bits-2 downto 0 generate
    outp_2(i) <= outp_2(i+1) xor mask_2(i);
  end generate gen_secondary_outp_bits;

  p_secondary_outp_delay : process (clk_i, rst_n_i)
  begin
    if rising_edge(clk_i) then
      if (rst_n_i = '0') then
        outp_2_d0 <= '0';
      else
        outp_2_d0 <= outp_2(0);
      end if;
    end if;
  end process p_secondary_outp_delay;

end generate gen_secondary_outp_logic;

  --===========================================================================
  -- Output register
  --===========================================================================
gen_outp_reg_simple : if (g_selectable_duty_cycle = false) generate
  p_outp_reg : process (clk_i, rst_n_i)
  begin
    if rising_edge(clk_i) then
      if (rst_n_i = '0') then
        serdes_dat_o <= (others => '0');
      else
        serdes_dat_o <= outp_1;
      end if;
    end if;
  end process p_outp_reg;
end generate gen_outp_reg_simple;

gen_outp_reg_xored : if (g_selectable_duty_cycle = true) generate
  p_outp_reg : process (clk_i, rst_n_i)
  begin
    if rising_edge(clk_i) then
      if (rst_n_i = '0') then
        serdes_dat_o <= (others => '0');
      else
        serdes_dat_o <= outp_1 xor outp_2;
      end if;
    end if;
  end process p_outp_reg;
end generate gen_outp_reg_xored;

end architecture arch;
--==============================================================================
--  architecture end
--==============================================================================
