------------------------------------------------------------------------------
-- Title      : Current time Clock crossing
-- Project    : FTM
------------------------------------------------------------------------------
-- File       : wb_irq_timer.vhd
-- Author     : Mathias Kreider
-- Company    : GSI
-- Created    : 2013-08-10
-- Last update: 2013-08-10
-- Platform   : FPGA-generic
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: Syncs the current time (ECA format) to another clock domain
-------------------------------------------------------------------------------
-- Copyright (c) 2013 GSI
-------------------------------------------------------------------------------
-- Revisions  :
-- Date        Version  Author          Description
-- 2013-08-10  1.0      mkreider        Created
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.genram_pkg.all;
use work.gencores_pkg.all;


entity time_clk_cross is
generic (g_delay_comp   : natural := 16);
  port    (clk_ref_i    : in std_logic;
           time_ref_i   : in  std_logic_vector(63 downto 0); 

           clk_2_i      : in std_logic;           
           rst_2_n_i    : in std_logic;             
           time_2_o     : out std_logic_vector(63 downto 0) 
  );
end entity;


architecture behavioral of time_clk_cross is


signal r_time_ref_cor : std_logic_vector(64 downto 0);
signal r_time_ref_gray  : std_logic_vector(63 downto 0);
signal r_time_2_bin,    r_time_2_gray    : std_logic_vector(63 downto 0);
signal r_time_2_bin_cor : std_logic_vector(64 downto 0);
signal delta : integer; 

 
begin

   gray_en : process(clk_ref_i)
   begin
      if rising_edge(clk_ref_i) then
        -- TODO: Timing wise, this borderline dangerous ... change to big_adder
        r_time_ref_cor <= f_big_ripple(time_ref_i, std_logic_vector(to_unsigned(g_delay_comp, time_ref_i'length)), '0');  
      
        r_time_ref_gray <= f_gray_encode(r_time_ref_cor(time_ref_i'range)); 
      end if;
   end process gray_en;

   G1: for I in 0 to time_ref_i'length-1 generate
      sync_trig_edge_reg : gc_sync_ffs
      generic map (
        g_sync_edge => "positive")
      port map (
        clk_i    => clk_2_i,
        rst_n_i  => rst_2_n_i,
        data_i   => r_time_ref_gray(I),
        synced_o => r_time_2_gray(i),
        npulse_o => open,
        ppulse_o => open);
   end generate;
    
   gray_de : process(clk_2_i)
   begin
      if rising_edge(clk_2_i) then
        r_time_2_bin <= f_gray_decode(r_time_2_gray, 1);
        end if;
   end process gray_de;    

  time_2_o <= r_time_2_bin; 
  delta <= to_integer(signed(time_ref_i(31 downto 0))) - to_integer(signed(r_time_2_bin(31 downto 0)));

end architecture behavioral;
