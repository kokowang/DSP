`timescale 1ns/10ps

module fir_poly #
    ( 
        parameter integer AXIS_TDATA_WIDTH  = 16, 
        parameter integer BUFFER_LENGTH     = 4,
        parameter integer COEF_WIDTH        = 16,
        parameter integer ORDER             = 3,
        parameter integer PIECES            = 2,
        parameter integer OUTPUT_WIDTH      = 16
    )
    (
        input                                                aclk,
        input                                                aresetn,
		input   [AXIS_TDATA_WIDTH                   - 1 : 0] s_axis_tdata,
		input                                                s_axis_tvalid,
        input   [32 * (ORDER + 1) * PIECES          - 1 : 0] coef_flat,     //int32 array of coefficients, ORDER first.
        input   [16 * (PIECES - 1)                  - 1 : 0] delay_flat,    //int16 array of delays
		output                                               m_axis_tvalid,
		output  [OUTPUT_WIDTH                       - 1 : 0] m_axis_tdata
    );
    wire signed [COEF_WIDTH        - 1 : 0] coef        [PIECES - 1 : 0][ORDER : 0];
    wire signed [BUFFER_LENGTH     - 1 : 1] delay       [PIECES - 1 : 0];
  	wire signed [AXIS_TDATA_WIDTH  - 1 : 0] delay_out   [PIECES - 1 : 0];
    reg         [BUFFER_LENGTH     - 1 : 0] wptr = 0;
    reg  signed [AXIS_TDATA_WIDTH + (ORDER + 1) * BUFFER_LENGTH + COEF_WIDTH + $clog2(PIECES) - 1 : 0] sum [PIECES - 1 : 1];
    
    assign delay_out[0] = s_axis_tdata;
    assign m_axis_tvalid = s_axis_tvalid;
    assign m_axis_tdata = sum[PIECES-1];
    
    genvar i;
    genvar j;

    //Reformatting to arrays
    generate
        for (i = 0; i < PIECES; i = i + 1) begin
            for (j = 0; j < (ORDER + 1); j = j + 1) begin
                assign coef[i][j] = coef_flat[32 * (i * (ORDER+1) + j) + COEF_WIDTH - 1 : 32 * (i * (ORDER+1) + j)];
            end
        end
    endgenerate

    generate 
        for (i = 0; i < PIECES-1; i = i + 1) begin
            assign delay[i+1] = delay_flat[16 * i + BUFFER_LENGTH - 1: 16 * i];
        end
    endgenerate


    //Chain buffer  
    generate
        for (i = 1; i < PIECES; i = i + 1) begin : chain_buffer
            bram # (BUFFER_LENGTH,AXIS_TDATA_WIDTH)
                ram (
                    .clk    (aclk),
                    .ena    (1'b1), 
                    .enb    (1'b1), 
                    .wea    (1'b1), 
                    .addra  (wptr), 
                    .addrb  (wptr - delay[i] + 1'b1), 
                  	.dia    (delay_out[i - 1]), 
                    .dob    (delay_out[i])
                );
        end
    endgenerate

    //MACC_SUM
    generate
        for (i = 0; i < PIECES; i = i + 1) begin : piece
            for (j = 0; j < ORDER + 1; j = j + 1) begin : order
                wire [AXIS_TDATA_WIDTH + j * BUFFER_LENGTH                  - 1 : 0] acc_out;
                wire [AXIS_TDATA_WIDTH + j * BUFFER_LENGTH  + COEF_WIDTH    - 1 : 0] macc_out;
                if (j == 0) begin
                    reg  [AXIS_TDATA_WIDTH + COEF_WIDTH -1 : 0] r_macc_out;
                    assign acc_out = delay_out[i];
                    assign macc_out = r_macc_out;
                    always @ (posedge aclk) 
                        if (~aresetn)
                            r_macc_out <= 0;
                        else
                            r_macc_out <= coef[i][0] * delay_out[i];
                end
            end
        end
    endgenerate

    generate
        for (i = 0; i < PIECES; i = i + 1) begin : macc_piece
            for (j = 1; j < ORDER + 1; j = j + 1) begin : macc_order
                macc_sum # (
                    .I_ACC_WIDTH    (AXIS_TDATA_WIDTH + (j - 1) * BUFFER_LENGTH),
                    .O_ACC_WIDTH    (AXIS_TDATA_WIDTH + j * BUFFER_LENGTH),
                    .I_MACC_WIDTH   (AXIS_TDATA_WIDTH + (j - 1) * BUFFER_LENGTH + COEF_WIDTH),
                    .COEF_WIDTH     (COEF_WIDTH)
                )   mac (
                        .clk        (aclk),
                        .aresetn    (aresetn),
                        .coef       (coef[i][j]),
                        .acc_in     (piece[i].order[j - 1].acc_out),
                        .macc_in    (piece[i].order[j - 1].macc_out),
                        .acc_out    (piece[i].order[j].acc_out),
                        .macc_out   (piece[i].order[j].macc_out)
                    );
            end
        end
    endgenerate


    always @ (posedge aclk) begin
        wptr <= wptr + 1'b1;
    end
    generate
        for (i = 1; i < PIECES; i = i + 1) begin 
            always @ (posedge aclk) begin
                if (~aresetn) begin
                    sum[i] <= 0;
                end
                else begin
                    if (i == 1) 
                        sum[i] = piece[i].order[ORDER].macc_out + piece[i-1].order[ORDER].macc_out;
                    else
                        sum[i] = sum[i-1] + piece[i].order[ORDER].macc_out;
                end
            end
        end
    endgenerate


endmodule


