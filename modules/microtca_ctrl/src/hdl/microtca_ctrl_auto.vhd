--! @file        microtca_ctrl_auto.vhd
--  DesignUnit   microtca_ctrl_auto
--! @author      A. Hahn <a.hahn@gsi.de>
--! @date        19/11/2015
--! @version     0.0.1
--! @copyright   2015 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!

--! @brief AUTOGENERATED WISHBONE-SLAVE CORE FOR microtca_ctrl.vhd
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

-- ***********************************************************
-- ** WARNING - THIS IS AUTO-GENERATED CODE! DO NOT MODIFY! **
-- ***********************************************************
--
-- If you want to change the interface,
-- modify microtca_ctrl.xml and re-run 'python wbgenplus.py microtca_ctrl.xml' !

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.microtca_ctrl_auto_pkg.all;

entity microtca_ctrl_auto is
Port(
   clk_sys_i            : in  std_logic;
   rst_n_i              : in  std_logic;

   slave_regs_clk_sys_o : out t_slave_regs_clk_sys_o;
   slave_regs_clk_sys_i : in  t_slave_regs_clk_sys_i;
   slave_i              : in  t_wishbone_slave_in;
   slave_o              : out t_wishbone_slave_out
   
);
end microtca_ctrl_auto;

architecture rtl of microtca_ctrl_auto is

   --+******************************************************************************************+
   --|  ------------------------------------- WB Registers -------------------------------------|
   --+******************************************************************************************+

   --| WBS Regs ---------------------------- slave ---------------------------------------------|
   signal r_slave             : t_slave_regs_o;
   signal s_slave             : t_slave_regs_i;
   signal r_slave_out_stall   : std_logic;
   signal r_slave_out_ack0,
          r_slave_out_ack1,
          r_slave_out_err0,
          r_slave_out_err1    : std_logic;
   signal r_slave_out_dat0,
          r_slave_out_dat1    : std_logic_vector(31 downto 0);



begin

   --+******************************************************************************************+
   --| Sync Signal Assignments ------ slave ----------------------------------------------------|
   --+******************************************************************************************+
   -- slave sys domain out
   slave_regs_clk_sys_o.CLOCK_CONTROL_OE  <= r_slave.CLOCK_CONTROL_OE;
   slave_regs_clk_sys_o.LOGIC_CONTROL_OE  <= r_slave.LOGIC_CONTROL_OE;
   slave_regs_clk_sys_o.LOGIC_OUTPUT      <= r_slave.LOGIC_OUTPUT;
   slave_regs_clk_sys_o.BACKPLANE_CONF0   <= r_slave.BACKPLANE_CONF0;
   slave_regs_clk_sys_o.BACKPLANE_CONF1   <= r_slave.BACKPLANE_CONF1;
   slave_regs_clk_sys_o.BACKPLANE_CONF2   <= r_slave.BACKPLANE_CONF2;
   slave_regs_clk_sys_o.BACKPLANE_CONF3   <= r_slave.BACKPLANE_CONF3;
   slave_regs_clk_sys_o.BACKPLANE_CONF4   <= r_slave.BACKPLANE_CONF4;
   slave_regs_clk_sys_o.BACKPLANE_CONF5   <= r_slave.BACKPLANE_CONF5;
   slave_regs_clk_sys_o.BACKPLANE_CONF6   <= r_slave.BACKPLANE_CONF6;
   slave_regs_clk_sys_o.BACKPLANE_CONF7   <= r_slave.BACKPLANE_CONF7;
   slave_regs_clk_sys_o.BACKPLANE_CONF8   <= r_slave.BACKPLANE_CONF8;
   slave_regs_clk_sys_o.BACKPLANE_CONF9   <= r_slave.BACKPLANE_CONF9;
   slave_regs_clk_sys_o.BACKPLANE_CONF10  <= r_slave.BACKPLANE_CONF10;
   slave_regs_clk_sys_o.BACKPLANE_CONF11  <= r_slave.BACKPLANE_CONF11;
   slave_regs_clk_sys_o.BACKPLANE_CONF12  <= r_slave.BACKPLANE_CONF12;
   slave_regs_clk_sys_o.BACKPLANE_CONF13  <= r_slave.BACKPLANE_CONF13;
   slave_regs_clk_sys_o.BACKPLANE_CONF14  <= r_slave.BACKPLANE_CONF14;
   slave_regs_clk_sys_o.BACKPLANE_CONF15  <= r_slave.BACKPLANE_CONF15;
   -- slave sys domain in
   s_slave.STALL                          <= slave_regs_clk_sys_i.STALL;
   s_slave.ERR                            <= slave_regs_clk_sys_i.ERR;
   s_slave.HEX_SWITCH                     <= slave_regs_clk_sys_i.HEX_SWITCH;
   s_slave.PUSH_BUTTON                    <= slave_regs_clk_sys_i.PUSH_BUTTON;
   s_slave.HEX_SWITCH_CPLD                <= slave_regs_clk_sys_i.HEX_SWITCH_CPLD;
   s_slave.PUSH_BUTTON_CPLD               <= slave_regs_clk_sys_i.PUSH_BUTTON_CPLD;
   s_slave.LOGIC_INPUT                    <= slave_regs_clk_sys_i.LOGIC_INPUT;
   
   --+******************************************************************************************+
   --| WBS FSM ------------------------------ slave --------------------------------------------|
   --+******************************************************************************************+
   slave : process(clk_sys_i)
      variable v_dat_i  : t_wishbone_data;
      variable v_dat_o  : t_wishbone_data;
      variable v_adr    : natural;
      variable v_page   : natural;
      variable v_sel    : t_wishbone_byte_select;
      variable v_we     : std_logic;
      variable v_en     : std_logic;
   begin
      if rising_edge(clk_sys_i) then
         if(rst_n_i = '0') then
            r_slave.CLOCK_CONTROL_OE   <= (others => '0');
            r_slave.LOGIC_CONTROL_OE   <= (others => '0');
            r_slave.LOGIC_OUTPUT       <= (others => '0');
            r_slave.BACKPLANE_CONF0    <= (others => '0');
            r_slave.BACKPLANE_CONF1    <= (others => '0');
            r_slave.BACKPLANE_CONF2    <= (others => '0');
            r_slave.BACKPLANE_CONF3    <= (others => '0');
            r_slave.BACKPLANE_CONF4    <= (others => '0');
            r_slave.BACKPLANE_CONF5    <= (others => '0');
            r_slave.BACKPLANE_CONF6    <= (others => '0');
            r_slave.BACKPLANE_CONF7    <= (others => '0');
            r_slave.BACKPLANE_CONF8    <= (others => '0');
            r_slave.BACKPLANE_CONF9    <= (others => '0');
            r_slave.BACKPLANE_CONF10   <= (others => '0');
            r_slave.BACKPLANE_CONF11   <= (others => '0');
            r_slave.BACKPLANE_CONF12   <= (others => '0');
            r_slave.BACKPLANE_CONF13   <= (others => '0');
            r_slave.BACKPLANE_CONF14   <= (others => '0');
            r_slave.BACKPLANE_CONF15   <= (others => '0');
            r_slave_out_stall          <= '0';
            r_slave_out_ack0           <= '0';
            r_slave_out_err0           <= '0';
            r_slave_out_dat0           <= (others => '0');
            r_slave_out_ack1           <= '0';
            r_slave_out_err1           <= '0';
            r_slave_out_dat1           <= (others => '0');
         else
            -- short names
            v_dat_i           := slave_i.dat;
            v_adr             := to_integer(unsigned(slave_i.adr(6 downto 2)) & "00");
            v_sel             := slave_i.sel;
            v_en              := slave_i.cyc and slave_i.stb and not (r_slave_out_stall or slave_regs_clk_sys_i.STALL);
            v_we              := slave_i.we;

            --interface outputs
            r_slave_out_stall   <= '0';
            r_slave_out_ack0    <= '0';
            r_slave_out_err0    <= '0';
            r_slave_out_dat0    <= (others => '0');

            r_slave_out_ack1    <= r_slave_out_ack0;
            r_slave_out_err1    <= r_slave_out_err0;
            r_slave_out_dat1    <= r_slave_out_dat0;

            
            if(v_en = '1') then
               r_slave_out_ack0  <= '1';
               if(v_we = '1') then
                  -- WISHBONE WRITE ACTIONS
                  case v_adr is
                     when c_slave_CLOCK_CONTROL_OE_RW    => r_slave.CLOCK_CONTROL_OE   <= f_wb_wr(r_slave.CLOCK_CONTROL_OE,   v_dat_i, v_sel, "owr"); -- External input clock output enable
                     when c_slave_LOGIC_CONTROL_OE_RW    => r_slave.LOGIC_CONTROL_OE   <= f_wb_wr(r_slave.LOGIC_CONTROL_OE,   v_dat_i, v_sel, "owr"); -- External logic analyzer output enable
                     when c_slave_LOGIC_OUTPUT_RW        => r_slave.LOGIC_OUTPUT       <= f_wb_wr(r_slave.LOGIC_OUTPUT,       v_dat_i, v_sel, "owr"); -- External logic analyzer output (write)
                     when c_slave_BACKPLANE_CONF0_RW     => r_slave.BACKPLANE_CONF0    <= f_wb_wr(r_slave.BACKPLANE_CONF0,    v_dat_i, v_sel, "owr"); -- Backplane
                     when c_slave_BACKPLANE_CONF1_RW     => r_slave.BACKPLANE_CONF1    <= f_wb_wr(r_slave.BACKPLANE_CONF1,    v_dat_i, v_sel, "owr"); -- Backplane
                     when c_slave_BACKPLANE_CONF2_RW     => r_slave.BACKPLANE_CONF2    <= f_wb_wr(r_slave.BACKPLANE_CONF2,    v_dat_i, v_sel, "owr"); -- Backplane
                     when c_slave_BACKPLANE_CONF3_RW     => r_slave.BACKPLANE_CONF3    <= f_wb_wr(r_slave.BACKPLANE_CONF3,    v_dat_i, v_sel, "owr"); -- Backplane
                     when c_slave_BACKPLANE_CONF4_RW     => r_slave.BACKPLANE_CONF4    <= f_wb_wr(r_slave.BACKPLANE_CONF4,    v_dat_i, v_sel, "owr"); -- Backplane
                     when c_slave_BACKPLANE_CONF5_RW     => r_slave.BACKPLANE_CONF5    <= f_wb_wr(r_slave.BACKPLANE_CONF5,    v_dat_i, v_sel, "owr"); -- Backplane
                     when c_slave_BACKPLANE_CONF6_RW     => r_slave.BACKPLANE_CONF6    <= f_wb_wr(r_slave.BACKPLANE_CONF6,    v_dat_i, v_sel, "owr"); -- Backplane
                     when c_slave_BACKPLANE_CONF7_RW     => r_slave.BACKPLANE_CONF7    <= f_wb_wr(r_slave.BACKPLANE_CONF7,    v_dat_i, v_sel, "owr"); -- Backplane
                     when c_slave_BACKPLANE_CONF8_RW     => r_slave.BACKPLANE_CONF8    <= f_wb_wr(r_slave.BACKPLANE_CONF8,    v_dat_i, v_sel, "owr"); -- Backplane
                     when c_slave_BACKPLANE_CONF9_RW     => r_slave.BACKPLANE_CONF9    <= f_wb_wr(r_slave.BACKPLANE_CONF9,    v_dat_i, v_sel, "owr"); -- Backplane
                     when c_slave_BACKPLANE_CONF10_RW    => r_slave.BACKPLANE_CONF10   <= f_wb_wr(r_slave.BACKPLANE_CONF10,   v_dat_i, v_sel, "owr"); -- Backplane
                     when c_slave_BACKPLANE_CONF11_RW    => r_slave.BACKPLANE_CONF11   <= f_wb_wr(r_slave.BACKPLANE_CONF11,   v_dat_i, v_sel, "owr"); -- Backplane
                     when c_slave_BACKPLANE_CONF12_RW    => r_slave.BACKPLANE_CONF12   <= f_wb_wr(r_slave.BACKPLANE_CONF12,   v_dat_i, v_sel, "owr"); -- Backplane
                     when c_slave_BACKPLANE_CONF13_RW    => r_slave.BACKPLANE_CONF13   <= f_wb_wr(r_slave.BACKPLANE_CONF13,   v_dat_i, v_sel, "owr"); -- Backplane
                     when c_slave_BACKPLANE_CONF14_RW    => r_slave.BACKPLANE_CONF14   <= f_wb_wr(r_slave.BACKPLANE_CONF14,   v_dat_i, v_sel, "owr"); -- Backplane
                     when c_slave_BACKPLANE_CONF15_RW    => r_slave.BACKPLANE_CONF15   <= f_wb_wr(r_slave.BACKPLANE_CONF15,   v_dat_i, v_sel, "owr"); -- Backplane
                     when others => r_slave_out_ack0 <= '0'; r_slave_out_err0 <= '1';
                  end case;
               else
                  -- WISHBONE READ ACTIONS
                  case v_adr is
                     when c_slave_HEX_SWITCH_GET         => r_slave_out_dat0(3 downto 0)  <= s_slave.HEX_SWITCH;        -- Shows hex switch inputs
                     when c_slave_PUSH_BUTTON_GET        => r_slave_out_dat0(0 downto 0)  <= s_slave.PUSH_BUTTON;       -- Shows status of the push button
                     when c_slave_HEX_SWITCH_CPLD_GET    => r_slave_out_dat0(3 downto 0)  <= s_slave.HEX_SWITCH_CPLD;   -- Shows hex switch inputs (CPLD)
                     when c_slave_PUSH_BUTTON_CPLD_GET   => r_slave_out_dat0(0 downto 0)  <= s_slave.PUSH_BUTTON_CPLD;  -- Shows status of the push button (CPLD)
                     when c_slave_CLOCK_CONTROL_OE_RW    => r_slave_out_dat0(0 downto 0)  <= r_slave.CLOCK_CONTROL_OE;  -- External input clock output enable
                     when c_slave_LOGIC_CONTROL_OE_RW    => r_slave_out_dat0(16 downto 0) <= r_slave.LOGIC_CONTROL_OE;  -- External logic analyzer output enable
                     when c_slave_LOGIC_OUTPUT_RW        => r_slave_out_dat0(16 downto 0) <= r_slave.LOGIC_OUTPUT;      -- External logic analyzer output (write)
                     when c_slave_LOGIC_INPUT_GET        => r_slave_out_dat0(16 downto 0) <= s_slave.LOGIC_INPUT;       -- External logic analyzer input (read)
                     when c_slave_BACKPLANE_CONF0_RW     => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF0;   -- Backplane
                     when c_slave_BACKPLANE_CONF1_RW     => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF1;   -- Backplane
                     when c_slave_BACKPLANE_CONF2_RW     => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF2;   -- Backplane
                     when c_slave_BACKPLANE_CONF3_RW     => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF3;   -- Backplane
                     when c_slave_BACKPLANE_CONF4_RW     => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF4;   -- Backplane
                     when c_slave_BACKPLANE_CONF5_RW     => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF5;   -- Backplane
                     when c_slave_BACKPLANE_CONF6_RW     => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF6;   -- Backplane
                     when c_slave_BACKPLANE_CONF7_RW     => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF7;   -- Backplane
                     when c_slave_BACKPLANE_CONF8_RW     => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF8;   -- Backplane
                     when c_slave_BACKPLANE_CONF9_RW     => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF9;   -- Backplane
                     when c_slave_BACKPLANE_CONF10_RW    => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF10;  -- Backplane
                     when c_slave_BACKPLANE_CONF11_RW    => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF11;  -- Backplane
                     when c_slave_BACKPLANE_CONF12_RW    => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF12;  -- Backplane
                     when c_slave_BACKPLANE_CONF13_RW    => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF13;  -- Backplane
                     when c_slave_BACKPLANE_CONF14_RW    => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF14;  -- Backplane
                     when c_slave_BACKPLANE_CONF15_RW    => r_slave_out_dat0(31 downto 0) <= r_slave.BACKPLANE_CONF15;  -- Backplane
                     when others => r_slave_out_ack0 <= '0'; r_slave_out_err0 <= '1';
                  end case;
               end if; -- v_we
            end if; -- v_en
         end if; -- rst
      end if; -- clk edge
   end process;

   slave_o.stall  <= r_slave_out_stall or slave_regs_clk_sys_i.STALL;
   slave_o.dat    <= r_slave_out_dat1;
   slave_o.ack    <= r_slave_out_ack1 and not slave_regs_clk_sys_i.ERR;
   slave_o.err    <= r_slave_out_err1 or      slave_regs_clk_sys_i.ERR;


end rtl;
