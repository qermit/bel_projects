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
    clk_i         : in  std_logic;
    rst_n_i       : in  std_logic;

    -- Inputs from registers, synchronous to clk_i
    ld_reg_p0_i   : in  std_logic;
    per_i         : in  std_logic_vector(31 downto 0);
    per_hi_i      : in  std_logic_vector(31 downto 0);
    frac_i        : in  std_logic_vector(31 downto 0);
    mask_i        : in  std_logic_vector(31 downto 0);

    -- Counter load ports for external synchronization machine
    ld_lo_p0_i    : in  std_logic;
    ld_hi_p0_i    : in  std_logic;
    per_count_i   : in  std_logic_vector(31 downto 0);
    frac_count_i  : in  std_logic_vector(31 downto 0);
    frac_carry_i  : in  std_logic;

    -- Data output to SERDES, synchronous to clk_i
    serdes_dat_o  : out std_logic_vector(7 downto 0)
  );
end entity serdes_clk_gen;


architecture arch of serdes_clk_gen is

  --============================================================================
  -- Signal declarations
  --============================================================================
  signal per_count_hi  : unsigned(31 downto 0);
  signal per_add_hi    : unsigned(33 downto 0);
  signal frac_count_hi : unsigned(31 downto 0);
  signal frac_add_hi   : unsigned(32 downto 0);
  signal frac_carry_hi : std_logic;

  signal msk_hi        : unsigned(31 downto 0);
  signal shmsk_hi      : unsigned(31 downto 0);
  signal mask_hi       : std_logic_vector(g_num_serdes_bits-1 downto 0);

  signal outp_hi       : std_logic_vector(g_num_serdes_bits-1 downto 0);
  signal outp_hi_d0    : std_logic;

  signal per_count_lo  : unsigned(31 downto 0);
  signal per_add_lo    : unsigned(33 downto 0);
  signal frac_count_lo : unsigned(31 downto 0);
  signal frac_add_lo   : unsigned(32 downto 0);
  signal frac_carry_lo : std_logic;

  signal msk_lo        : unsigned(31 downto 0);
  signal shmsk_lo      : unsigned(31 downto 0);
  signal mask_lo       : std_logic_vector(g_num_serdes_bits-1 downto 0);

  signal outp_lo       : std_logic_vector(g_num_serdes_bits-1 downto 0);
  signal outp_lo_d0    : std_logic;

--==============================================================================
--  architecture begin
--==============================================================================
begin

--------------------------------------------------------------------------------
gen_frac_yes : if (g_with_frac_counter = true) generate

  per_add_hi  <= ('0' & per_count_hi & '1') +
                 ('1' & unsigned(to_signed(-g_num_serdes_bits, 32)) & frac_carry_hi);
  frac_add_hi <= ('0' & frac_count_hi) + ('0' & unsigned(frac_i));

  p_counters : process (clk_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      per_count_hi  <= (others => '0');
      frac_count_hi <= (others => '0');
      frac_carry_hi <= '0';
    elsif rising_edge(clk_i) then
      if (ld_reg_p0_i = '1') then
        per_count_hi  <= (others => '0');
        frac_count_hi <= (others => '0');
      elsif (ld_hi_p0_i = '1') then
        per_count_hi  <= unsigned(per_count_i);
        frac_count_hi <= unsigned(frac_count_i);
        frac_carry_hi <= frac_carry_i;
      elsif (per_add_hi(per_add_hi'high) = '1') then
        per_count_hi  <= per_add_hi(32 downto 1) + unsigned(per_i);
        frac_count_hi <= frac_add_hi(frac_count_hi'range);
        frac_carry_hi <= frac_add_hi(frac_add_hi'high);
      else
        per_count_hi  <= per_add_hi(32 downto 1);
        frac_carry_hi <= '0';
      end if;
    end if;
  end process p_counters;

  gen_secondary_counters : if (g_selectable_duty_cycle = true) generate
    per_add_lo  <= ('0' & per_count_lo & '1') +
                   ('1' & unsigned(to_signed(-g_num_serdes_bits, 32)) & frac_carry_lo);
    frac_add_lo <= ('0' & frac_count_lo) + ('0' & unsigned(frac_i));

    p_secondary_counters : process (clk_i, rst_n_i)
    begin
      if (rst_n_i = '0') then
        per_count_lo  <= (others => '0');
        frac_count_lo <= (others => '0');
        frac_carry_lo <= '0';
      elsif rising_edge(clk_i) then
        if (ld_reg_p0_i = '1') then
          per_count_lo  <= unsigned(per_hi_i);
          frac_count_lo <= (others => '0');
        elsif (ld_lo_p0_i = '1') then
          per_count_lo  <= unsigned(per_count_i);
          frac_count_lo <= unsigned(frac_count_i);
          frac_carry_lo <= frac_carry_i;
        elsif (per_add_lo(per_add_lo'high) = '1') then
          per_count_lo  <= per_add_lo(32 downto 1) + unsigned(per_i);
          frac_count_lo <= frac_add_lo(frac_count_lo'range);
          frac_carry_lo <= frac_add_lo(frac_add_lo'high);
        else
          per_count_lo  <= per_add_lo(32 downto 1);
          frac_carry_lo <= '0';
        end if;
      end if;
    end process p_secondary_counters;

  end generate gen_secondary_counters;

end generate gen_frac_yes;
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
gen_frac_no : if (g_with_frac_counter = false) generate

  frac_carry_hi <= '0';

  per_add_hi <= ('0' & per_count_hi & '1') +
                ('1' & unsigned(to_signed(-g_num_serdes_bits, 32)) & frac_carry_hi);

  p_counter : process (clk_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      per_count_hi  <= (others => '0');
    elsif rising_edge(clk_i) then
      if (ld_reg_p0_i = '1') then
        per_count_hi <= (others => '0');
      elsif (ld_hi_p0_i = '1') then
        per_count_hi <= unsigned(per_count_i);
      elsif (per_add_hi(per_add_hi'high) = '1') then
        per_count_hi <= per_add_hi(32 downto 1) + unsigned(per_i);
      else
        per_count_hi <= per_add_hi(32 downto 1);
      end if;
    end if;
  end process p_counter;

  gen_secondary_counter : if (g_selectable_duty_cycle = true) generate

    frac_carry_lo <= '0';

    per_add_lo <= ('0' & per_count_lo & '1') +
                  ('1' & unsigned(to_signed(-g_num_serdes_bits, 32)) & frac_carry_lo);

    p_secondary_counter : process (clk_i, rst_n_i)
    begin
      if (rst_n_i = '0') then
        per_count_lo <= (others => '0');
      elsif rising_edge(clk_i) then
        if (ld_reg_p0_i = '1') then
          per_count_lo <= unsigned(per_hi_i);
        elsif (ld_lo_p0_i = '1') then
          per_count_lo <= unsigned(per_count_i);
        elsif (per_add_lo(per_add_lo'high) = '1') then
          per_count_lo <= per_add_lo(32 downto 1) + unsigned(per_i);
        else
          per_count_lo <= per_add_lo(32 downto 1);
        end if;
      end if;
    end process p_secondary_counter;

  end generate gen_secondary_counter;

end generate gen_frac_no;
--------------------------------------------------------------------------------

  -- Saturated barrel shifter, shifts the mask by the number of bits indicated by
  -- per_count, or fully if the counter is saturated from the point of view of the
  -- shifter (shift value > SERDES number of bits).
  --
  -- The lower bits of the bit mask are presented to the XOR chain below prior to
  -- outputting to the SERDES.
  msk_hi   <= unsigned(mask_i(31 downto 0)) when (frac_carry_hi = '0') else
              unsigned(mask_i(31 downto g_num_serdes_bits) & ('0' & mask_i(g_num_serdes_bits-1 downto 1)));
  shmsk_hi <= shift_right(msk_hi, to_integer(per_count_hi(f_log2_size(2*g_num_serdes_bits)-1 downto 0)))
                when (per_count_hi < 2*g_num_serdes_bits) else
              (others => '0');
  mask_hi  <= std_logic_vector(shmsk_hi(g_num_serdes_bits-1 downto 0));

  -- Output bit-flip based on mask and value of output on prev. cycle
  outp_hi(g_num_serdes_bits-1) <= outp_hi_d0 xor mask_hi(g_num_serdes_bits-1);
  gen_outp_bits : for i in g_num_serdes_bits-2 downto 0 generate
    outp_hi(i) <= outp_hi(i+1) xor mask_hi(i);
  end generate gen_outp_bits;

  p_outp_delay : process (clk_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      outp_hi_d0 <= '0';
    elsif rising_edge(clk_i) then
      if (ld_reg_p0_i = '1') then
        outp_hi_d0 <= '0';
      else
        outp_hi_d0 <= outp_hi(0);
      end if;
    end if;
--    if rising_edge(clk_i) then
--      if (rst_n_i = '0') or (ld_lo_p0_i = '1') then
--        outp_hi_d0 <= '0';
--      else
--        outp_hi_d0 <= outp_hi(0);
--      end if;
--    end if;
  end process p_outp_delay;

gen_secondary_outp_logic : if (g_selectable_duty_cycle = true) generate

  msk_lo   <= unsigned(mask_i(31 downto 0)) when (frac_carry_lo = '0') else
              unsigned(mask_i(31 downto g_num_serdes_bits) & ('0' & mask_i(g_num_serdes_bits-1 downto 1)));
  shmsk_lo <= shift_right(msk_lo, to_integer(per_count_lo(f_log2_size(2*g_num_serdes_bits)-1 downto 0)))
                when (per_count_lo < 2*g_num_serdes_bits) else
              (others => '0');
  mask_lo  <= std_logic_vector(shmsk_lo(g_num_serdes_bits-1 downto 0));

  outp_lo(g_num_serdes_bits-1) <= outp_lo_d0 xor mask_lo(g_num_serdes_bits-1);
  gen_secondary_outp_bits : for i in g_num_serdes_bits-2 downto 0 generate
    outp_lo(i) <= outp_lo(i+1) xor mask_lo(i);
  end generate gen_secondary_outp_bits;

  p_secondary_outp_delay : process (clk_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      outp_lo_d0 <= '0';
    elsif rising_edge(clk_i) then
      if (ld_reg_p0_i = '1') then
        outp_lo_d0 <= '0';
      else
        outp_lo_d0 <= outp_lo(0);
      end if;
    end if;
--    if rising_edge(clk_i) then
--      if (rst_n_i = '0') or (ld_p0_i = '1') then
--        outp_lo_d0 <= '0';
--      else
--        outp_lo_d0 <= outp_lo(0);
--      end if;
--    end if;
  end process p_secondary_outp_delay;

end generate gen_secondary_outp_logic;

  --===========================================================================
  -- Output register
  --===========================================================================
gen_outp_reg_simple : if (g_selectable_duty_cycle = false) generate
  p_outp_reg : process (clk_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      serdes_dat_o <= (others => '0');
    elsif rising_edge(clk_i) then
      serdes_dat_o <= outp_hi;
    end if;
  end process p_outp_reg;
end generate gen_outp_reg_simple;

gen_outp_reg_xored : if (g_selectable_duty_cycle = true) generate
  p_outp_reg : process (clk_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      serdes_dat_o <= (others => '0');
    elsif rising_edge(clk_i) then
      serdes_dat_o <= outp_hi xor outp_lo;
    end if;
  end process p_outp_reg;
end generate gen_outp_reg_xored;

end architecture arch;
--==============================================================================
--  architecture end
--==============================================================================
