library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity bm_sync is
  port (
    clk:        in std_logic;
    ena_100ns:  in std_logic;
    n_ena:      in std_logic;
    n_str:      in std_logic;
    sclr:       in std_logic;
    n_ena_o:    out std_logic;
    n_str_o:    out std_logic);
end entity;

architecture bm_sync_arch of bm_sync is
  component debounce_ifa
    generic
      (
      cnt:     integer := 4 );
    port
      (
       sig:     in std_logic;
       sel:     in std_logic;
       cnt_en:  in std_logic;
       clk:     in std_logic;
       res:     in std_logic;
       sig_deb: out std_logic);
   end component;
   signal s_str_o: std_logic;
   signal s_ena_o: std_logic;
begin
  ena_deb: debounce_ifa
    generic map (cnt => 4)
    port map (  sig => not n_ena,
                sel => '0',
                cnt_en => ena_100ns,
                clk => clk,
                res => sclr,
                sig_deb => s_ena_o);
    
  str_deb: debounce_ifa
    generic map (cnt => 4)
    port map (  sig => not n_str,
                sel => '0',
                cnt_en => ena_100ns,
                clk => clk,
                res => sclr,
                sig_deb => s_str_o);

    n_ena_o <= not s_ena_o;
    n_str_o <= not s_str_o;
                
end architecture;
  