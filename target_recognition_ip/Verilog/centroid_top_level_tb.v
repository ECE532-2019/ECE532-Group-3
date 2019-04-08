`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/28/2019 03:09:05 AM
// Design Name: 
// Module Name: centroid_top_level_tb
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


module centroid_top_level_tb(

    );

    //DUT IO
    reg Image_data;
    wire [18:0] Image_addr;
    reg clk;
    reg resetn;
    reg go;
    wire [10:0] x;
    wire [19:0] y;

    //Instantiate DUT
    centroid_top_level
        DUT(.clk(clk),
            .resetn(resetn),
            .go(go),
            .source_data(Image_data),
            .source_addr(Image_addr),
            .x(x),
            .y(y));

    //ImageData Buffer
    reg wen;
    reg [18:0] waddr;
    reg wdata;
    wire rdata;
    reg [18:0] raddr;
    ram #(.C_DATA_WIDTH(1), .C_ADDR_WIDTH(19)) ImageData(clk, wen, wdata, waddr, rdata, raddr);

    //Generate the Clock and connect ram to centroid module
    initial begin
        clk <= 0;
        forever begin
            #5 clk <= ~clk;
            Image_data <= rdata;
            raddr <= Image_addr;
        end
    end

    //File handles
    integer data_file; //file handle
    integer scan_file; //file handle
    reg read_data;
    `define NULL 0

    
    initial begin

        //Reset
        assign resetn = 0;
        assign waddr = 0;
        assign go = 0;
        #20;
        assign resetn = 1;
        #3;

        //Load ppm file into ImageData Buffer
        data_file = $fopen("C:/tmp/centroid/centroids/centroids.srcs/sim_1/imports/Images/Stage3_Spatial.ppm", "r");
        if (data_file == `NULL) begin
            $display("data_file handle was NULL");
            $finish;
        end
        
        while (!$feof(data_file)) begin
            scan_file = $fscanf(data_file, "%d\n", read_data);
            if (!$feof(data_file)) begin
                if(read_data == 1) begin
                    $display("read %d at pixel %d",read_data, waddr);
                end
                assign wen = 1;
                assign wdata = read_data;
                #10;
                assign waddr = waddr + 1;
            end
        end
        
        $fclose(data_file);
        
        //ImageData Now Contains the output of Stage3_Spatial a 640x480 array of bits
        #20
        $display("Loaded the Spatial Image into Buffer\n");
        assign wen = 0;
        assign go = 1;
    end

endmodule
