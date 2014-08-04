onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider System
add wave -noupdate /wbmux1xn_test_bench/s_tb_clk_system
add wave -noupdate /wbmux1xn_test_bench/s_tb_rst_n_system
add wave -noupdate -radix hexadecimal /wbmux1xn_test_bench/s_tb_cyc_counter
add wave -noupdate -divider Wishbone
add wave -noupdate -radix hexadecimal /wbmux1xn_test_bench/s_tb_slave_addr_out
add wave -noupdate -radix hexadecimal /wbmux1xn_test_bench/s_tb_slave_data_out
add wave -noupdate /wbmux1xn_test_bench/s_tb_slave_we_out
add wave -noupdate /wbmux1xn_test_bench/s_tb_slave_cyc_out
add wave -noupdate /wbmux1xn_test_bench/s_tb_slave_stb_out
add wave -noupdate -radix hexadecimal /wbmux1xn_test_bench/s_tb_slave_data_in
add wave -noupdate /wbmux1xn_test_bench/s_tb_slave_stall_in
add wave -noupdate /wbmux1xn_test_bench/s_tb_slave_ack_in
add wave -noupdate /wbmux1xn_test_bench/s_tb_slave_err_in
add wave -noupdate /wbmux1xn_test_bench/s_tb_slave_rty_in
add wave -noupdate /wbmux1xn_test_bench/s_tb_slave_int_in
add wave -noupdate -divider Inputs/Outputs
add wave -noupdate /wbmux1xn_test_bench/s_tb_mux_tx_in(1)
add wave -noupdate /wbmux1xn_test_bench/s_tb_mux_tx_in(0)
add wave -noupdate -color Cyan /wbmux1xn_test_bench/s_tb_mux_tx_out_lt(1)
add wave -noupdate -color Cyan /wbmux1xn_test_bench/s_tb_mux_tx_out_lt(0)
add wave -noupdate -color Cyan /wbmux1xn_test_bench/s_tb_mux_tx_out
add wave -noupdate /wbmux1xn_test_bench/s_tb_mux_rx_in
add wave -noupdate -color Gold /wbmux1xn_test_bench/s_tb_mux_rx_out_lt
add wave -noupdate -color Gold /wbmux1xn_test_bench/s_tb_mux_rx_out(1)
add wave -noupdate -color Gold /wbmux1xn_test_bench/s_tb_mux_rx_out(0)
add wave -noupdate -divider Internal
add wave -noupdate -radix hexadecimal -childformat {{/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(31) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(30) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(29) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(28) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(27) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(26) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(25) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(24) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(23) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(22) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(21) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(20) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(19) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(18) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(17) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(16) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(15) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(14) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(13) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(12) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(11) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(10) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(9) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(8) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(7) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(6) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(5) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(4) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(3) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(2) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(1) -radix hexadecimal} {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(0) -radix hexadecimal}} -subitemconfig {/wbmux1xn_test_bench/wb_mux1xn/s_control_reg(31) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(30) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(29) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(28) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(27) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(26) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(25) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(24) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(23) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(22) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(21) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(20) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(19) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(18) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(17) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(16) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(15) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(14) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(13) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(12) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(11) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(10) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(9) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(8) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(7) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(6) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(5) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(4) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(3) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(2) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(1) {-height 13 -radix hexadecimal} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg(0) {-height 13 -radix hexadecimal}} /wbmux1xn_test_bench/wb_mux1xn/s_control_reg
add wave -noupdate /wbmux1xn_test_bench/wb_mux1xn/signal_rx_i
add wave -noupdate -expand /wbmux1xn_test_bench/wb_mux1xn/signal_rx_o
add wave -noupdate /wbmux1xn_test_bench/wb_mux1xn/signal_tx_o
add wave -noupdate -expand /wbmux1xn_test_bench/wb_mux1xn/signal_tx_i
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {1344035 ps} 0}
configure wave -namecolwidth 257
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 1
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ps
update
WaveRestoreZoom {553122 ps} {6143790 ps}
