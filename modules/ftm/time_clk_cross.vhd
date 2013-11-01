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
-- Description: Syncs the current (ECA format) to another domain
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
use work.eca_pkg.all;


entity time_clk_cross is
  port    (clk_ref_i    : in std_logic;
           time_ref_i   : in  t_time; 

           clk_2_i      : in std_logic;           
           rst_2_n_i    : in std_logic;             
           time_2_o     : out std_logic
  );
end entity;


architecture behavioral of time_clk_cross is

constant c_lag : unsigned(t_time'length-1 downto 0) := 4;


signal r_time_ref_bin,  r_time_ref_gray  : t_time;
signal r_time_2_bin,    r_time_2_gray    : t_time;


  component gc_sync_ffs
    generic (
      g_sync_edge : string);
    port (
      clk_i    : in  std_logic;
      rst_n_i  : in  std_logic;
      data_i   : in  std_logic;
      synced_o : out std_logic;
      npulse_o : out std_logic;
      ppulse_o : out std_logic);
  end component;

begin

   gray_en : process(clk_ref_i)
   begin
      if rising_edge(clk_ref_i) then
        r_time_ref_gray <= f_eca_gray_encode(time_ref_i); 
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
        r_time_2_bin <= f_eca_gray_decode(r_time_2_gray, 1);
        time_2_o <= std_logic_vector(unsigned(r_time_2_bin) + c_lag); 
      end if;
   end process gray_de;    

   

end architecture behavioral;
