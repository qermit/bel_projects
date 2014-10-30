library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.RandomPkg.all;
use work.wb_testsuite_pkg.all;
use work.genram_pkg.all;

entity wb_data_gen is
   generic(
      g_override_rd_bsel   : std_logic_vector(3 downto 0);
      g_freeze_timeout  : natural);
   port(
      
      
      clk_i    : in  std_logic;
      rst_n_i  : in  std_logic;
      
      start_i  : in  std_logic;
      busy_o   : out std_logic;
      done_o   : out std_logic;
      err_o    : out std_logic;
      
      data_i   : t_wb_data_gen_array;
      
      report_o : out std_logic_vector(31 downto 0);
      
      -- Master out
      master_o : out t_wishbone_master_out;
      master_i : in  t_wishbone_master_in := ('0', '0', '0', '0', '0', x"00000000")
      );
end wb_data_gen;

architecture rtl of wb_data_gen is
   
   type t_state is (e_IDLE, e_RUNNING, e_WAITING, e_ERROR, e_FINISH0, e_FINISH1, e_FINISH2, e_FINISH3, e_FINISH4);
   
   signal r_tx_state, r_rx_state : t_state;
   
   signal   s_report : t_wb_dgen_rep;
   signal   s_busy,
            s_done,
            s_err    : std_logic;
            
   signal ref_fifo_push,
          ref_fifo_pop, 
          ref_fifo_empty,
          ref_fifo_full        : std_logic;
   
   signal ref_fifo_q,
          ref_fifo_d           : t_wb_ref; 

   signal rx_fifo_push,
          rx_fifo_pop, 
          rx_fifo_empty,
          rx_fifo_full        : std_logic;
   
   signal rx_fifo_q,
          rx_fifo_d           : t_wb_rx; 


   signal ref_fifo_cnt,
          rx_fifo_cnt         : std_logic_vector(5 downto 0); 

   signal s_tx_en,
          s_ena,
          s_do_cmp,
          s_stall_tmo,
          s_freeze_tmo,
          s_next,
          s_send_now,
          s_rx_tmo,
          s_tx_busy,
          s_tx_done,
          s_rx_done,
          r_cyc : std_logic;
   
   signal r_cyc_cnt,
          r_time,
          r_delay,
          r_stall_tmo,
          s_gtmo_diff,
          s_delay_diff,
          s_rx_tmo_diff,
          s_stall_tmo_diff,
          r_op_cnt : unsigned (31 downto 0);
          
   
   signal s_cmp_val  : std_logic_vector(t_wb_ref_dat'length-1 downto 0);
   signal s_cmp_flag : std_logic_vector(t_wb_ref_exres'length-1 downto 0);
   
   signal s_master_o : t_wishbone_master_out;
   
   signal rep : t_wb_dgen_rep;

begin
    
--+--------------------------------------------------------------------------------+
--| Master Output side: TX FSM, Flowcontrol, Delay, Stall Timeout, Global timeout--|
--+--------------------------------------------------------------------------------+

   
   busy_o   <= s_busy;
   done_o   <= s_done;
   err_o    <= s_err;
   
   s_next   <= s_master_o.cyc and s_master_o.stb and not master_i.stall;
   
   --global freeze timeout. subtract clk cycle count from allowed total count   
   s_gtmo_diff    <= to_unsigned(g_freeze_timeout, r_cyc_cnt'length) - r_cyc_cnt; 
   s_freeze_tmo   <= s_gtmo_diff(s_gtmo_diff'left);
   
   
   -- pause between ops
   -- subtract delay clk cycles from required delay. inc opcount resets delay counter
   -- enabled by wb cyc line
   s_delay_diff   <= to_unsigned(data_i(to_integer(r_op_cnt)).delay, r_delay'length) - r_delay - 1;
   s_send_now     <= s_delay_diff(s_delay_diff'left) and s_tx_en;
   
   -- timeout for stall of an op
   -- subtract number of stalled clk cycles from allowed stalls. not stall resets counter.
   -- enabled by wb cyc line
   s_stall_tmo_diff <= to_unsigned(data_i(to_integer(r_op_cnt)).stall, r_stall_tmo'length) - r_stall_tmo;
   s_stall_tmo          <= s_stall_tmo_diff(s_stall_tmo_diff'left);
   
   cyc_cnt : process(clk_i, rst_n_i)
   begin
      if (rst_n_i = '0') then
          r_op_cnt    <= (others => '0'); -- tx wb op counter
          r_cyc_cnt   <= (others => '0'); -- global clk cycle counter
          r_delay     <= (others => '0'); -- delay before op counter
          r_stall_tmo <= (others => '0'); -- stall timeout counter
      end if;
      if (rising_edge(clk_i)) then
         if(start_i = '1') then
            r_op_cnt    <= (others => '0');
            r_cyc_cnt   <= (others => '0');
            r_stall_tmo <= (others => '0');
            r_delay     <= (others => '0');
          end if;
          
         r_cyc_cnt <=  r_cyc_cnt + 1;
         
         if(s_master_o.cyc = '1') then
            
            r_delay     <= r_delay +1;
            if(master_i.stall = '1') then
               r_stall_tmo <= r_stall_tmo +1;
            else
               r_stall_tmo <= (others => '0');    
            end if;
            
            if(s_next = '1')  then
               if(r_op_cnt < data_i'length -1) then
                  r_op_cnt <= r_op_cnt + 1;  
               end if;   
               r_delay <= (others => '0'); -- reset delay on new op
            end if;
            
         end if;
      end if;
   end process;
   
   
   master_o <= s_master_o;
   
   s_master_o <= f_set_wb_out(data_i(to_integer(r_op_cnt)), r_cyc, s_send_now, g_override_rd_bsel);
   
   s_tx_en <= '1' when r_tx_state = e_RUNNING
         else '0';
   
   tx : process(clk_i, rst_n_i)
      variable i : natural;
   begin
      if (rst_n_i = '0') then
         s_busy   <= '0';
         --s_master_o <= ('0', '0', x"00000000", x"F", '0', x"00000000");
         r_tx_state <= e_IDLE;
         r_delay <= (others => '0');
         r_cyc <= '0';
      end if;
      if (rising_edge(clk_i)) then
         i := to_integer(r_op_cnt);      
         case r_tx_state is
            when e_IDLE    => r_cyc    <= '0';
                              s_busy   <= '0';
                              if(start_i = '1') then
                                 r_tx_state <= e_RUNNING;
                                 r_cyc    <= '1';
                                 s_busy   <= '1';
                              end if;
            when e_RUNNING => -- send the test op
                              -- are we done sending?
                              if(((i = data_i'length -1) and (s_next = '1')) or (s_freeze_tmo = '1')) then
                                 r_tx_state <= e_FINISH0;
                              end if;
            
            when e_FINISH0  => if(s_rx_done = '1') then
                                 r_tx_state <= e_IDLE;
                              end if;-- raise done bit and flags while shifting report out
            when e_ERROR   => r_tx_state <= e_IDLE; 
            when others    => r_tx_state <= e_ERROR;
         end case;    
      end if;
   end process;


--+--------------------------------------------------------------------------------+
--|   Master Input side: Reference FIFO, RX FIFO, Comparator, Report Generator  --|
--+--------------------------------------------------------------------------------+
   


   s_do_cmp <= (not ref_fifo_empty) and (not rx_fifo_empty);
   
     ref_fifo_push   <= s_next; 
     ref_fifo_pop    <= s_do_cmp;
     ref_fifo_d      <= f_set_wb_ref(data_i(to_integer(r_op_cnt)), r_cyc_cnt);
     
     
     
   -- reference fifo
   ref_fifo_in : generic_sync_fifo
   generic map(
      g_data_width             => ref_fifo_d'length,
      g_size                   => 50,
      g_show_ahead             => true,
      g_with_empty             => true,
      g_with_full              => true,
      g_with_almost_full       => false,
      g_almost_full_threshold  => 6)
   port map(
      rst_n_i        => rst_n_i,
      clk_i          => clk_i,
      d_i            => ref_fifo_d,
      we_i           => ref_fifo_push,
      q_o            => ref_fifo_q,
      rd_i           => ref_fifo_pop,
      empty_o        => ref_fifo_empty,
      full_o         => ref_fifo_full,
      count_o        => ref_fifo_cnt
   );
   
   rx_fifo_push   <= master_i.ack or master_i.err;-- or s_rx_tmo; 
   rx_fifo_pop    <= s_do_cmp;
   rx_fifo_d      <= f_set_wb_rx(master_i, r_cyc_cnt);
   
   -- timeout for acknowledgement of an op
   s_rx_tmo_diff     <= unsigned(ref_fifo_q(t_wb_ref_tmo'range)) - unsigned(rx_fifo_q(t_wb_rx_ts'range)) - 1; 
   s_rx_tmo          <= s_rx_tmo_diff(s_rx_tmo_diff'left);
   
   s_cmp_val      <= ref_fifo_q(t_wb_ref_dat'range) xor (ref_fifo_q(t_wb_ref_msk'range) and rx_fifo_q(t_wb_rx_dat'range));
   s_cmp_flag     <= ref_fifo_q(t_wb_ref_exres'range) xor (s_rx_tmo & rx_fifo_q(t_wb_rx_res'range));
   
   
   -- receiver fifo
   rx_fifo_in : generic_sync_fifo
   generic map(
      g_data_width             => rx_fifo_d'length,
      g_size                   => 50,
      g_show_ahead             => true,
      g_with_empty             => true,
      g_with_full              => true,
      g_with_almost_full       => false,
      g_almost_full_threshold  => 6)
   port map(
      rst_n_i        => rst_n_i,
      clk_i          => clk_i,
      d_i            => rx_fifo_d,
      we_i           => rx_fifo_push,
      q_o            => rx_fifo_q,
      rd_i           => rx_fifo_pop,
      empty_o        => rx_fifo_empty,
      full_o         => rx_fifo_full,
      count_o        => rx_fifo_cnt
   );


   
   
   report_generator : process(clk_i, rst_n_i)
   begin
      if (rst_n_i = '0') then
         s_done   <= '0';
         s_err    <= '0';
         r_rx_state <= e_IDLE;
         rep <= c_empty_rep;
      end if;
      if (rising_edge(clk_i)) then
         s_done   <= '0';
         
         case r_rx_state is
            when e_IDLE       => rep <= c_empty_rep;
                                 s_rx_done <= '0';
                                 if(start_i = '1') then
                                    r_rx_state <= e_RUNNING;
                                 end if;
            when e_RUNNING    => -- TODO: TX op counts
                                 rep.duration <= rep.duration +1;
                                 if(ref_fifo_pop = '1') then
                                 rep.total <= rep.total +1;
                              
                                 
                                 if(s_freeze_tmo = '1') then
                                    -- freeze ?
                                    rep.err_tmo <= rep.err_tmo +1;
                                    rep.flags   <= rep.flags or c_ERR_FRZ;
                                    -- note index of first error
                                    if(rep.flags = c_ERR_NONE) then
                                       rep.idx_1st_err   <= rep.total;
                                       rep.type_1st_err  <= c_ERR_FRZ;
                                    end if;
                                 end if;
                                 
                                  if(s_cmp_flag(t_cmp_tmo'range) /= "0") then
                                    -- unexpected timeout
                                    rep.err_tmo <= rep.err_tmo +1;
                                    rep.flags   <= rep.flags or c_ERR_TMO;
                                    -- note index of first error
                                    if(rep.flags = c_ERR_NONE) then
                                       rep.idx_1st_err   <= rep.total;
                                       rep.type_1st_err  <= c_ERR_TMO;
                                       rep.rx_sig        <= (s_rx_tmo & rx_fifo_q(t_wb_rx_res'range));
                                       rep.rx_val        <= (ref_fifo_q(t_wb_ref_msk'range) and rx_fifo_q(t_wb_rx_dat'range));
                                    end if;
                                 end if;   
                                 
                                 
                                 if(s_cmp_flag(t_cmp_err_ack'range) /= "00") then
                                    -- if expected ack/err does not match...
                                    rep.err_res <= rep.err_res +1;
                                    rep.flags   <= rep.flags or c_ERR_RES;
                                    -- note index of first error
                                    if(rep.flags = c_ERR_NONE) then
                                       rep.idx_1st_err   <= rep.total;
                                       rep.type_1st_err  <= c_ERR_RES;
                                       rep.rx_sig        <= (s_rx_tmo & rx_fifo_q(t_wb_rx_res'range));
                                       rep.rx_val        <= (ref_fifo_q(t_wb_ref_msk'range) and rx_fifo_q(t_wb_rx_dat'range));
                                    end if;
                                 end if;
                              
                                 if(s_cmp_val /= std_logic_vector(to_unsigned(0, s_cmp_val'length))) then
                                    -- if expected values do not match, there was a value error
                                    rep.err_val <= rep.err_val +1;
                                    rep.flags   <= rep.flags or c_ERR_VAL;
                                    -- note index of first error
                                    if(rep.flags = c_ERR_NONE) then
                                       rep.idx_1st_err   <= rep.total;
                                       rep.type_1st_err  <= c_ERR_VAL;
                                       rep.rx_sig        <= (s_rx_tmo & rx_fifo_q(t_wb_rx_res'range));
                                       rep.rx_val        <= (ref_fifo_q(t_wb_ref_msk'range) and rx_fifo_q(t_wb_rx_dat'range));
                                    end if;
                                 end if;
                                 
                                 -- if, including current element, these were all, or freeze timeout occurred, we're done
                                 if(rep.total = data_i'length or s_freeze_tmo = '1') then
                                    s_rx_done <= '1';
                                    if(rep.flags = c_ERR_NONE) then
                                       rep.flags <= c_PASS;
                                    end if;
                                    r_rx_state <= e_FINISH0;
                                 end if;
                              end if;
                              
                              -- if these were all, or freeze timeout occurred, we're done
                              if(rep.total = data_i'length or s_freeze_tmo = '1') then
                                 s_rx_done <= '1';
                                 if(rep.flags = c_ERR_NONE) then
                                    rep.flags <= c_PASS;
                                 end if;
                                 r_rx_state <= e_FINISH0;
                              end if;
                              
            when e_FINISH0  =>   
                                 s_done   <= '1';
                                 
                                 
                                 report_o <= std_logic_vector(rep.duration);
                                 r_rx_state <= e_FINISH1; 
            when e_FINISH1  =>   s_done   <= '1';
                                 report_o <= rep.flags & std_logic_vector(rep.total);
                                 r_rx_state <= e_FINISH2;
            when e_FINISH2  =>   s_done   <= '1';
                                 report_o <= std_logic_vector(rep.err_res & rep.err_val);
                                 r_rx_state <= e_FINISH3;
            when e_FINISH3  =>   s_done   <= '1';
                                 report_o <= std_logic_vector(rep.err_stall & rep.err_tmo);
                                 r_rx_state <= e_FINISH4;
            when e_FINISH4  =>   s_done   <= '1';
                                 report_o <= rep.type_1st_err & std_logic_vector(rep.idx_1st_err);
                                 r_rx_state <= e_IDLE;
                                 if(rep.flags = c_PASS) then
                                    report lf & lf & "##############################" & lf & lf & "#  +++ Test PASSED +++" & lf & lf & 
                                    "#  Elapsed Time      : " & integer'image(to_integer(rep.duration)) & lf &
                                    "#  WB Operations     : " & integer'image(to_integer(rep.total)) & lf & 
                                    "#  Flags             : " & f_bits2string(rep.flags) & lf & 
                                    "##############################"  & lf & lf severity failure;
                                    
                                 else
                                   report lf & lf & 
                                    "##############################" & lf & lf & 
                                    "#  +++ !!!Test FAILED!!! +++" & lf & lf & 
                                    "#  Elapsed Time      : " & integer'image(to_integer(rep.duration)) & lf &
                                    "#  WB Operations     : " & integer'image(to_integer(rep.total)) & lf & 
                                    "#  Flags             : " & f_bits2string(rep.flags) & lf &
                                    "#  bad ERR/ACK       : " & integer'image(to_integer(rep.err_res)) & lf & 
                                    "#  bad readback val  : " & integer'image(to_integer(rep.err_val)) & lf & 
                                    "#  stalled too long  : " & integer'image(to_integer(rep.err_stall)) & lf & 
                                    "#  reply timed out   : " & integer'image(to_integer(rep.err_tmo)) & lf & 
                                    "#  Idx  of 1st error : " & integer'image(to_integer(rep.idx_1st_err)) & lf &
                                    "#  Type of 1st error : " & f_bits2string(rep.type_1st_err) & lf & lf &
                                    "##############################" & lf & lf &
                                    "#  Failed Op @ " & integer'image(to_integer(rep.idx_1st_err)) & " ***" & lf &
                                    "#     We               : " & f_bits2string((0 => data_i(to_integer(rep.idx_1st_err)).we)) & lf &
                                    "#     Address          : " & f_bits2string(data_i(to_integer(rep.idx_1st_err)).adr) & lf &
                                    "#     Data             : " & f_bits2string(data_i(to_integer(rep.idx_1st_err)).dat) & lf &
                                    "#     Mask             : " & f_bits2string(data_i(to_integer(rep.idx_1st_err)).msk) & lf &
                                    "#     Delay            : " & integer'image(data_i(to_integer(rep.idx_1st_err)).delay) & lf &
                                    "#     Stall            : " & integer'image(data_i(to_integer(rep.idx_1st_err)).stall) & lf &
                                    "#     Timeout          : " & integer'image(data_i(to_integer(rep.idx_1st_err)).tmo) & lf & lf &
                                    "#     Exp. Readback    : " & f_bits2string(data_i(to_integer(rep.idx_1st_err)).dat and data_i(to_integer(rep.idx_1st_err)).msk) & lf & 
                                    "#     Rec. Readback    : " & f_bits2string(rep.rx_val) & lf & lf &
                                    "#     Exp. TMO/ERR/ACK : " & f_bits2string(data_i(to_integer(rep.idx_1st_err)).exres) & lf & 
                                    "#     Rec. TMO/ERR/ACK : " & f_bits2string(rep.rx_sig) & lf & lf &
                                    "##############################"  & lf & lf severity failure;
                                 end if;
            
            when e_ERROR   => r_rx_state <= e_IDLE; 
            when others    => r_rx_state <= e_ERROR;
         end case;    
      end if;
   end process;
   
end;
   






