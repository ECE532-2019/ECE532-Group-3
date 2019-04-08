`timescale 1ns / 1ns
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/28/2019 01:34:29 AM
// Design Name: 
// Module Name: centroid_top_level
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module centroid_top_level
#(
    parameter SOURCE_WIDTH = 640,
    parameter SOURCE_HEIGHT = 480,
    parameter SOURCE_DATA_WIDTH = 1,
    parameter SOURCE_ADDR_WIDTH = 19
)
(
    input clk,
    input resetn,
    input go,
    input [SOURCE_DATA_WIDTH-1:0] source_data,
    output [SOURCE_ADDR_WIDTH-1:0] source_addr,
    output reg [SOURCE_ADDR_WIDTH-1:0] x,
    output reg [SOURCE_ADDR_WIDTH-1:0] y
);

    //Control Variables
    reg reducer_1_go;
    reg reducer_2_go;
    reg reducer_3_go;
    reg done;

    //Reduction Level 1 Data Path
    localparam REDUCER_1_OUTPUT_ADDR_WIDTH = SOURCE_ADDR_WIDTH - 2;
    localparam REDUCER_1_OUTPUT_DATA_WIDTH = SOURCE_DATA_WIDTH;
    
    wire reducer_1_result_write_enable;
    wire [REDUCER_1_OUTPUT_DATA_WIDTH-1:0] reducer_1_result_data;
    wire [REDUCER_1_OUTPUT_ADDR_WIDTH-1:0] reducer_1_result_addr;
    wire reducer_1_done;
    wire [REDUCER_1_OUTPUT_DATA_WIDTH-1:0] reducer_1_source_data;
    wire [REDUCER_1_OUTPUT_ADDR_WIDTH-1:0] reducer_1_source_addr;

    ram #(.C_ADDR_WIDTH(REDUCER_1_OUTPUT_ADDR_WIDTH),
          .C_DATA_WIDTH(REDUCER_1_OUTPUT_DATA_WIDTH))
                     reducer_1_result(
                         .clk(clk),
                         .wen(reducer_1_result_write_enable),
                         .wdata(reducer_1_result_data),
                         .waddr(reducer_1_result_addr),
                         .rdata(reducer_1_source_data),
                         .raddr(reducer_1_source_addr)
                     );

    reduction #(.SOURCE_WIDTH(SOURCE_WIDTH), 
                .SOURCE_HEIGHT(SOURCE_HEIGHT),
                .SOURCE_DATA_WIDTH(SOURCE_DATA_WIDTH),
                .SOURCE_ADDR_WIDTH(SOURCE_ADDR_WIDTH),
                .DEST_ADDR_WIDTH(REDUCER_1_OUTPUT_ADDR_WIDTH),
                .DEST_DATA_WIDTH(REDUCER_1_OUTPUT_DATA_WIDTH))
                reducer_1(
                    .clk(clk),
                    .resetn(resetn),
                    .go(reducer_1_go),
                    .source_data(source_data),
                    .source_addr(source_addr),
                    .dest_addr(reducer_1_result_addr),
                    .dest_data(reducer_1_result_data),
                    .dest_write_enable(reducer_1_result_write_enable),
                    .done(reducer_1_done));

    //Reduction Level 2 Data Path
    localparam REDUCER_2_OUTPUT_ADDR_WIDTH = REDUCER_1_OUTPUT_ADDR_WIDTH - 2;
    localparam REDUCER_2_OUTPUT_DATA_WIDTH = REDUCER_1_OUTPUT_DATA_WIDTH;

    wire reducer_2_result_write_enable;
    wire [REDUCER_2_OUTPUT_DATA_WIDTH-1:0] reducer_2_result_data;
    wire [REDUCER_2_OUTPUT_ADDR_WIDTH-1:0] reducer_2_result_addr;
    wire reducer_2_done;

    ram #(.C_ADDR_WIDTH(REDUCER_2_OUTPUT_ADDR_WIDTH),
          .C_DATA_WIDTH(REDUCER_2_OUTPUT_DATA_WIDTH))
                     reducer_2_result(
                         .clk(clk),
                         .wen(reducer_2_result_write_enable),
                         .wdata(reducer_2_result_data),
                         .waddr(reducer_2_result_addr),
                         .rdata(reducer_2_source_data),
                         .raddr(reducer_2_source_addr)
                     );

    reduction #(.SOURCE_WIDTH(SOURCE_WIDTH/2), 
                .SOURCE_HEIGHT(SOURCE_HEIGHT/2),
                .SOURCE_DATA_WIDTH(REDUCER_1_OUTPUT_DATA_WIDTH),
                .SOURCE_ADDR_WIDTH(REDUCER_1_OUTPUT_ADDR_WIDTH),
                .DEST_ADDR_WIDTH(REDUCER_2_OUTPUT_ADDR_WIDTH),
                .DEST_DATA_WIDTH(REDUCER_2_OUTPUT_DATA_WIDTH))
                reducer_2(
                    .clk(clk),
                    .resetn(resetn),
                    .go(reducer_2_go),
                    .source_data(reducer_1_source_data),
                    .source_addr(reducer_1_source_addr),
                    .dest_addr(reducer_2_result_addr),
                    .dest_data(reducer_2_result_data),
                    .dest_write_enable(reducer_2_result_write_enable),
                    .done(reducer_2_done));

    //Reduction Level 3 Data Path
    localparam REDUCER_3_OUTPUT_ADDR_WIDTH = REDUCER_2_OUTPUT_ADDR_WIDTH - 2;
    localparam REDUCER_3_OUTPUT_DATA_WIDTH = REDUCER_2_OUTPUT_DATA_WIDTH;

    wire reducer_3_result_write_enable;
    wire [REDUCER_3_OUTPUT_DATA_WIDTH-1:0] reducer_3_result_data;
    wire [REDUCER_3_OUTPUT_ADDR_WIDTH-1:0] reducer_3_result_addr;
    wire reducer_3_done;

    ram #(.C_ADDR_WIDTH(REDUCER_3_OUTPUT_ADDR_WIDTH),
          .C_DATA_WIDTH(REDUCER_3_OUTPUT_DATA_WIDTH))
                     reducer_3_result(
                         .clk(clk),
                         .wen(reducer_3_result_write_enable),
                         .wdata(reducer_3_result_data),
                         .waddr(reducer_3_result_addr),
                         .rdata(reducer_3_source_data),
                         .raddr(reducer_3_source_addr)
                     );

    reduction #(.SOURCE_WIDTH(SOURCE_WIDTH/4), 
                .SOURCE_HEIGHT(SOURCE_HEIGHT/4),
                .SOURCE_DATA_WIDTH(REDUCER_2_OUTPUT_DATA_WIDTH),
                .SOURCE_ADDR_WIDTH(REDUCER_2_OUTPUT_ADDR_WIDTH),
                .DEST_ADDR_WIDTH(REDUCER_3_OUTPUT_ADDR_WIDTH),
                .DEST_DATA_WIDTH(REDUCER_3_OUTPUT_DATA_WIDTH))
                reducer_3(
                    .clk(clk),
                    .resetn(resetn),
                    .go(reducer_3_go),
                    .source_data(reducer_2_source_data),
                    .source_addr(reducer_2_source_addr),
                    .dest_addr(reducer_3_result_addr),
                    .dest_data(reducer_3_result_data),
                    .dest_write_enable(reducer_3_result_write_enable),
                    .done(reducer_3_done));


    //States
    localparam  RESET = 0,
                REDUCE_1 = 1,
                REDUCE_2 = 2,
                REDUCE_3 = 3,
                CENTROID = 4,
                DONE = 5;

    reg [4:0] curState, nextState;

    //State Loading
    always@(posedge clk) begin
        if(resetn == 1'b0)
            curState <= RESET;
        else
            curState <= nextState;
    end

    //State Transitions
    always@(*)begin
        case(curState) 
            RESET: begin
                if(go)
                    nextState <= REDUCE_1;
                else
                    nextState <= RESET;
            end
            REDUCE_1: begin
                if(reducer_1_done)
                    nextState <= REDUCE_2;
                else
                    nextState <= REDUCE_1;
            end
            REDUCE_2: begin
                if(reducer_2_done)
                    nextState <= REDUCE_3;
                else
                    nextState <= REDUCE_2;
            end
            REDUCE_3: begin
                if(reducer_3_done)
                    nextState <= DONE;
                else
                    nextState <= REDUCE_3;
            end
        endcase
    end

    //control Path
    always@(posedge clk) begin
        case (curState)
            RESET: begin
                reducer_1_go <= 0;
                reducer_2_go <= 0;
                reducer_3_go <= 0;
                done <= 0;
            end
            REDUCE_1: begin
                reducer_1_go <= 1;
                reducer_2_go <= 0;
                reducer_3_go <= 0;
                done <= 0;
            end
            REDUCE_2: begin
                reducer_1_go <= 0;
                reducer_2_go <= 1;
                reducer_3_go <= 0;
                done <= 0;
            end
            REDUCE_3: begin
                reducer_1_go <= 0;
                reducer_2_go <= 0;
                reducer_3_go <= 1;
                done <= 0;
            end
            DONE: begin
                reducer_1_go <= 0;
                reducer_2_go <= 0;
                reducer_3_go <= 0;
                done <= 1;
            end
        endcase
    end

endmodule
