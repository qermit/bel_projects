module altera_wrapper_pwr (
  clk,
  rst_n,
  signal_in,
  pulse_out
);

  input  wire clk;
  input  wire rst_n;
  input  wire signal_in;
  output wire pulse_out;

  parameter PULSE_EXT = 2;
  parameter EDGE_TYPE = 1;
  parameter IGNORE_RST_WHILE_BUSY = 1;

  altera_edge_detector pulse_warm_reset (
    .clk       (clk),
    .rst_n     (rst_n),
    .signal_in (signal_in),
    .pulse_out (pulse_out)
  );

endmodule