-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;

-- package declaration
package wb_mux1xn_pkg is
  component wb_mux1xn
    generic (
      g_address_size   : natural   := 32;   -- in bit(s)
      g_data_size      : natural   := 32;   -- in bit(s)
      g_input_size     : natural   := 2;    -- in bit(s)
      g_clocked_mux    : boolean   := true; -- clocked multiplexer y/n
      g_default_output : std_logic := 'Z'   -- default output
    );
    port (
      clk_sys_i   : in  std_logic;
      rst_n_i     : in  std_logic;
      -- wishbone slave interface
      slave_i     : in  t_wishbone_slave_in;
      slave_o     : out t_wishbone_slave_out;
      -- multiplexed signals
      signal_rx_i : in  std_logic;
      signal_rx_o : out std_logic_vector((g_input_size-1) downto 0);
      signal_tx_o : out std_logic;
      signal_tx_i : in  std_logic_vector((g_input_size-1) downto 0)
  );    
  end component;

  constant c_wb_mux1xn_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"4", -- 32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000000000f",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"C07F1680",
    version       => x"00000001",
    date          => x"20140730",
    name          => "CONFIGURABLE_MUX1XN")));
  
end wb_mux1xn_pkg;
