library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;


entity arbiter is
generic(
  g_depth     : natural := 16;
  g_slaves    : natural := 8 
);
port(
  clk_i       : in  std_logic;
  rst_n_i     : in  std_logic;
  
  slave_i     : in t_wishbone_slave_in_array(g_slaves-1 downto 0);
  slave_o     : out t_wishbone_slave_out_array(g_slaves-1 downto 0);
  
  master_o    : out t_wishbone_master_out;
  master_i    : in t_wishbone_master_in;
  
  ts_valid_o  : out std_logic_vector(g_slaves-1 downto 0)
  
);
begin
assert (g_slaves >= 1 or g_slaves <= 9) report "Timing Arbiter must have 1-9 slave ports" severity failure;
assert (g_words >= 2) report "Arbiter qeue depth must be >= 2" severity failure;
end entity;

architecture behavioral of arbiter is
begin
end architecture;
