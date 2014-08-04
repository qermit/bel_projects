-- libraries and packages
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;

-- submodules
library work;
use work.wb_mux1xn_pkg.all;

-- entity
entity wbmux1xn_test_bench is
end wbmux1xn_test_bench;

-- architecture
architecture rtl of wbmux1xn_test_bench is

  -- generic test signals
  signal s_tb_clk_system     : std_logic;
  signal s_tb_rst_n_system   : std_logic;
  
  -- wishbone test signals
  signal s_tb_slave_addr_out : std_logic_vector(31 downto 0);
  signal s_tb_slave_data_out : std_logic_vector(31 downto 0);  
  signal s_tb_slave_we_out   : std_logic;
  signal s_tb_slave_cyc_out  : std_logic;
  signal s_tb_slave_stb_out  : std_logic;
  signal s_tb_slave_data_in  : std_logic_vector(31 downto 0);
  signal s_tb_slave_stall_in : std_logic;
  signal s_tb_slave_ack_in   : std_logic;
  signal s_tb_slave_err_in   : std_logic;
  signal s_tb_slave_rty_in   : std_logic;
  signal s_tb_slave_int_in   : std_logic;
  
  -- multiplex signals
  signal s_tb_mux_tx_in      : std_logic_vector(1 downto 0);
  signal s_tb_mux_tx_out     : std_logic;
  signal s_tb_mux_tx_out_lt  : std_logic_vector(1 downto 0);
  signal s_tb_mux_rx_in      : std_logic;
  signal s_tb_mux_rx_out     : std_logic_vector(1 downto 0);
  signal s_tb_mux_rx_out_lt  : std_logic;
  
  -- signals for test cases
  signal s_tb_cyc_counter       : std_logic_vector(31 downto 0);
  
  -- test bench settings
  constant c_system_clock_cycle : time := 16 ns;  -- i.e.: 16 => 62.5Mhz
  constant c_system_reset_delay : time := 100 ns; -- reset duration at test bench start

begin

  -- instantiate the driver
  wb_mux1xn : entity work.wb_mux1xn
  port map (
    clk_sys_i     => s_tb_clk_system,
    rst_n_i       => s_tb_rst_n_system,
    slave_i.adr   => s_tb_slave_addr_out,
    slave_i.dat   => s_tb_slave_data_out, 
    slave_i.we    => s_tb_slave_we_out,
    slave_i.cyc   => s_tb_slave_cyc_out,
    slave_i.stb   => s_tb_slave_stb_out,
    slave_i.sel   => (others => 'Z'),
    slave_o.dat   => s_tb_slave_data_in,
    slave_o.stall => s_tb_slave_stall_in,
    slave_o.ack   => s_tb_slave_ack_in,
    slave_o.err   => s_tb_slave_err_in,
    slave_o.rty   => s_tb_slave_rty_in,
    slave_o.int   => s_tb_slave_int_in,
    signal_rx_i   => s_tb_mux_rx_in,
    signal_rx_o   => s_tb_mux_rx_out,
    signal_tx_o   => s_tb_mux_tx_out,
    signal_tx_i   => s_tb_mux_tx_in
  );   
  
  -- system clock
  p_clock : process
  begin
    s_tb_clk_system <= '0';
    wait for (c_system_clock_cycle/2);
    s_tb_clk_system <= '1';
    wait for (c_system_clock_cycle/2); 
  end process;

  -- system reset
  p_reset : process
  begin
    s_tb_rst_n_system <= '0';
    wait for c_system_reset_delay;
    s_tb_rst_n_system <= '1';
    wait;       
  end process;

  -- system counter/time
  p_time : process (s_tb_clk_system, s_tb_rst_n_system)
  begin
    if (s_tb_rst_n_system = '0') then
      s_tb_cyc_counter <= (others => '0');
    elsif (rising_edge(s_tb_clk_system)) then
      s_tb_cyc_counter <= std_logic_vector(unsigned(s_tb_cyc_counter)+1);
    end if;
  end process;
  
  -- provide test signals and latched values for comparison at p_wb_stimulate_device
  p_latch_test_signals : process (s_tb_clk_system, s_tb_rst_n_system)
  begin
    s_tb_mux_tx_in(0)     <= s_tb_cyc_counter(0);
    s_tb_mux_tx_in(1)     <= s_tb_cyc_counter(1);
    s_tb_mux_rx_in        <= s_tb_cyc_counter(2);
    s_tb_mux_tx_out_lt(0) <= s_tb_mux_tx_in(0);
    s_tb_mux_tx_out_lt(1) <= s_tb_mux_tx_in(1);
    s_tb_mux_rx_out_lt    <= s_tb_mux_rx_in;
  end process;
  
   -- state machine for test case selection
  p_test_case_selection : process (s_tb_clk_system, s_tb_rst_n_system) 
  begin
    if (s_tb_rst_n_system = '0') then
      null;
    elsif (rising_edge(s_tb_clk_system)) then
      if (unsigned(s_tb_cyc_counter) < 8000) then
        null;
      else
        report "Simulation completed successfully!" severity failure;
      end if;
    end if;  
  end process;
  
  -- test cases for simple stimulation
  p_wb_stimulate_device : process (s_tb_clk_system, s_tb_rst_n_system)
  begin
    if (s_tb_rst_n_system = '0') then
      s_tb_slave_addr_out <= (others => 'Z');
      s_tb_slave_data_out <= (others => 'Z');
      s_tb_slave_we_out   <= '0';
      s_tb_slave_cyc_out  <= '0';
      s_tb_slave_stb_out  <= '0';
    elsif (rising_edge(s_tb_clk_system)) then
      case s_tb_cyc_counter is
      
        --------------------------------------------------------------------------------
        -- single pipeline write (invalid multiplex option)
        when x"0000000a" =>
          s_tb_slave_addr_out <= x"00000000";
          s_tb_slave_data_out <= x"00000011";
          s_tb_slave_we_out   <= '1';
          s_tb_slave_cyc_out  <= '1';
          s_tb_slave_stb_out  <= '1';
        when x"0000000b" =>
          s_tb_slave_we_out   <= '0';
          s_tb_slave_stb_out  <= '0';
        when x"0000000c" =>
          if (s_tb_slave_ack_in='1') then
            s_tb_slave_addr_out <= (others => 'Z');
            s_tb_slave_data_out <= (others => 'Z');
            s_tb_slave_cyc_out  <= '0';
          else
            report "Missing ack!" severity failure;	
          end if;
        when x"0000000d" =>
          -- check TX
          if (s_tb_mux_tx_out/='Z') then
            report "TX Multiplexer should output be ..." severity failure;
          end if;
          -- check RX
          if (s_tb_mux_rx_out(0)/='Z' and s_tb_mux_rx_out(1)/='Z') then
            report "RX Multiplexer should output be ..." severity failure;
          end if;
         
        --------------------------------------------------------------------------------
        -- single pipeline write (use s_tb_mux_input(0))
        when x"0000001a" =>
          s_tb_slave_addr_out <= x"00000000";
          s_tb_slave_data_out <= x"00000000";
          s_tb_slave_we_out   <= '1';
          s_tb_slave_cyc_out  <= '1';
          s_tb_slave_stb_out  <= '1';
        when x"0000001b" =>
          s_tb_slave_we_out   <= '0';
          s_tb_slave_stb_out  <= '0';
        when x"0000001c" =>
          if (s_tb_slave_ack_in='1') then
            s_tb_slave_addr_out <= (others => 'Z');
            s_tb_slave_data_out <= (others => 'Z');
            s_tb_slave_cyc_out  <= '0';
          else
            report "Missing ack!" severity failure;	
          end if;
        when x"0000001d" =>
          -- check TX
          if (s_tb_mux_tx_out/=s_tb_mux_tx_out_lt(0)) then
            report "TX Multiplexer should output 'Z'" severity failure;
          end if;
          -- check RX
          if (s_tb_mux_rx_out(0)/=s_tb_mux_rx_out_lt and s_tb_mux_rx_out(1)/='Z') then
            report "RX Multiplexer should output 'Z'" severity failure;
          end if;
          
        --------------------------------------------------------------------------------
        -- single pipeline write (use s_tb_mux_input(1))
        when x"0000002a" =>
          s_tb_slave_addr_out <= x"00000000";
          s_tb_slave_data_out <= x"00000001";
          s_tb_slave_we_out   <= '1';
          s_tb_slave_cyc_out  <= '1';
          s_tb_slave_stb_out  <= '1';
        when x"0000002b" =>
          s_tb_slave_we_out   <= '0';
          s_tb_slave_stb_out  <= '0';
        when x"0000002c" =>
          if (s_tb_slave_ack_in='1') then
            s_tb_slave_addr_out <= (others => 'Z');
            s_tb_slave_data_out <= (others => 'Z');
            s_tb_slave_cyc_out  <= '0';
          else
            report "Missing ack!" severity failure;	
          end if;
        when x"0000002d" =>
          -- check TX
          if (s_tb_mux_tx_out/=s_tb_mux_tx_out_lt(1)) then
            report "TX Multiplexer should output 'Z'" severity failure;
          end if;
          -- check RX
          if (s_tb_mux_rx_out(0)/='Z' and s_tb_mux_rx_out(1)/=s_tb_mux_rx_out_lt) then
            report "RX Multiplexer should output 'Z'" severity failure;
          end if;
          
        --------------------------------------------------------------------------------
        -- single pipeline write (invalid multiplex option)
        when x"0000003a" =>
          s_tb_slave_addr_out <= x"00000000";
          s_tb_slave_data_out <= x"00000003";
          s_tb_slave_we_out   <= '1';
          s_tb_slave_cyc_out  <= '1';
          s_tb_slave_stb_out  <= '1';
        when x"0000003b" =>
          s_tb_slave_we_out   <= '0';
          s_tb_slave_stb_out  <= '0';
        when x"0000003c" =>
          if (s_tb_slave_ack_in='1') then
            s_tb_slave_addr_out <= (others => 'Z');
            s_tb_slave_data_out <= (others => 'Z');
            s_tb_slave_cyc_out  <= '0';
          else
            report "Missing ack!" severity failure;	
          end if;
        when x"0000003d" =>
          -- check TX
          if (s_tb_mux_tx_out/='Z') then
            report "TX Multiplexer should output 'Z'" severity failure;
          end if;
          -- check RX
          if (s_tb_mux_rx_out(0)/='Z' and s_tb_mux_rx_out(1)/='Z') then
            report "RX Multiplexer should output 'Z'" severity failure;
          end if;
          
        --------------------------------------------------------------------------------
        -- single pipeline write (use s_tb_mux_input(0))
        when x"00000040" =>
          s_tb_slave_addr_out <= x"00000000";
          s_tb_slave_data_out <= x"00000000";
          s_tb_slave_we_out   <= '1';
          s_tb_slave_cyc_out  <= '1';
          s_tb_slave_stb_out  <= '1';
        when x"00000041" =>
          s_tb_slave_we_out   <= '0';
          s_tb_slave_stb_out  <= '0';
        when x"00000042" =>
          if (s_tb_slave_ack_in='1') then
            s_tb_slave_addr_out <= (others => 'Z');
            s_tb_slave_data_out <= (others => 'Z');
            s_tb_slave_cyc_out  <= '0';
          else
            report "Missing ack!" severity failure;	
          end if;
        when x"00000043" =>
          -- check TX
          if (s_tb_mux_tx_out/=s_tb_mux_tx_out_lt(0)) then
            report "TX Multiplexer should output be ..." severity failure;
          end if;
          -- check RX
          if (s_tb_mux_rx_out(0)/=s_tb_mux_rx_out_lt and s_tb_mux_rx_out(1)/='Z') then
            report "RX Multiplexer should output be ..." severity failure;
          end if;
          
        --------------------------------------------------------------------------------
        -- single pipeline write (use s_tb_mux_input(1))
        when x"00000045" =>
          s_tb_slave_addr_out <= x"00000000";
          s_tb_slave_data_out <= x"00000001";
          s_tb_slave_we_out   <= '1';
          s_tb_slave_cyc_out  <= '1';
          s_tb_slave_stb_out  <= '1';
        when x"00000046" =>
          s_tb_slave_we_out   <= '0';
          s_tb_slave_stb_out  <= '0';
        when x"00000047" =>
          if (s_tb_slave_ack_in='1') then
            s_tb_slave_addr_out <= (others => 'Z');
            s_tb_slave_data_out <= (others => 'Z');
            s_tb_slave_cyc_out  <= '0';
          else
            report "Missing ack!" severity failure;	
          end if;
        when x"00000048" =>
          -- check TX
          if (s_tb_mux_tx_out/=s_tb_mux_tx_out_lt(0)) then
            report "TX Multiplexer should output 'Z'" severity failure;
          end if;
          -- check RX
          if (s_tb_mux_rx_out(0)/='Z' and s_tb_mux_rx_out(1)/=s_tb_mux_rx_out_lt) then
            report "RX Multiplexer should output 'Z'" severity failure;
          end if;
          
        --------------------------------------------------------------------------------
        -- reset bus signals
        when others =>
          s_tb_slave_addr_out <= (others => 'Z');
          s_tb_slave_data_out <= (others => 'Z');
          s_tb_slave_we_out   <= '0';
          s_tb_slave_cyc_out  <= '0';
          s_tb_slave_stb_out  <= '0';
          
        end case;
    end if;
  end process;
  
end rtl;

