--==============================================================================
-- GSI Helmholz center for Heavy Ion Research GmbH
-- SERDES clock generator with Wishbone interface
--==============================================================================
--
-- author: Theodor Stana (t.stana@gsi.de)
--
-- date of creation: 2015-03-25
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
--    2015-03-25   Theodor Stana     File created
--==============================================================================
-- TODO: -
--==============================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.altera_lvds_pkg.all;
use work.genram_pkg.all;


entity wb_serdes_clk_gen is
  generic
  (
    g_num_outputs           : natural;
    g_num_serdes_bits       : natural;
    g_with_frac_counter     : boolean := false;
    g_selectable_duty_cycle : boolean := false;
    g_with_sync             : boolean := false
  );
  port
  (
    ---------------------------------------------------------------------------
    -- Ports in clk_sys_i domain
    ---------------------------------------------------------------------------
    clk_sys_i         : in  std_logic;
    rst_sys_n_i       : in  std_logic;

    wb_adr_i          : in  std_logic_vector( 2 downto 0);
    wb_dat_i          : in  std_logic_vector(31 downto 0);
    wb_dat_o          : out std_logic_vector(31 downto 0);
    wb_cyc_i          : in  std_logic;
    wb_sel_i          : in  std_logic_vector(3 downto 0);
    wb_stb_i          : in  std_logic;
    wb_we_i           : in  std_logic;
    wb_ack_o          : out std_logic;
    wb_stall_o        : out std_logic;

    ---------------------------------------------------------------------------
    -- Ports in clk_ref_i domain
    ---------------------------------------------------------------------------
    clk_ref_i         : in  std_logic;
    rst_ref_n_i       : in  std_logic;

    eca_time_i        : in std_logic_vector(63 downto 0);
    eca_time_valid_i  : in std_logic;

    serdes_dat_o      : out t_lvds_byte_array
  );
end entity wb_serdes_clk_gen;


architecture arch of wb_serdes_clk_gen is

  --============================================================================
  -- Type declarations
  --============================================================================
  type t_reg_array is array (natural range g_num_outputs-1 downto 0)
                        of std_logic_vector(31 downto 0);

  type t_state is (
    START,
    LOAD_REGS,
    SUB_PHASE_FROM_TIME,
    LOAD_DIV,
    CHECK_TIME,
    DIVIDE_TIME_BY_PER,
    SET_FRAC_COUNTER,
    PREDICT_COUNTER_STATE,
    SET_PER_COUNTER,
    SET_DELAYED_BIT,
    APPLY_COUNTER_VALS
  );

  --============================================================================
  -- Component declarations
  --============================================================================
  component serdes_clk_gen is
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
      mask_normal_i : in  std_logic_vector(31 downto 0);
      mask_skip_i   : in  std_logic_vector(31 downto 0);

      -- Counter load ports for external synchronization machine
      ld_lo_p0_i    : in  std_logic;
      ld_hi_p0_i    : in  std_logic;
      per_count_i   : in  std_logic_vector(31 downto 0);
      frac_count_i  : in  std_logic_vector(31 downto 0);
      frac_carry_i  : in  std_logic;
      last_bit_i    : in  std_logic;

      -- Data output to SERDES, synchronous to clk_i
      serdes_dat_o  : out std_logic_vector(7 downto 0)
    );
  end component serdes_clk_gen;

  component serdes_clk_gen_regs is
    port (
      rst_n_i                                  : in     std_logic;
      clk_sys_i                                : in     std_logic;
      wb_adr_i                                 : in     std_logic_vector(2 downto 0);
      wb_dat_i                                 : in     std_logic_vector(31 downto 0);
      wb_dat_o                                 : out    std_logic_vector(31 downto 0);
      wb_cyc_i                                 : in     std_logic;
      wb_sel_i                                 : in     std_logic_vector(3 downto 0);
      wb_stb_i                                 : in     std_logic;
      wb_we_i                                  : in     std_logic;
      wb_ack_o                                 : out    std_logic;
      wb_stall_o                               : out    std_logic;
      clk_ref_i                                : in     std_logic;
  -- Port for std_logic_vector field: 'Channel select bits' in reg: 'CHSELR'
      reg_chsel_o                              : out    std_logic_vector(31 downto 0);
  -- Ports for asynchronous (clock: clk_ref_i) std_logic_vector field: 'Bits of currently selected banked register' in reg: 'PERR'
      reg_per_o                                : out    std_logic_vector(31 downto 0);
      reg_per_i                                : in     std_logic_vector(31 downto 0);
      reg_per_load_o                           : out    std_logic;
  -- Ports for asynchronous (clock: clk_ref_i) std_logic_vector field: 'Bits of currently selected banked register' in reg: 'PERHIR'
      reg_perhi_o                              : out    std_logic_vector(31 downto 0);
      reg_perhi_i                              : in     std_logic_vector(31 downto 0);
      reg_perhi_load_o                         : out    std_logic;
  -- Ports for asynchronous (clock: clk_ref_i) std_logic_vector field: 'Bits of currently selected banked register' in reg: 'FRACR'
      reg_frac_o                               : out    std_logic_vector(31 downto 0);
      reg_frac_i                               : in     std_logic_vector(31 downto 0);
      reg_frac_load_o                          : out    std_logic;
  -- Ports for asynchronous (clock: clk_ref_i) std_logic_vector field: 'Bits of currently selected banked register' in reg: 'NORMMASKR'
      reg_mask_normal_o                        : out    std_logic_vector(31 downto 0);
      reg_mask_normal_i                        : in     std_logic_vector(31 downto 0);
      reg_mask_normal_load_o                   : out    std_logic;
  -- Ports for asynchronous (clock: clk_ref_i) std_logic_vector field: 'Bits of currently selected banked register' in reg: 'SKIPMASKR'
      reg_mask_skip_o                          : out    std_logic_vector(31 downto 0);
      reg_mask_skip_i                          : in     std_logic_vector(31 downto 0);
      reg_mask_skip_load_o                     : out    std_logic;
  -- Ports for asynchronous (clock: clk_ref_i) std_logic_vector field: 'Bits of currently selected banked register' in reg: 'PHOFSLR'
      reg_phofsl_o                             : out    std_logic_vector(31 downto 0);
      reg_phofsl_i                             : in     std_logic_vector(31 downto 0);
      reg_phofsl_load_o                        : out    std_logic;
  -- Ports for asynchronous (clock: clk_ref_i) std_logic_vector field: 'Bits of currently selected banked register' in reg: 'PHOFSHR'
      reg_phofsh_o                             : out    std_logic_vector(31 downto 0);
      reg_phofsh_i                             : in     std_logic_vector(31 downto 0);
      reg_phofsh_load_o                        : out    std_logic
    );
  end component serdes_clk_gen_regs;

  --============================================================================
  -- Signal declarations
  --============================================================================
  signal chsel                : std_logic_vector(31 downto 0);

  signal per_bank             : t_reg_array;
  signal per_reg_out          : std_logic_vector(31 downto 0);
  signal per_reg_in           : std_logic_vector(31 downto 0);
  signal per_reg_ld           : std_logic;

  signal frac_bank            : t_reg_array;
  signal frac_reg_out         : std_logic_vector(31 downto 0);
  signal frac_reg_in          : std_logic_vector(31 downto 0);
  signal frac_reg_ld          : std_logic;

  signal mask_normal_bank     : t_reg_array;
  signal mask_normal_reg_out  : std_logic_vector(31 downto 0);
  signal mask_normal_reg_in   : std_logic_vector(31 downto 0);
  signal mask_normal_reg_ld   : std_logic;

  signal mask_skip_bank       : t_reg_array;
  signal mask_skip_reg_out    : std_logic_vector(31 downto 0);
  signal mask_skip_reg_in     : std_logic_vector(31 downto 0);
  signal mask_skip_reg_ld     : std_logic;

  signal phofsl_bank          : t_reg_array;
  signal phofsl_reg_out       : std_logic_vector(31 downto 0);
  signal phofsl_reg_in        : std_logic_vector(31 downto 0);
  signal phofsl_reg_ld        : std_logic;
  signal phofsh_bank          : t_reg_array;
  signal phofsh_reg_out       : std_logic_vector(31 downto 0);
  signal phofsh_reg_in        : std_logic_vector(31 downto 0);
  signal phofsh_reg_ld        : std_logic;

  signal perhi_bank           : t_reg_array;
  signal perhi_reg_out        : std_logic_vector(31 downto 0);
  signal perhi_reg_in         : std_logic_vector(31 downto 0);
  signal perhi_reg_ld         : std_logic;

  signal ld                   : std_logic;
  signal ld_clkgen_p0         : std_logic_vector(g_num_outputs-1 downto 0);
  signal ld_lo_p0             : std_logic_vector(g_num_outputs-1 downto 0);
  signal ld_hi_p0             : std_logic_vector(g_num_outputs-1 downto 0);
  signal ld_regs_p0           : std_logic_vector(g_num_outputs-1 downto 0);

  -- Signals used for synchronization
  signal state                : t_state;
  signal channel_count        : natural range g_num_outputs-1 downto 0;
  signal div_count            : unsigned(  5 downto 0);
  signal numerator            : unsigned(126 downto 0);
  signal denominator          : unsigned(126 downto 0);
  signal did_subtract         : std_logic;

  signal last_bit             : std_logic;
  signal frac_carry           : std_logic;
  signal int                  : unsigned(31 downto 0);
  signal fraction             : unsigned(31 downto 0);
  signal count_integer        : unsigned(31 downto 0);
  signal count_fraction       : unsigned(32 downto 0);
  signal phase                : unsigned(63 downto 0);
  signal mask                 : unsigned(31 downto 0);
  signal mask_bit0            : std_logic;
  signal parity               : std_logic;
  signal phase_addend         : std_logic_vector(63 downto 0);
  signal perhi_sync           : std_logic_vector(31 downto 0);
  signal set_lo_count         : std_logic;

  signal sub_time             : unsigned(64 downto 0);

--==============================================================================
--   architecture begin
--==============================================================================
begin

  --============================================================================
  -- WB registers component
  --============================================================================
  cmp_wb_regs : serdes_clk_gen_regs
    port map
    (
      rst_n_i                 => rst_sys_n_i,
      clk_sys_i               => clk_sys_i,
      wb_adr_i                => wb_adr_i,
      wb_dat_i                => wb_dat_i,
      wb_dat_o                => wb_dat_o,
      wb_cyc_i                => wb_cyc_i,
      wb_sel_i                => wb_sel_i,
      wb_stb_i                => wb_stb_i,
      wb_we_i                 => wb_we_i,
      wb_ack_o                => wb_ack_o,
      wb_stall_o              => wb_stall_o,

      clk_ref_i               => clk_ref_i,

      reg_chsel_o             => chsel,

      reg_per_o               => per_reg_out,
      reg_per_i               => per_reg_in,
      reg_per_load_o          => per_reg_ld,

      reg_perhi_o             => perhi_reg_out,
      reg_perhi_i             => perhi_reg_in,
      reg_perhi_load_o        => perhi_reg_ld,

      reg_frac_o              => frac_reg_out,
      reg_frac_i              => frac_reg_in,
      reg_frac_load_o         => frac_reg_ld,

      reg_mask_normal_o       => mask_normal_reg_out,
      reg_mask_normal_i       => mask_normal_reg_in,
      reg_mask_normal_load_o  => mask_normal_reg_ld,

      reg_mask_skip_o         => mask_skip_reg_out,
      reg_mask_skip_i         => mask_skip_reg_in,
      reg_mask_skip_load_o    => mask_skip_reg_ld,

      reg_phofsl_o            => phofsl_reg_out,
      reg_phofsl_i            => phofsl_reg_in,
      reg_phofsl_load_o       => phofsl_reg_ld,

      reg_phofsh_o            => phofsh_reg_out,
      reg_phofsh_i            => phofsh_reg_in,
      reg_phofsh_load_o       => phofsh_reg_ld
    );

  --============================================================================
  -- Register banks
  --============================================================================
  ld <= per_reg_ld or perhi_reg_ld or frac_reg_ld or mask_normal_reg_ld or
          mask_skip_reg_ld or phofsl_reg_ld or phofsh_reg_ld;

  p_reg_banks : process (clk_ref_i, rst_ref_n_i)
  begin
    if (rst_ref_n_i = '0') then

      per_bank            <= (others => (others => '0'));
      perhi_bank          <= (others => (others => '0'));
      frac_bank           <= (others => (others => '0'));
      mask_normal_bank    <= (others => (others => '0'));
      mask_skip_bank      <= (others => (others => '0'));
      phofsl_bank         <= (others => (others => '0'));
      phofsh_bank         <= (others => (others => '0'));
      per_reg_in          <= (others => '0');
      perhi_reg_in        <= (others => '0');
      frac_reg_in         <= (others => '0');
      mask_normal_reg_in  <= (others => '0');
      mask_skip_reg_in    <= (others => '0');
      phofsl_reg_in       <= (others => '0');
      phofsh_reg_in       <= (others => '0');
      ld_regs_p0          <= (others => '0');

    elsif rising_edge(clk_ref_i) then

      per_reg_in <= per_bank(to_integer(unsigned(chsel)));
      if (per_reg_ld = '1') then
        per_bank(to_integer(unsigned(chsel))) <= per_reg_out;
      end if;

      perhi_reg_in <= perhi_bank(to_integer(unsigned(chsel)));
      if (perhi_reg_ld = '1') then
        perhi_bank(to_integer(unsigned(chsel))) <= perhi_reg_out;
      end if;

      frac_reg_in <= frac_bank(to_integer(unsigned(chsel)));
      if (frac_reg_ld = '1') then
        frac_bank(to_integer(unsigned(chsel))) <= frac_reg_out;
      end if;

      mask_normal_reg_in <= mask_normal_bank(to_integer(unsigned(chsel)));
      if (mask_normal_reg_ld = '1') then
        mask_normal_bank(to_integer(unsigned(chsel))) <= mask_normal_reg_out;
      end if;

      mask_skip_reg_in <= mask_skip_bank(to_integer(unsigned(chsel)));
      if (mask_skip_reg_ld = '1') then
        mask_skip_bank(to_integer(unsigned(chsel))) <= mask_skip_reg_out;
      end if;

      phofsl_reg_in <= phofsl_bank(to_integer(unsigned(chsel)));
      if (phofsl_reg_ld = '1') then
        phofsl_bank(to_integer(unsigned(chsel))) <= phofsl_reg_out;
      end if;

      phofsh_reg_in <= phofsh_bank(to_integer(unsigned(chsel)));
      if (phofsh_reg_ld = '1') then
        phofsh_bank(to_integer(unsigned(chsel))) <= phofsh_reg_out;
      end if;

      ld_regs_p0 <= (others => '0');
      if (ld = '1') then
        ld_regs_p0(to_integer(unsigned(chsel))) <= '1';
      end if;

    end if; -- rising_edge()
  end process p_reg_banks;

  --============================================================================
  -- Synchronization with clocks of the same frequency
  --============================================================================
gen_clock_sync_no : if (g_with_sync = false) generate

  ld_lo_p0        <= (others => '0');
  ld_hi_p0        <= (others => '0');
  count_integer   <= (others => '0');
  count_fraction  <= (others => '0');
  frac_carry      <= '0';

end generate gen_clock_sync_no;

gen_clock_sync_yes : if (g_with_sync = true) generate

  -- Perform phase alignment of two clocks of the same frequency by dividing the
  -- current ECA time by the period assigned to the counter.
  phase_addend <= x"00000000" & perhi_bank(channel_count) when (set_lo_count = '1') else
                  (others => '0');

  p_sync_fsm : process (clk_ref_i, rst_ref_n_i)
    variable tmp_phase  : std_logic_vector(phase'length-1 downto 0);
    variable tmp_mask   : unsigned(31 downto 0);
    variable tmp_parity : unsigned(31 downto 0);
  begin
    if (rst_ref_n_i = '0') then

      state           <= START;
      div_count       <= (others => '0');
      channel_count   <= 0;
      numerator       <= (others => '0');
      denominator     <= (others => '0');
      did_subtract    <= '0';
      int             <= (others => '0');
      fraction        <= (others => '0');
      phase           <= (others => '0');
      mask            <= (others => '0');
      mask_bit0         <= '0';
      parity          <= '0';
      sub_time        <= (others => '0');
      count_integer   <= (others => '0');
      count_fraction  <= (others => '0');
      frac_carry      <= '0';
      last_bit        <= '0';
      set_lo_count    <= '0';
      ld_lo_p0        <= (others => '0');
      ld_hi_p0        <= (others => '0');

    elsif rising_edge(clk_ref_i) then

      case state is

        when START =>
          ld_lo_p0    <= (others => '0');
          ld_hi_p0    <= (others => '0');
          frac_carry  <= '0';
          div_count   <= (others => '0');
          state       <= LOAD_REGS;

        when LOAD_REGS =>
          int       <= unsigned(per_bank(channel_count));
          fraction  <= unsigned(frac_bank(channel_count));
          tmp_phase := phofsh_bank(channel_count) & phofsl_bank(channel_count);
          phase     <= unsigned(tmp_phase) + unsigned(phase_addend);
          mask      <= unsigned(mask_normal_bank(channel_count));
          state     <= SUB_PHASE_FROM_TIME;

        when SUB_PHASE_FROM_TIME =>
          -- NOTE: ECA time shifted left by 3 to be multiplied by 8 (8 ns increments)
          sub_time <= unsigned('0' & shift_left(unsigned(eca_time_i), 3)) -
                      unsigned('0' & phase);
          state    <= LOAD_DIV;

        when LOAD_DIV =>
          numerator <= (numerator(126 downto 96)'range => sub_time(64)) &
                       sub_time(63 downto 0) &
                       (numerator(31 downto 0)'range => '0');

          denominator(126 downto 95)  <= unsigned(int);
          denominator( 94 downto 63)  <= unsigned(fraction);
          denominator( 62 downto  0)  <= (others => '0');

          state <= CHECK_TIME;

        when CHECK_TIME =>
          if (sub_time(64) = '1') or (sub_time = 0) then
            numerator <= numerator + denominator;
          end if;
          state <= DIVIDE_TIME_BY_PER;

        when DIVIDE_TIME_BY_PER =>
          did_subtract <= '0';
          if (numerator > denominator) then
            numerator <= numerator - denominator;
            did_subtract <= '1';
          end if;
          denominator <= shift_right(denominator, 1);
          div_count   <= div_count + 1;
          if (div_count = 63) then
            state <= SET_FRAC_COUNTER;
          end if;

        when SET_FRAC_COUNTER =>
          count_fraction <= ('0' & fraction) -
                            ('0' & numerator(31 downto 0));
          state <= PREDICT_COUNTER_STATE;

        when PREDICT_COUNTER_STATE =>
          frac_carry <= '0';
          if (numerator(63 downto 32) < g_num_serdes_bits) and
                (count_fraction(31 downto 0) < fraction) then
            frac_carry <= '1';
          end if;
          state <= SET_PER_COUNTER;

        when SET_PER_COUNTER =>
      -- !!! two borrow bits
          if (count_fraction(32) = '1') and (frac_carry = '1') then
            count_integer <= int - numerator(63 downto 32) - 2;
          elsif (count_fraction(32) = '1') or (frac_carry = '1') then
            count_integer <= int - numerator(63 downto 32) - 1;
          else
            count_integer <= int - numerator(63 downto 32);
          end if;
          state <= SET_DELAYED_BIT;

        when SET_DELAYED_BIT =>
          if (count_integer >= 16) then
            tmp_mask := (others => '0');
          else
            tmp_mask := x"000000" & mask(15 downto 8);
            tmp_mask := shift_right(tmp_mask, to_integer(count_integer));
            tmp_mask := tmp_mask and x"000000ff";
          end if;

          for i in 6 downto 0 loop
            tmp_mask(i) := tmp_mask(i+1) xor tmp_mask(i);
          end loop;

          tmp_parity := shift_right(mask, 7);
          tmp_parity := tmp_parity and x"000000ff";
          for i in 6 downto 0 loop
            tmp_parity(i) := tmp_parity(i+1) xor tmp_parity(i);
          end loop;

          mask_bit0 <= tmp_mask(0);
          parity    <= tmp_parity(0);
          state     <= APPLY_COUNTER_VALS;

        when APPLY_COUNTER_VALS =>
          last_bit <= (parity and did_subtract) xor mask_bit0 xor parity;

          if (set_lo_count = '0') then
            ld_hi_p0(channel_count) <= '1';
          else
            ld_lo_p0(channel_count) <= '1';
            channel_count <= channel_count + 1;
            if (channel_count = g_num_outputs-1) then
              channel_count <= 0;
            end if;
          end if;
          set_lo_count <= set_lo_count xor '1';
          state        <= START;

        when others =>
          state <= START;

      end case;

    end if; -- rising_edge()

  end process p_sync_fsm;

end generate gen_clock_sync_yes;

  --============================================================================
  -- SERDES clock generators
  --============================================================================
  gen_components : for i in 0 to g_num_outputs-1 generate
    cmp_clk_gen : serdes_clk_gen
      generic map
      (
        g_num_serdes_bits       => g_num_serdes_bits,
        g_with_frac_counter     => g_with_frac_counter,
        g_selectable_duty_cycle => g_selectable_duty_cycle
      )
      port map
      (
        clk_i           => clk_ref_i,
        rst_n_i         => rst_ref_n_i,

        ld_reg_p0_i     => ld_regs_p0(i),
        per_i           => per_bank(i),
        per_hi_i        => perhi_bank(i),
        frac_i          => frac_bank(i),
        mask_normal_i   => mask_normal_bank(i),
        mask_skip_i     => mask_skip_bank(i),

        ld_lo_p0_i      => ld_lo_p0(i),
        ld_hi_p0_i      => ld_hi_p0(i),
        per_count_i     => std_logic_vector(count_integer),
        frac_count_i    => std_logic_vector(count_fraction(31 downto 0)),
        frac_carry_i    => frac_carry,
        last_bit_i      => last_bit,

        serdes_dat_o    => serdes_dat_o(i)
      );
  end generate gen_components;

end architecture arch;
--==============================================================================
-- architecture end
--==============================================================================

