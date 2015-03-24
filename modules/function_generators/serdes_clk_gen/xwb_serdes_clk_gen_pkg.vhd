--! @file        xwb_serdes_clk_gen_pkg.vhd
--  DesignUnit   xwb_serdes_clk_gen
--! @author      Theodor Stana <>
--! @date        24/03/2015
--! @version     
--! @copyright   2015 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!

--TODO: This is a stub, finish/update it yourself
--! @brief Package for xwb_serdes_clk_gen.vhd
--! If you modify the outer entity, don't forget to update this component! 
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
package xwb_serdes_clk_gen_pkg is

      --| Component -------------------- xwb_serdes_clk_gen ---------------------------------------|
   component xwb_serdes_clk_gen is
   Port(
      clk_sys_i   : in  std_logic;
      clk_ref_i   : in  std_logic;
      rst_n_i     : in  std_logic;

      wbs_i       : in  t_wishbone_slave_in;
      wbs_o       : out t_wishbone_slave_out
   );
   end component;

   constant c_xwb_serdes_clk_gen_wbs_sdb : t_sdb_device := work.xwb_serdes_clk_gen_auto_pkg.c_xwb_serdes_clk_gen_wbs_sdb;
   
end xwb_serdes_clk_gen_pkg;
package body xwb_serdes_clk_gen_pkg is
end xwb_serdes_clk_gen_pkg;
