-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;

-- entity
entity wb_mux1xn is
  generic (
    g_address_size   : natural   := 32;   -- in bit(s)
    g_data_size      : natural   := 32;   -- in bit(s)
    g_input_size     : natural   := 2;    -- in bit(s)
    g_clocked_mux    : boolean   := true; -- clocked multiplexer y/n
    g_default_output : std_logic := 'Z'   -- default output
  );
  port (
    -- generic system interface
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
end wb_mux1xn;

-- architecture
architecture rtl of wb_mux1xn is

  -- wb signals
  signal s_ack                   : std_logic;

  -- registers
  signal s_control_reg           : std_logic_vector (g_data_size-1 downto 0); -- Configuration register   [W/R]
  
  -- register mapping
  constant c_address_control_reg : std_logic_vector (1 downto 0):= "00"; 
  
  -- constant wishbone bus error (register is not readable or writable, ...)
  constant c_wb_bus_read_error   : std_logic_vector (g_data_size-1 downto 0):= x"DEADBEEF";
  
begin
  
  -- process wishbone acknowlegde
  p_wb_ack : process(s_ack)
  begin
    slave_o.ack <= s_ack;
  end process;
   
  -- process handle wishbone requests
  p_wb_handle_requests : process(clk_sys_i, rst_n_i)
  begin
  -- reset detection
    if (rst_n_i = '0') then
      s_ack         <= '0';
      slave_o.stall <= '0';
      slave_o.int   <= '0';
      slave_o.err   <= '0';
      slave_o.rty   <= '0';
      slave_o.dat   <= (others => '0');
      s_control_reg <= (others => '0');
      -- process with normal flow 
    elsif (rising_edge(clk_sys_i)) then
      -- generate ack and others wishbone signals
      s_ack         <= slave_i.cyc and slave_i.stb;
      slave_o.stall <= '0';
      slave_o.int   <= '0';
      slave_o.err   <= '0';
      slave_o.rty   <= '0';
      -- check if a request is incoming   
      if (slave_i.stb='1' and slave_i.cyc='1') then
        -- evaluate address and write enable signals
        case slave_i.adr(3 downto 2) is
          -- handle requests for tx data register
          when c_address_control_reg =>
            if (slave_i.we='1') then
              s_control_reg <= slave_i.dat;
            end if;
            slave_o.dat     <= s_control_reg; -- return configuration register
          -- unknown access
          when others =>
            slave_o.dat     <= c_wb_bus_read_error; -- this is no write or read address
        end case; -- end address based switching
      -- no cycle or strobe
      else
        slave_o.dat         <= (others => '0');
      end if; -- check for cycle and strobe
    end if; -- check reset
  end process;
  
  --  multiplexing without clock -- multiplex signals depending on s_control_reg 
  no_clocked_mux_n : if not g_clocked_mux generate
  p_mux_signals : process(signal_rx_i, signal_tx_i)
  begin
  -- plausibility check
    if (g_input_size > to_integer(unsigned(s_control_reg(g_data_size-1 downto 0)))) then
      signal_rx_o                                                              <= (others => g_default_output);
      signal_rx_o(to_integer(unsigned(s_control_reg(g_data_size-1 downto 0)))) <= signal_rx_i;
      signal_tx_o                                                              <= signal_tx_i(to_integer(unsigned(s_control_reg(g_data_size-1 downto 0))));
    else
      signal_rx_o <= (others => g_default_output);
      signal_tx_o <= g_default_output;
    end if; -- plausibility check
  end process;
  end generate no_clocked_mux_n;
  
  -- multiplexing with clock -- multiplex signals depending on s_control_reg 
  clocked_mux_y : if g_clocked_mux generate
  p_mux_signals : process(clk_sys_i, rst_n_i)
  begin
  -- reset detection
    if (rst_n_i = '0') then
      signal_rx_o <= (others => g_default_output);
      signal_tx_o <= g_default_output;
    -- process with normal flow 
    elsif (rising_edge(clk_sys_i)) then
      -- plausibility check
      if (g_input_size > to_integer(unsigned(s_control_reg(g_data_size-1 downto 0)))) then
        signal_rx_o                                                              <= (others => g_default_output);
        signal_rx_o(to_integer(unsigned(s_control_reg(g_data_size-1 downto 0)))) <= signal_rx_i;
        signal_tx_o                                                              <= signal_tx_i(to_integer(unsigned(s_control_reg(g_data_size-1 downto 0))));
      else
        signal_rx_o <= (others => g_default_output);
        signal_tx_o <= g_default_output;
      end if; -- plausibility check
    end if; -- check reset
  end process;
  end generate clocked_mux_y;
  
end rtl;
