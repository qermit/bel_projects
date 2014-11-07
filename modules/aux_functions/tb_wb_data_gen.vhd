library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.RandomPkg.all;
use work.wb_testsuite_pkg.all;
use work.tlu_pkg.all;

entity tb_wb_data_gen is
end tb_wb_data_gen;

architecture rtl of tb_wb_data_gen is 

constant clk_period     : time      := 8 ns;
constant c_elements     : natural := 24;
constant c_seed         : natural := 12345;
constant c_max_stall    : natural := 3;
constant c_prob_stall   : natural := 50;
constant c_stall_weight : integer_vector( 0 to 1) := ( 100 - c_prob_stall, c_prob_stall );

constant c_num_triggers : natural := 3; 

signal s_clk_i    : std_logic;
signal s_rst_n_i  : std_logic;
signal s_start_i  : std_logic;
signal s_busy_o   : std_logic;
signal s_done_o   : std_logic;
signal s_err_o    : std_logic;
signal s_report_o : std_logic_vector(31 downto 0);
signal s_master_o : t_wishbone_master_out;
signal s_master_i : t_wishbone_master_in := ('0', '0', '0', '0', '0', x"00000000");

signal r_cnt_stall    : natural;
signal r_time : std_logic_vector(63 downto 0);

signal s_triggers : t_trigger_array(c_num_triggers-1 downto 0);

signal rangetest0 : std_logic_vector(15 downto 8);
signal rangetest1 : std_logic_vector(7 downto 0);

--+###############################################################################################+
--|                                DUT Register Layout                                            |
--+-----------------------------------------------------------------------------------------------+
   --wb registers
   constant c_STAT      : natural := 0;               --0x00, ro, fifo n..0 status (0 empty, 1 ne)
   constant c_CLR       : natural := c_STAT     +4;   --0x04, wo, Clear channels n..0
   constant c_TEST      : natural := c_CLR      +4;   --0x08, ro, trigger n..0 status
   constant c_ACT_GET   : natural := c_TEST     +4;   --0x0C, ro, trigger n..0 status
   constant c_ACT_SET   : natural := c_ACT_GET  +4;   --0x10, wo, Activate trigger n..0
   constant c_ACT_CLR   : natural := c_ACT_SET  +4;   --0x14, wo, deactivate trigger n..0
   constant c_EDG_GET   : natural := c_ACT_CLR  +4;   --0x18, ro, trigger n..0 latch edge (1 pos, 0 neg)
   constant c_EDG_POS   : natural := c_EDG_GET  +4;   --0x1C, wo, latch trigger n..0 pos
   constant c_EDG_NEG   : natural := c_EDG_POS  +4;   --0x20, wo, latch trigger n..0 neg
   constant c_IE        : natural := c_EDG_NEG  +4;   --0x24, rw, Global IRQ enable
   constant c_MSK_GET   : natural := c_IE       +4;   --0x28, ro, IRQ channels mask
   constant c_MSK_SET   : natural := c_MSK_GET  +4;   --0x2C, wo, set IRQ channels mask n..0
   constant c_MSK_CLR   : natural := c_MSK_SET  +4;   --0x30, wo, clr IRQ channels mask n..0
   constant c_CH_NUM    : natural := c_MSK_CLR  +4;   --0x34, ro, number channels present              
   constant c_CH_DEPTH  : natural := c_CH_NUM   +4;   --0x38, ro, channels depth
   --reserved
   constant c_TC_HI     : natural := 16#50#;          --0x50, ro, Current time (Cycle Count) Hi. read Hi, then Lo 
   constant c_TC_LO     : natural := c_TC_HI    +4;   --0x54, ro, Current time (Cycle Count) Lo
   constant c_CH_SEL    : natural := c_TC_LO    +4;   --0x58, rw, channels select  
-- ***** CAREFUL! From here on, all addresses depend on channels select Reg !
   constant c_TS_POP    : natural := c_CH_SEL   +4;   --0x5C, wo  writing anything here will pop selected channel
   constant c_TS_TEST   : natural := c_TS_POP   +4;   --0x60, wo, Test selected channel
   constant c_TS_CNT    : natural := c_TS_TEST  +4;   --0x64, ro, fifo fill count
   constant c_TS_HI     : natural := c_TS_CNT   +4;   --0x68, ro, fifo q - Cycle Count Hi
   constant c_TS_LO     : natural := c_TS_HI    +4;   --0x6C, ro, fifo q - Cycle Count Lo  
   constant c_TS_SUB    : natural := c_TS_LO    +4;   --0x70, ro, fifo q - Sub cycle word
   constant c_STABLE    : natural := c_TS_SUB   +4;   --0x70, rw, stable time in ns, how long a signal has to be constant before edges are detected
   constant c_TS_MSG    : natural := c_STABLE   +4;   --0x74, rw, MSI msg to be sent 
   constant c_TS_DST_ADR: natural := c_TS_MSG   +4;   --0x78, rw, MSI adr to send to

--+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  end DUT Register Layout  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+


--+###############################################################################################+
--|                                Start test case                                                |
--+-----------------------------------------------------------------------------------------------+
-- Example test case creation. A test case should be stored in one or more constants
--+-----------------------------------------------------------------------------------------------+
--

-- !!! impartant data types and conversion functions !!!
--+-----------------------------------------------------------------------------------------------+

-- t_slv32        -- 32 bit standard logic vector (slv)
-- t_slv32_array  -- array of 32 bit slv
 
-- type t_wb_data_gen is record
-----WB operation parameters
--    we      : std_logic;                     -- Read or Write Operation
--    adr     : t_slv32;                       -- Target Address
--    dat     : t_slv32;                       -- Write Data / Expected Read back Value
--    msk     : t_slv32;                       -- bitmask, applied on 'dat' field before write/ read back comparison.
--                                             -- Used to generate select lines for Writes. Read select lines can be overridden by g_override_rd_bsel
--    delay   : natural range 255 downto 0;    -- delay before operation is strobed
--    
---- validation criteria. not matching a criterium will increase the error counter
--
--    stall   : natural range 255 downto 0;    -- maximum tolerated stall before err count is incresed
--    tmo     : natural range 65535 downto 0;  -- timeout for acknowledgement of operation
--    exres   : std_logic_vector(2 downto 0);  -- expected result of operation. Can be one or more of ACK, ERR or timeout
--
--end record t_wb_data_gen;
 
-- function to_slv32_vector(a : integer_vector) return t_slv32_array;
-- function to_slv32(a : natural) return t_slv32;


-- !!! impartant constants !!! 
--+-----------------------------------------------------------------------------------------------+

-- c_NUL -- 32 bit slv, no bits set
-- c_ALL -- 32 bit slv, all bits set

-- c_WR -- WB Write Operation
-- c_RD -- WB Read Operation

--expected result constants
-- c_ACK -- expect ACK
-- c_ERR -- expect ERR
-- c_TMO -- expect Timeout
-- c_XDC -- expect ACK or ERR or Timeout

--integer version of above for integervector creation
-- c_iACK, c_iERR, c_iTMO, c_iXDC


-- handcrafted control register test case
--+-----------------------------------------------------------------------------------------------+  
   constant c_dat0 : t_wb_data_gen_array := (
    --WE     ADR                 VAL            MSK            Delay Stall Timeout  Result  
   (c_WR,   to_slv32(c_ACT_SET), to_slv32(2),   c_ALL,         0,    5,    5,       c_ACK), --0
   (c_WR,   to_slv32(c_ACT_GET), c_ALL,         c_NUL,         0,    5,    5,       c_ERR or c_TMO),
   (c_RD,   to_slv32(c_ACT_GET), x"00000002",   c_ALL,         3,    5,    5,       c_ACK), --2
   (c_WR,   to_slv32(c_ACT_SET), to_slv32(4),   c_ALL,         0,    5,    5,       c_ACK),  
   (c_RD,   to_slv32(c_ACT_GET), x"00000006",   c_ALL,         3,    5,    5,       c_ACK), --4
   (c_RD,   to_slv32(c_ACT_GET), x"00000002",   c_ALL,         3,    5,    5,       c_ACK), 
   (c_WR,   to_slv32(c_STABLE),  to_slv32(4),   c_ALL,         0,    5,    5,       c_ACK), --6 
   (c_WR,   to_slv32(c_EDG_POS), c_ALL,         c_ALL,         0,    5,    5,       c_ACK), 
   (c_WR,   to_slv32(c_CH_SEL),  to_slv32(1),   c_ALL,         0,    5,    5,       c_ACK), --8 
   (c_RD,   to_slv32(c_TS_CNT),  c_NUL,         c_ALL,         10,   5,    5,       c_ACK), 
   (c_WR,   to_slv32(c_TS_TEST), c_ALL,         c_ALL,         0,    5,    5,       c_ACK), --10
   (c_RD,   to_slv32(c_TS_CNT),  to_slv32(1),   c_ALL,         20,   5,    5,       c_ACK),  
   (c_WR,   to_slv32(c_TS_TEST), c_ALL,         c_ALL,         0,    5,    5,       c_ACK), --12 
   (c_RD,   to_slv32(c_TS_CNT),  to_slv32(2),   c_ALL,         20,   5,    5,       c_ACK)
   );


-- auto generated address range vector
--+-----------------------------------------------------------------------------------------------+   
   --function f_lin_space( first : natural;
   --                      last  : natural;
   --                      step  : natural;
   --                      qty   : natural := 0)
   --return integer_vector;
   
   -- f_lin_space returns a number of values spaced between <first> and <last> 
   -- with a stepsize of <step>. If <Qty> is not supplied, length is calculated
   -- automatically. If it is given and less then the natural qty, the output is
   -- truncated to <qty>, if it is greater, output will be padded with the last value.
   -- 
   -- <step> is unsigned. In order to get a descending sequence, make <first> greater than <last>
   
   constant c_dat1_aux0 : integer_vector := f_lin_space(16#4C#, 16#3C#, 4);

-- auto generated random data vector
--+-----------------------------------------------------------------------------------------------+   
   --impure function f_rnd_bound(lower_bound : integer;
   --                            upper_Bound : integer;
   --                            qty         : natural )
   --return integer_vector;
   
   -- f_rnd_bound returns <qty> uniformly distributed random integer numbers bewteen
   -- <lower_bound> and <upper_Bound>.
   
   constant c_dat1_aux1 : integer_vector := f_rnd_bound(0, 1024, c_dat1_aux0'length);


-- auto completed test cases
--+-----------------------------------------------------------------------------------------------+   
   --function f_new_partial_dataset ( we : std_logic_vector   := (0 => '0');
   --                              adr : t_slv32_array     := (0 => c_NUL);
   --                              dat : t_slv32_array     := (0 => C_NUL);
   --                              msk : t_slv32_array     := (0 => c_NUL);
   --                              delay : integer_vector  := (0 => 0);
   --                              stall : integer_vector  := (0 => 0);
   --                              tmo   : integer_vector  := (0 => 65535);
   --                              exres : integer_vector  := (0 => c_iACK);
   --                              qty   : natural := 0)
   --return t_wb_data_gen_array;
   
   -- f_new_partial_dataset is an easy way of creating a test case dataset by only supplying the
   -- most important parameters.
   --
   -- Output array will be the size of the parameter with biggest supplied dimension.
   -- all shorter parameters will be padded to that length using their last value.
   -- This means that for all elements in a column to be the same, you only supply 1 value.
   --
   -- Keep in mind that you must use array/vector notation even for single elements and (others=> x)
   -- does not work because the arrays are not constrained yet.
   
   
   constant c_dat1 : t_wb_data_gen_array := (
      f_new_partial_dataset (
         we  => (0 => c_WR),
         adr => to_slv32_vector(c_dat1_aux0),
         dat => to_slv32_vector(c_dat1_aux1),
         msk => (0 => c_ALL),
         exres => (0 => c_iERR)
      ) 
   );

   constant c_dat2 : t_wb_data_gen_array := (
      f_new_partial_dataset (
         qty => 10,
         we  => (0 => c_RD),
         adr => (0 => to_slv32(c_TS_CNT)),
         msk => (0 => x"0000ffff"),
         dat => (0 => to_slv32(2)),
         delay => (0 => 00)
      )      
   );  

-- putting it all together
--+-----------------------------------------------------------------------------------------------+
-- t_wb_data_gen_array's can be concatenated like strings with the "&" operator 

   constant c_data : t_wb_data_gen_array := c_dat0 & c_dat1 & c_dat2;
   
--+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  end test case  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+

       
begin

--+###############################################################################################+
--|                                WB Data Generator Testbench                                    |
--+-----------------------------------------------------------------------------------------------+

   dgen :  wb_data_gen
   generic map(g_override_rd_bsel => x"f",
               g_freeze_timeout  => 5000)
   port map(clk_i    => s_clk_i,
            rst_n_i  => s_rst_n_i,
      
            start_i  => s_start_i,
            busy_o   => s_busy_o,
            done_o   => s_done_o,
            err_o    => s_err_o,
            data_i   => c_data,        --   <--- insert your test case here !!!
            report_o => s_report_o,
            master_o => s_master_o,
            master_i => s_master_i
      );

   clk_process :process
   begin
        s_clk_i <= '0';
        wait for clk_period/2;  
        s_clk_i <= '1';
        wait for clk_period/2;
   end process;

   rst: process
   begin        
      s_rst_n_i  <= '0';
      wait until rising_edge(s_clk_i);
      wait for clk_period*5;
      s_rst_n_i <= '1';
      wait for clk_period*10;
      wait until s_rst_n_i = '0';
   end process;

   time_gen :process(s_clk_i)
   begin
      if(rising_edge(s_clk_i)) then
         if(s_rst_n_i = '0') then
            r_time <= (others => '0' );
         else
            r_time <= std_logic_vector(unsigned(r_time) +1);
         end if;
      end if;  
   end process;
   
--+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  end WB Data Generator Testbench ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
   
   
--+###############################################################################################+
--|                                DUT instanciation                                              |
--+-----------------------------------------------------------------------------------------------+
   
   DUT : wr_tlu 
   generic map(g_num_triggers => c_num_triggers,
               g_fifo_depth   => 64,
               g_auto_msg     => false) 
   port map(
      clk_ref_i         => s_clk_i,    -- tranceiver clock domain
      rst_ref_n_i       => s_rst_n_i,
      clk_sys_i         => s_clk_i,    -- local clock domain
      rst_sys_n_i       => s_rst_n_i,
      triggers_i        => s_triggers, -- trigger vectors for latch. meant to be used with 8bits from lvds derserializer for 1GhZ res
                                       -- if you only have a normal signal, connect it as triggers_i(m) => (others => s_my_signal)
      tm_tai_cyc_i      => r_time,     -- TAI Timestamp in 8ns cycles  

      ctrl_slave_i      => s_master_o, -- Wishbone slave interface (sys_clk domain)
      ctrl_slave_o      => s_master_i,
      
      irq_master_o      => open,           -- msi irq src 
      irq_master_i      => ('0', '0', '0', '0', '0', x"00000000")
   ); 
--+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  end DUT instanciation ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+


--+###############################################################################################+
--|                                Stimuli                                                        |
--+-----------------------------------------------------------------------------------------------+
   
-- s_master_i.stall    <= '1' when r_cnt_stall > 0
--              else '0'; 
   
   
-- rnd_stall : process(s_clk_i, s_rst_n_i)
-- begin
--    if (s_rst_n_i = '0') then
--       r_cnt_stall <= 0;
--    end if;
--      if (rising_edge(s_clk_i)) then
--      if r_cnt_stall = 0 then
--         r_cnt_stall <= RV.DistInt( c_stall_weight ) * RV.RandInt(1, c_max_stall);
--      else
--         r_cnt_stall <= r_cnt_stall - 1;
--      end if;
--      end if;
-- end process;
   
   rangetest1 <= rangetest0(rangetest0'range - 8); 
   
   stimulus: process
      variable i, j : natural;
   begin        
        
        s_start_i <= '0';
        RV.InitSeed(c_seed);
        s_triggers <= (others => (others => '0'));                     
        
        wait until s_rst_n_i = '1';
        wait until rising_edge(s_clk_i);
        
        wait for clk_period*5; 
        
        report "+++++++++++++++ +++++++++++++++ +++++++++++++ Starting Test" severity warning;
        
        s_start_i <= '1';
        wait for clk_period;
        s_start_i <= '0'; 
         
        wait until s_rst_n_i = '0';
  end process;
--+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  end Stimuli ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+


end architecture;
