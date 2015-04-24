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
use work.wishbone_pkg.all;
use work.altera_lvds_pkg.all;
use work.wb_serdes_clk_gen_pkg.all;

entity xwb_serdes_clk_gen is
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
    --------------------------------------------------------------------------
    -- Ports in clk_sys_i domain
    --------------------------------------------------------------------------
    clk_sys_i         : in  std_logic;
    rst_sys_n_i       : in  std_logic;

    wbs_i             : in  t_wishbone_slave_in;
    wbs_o             : out t_wishbone_slave_out;

    --------------------------------------------------------------------------
    -- Ports in clk_ref_i domain
    --------------------------------------------------------------------------
    clk_ref_i         : in  std_logic;
    rst_ref_n_i       : in  std_logic;

    eca_time_i        : in std_logic_vector(63 downto 0);
    eca_time_valid_i  : in std_logic;

    serdes_dat_o      : out t_lvds_byte_array
  );
end entity xwb_serdes_clk_gen;


architecture arch of xwb_serdes_clk_gen is

--==============================================================================
--  architecture begin
--==============================================================================
begin

  cmp_wrapped_component : wb_serdes_clk_gen
    generic map
    (
      g_num_outputs           => g_num_outputs,
      g_num_serdes_bits       => g_num_serdes_bits,
      g_with_frac_counter     => g_with_frac_counter,
      g_selectable_duty_cycle => g_selectable_duty_cycle,
      g_with_sync             => g_with_sync
    )
    port map
    (
      ---------------------------------------------------------------------------
      -- Ports in clk_sys_i domain
      ---------------------------------------------------------------------------
      clk_sys_i         => clk_sys_i,
      rst_sys_n_i       => rst_sys_n_i,

      wb_adr_i          => wbs_i.adr(4 downto 2),
      wb_dat_i          => wbs_i.dat,
      wb_dat_o          => wbs_o.dat,
      wb_cyc_i          => wbs_i.cyc,
      wb_sel_i          => wbs_i.sel,
      wb_stb_i          => wbs_i.stb,
      wb_we_i           => wbs_i.we,
      wb_ack_o          => wbs_o.ack,
      wb_stall_o        => wbs_o.stall,

      ---------------------------------------------------------------------------
      -- Ports in clk_ref_i domain
      ---------------------------------------------------------------------------
      clk_ref_i         => clk_ref_i,
      rst_ref_n_i       => rst_ref_n_i,

      eca_time_i        => eca_time_i,
      eca_time_valid_i  => eca_time_valid_i,

      serdes_dat_o      => serdes_dat_o
    );

    wbs_o.err <= '0';
    wbs_o.rty <= '0';
    wbs_o.int <= '0';

end architecture arch;
--==============================================================================
--  architecture end
--==============================================================================
