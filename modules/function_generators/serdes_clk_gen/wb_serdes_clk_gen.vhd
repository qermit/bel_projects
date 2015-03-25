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
    g_num_outputs : natural
  );
  port
  (
    ---------------------------------------------------------------------------
    -- Ports in clk_sys_i domain
    ---------------------------------------------------------------------------
    clk_sys_i    : in  std_logic;
    rst_sys_n_i  : in  std_logic;

    wb_adr_i     : in  std_logic_vector(1 downto 0);
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
    port
    (
      -- Clock and reset signals
      clk_i        : in  std_logic;
      rst_n_i      : in  std_logic;

      -- Period and maks register inputs, synchronous to clk_ref_i
      hperr_i      : in  std_logic_vector(31 downto 0);
      maskr_i      : in  std_logic_vector(31 downto 0);

      -- Data output to SERDES, synchronous to clk_ref_i
      serdes_dat_o : out std_logic_vector(7 downto 0)
    );
  end component serdes_clk_gen;

  component serdes_clk_gen_regs is
    port (
      rst_n_i                                  : in     std_logic;
      clk_sys_i                                : in     std_logic;
      wb_adr_i                                 : in     std_logic_vector(1 downto 0);
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
      reg_chselr_o                             : out    std_logic_vector(31 downto 0);
  -- Ports for asynchronous (clock: clk_ref_i) std_logic_vector field: 'Bits of currently selected banked register' in reg: 'HPERR'
      reg_hperr_o                              : out    std_logic_vector(31 downto 0);
      reg_hperr_i                              : in     std_logic_vector(31 downto 0);
      reg_hperr_load_o                         : out    std_logic;
  -- Ports for asynchronous (clock: clk_ref_i) std_logic_vector field: 'Bits of currently selected banked register' in reg: 'MASKR'
      reg_maskr_o                              : out    std_logic_vector(31 downto 0);
      reg_maskr_i                              : in     std_logic_vector(31 downto 0);
      reg_maskr_load_o                         : out    std_logic
    );
  end component serdes_clk_gen_regs;

  --============================================================================
  -- Signal declarations
  --============================================================================
  signal chselr           : std_logic_vector(31 downto 0);
  signal hperr_fr_regs    : std_logic_vector(31 downto 0);
  signal hperr_to_regs    : std_logic_vector(31 downto 0);
  signal hperr_fr_regs_ld : std_logic;
  signal maskr_fr_regs    : std_logic_vector(31 downto 0);
  signal maskr_to_regs    : std_logic_vector(31 downto 0);
  signal maskr_fr_regs_ld : std_logic;

  signal hperr            : t_reg_array;
  signal maskr            : t_reg_array;

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

      reg_chselr_o     => chselr,

      reg_hperr_o      => hperr_fr_regs,
      reg_hperr_i      => hperr_to_regs,
      reg_hperr_load_o => hperr_fr_regs_ld,

      reg_maskr_o      => maskr_fr_regs,
      reg_maskr_i      => maskr_to_regs,
      reg_maskr_load_o => maskr_fr_regs_ld
    );

  --============================================================================
  -- Register banks
  --============================================================================
  p_reg_banks : process (clk_ref_i)
  begin
    if rising_edge(clk_ref_i) then
      if (rst_ref_n_i = '0') then
        hperr         <= (others => (others => '0'));
        maskr         <= (others => (others => '0'));
        hperr_to_regs <= (others => '0');
        maskr_to_regs <= (others => '0');
      else
        hperr_to_regs <= hperr(to_integer(unsigned(chselr)));
        if (hperr_fr_regs_ld = '1') then
          hperr(to_integer(unsigned(chselr))) <= hperr_fr_regs;
        end if;

        maskr_to_regs <= maskr(to_integer(unsigned(chselr)));
        if (maskr_fr_regs_ld = '1') then
          maskr(to_integer(unsigned(chselr))) <= maskr_fr_regs;
        end if;
      end if; -- !rst_ref_n_i
    end if; -- rising_edge()
  end process p_reg_banks;

  --============================================================================
  -- SERDES clock generators
  --============================================================================
  gen_components : for i in 0 to g_num_outputs-1 generate
    cmp_clk_gen : serdes_clk_gen
      port map
      (
        clk_i        => clk_ref_i,
        rst_n_i      => rst_ref_n_i,

        hperr_i      => hperr(i),
        maskr_i      => maskr(i),

        serdes_dat_o => serdes_dat_o(i)
      );
  end generate gen_components;

end architecture arch;
--==============================================================================
--  architecture end
--==============================================================================
