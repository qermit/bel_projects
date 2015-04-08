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


entity wb_serdes_clk_gen is
  generic
  (
    g_num_outputs           : natural;
    g_with_frac_counter     : boolean := false;
    g_selectable_duty_cycle : boolean := false
  );
  port
  (
    ---------------------------------------------------------------------------
    -- Ports in clk_sys_i domain
    ---------------------------------------------------------------------------
    clk_sys_i    : in  std_logic;
    rst_sys_n_i  : in  std_logic;

    wb_adr_i     : in  std_logic_vector( 2 downto 0);
    wb_dat_i     : in  std_logic_vector(31 downto 0);
    wb_dat_o     : out std_logic_vector(31 downto 0);
    wb_cyc_i     : in  std_logic;
    wb_sel_i     : in  std_logic_vector(3 downto 0);
    wb_stb_i     : in  std_logic;
    wb_we_i      : in  std_logic;
    wb_ack_o     : out std_logic;
    wb_stall_o   : out std_logic;

    ---------------------------------------------------------------------------
    -- Ports in clk_ref_i domain
    ---------------------------------------------------------------------------
    clk_ref_i    : in  std_logic;
    rst_ref_n_i  : in  std_logic;

    serdes_dat_o : out t_lvds_byte_array
  );
end entity wb_serdes_clk_gen;


architecture arch of wb_serdes_clk_gen is

  --============================================================================
  -- Type declarations
  --============================================================================
  type t_reg_array is array (natural range g_num_outputs-1 downto 0)
                        of std_logic_vector(31 downto 0);

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
      reg_chsel_o                             : out    std_logic_vector(31 downto 0);
      -- Ports for asynchronous (clock: clk_ref_i) std_logic_vector field: 'Bits of currently selected banked register' in reg: 'PR'
      reg_per_o                                : out    std_logic_vector(31 downto 0);
      reg_per_i                                : in     std_logic_vector(31 downto 0);
      reg_per_load_o                           : out    std_logic;
      -- Ports for asynchronous (clock: clk_ref_i) std_logic_vector field: 'Bits of currently selected banked register' in reg: 'FRACR'
      reg_frac_o                               : out    std_logic_vector(31 downto 0);
      reg_frac_i                               : in     std_logic_vector(31 downto 0);
      reg_frac_load_o                          : out    std_logic;
      -- Ports for asynchronous (clock: clk_ref_i) std_logic_vector field: 'Bits of currently selected banked register' in reg: 'MASKR'
      reg_mask_o                               : out    std_logic_vector(31 downto 0);
      reg_mask_i                               : in     std_logic_vector(31 downto 0);
      reg_mask_load_o                          : out    std_logic;
  -- Ports for asynchronous (clock: clk_ref_i) std_logic_vector field: 'Bits of currently selected banked register' in reg: 'PHSHR'
      reg_phsh_o                               : out    std_logic_vector(31 downto 0);
      reg_phsh_i                               : in     std_logic_vector(31 downto 0);
      reg_phsh_load_o                          : out    std_logic
    );
  end component serdes_clk_gen_regs;

  --============================================================================
  -- Signal declarations
  --============================================================================
  signal chsel           : std_logic_vector(31 downto 0);

  signal per             : t_reg_array;
  signal per_fr_regs     : std_logic_vector(31 downto 0);
  signal per_to_regs     : std_logic_vector(31 downto 0);
  signal per_fr_regs_ld  : std_logic;

  signal frac            : t_reg_array;
  signal frac_fr_regs    : std_logic_vector(31 downto 0);
  signal frac_to_regs    : std_logic_vector(31 downto 0);
  signal frac_fr_regs_ld : std_logic;

  signal mask            : t_reg_array;
  signal mask_fr_regs    : std_logic_vector(31 downto 0);
  signal mask_to_regs    : std_logic_vector(31 downto 0);
  signal mask_fr_regs_ld : std_logic;

  signal phsh            : t_reg_array;
  signal phsh_fr_regs    : std_logic_vector(31 downto 0);
  signal phsh_to_regs    : std_logic_vector(31 downto 0);
  signal phsh_fr_regs_ld : std_logic;

  signal ld              : std_logic;
  signal ld_clkgen_p0    : std_logic_vector(g_num_outputs-1 downto 0);

  signal rst_ref_n_array : std_logic_vector(g_num_outputs-1 downto 0);
  signal rst_n_array     : std_logic_vector(g_num_outputs-1 downto 0);

--==============================================================================
--  architecture begin
--==============================================================================
begin

  --============================================================================
  -- WB registers component
  --============================================================================
  cmp_wb_regs : serdes_clk_gen_regs
    port map
    (
      rst_n_i          => rst_sys_n_i,
      clk_sys_i        => clk_sys_i,
      wb_adr_i         => wb_adr_i,
      wb_dat_i         => wb_dat_i,
      wb_dat_o         => wb_dat_o,
      wb_cyc_i         => wb_cyc_i,
      wb_sel_i         => wb_sel_i,
      wb_stb_i         => wb_stb_i,
      wb_we_i          => wb_we_i,
      wb_ack_o         => wb_ack_o,
      wb_stall_o       => wb_stall_o,

      clk_ref_i        => clk_ref_i,

      reg_chsel_o      => chsel,

      reg_per_o        => per_fr_regs,
      reg_per_i        => per_to_regs,
      reg_per_load_o   => per_fr_regs_ld,

      reg_frac_o       => frac_fr_regs,
      reg_frac_i       => frac_to_regs,
      reg_frac_load_o  => frac_fr_regs_ld,

      reg_mask_o       => mask_fr_regs,
      reg_mask_i       => mask_to_regs,
      reg_mask_load_o  => mask_fr_regs_ld,

      reg_phsh_o       => phsh_fr_regs,
      reg_phsh_i       => phsh_to_regs,
      reg_phsh_load_o  => phsh_fr_regs_ld
    );

  --============================================================================
  -- Register banks
  --============================================================================
  ld <= per_fr_regs_ld or frac_fr_regs_ld or mask_fr_regs_ld or phsh_fr_regs_ld;

  p_reg_banks : process (clk_ref_i, rst_ref_n_i)
  begin
    if (rst_ref_n_i = '0') then

      per          <= (others => (others => '0'));
      frac         <= (others => (others => '0'));
      mask         <= (others => (others => '0'));
      per_to_regs  <= (others => '0');
      frac_to_regs <= (others => '0');
      mask_to_regs <= (others => '0');
      phsh_to_regs <= (others => '0');
      ld_clkgen_p0 <= (others => '0');

    elsif rising_edge(clk_ref_i) then

      per_to_regs <= per(to_integer(unsigned(chsel)));
      if (per_fr_regs_ld = '1') then
        per(to_integer(unsigned(chsel))) <= per_fr_regs;
      end if;

      frac_to_regs <= frac(to_integer(unsigned(chsel)));
      if (frac_fr_regs_ld = '1') then
        frac(to_integer(unsigned(chsel))) <= frac_fr_regs;
      end if;

      mask_to_regs <= mask(to_integer(unsigned(chsel)));
      if (mask_fr_regs_ld = '1') then
        mask(to_integer(unsigned(chsel))) <= mask_fr_regs;
      end if;

      phsh_to_regs <= phsh(to_integer(unsigned(chsel)));
      if (phsh_fr_regs_ld = '1') then
        phsh(to_integer(unsigned(chsel))) <= phsh_fr_regs;
      end if;

      ld_clkgen_p0 <= (others => '0');
      if (ld = '1') then
        ld_clkgen_p0(to_integer(unsigned(chsel))) <= '1';
      end if;

    end if; -- rising_edge()
  end process p_reg_banks;

  --============================================================================
  -- SERDES clock generators
  --============================================================================
  rst_ref_n_array <= (others => rst_ref_n_i);
  rst_n_array     <= rst_ref_n_array and (not ld_clkgen_p0);
  gen_components : for i in 0 to g_num_outputs-1 generate
    cmp_clk_gen : serdes_clk_gen
      generic map
      (
        g_num_serdes_bits       => 8,
        g_with_frac_counter     => g_with_frac_counter,
        g_selectable_duty_cycle => g_selectable_duty_cycle
      )
      port map
      (
        clk_i        => clk_ref_i,
        rst_n_i      => rst_n_array(i),

        per_i        => per(i),
        frac_i       => frac(i),
        mask_i       => mask(i),
        ph_shift_i   => phsh(i),

        serdes_dat_o => serdes_dat_o(i)
      );
  end generate gen_components;

end architecture arch;
--==============================================================================
--  architecture end
--==============================================================================
