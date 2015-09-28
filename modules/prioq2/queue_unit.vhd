library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;


entity queue_unit is
generic(
  g_depth     : natural := 16;
  g_words     : natural := 8
);
port(
  clk_i       : in  std_logic;
  rst_n_i     : in  std_logic;
  
  slave_i     : in t_wishbone_slave_in;
  slave_o     : out t_wishbone_slave_out;
  
  master_o    : out t_wishbone_master_out;
  master_i    : in t_wishbone_master_in;
  full_o      : out std_logic;
  ts_o        : out std_logic_vector(63 downto 0);
  ts_valid_o  : out std_logic

);
end entity;

architecture behavioral of queue_unit is

  
  signal  r_ts_hi, s_q1, s_q0 : t_wishbone_data;
  signal  s_master_o : t_wishbone_master_out;
  signal  s_slave_o : t_wishbone_slave_out;
  
  signal  s_pop0, s_pop1, s_pop,
          s_push0, s_push1, s_push,
          s_full0, s_full1, s_full,
          s_empty0, s_empty1, s_empty,
          r_toggle,
          s_cnt_done,
          r_sel,
          s_ts_valid, r_ts_valid,
          s_ackwait,
          s_reloadwait,
          r_cyc  : std_logic;
  
  signal r_cnt,
         r_ack_cnt,
         r_reload_cnt : unsigned(3 downto 0);


begin

  
  
  ts_low : generic_sync_fifo
  generic map (
    g_data_width    => 32, 
    g_size          => g_depth/2,
    g_show_ahead    => true,
    g_with_empty    => true,
    g_with_full     => true)
  port map (
    rst_n_i        => rst_n_i,         
    clk_i          => clk_i,
    d_i            => slave_i.dat,
    we_i           => s_push0,
    q_o            => s_q0,
    rd_i           => s_pop0,
    empty_o        => s_empty0,
    full_o         => s_full0,
    almost_empty_o => open,
    almost_full_o  => open,
    count_o        => open);

 ts_high : generic_sync_fifo
  generic map (
    g_data_width    => 32, 
    g_size          => g_depth/2,
    g_show_ahead    => true,
    g_with_empty    => true,
    g_with_full     => true)
  port map (
    rst_n_i        => rst_n_i,         
    clk_i          => clk_i,
    d_i            => slave_i.dat,
    we_i           => s_push1,
    q_o            => s_q1,
    rd_i           => s_pop1,
    empty_o        => s_empty1,
    full_o         => s_full1,
    almost_empty_o => open,
    almost_full_o  => open,
    count_o        => open);

  full_o <= s_full0;

  master_o        <= s_master_o;

  s_master_o.we   <= '1';
  s_master_o.dat  <= s_q1 when r_cnt(r_cnt'low) = '1'
                else s_q0;
  s_master_o.cyc  <= (not s_cnt_done and r_sel) or s_ackwait or s_reloadwait; -- only raise cycle if our queue was selected by ts comparator
  s_master_o.stb  <= not s_cnt_done and r_sel and not s_empty; -- only strobe if our queue was selected by ts comparator
  s_master_o.sel  <= x"f";

  s_pop           <= s_master_o.cyc and s_master_o.stb and not master_i.stall;
  s_pop0          <= s_pop  and not r_cnt(r_cnt'low); --
  s_pop1          <= s_pop  and     r_cnt(r_cnt'low);
  s_empty         <= (s_empty0 and not r_cnt(r_cnt'low)) or (s_empty1 and r_cnt(r_cnt'low));

  s_slave_o.stall <= s_full; --output flow control
  s_slave_o.err   <= '0';
  s_slave_o.dat   <= (others => '0');

  slave_o         <= s_slave_o;

  s_push          <= slave_i.cyc and slave_i.stb and not s_slave_o.stall; 
  s_push0         <= s_push and not r_toggle;
  s_push1         <= s_push and     r_toggle;
  s_full          <= (s_full0 and not r_toggle)  or (s_full1 and r_toggle) ;
  

  -- ts valid when first and second ts buffer are filled and we either finished a message or want to send one
  s_ts_valid      <= '1' when (r_cnt = g_words-1 or s_cnt_done = '1') and s_empty0 = '0' and s_empty1 = '0' 
                else '0';

  ts_valid_o      <= s_ts_valid; -- and not r_ts_valid;       

  ts_o            <= s_q1 & s_q0 ;






  reg : process(clk_i)
  begin
    if(rising_edge(clk_i)) then
      
      if(rst_n_i = '0') then
        s_slave_o.ack <= '0';
        r_ts_valid    <= '0';
        r_cyc         <= '0';
        r_toggle      <= '1';
      else
        s_slave_o.ack <= s_push;
        r_ts_valid    <= s_ts_valid;
        r_cyc         <= s_master_o.cyc;

        r_toggle      <= r_toggle xor s_push; 
      end if;  
    end if; 
  end process;

  s_reloadwait <=  not r_reload_cnt(r_reload_cnt'high);

  reloadwait : process(clk_i)
  begin
    if(rising_edge(clk_i)) then
      if(rst_n_i = '0') then
        r_reload_cnt <= (others => '1');
      else
        if (s_ackwait = '1') then
          r_reload_cnt <= to_unsigned(2, r_reload_cnt'length);
        elsif(r_reload_cnt(r_reload_cnt'high) = '0') then 
          r_reload_cnt <= r_reload_cnt-1;
        end if;
      end if;
    end if; 
  end process;

  s_ackwait <= not r_ack_cnt(r_ack_cnt'high);

  ackcnt : process(clk_i)
  begin
    if(rising_edge(clk_i)) then
      if(rst_n_i = '0') then
        r_ack_cnt <= (others => '1');
      else
        if (s_master_o.cyc = '1' and r_cyc = '0') then
          r_ack_cnt <= to_unsigned(g_words-1, r_ack_cnt'length);
        elsif((master_i.ack = '1' or master_i.err = '1') and r_ack_cnt(r_ack_cnt'high) = '0') then 
          r_ack_cnt <= r_ack_cnt-1;
        else
          r_ack_cnt <= r_ack_cnt;
        end if;
      end if;
    end if; 
  end process;

  s_cnt_done <= r_cnt(r_cnt'high);

  cnt : process(clk_i)
  begin
    if(rising_edge(clk_i)) then
      if(rst_n_i = '0' or (s_cnt_done = '1' and s_empty1 = '0' and r_cyc = '0')) then
        r_cnt <= to_unsigned(g_words-1, r_cnt'length);
        r_sel <= '0';
      else
        r_sel <= r_sel or s_ts_valid;
        if(s_pop = '1') then 
          r_cnt <= r_cnt-1;
        end if;
      end if;
    end if; 
  end process;


end behavioral;

