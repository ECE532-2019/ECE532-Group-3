`timescale 1ns/1ns
// Reduction Block
// Reduces an n by m array into an  (n/2) by (m/2) array by
// sweeping a 2x2 box across the input array and ANDing the elements together
// Assumes a two cycle delay when accessing the source memory

module reduction
#( 
    parameter SOURCE_WIDTH = 640,
    parameter SOURCE_HEIGHT = 480,
    parameter SOURCE_DATA_WIDTH = 1,
    parameter SOURCE_ADDR_WIDTH = 19,
    parameter DEST_ADDR_WIDTH = 17,
    parameter DEST_DATA_WIDTH = 1
)
(
    input clk,
    input resetn,
    input go,
    input [SOURCE_DATA_WIDTH-1:0] source_data,
    output reg [SOURCE_ADDR_WIDTH-1:0] source_addr,
    output reg [DEST_ADDR_WIDTH-1:0] dest_addr,
    output reg [DEST_DATA_WIDTH-1:0] dest_data,
    output reg dest_write_enable,
    output reg done
);

    //States
    localparam RESET = 0,
               ISSUE_READ_TL = 1,
               ISSUE_READ_TR = 2,
               ISSUE_READ_BL_AND_TL_READY = 3,
               ISSUE_READ_BR_AND_TR_READY = 4,
               BL_READY = 5,
               BR_READY = 6,
               WRITE_RESULT = 7,
               DONE = 8;

     reg [4:0] curState, nextState;

     //State Loading
     always@(posedge clk) begin
         if(resetn == 1'b0)
             curState <= RESET;
         else
             curState <= nextState;
     end

     ////////////////////////////////////////////////////////////////
     //State Transitions
     always@(*) begin
         case (curState)
             RESET: begin
                if(go)
                    nextState <= ISSUE_READ_TL;
                else if(!go)
                    nextState <= RESET;
             end
             ISSUE_READ_TL: begin
                 nextState <= ISSUE_READ_TR;
             end
             ISSUE_READ_TR: begin
                 nextState <= ISSUE_READ_BL_AND_TL_READY;
             end
             ISSUE_READ_BL_AND_TL_READY: begin
                 nextState <= ISSUE_READ_BR_AND_TR_READY;
             end
             ISSUE_READ_BR_AND_TR_READY: begin
                 nextState <= BL_READY;
             end
             BL_READY: begin
                 nextState <= BR_READY;
             end
             BR_READY: begin
                 nextState <= WRITE_RESULT;
             end
             WRITE_RESULT: begin
                 if(dest_addr == (SOURCE_WIDTH * SOURCE_HEIGHT)/4)
                     nextState <= DONE;
                 else
                     nextState <= ISSUE_READ_TL;
             end
             DONE: begin
                 nextState <= DONE;
             end
         endcase
     end

     ////////////////////////////////////////////////////////////////
     reg [SOURCE_ADDR_WIDTH-1:0] source_index;
     reg last;

     //Control Signals
     always@(posedge clk) begin
        case (curState)
            RESET: begin
                source_index <= 0;
                source_addr <= 0;
                dest_addr <= 0;
                dest_data <= 0;
                dest_write_enable <= 0;
                done <= 0;
                last <= 0;
            end
            ISSUE_READ_TL: begin
                source_addr <= source_index;
            end
            ISSUE_READ_TR: begin
                source_addr <= source_index + 1;
            end
            ISSUE_READ_BL_AND_TL_READY: begin
                source_addr <= (source_index + SOURCE_WIDTH);
                dest_data <= source_data[0];
            end
            ISSUE_READ_BR_AND_TR_READY: begin
                source_addr <= (source_index + SOURCE_WIDTH + 1);
                dest_data <= (dest_data & source_data[0]);
            end
            BL_READY: begin
                source_addr <= 0;
                dest_data <=  (dest_data & source_data[0]);
            end
            BR_READY: begin
                source_addr <= 0;
                dest_data <= (dest_data & source_data[0]);
            end
            WRITE_RESULT: begin
                //If at the end of row -> update to the next next row
                if( (source_index != 0) && !last && (source_index % SOURCE_WIDTH == 0)) begin
                    source_index <= source_index + SOURCE_WIDTH;
                    last <= 1;
                end
                else begin
                    source_index <= source_index + 2;
                    last <= 0;
                end
                
                //Send the write
                dest_addr <= dest_addr + 1;
                dest_write_enable <= 1;
            end
            DONE: begin
                done <= 1'b1;
            end
            default: begin
                done <= 1'b0;
                dest_data <= 1'b0;
            end
        endcase
    end

endmodule
