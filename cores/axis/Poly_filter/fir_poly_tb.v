`timescale 1ns/1ps
module tb();
  reg clk;
  reg rst;
  reg [16 - 1 : 0] data;
  reg [32 * (3 + 1) * 2 - 1 : 0] coef;
  reg [16 * (2 - 1) - 1 : 0] delay;
  wire [16 - 1 : 0] out;
  wire valid;
  integer cnt = 0;
  fir_poly //# (16, 4, 16, 3, 2, 16)
  fir (
    .aclk(clk),
    .aresetn(rst),
    .s_axis_tdata(data),
    .s_axis_tvalid(1'b1),
    .coef_flat(coef),
    .delay_flat(delay),
    .m_axis_tvalid(valid),
    .m_axis_tdata(out)
  );
    
  always begin
    #1 clk = ~clk;
  end
  always @ (posedge clk) begin
    cnt = cnt + 1;
    $display("cnt=%d  rst=%d  data=%d  out=%d", cnt, rst, data, $signed(out));
  end
  initial begin
    clk = 0;
    #1
    rst = 0;
    data = 0;
    coef = {-32'd0,-32'd0,-32'd0,-32'd0,
             32'h100, 32'd16, 32'd1, 32'd0};
    delay = 4'd5;
    #40
    rst = 1;
    #4
    data = 1;
    #2
    data = 0;
  	#90 $finish;
  end
    
endmodule