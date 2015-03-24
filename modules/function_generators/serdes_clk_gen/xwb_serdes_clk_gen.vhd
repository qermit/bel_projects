--! @file        xwb_serdes_clk_gen.vhd
--  DesignUnit   xwb_serdes_clk_gen
--! @author      Theodor Stana <>
--! @date        24/03/2015
--! @version     
--! @copyright   2015 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!

--TODO: This is a stub, finish/update it yourself
--! @brief *** ADD BRIEF DESCRIPTION HERE ***
--!
--------------------------------------------------------------------------------
--! This library is free software; you can redistribute it and/or
--! modify it under the terms of the GNU Lesser General Public
--! License as published by the Free Software Foundation; either
--! version 3 of the License, or (at your option) any later version.
--!
--! This library is distributed in the hope that it will be useful,
--! but WITHOUT ANY WARRANTY; without even the implied warranty of
--! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
--! Lesser General Public License for more details.
--!
--! You should have received a copy of the GNU Lesser General Public
--! License along with this library. If not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.xwb_serdes_clk_gen_auto_pkg.all;

entity xwb_serdes_clk_gen is
Port(
   clk_sys_i   : in  std_logic;
   clk_ref_i   : in  std_logic;
   rst_n_i     : in  std_logic;

   wbs_i       : in  t_wishbone_slave_in;
   wbs_o       : out t_wishbone_slave_out
);
end xwb_serdes_clk_gen;

architecture rtl of xwb_serdes_clk_gen is

   signal s_wbs_regs_clk_sys_o   : t_wbs_regs_clk_sys_o;
   signal s_wbs_regs_clk_ref_o   : t_wbs_regs_clk_ref_o;
   signal s_wbs_regs_clk_sys_i   : t_wbs_regs_clk_sys_i;
   signal s_wbs_i                : t_wishbone_slave_in;
   signal s_wbs_o                : t_wishbone_slave_out;


begin

   INST_xwb_serdes_clk_gen_auto : xwb_serdes_clk_gen_auto
   port map (
      clk_sys_i            => clk_sys_i,
      clk_ref_i            => clk_ref_i,
      rst_n_i              => rst_n_i,

      wbs_regs_clk_sys_o   => s_wbs_regs_clk_sys_o,
      wbs_regs_clk_ref_o   => s_wbs_regs_clk_ref_o,
      wbs_regs_clk_sys_i   => s_wbs_regs_clk_sys_i,
      wbs_i                => s_wbs_i,
      wbs_o                => s_wbs_o
   );
end rtl;
