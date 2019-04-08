`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/20/2019 08:45:33 PM
// Design Name: 
// Module Name: CoreIP
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


module CoreIP(
    input [11:0] externalData,
    output [21:0] read_px_sel,
    output [11:0] target_x,
    output [8:0] target_y,
    input clk,
    input reset
    );
    //TO BE FIXED BY ANDREW'S CODE
    assign target_x = 0;
    assign target_y = 0;


	localparam BRAM_BUS_WIDTH = 7*3*4; //7 pixels read at a time

    //Parameters and variables pertaining to source image
    localparam WIDTH = 320;
    localparam HEIGHT= 240;

    localparam WIDTH_BITS = $clog2(WIDTH+1);
    localparam HEIGHT_BITS = $clog2(HEIGHT+1);

    wire[WIDTH_BITS-1:0] external_readX;
    wire[HEIGHT_BITS-1:0] external_readY;
    assign read_px_sel = (external_readY*WIDTH) + external_readX;

    //Parameters pertaining to spatial filtering box
    localparam NUM_PIXELS = WIDTH*HEIGHT;

    localparam SPATIAL_BOX_RADIUS = 3;
    localparam SPATIAL_BOX_SIDE_LENGTH = 2*SPATIAL_BOX_RADIUS+1;
    localparam SPATIAL_BOX_TOTAL_BITS = $clog2(SPATIAL_BOX_SIDE_LENGTH+1);

    //Variables for the center of the current spatial box being filtered
    wire incrementX, incrementY;
    wire resetX, resetY;

    wire [WIDTH_BITS-1:0] curX;
    //X, Y coordinates where the sweep will begin and end
    localparam MIN_X = SPATIAL_BOX_RADIUS;
    localparam MAX_X = (WIDTH-1) - SPATIAL_BOX_RADIUS;

    localparam MIN_Y = SPATIAL_BOX_RADIUS;
    localparam MAX_Y = (HEIGHT-1) - SPATIAL_BOX_RADIUS;
    
    Counter #(.WIDTH(WIDTH_BITS), .RESET_VAL(MIN_X)) curXCounter (.clk(clk), .reset(resetX), .data(curX), .increment(incrementX));

    wire [HEIGHT_BITS-1:0] curY;
    Counter #(.WIDTH(HEIGHT_BITS), .RESET_VAL(MIN_Y)) curYCounter (.clk(clk), .reset(resetY), .data(curY), .increment(incrementY));



    //Storage of the actual image 
    (* ram_style = "block" *) reg[NUM_PIXELS-1:0] binaryImage;

    wire [$clog2(NUM_PIXELS+1)-1:0] binaryImage_writeAddr = (curY*WIDTH) + curX;
    wire binaryImage_writeEn;

    always@(posedge clk) begin
        if(reset) begin
            binaryImage[NUM_PIXELS-1:0] <= 0;
        end

        else if(binaryImage_writeEn) begin
            binaryImage[binaryImage_writeAddr] <= spatialBoxResult;
        end
    end

    //parameters to keep track of where in the spatial box we are 
    wire incrementSpatialRow;
    wire resetSpatialRowCount;
    wire [SPATIAL_BOX_TOTAL_BITS-1:0] curSpatialRowCount;
    Counter #(.WIDTH(SPATIAL_BOX_TOTAL_BITS)) spatialRowCounter(.clk(clk), .reset(resetSpatialRowCount), .data(curSpatialRowCount), .increment(incrementSpatialRow));
    
    //Spatial filter processor
    wire enableSpatialShiftReg;
    wire spatialBoxResult;
    wire resetSpatialShiftRegister;
    rawRGBShiftRegister rowHandler(.rgbInputRow(data), 
                                    .reset(resetSpatialShiftRegister), 
                                    .clk(clk),
                                    .shiftEnable(enableSpatialShiftReg),
                                    .boxPassesThreshold(spatialBoxResult));

    //FSM state variables
    reg [$clog2(15+1)-1:0] curState, nextState;

    //FSM to actually do the load
    localparam  STATE_RESET_ALL = 0,
                STATE_SPATIAL_INIT = 1,
                STATE_SPATIAL_SET_EXTERNAL_ADDRESS = 2,
                STATE_SPATIAL_LOAD_DATA = 3,
                STATE_SPATIAL_INCREMENT_LOC = 4,
                STATE_SPATIAL_CHECK_BOUNDS = 5,
                STATE_SPATIAL_ALLOW_COMBO_LOGIC = 6,
                STATE_SPATIAL_STORE_BINARY_VAL = 7,
                
                STATE_STEPPING_SET_EXTERNAL_ADDRESS = 8,
                STATE_STEPPING_LOAD_DATA = 9,
                STATE_STEPPING_ALLOW_COMBO_LOGIC = 10,
                STATE_STEPPING_STORE_BINARY_VAL = 11,
                STATE_STEPPING_CHECK_BOUNDS = 12,
                STATE_STEPPING_INCREMENT_X = 13,
                STATE_STEPPING_INCREMENT_Y = 14;


    //Loading next states
    always@(posedge clk) begin
        if(reset) begin
            curState <= STATE_RESET_ALL;
        end

        else begin
            curState <= nextState;
        end
    end

    //Determining next state
    always@(*) begin
        case(curState)
            STATE_RESET_ALL: begin
                nextState = STATE_SPATIAL_INIT;                
            end

            STATE_SPATIAL_INIT: begin
                nextState = STATE_SPATIAL_SET_EXTERNAL_ADDRESS;
            end

            STATE_SPATIAL_SET_EXTERNAL_ADDRESS: begin
                nextState = STATE_SPATIAL_LOAD_DATA;
            end

            STATE_SPATIAL_LOAD_DATA: begin
                nextState = STATE_SPATIAL_INCREMENT_LOC;
            end

            STATE_SPATIAL_INCREMENT_LOC: begin
                nextState = STATE_SPATIAL_CHECK_BOUNDS;
            end

            STATE_SPATIAL_CHECK_BOUNDS: begin
                //Basically want to iterate until all rows of the spatial box are loaded
                //At that point, we can process and save
                if(curSpatialRowCount < SPATIAL_BOX_SIDE_LENGTH) begin
                    nextState = STATE_SPATIAL_SET_EXTERNAL_ADDRESS;
                end

                else begin
                    nextState = STATE_SPATIAL_ALLOW_COMBO_LOGIC;
                end
            end

            STATE_SPATIAL_ALLOW_COMBO_LOGIC: begin
                nextState = STATE_SPATIAL_STORE_BINARY_VAL;
            end

            STATE_SPATIAL_STORE_BINARY_VAL: begin
                nextState = STATE_STEPPING_SET_EXTERNAL_ADDRESS;
            end
            
            STATE_STEPPING_SET_EXTERNAL_ADDRESS: begin
                nextState = STATE_STEPPING_LOAD_DATA;
            end

            STATE_STEPPING_LOAD_DATA: begin
                nextState = STATE_STEPPING_ALLOW_COMBO_LOGIC;
            end

            STATE_STEPPING_ALLOW_COMBO_LOGIC: begin
                nextState = STATE_STEPPING_STORE_BINARY_VAL;
            end

            STATE_STEPPING_STORE_BINARY_VAL: begin
                nextState = STATE_STEPPING_CHECK_BOUNDS;
            end

            STATE_STEPPING_CHECK_BOUNDS: begin
                //Bottom of screen reached
                if(curY == MAX_Y) begin
                    //End of image reached
                    if(curX == MAX_X) begin
                        nextState = STATE_RESET_ALL;
                    end

                    //Just need to go to the next row
                    else begin
                        nextState = STATE_STEPPING_INCREMENT_X;
                    end
                end

                //Still processing a column - proceed
                else begin
                    nextState = STATE_STEPPING_INCREMENT_Y;
                end
            end

            STATE_STEPPING_INCREMENT_X: begin
                nextState = STATE_SPATIAL_INIT;
            end

            STATE_STEPPING_INCREMENT_Y: begin
                nextState = STATE_STEPPING_SET_EXTERNAL_ADDRESS; 
            end

            default:begin
                nextState = STATE_RESET_ALL;
            end
        endcase
    end

    //*************************************Data control*************************
    //Control X and Y location
    assign incrementX = (curState == STATE_STEPPING_INCREMENT_X);
    assign incrementY = (curState == STATE_STEPPING_INCREMENT_Y);
    assign resetX = (curState == STATE_RESET_ALL);
    assign resetY = (curState == STATE_RESET_ALL) | (curState == STATE_STEPPING_INCREMENT_X);

    //External read location
    assign external_readY = (curState == STATE_SPATIAL_SET_EXTERNAL_ADDRESS) ? ((curY - SPATIAL_BOX_SIDE_LENGTH) + curSpatialRowCount) :
                                                                                curY;
    assign external_readX = (curX - SPATIAL_BOX_SIDE_LENGTH);    
    
    //Read enable on the data
    assign  enableSpatialShiftReg = (curState == STATE_SPATIAL_LOAD_DATA) | 
            (curState == STATE_STEPPING_LOAD_DATA);
    assign incrementSpatialRow = (curState == STATE_SPATIAL_INCREMENT_LOC);
    assign resetSpatialRowCount = (curState == STATE_RESET_ALL) | (curState == STATE_SPATIAL_INIT);
    assign resetSpatialShiftRegister = (curState == STATE_RESET_ALL) | (curState == STATE_SPATIAL_INIT);



endmodule
