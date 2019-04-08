`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Simple RAM Module
//////////////////////////////////////////////////////////////////////////////////


module ram
#( 
    parameter C_ADDR_WIDTH = 10,
    parameter C_DATA_WIDTH = 32
)
(
    input clk,
    input wen,
    input [C_DATA_WIDTH-1: 0] wdata,
    input [C_ADDR_WIDTH-1: 0] waddr,
    output [C_DATA_WIDTH-1: 0] rdata,
    input  [C_ADDR_WIDTH-1: 0] raddr
    );

    (* ram_style = "block" *) reg [C_DATA_WIDTH-1:0] RAM[(2**C_ADDR_WIDTH)-1:0];

    reg [C_DATA_WIDTH-1:0] regr;
    assign rdata = regr;

    always @(posedge clk) begin
        if (wen) begin
            RAM[waddr] <= wdata;
        end
        else begin
            RAM[waddr] <= RAM[waddr];
        end
    end

    always @(posedge clk) begin
        regr <= RAM[raddr];
    end

endmodule
