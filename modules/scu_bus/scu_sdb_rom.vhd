-------------------------------------------------------------------------------
-- Title      : Scalable Control Unit Bus Interface -- WB version
-- Project    : SCU
-------------------------------------------------------------------------------
-- File       : scu_sdb_rom.vhd
-- Author     : Stefan Rauch and Wesley Terpstra
-- Company    : GSI
-- Created    : 2015-02-23
-- Last update: 2015-02-23
-- Platform   : FPGA-generics
-- Standard   : VHDL '93
-------------------------------------------------------------------------------
-- Description:
--
-- Master Bus Interface for the SCU Bus
-------------------------------------------------------------------------------
-- Revisions  :
-- Date        Version  Author          Description
-- 2015-02-24  1.0      -               Created
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.wishbone_pkg.all;
use work.scu_bus_pkg.all;

entity scu_sdb_rom is
  port(
    clk_i        : in  std_logic;
    s_presence_i : in  std_logic_vector(11 downto 0);
    s_adr_i      : in  std_logic_vector( 9 downto 0);
    s_dat_o      : out std_logic_vector(31 downto 0));
end scu_sdb_rom;

architecture rtl of scu_sdb_rom is

  constant c_wb_slave_scu : t_sdb_bridge := (
    sdb_child     => x"0000000001fff000",
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"0000000001ffffff",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"29088393",
    version       => x"00000001",
    date          => x"20150224",
    name          => "WB-SCU-SlaveBridge ")));
  
  constant c_used_entries   : natural := 12 + 1;
  constant c_rom_entries    : natural := 2**f_ceil_log2(c_used_entries); -- next power of 2
  constant c_sdb_words      : natural := c_sdb_device_length / c_wishbone_data_width;
  constant c_rom_words      : natural := c_rom_entries * c_sdb_words;
  constant c_rom_depth      : natural := f_ceil_log2(c_rom_words);
  constant c_rom_lowbits    : natural := f_ceil_log2(c_wishbone_data_width / 8);
  
  type t_rom is array(c_rom_words-1 downto 0) of t_wishbone_data;
  
  function f_build_rom
    return t_rom
  is
    variable res : t_rom := (others => (others => '0'));
    variable sdb_device : std_logic_vector(c_sdb_device_length-1 downto 0) := (others => '0');
    variable sdb_component : t_sdb_component;
    variable slave_address : unsigned(31 downto 0);
  begin
    sdb_device(511 downto 480) := x"5344422D"  ;                                     -- sdb_magic
    sdb_device(479 downto 464) := std_logic_vector(to_unsigned(c_used_entries, 16)); -- sdb_records
    sdb_device(463 downto 456) := x"01";                                             -- sdb_version
    sdb_device(455 downto 448) := x"00";                                             -- sdb_bus_type = sdb wishbone
    sdb_device(  7 downto   0) := x"00";                                             -- record_type  = sdb_interconnect
    
    sdb_component.addr_first := (others => '0');
    sdb_component.addr_last  := c_wb_master_scu.sdb_component.addr_last;
    sdb_component.product.vendor_id := x"0000000000000651"; -- GSI
    sdb_component.product.device_id := x"fec0aa08";
    sdb_component.product.version   := x"00000001";
    sdb_component.product.date      := x"20150224";
    sdb_component.product.name      := "WB-SCU-MasterCross ";
    sdb_device(447 downto   8) := f_sdb_embed_component(sdb_component, (others => '0'));
    
    for i in 0 to c_sdb_words-1 loop
      res(c_sdb_words-1-i) :=
        sdb_device((i+1)*c_wishbone_data_width-1 downto i*c_wishbone_data_width);
    end loop;
    
    for slave in 1 to c_used_entries-1 loop
      -- "02000000" * (slave-1)
      slave_address := (others => '0');
      slave_address(31 downto 24) := to_unsigned((slave-1)*2, 8);
      
      sdb_device(511 downto 0) := 
        f_sdb_embed_bridge(c_wb_slave_scu, std_logic_vector(slave_address));
        
      for i in 0 to c_sdb_words-1 loop
        res((slave+1)*c_sdb_words-1-i) :=
          sdb_device((i+1)*c_wishbone_data_width-1 downto i*c_wishbone_data_width);
      end loop;
    end loop;
    
    return res;
  end f_build_rom;

  signal rom : t_rom := f_build_rom;
  signal r_adr : unsigned(7 downto 0);
  
  signal s_presence : std_logic_vector(15 downto 0);
  
begin

  -- What records to show
  s_presence(0) <= '1';
  s_presence(12 downto  1) <=  s_presence_i;
  s_presence(15 downto 13) <= (others => '0');

  main : process(clk_i) is
  begin
    if rising_edge(clk_i) then
      r_adr <= unsigned(s_adr_i(9 downto 2));
      -- If the slave is not present, fill SDB entry with 1s => record_type=0xff=empty
      if s_presence(to_integer(r_adr(7 downto 4))) = '1' then
        s_dat_o <= rom(to_integer(r_adr));
      else
        s_dat_o <= (others => '1');
      end if;
    end if;
  end process;

end rtl;
