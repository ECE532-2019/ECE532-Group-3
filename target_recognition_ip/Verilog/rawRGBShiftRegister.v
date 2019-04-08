module rawRGBShiftRegister(
	rgbInputRow,	
	reset,
	clk,
	shiftEnable,
	boxPassesThreshold
);
	localparam ROW_WIDTH = 7;
	localparam ACCEPTANCE_THRESHOLD = 28; //Total count of 1's to accept the box - taken from C tuning
	input [3*4*ROW_WIDTH-1:0] rgbInputRow;
	input reset;
	input clk;
	input shiftEnable;

	//This binary output indicates if the data currently stored in the shift register 
	//has enough 1 bits to pass the threshold of acceptance
	output boxPassesThreshold; 

	//Shift register to hold 7 rows of data
	//Each row is 7 pixels wide
	//Each pixel is 3 channels * 4 bit depth = 12 bits
	reg [3*4*ROW_WIDTH-1:0] shiftReg [ROW_WIDTH-1:0]; 
	
	always@(posedge clk) begin
		if(reset) begin
			shiftReg[0] <= 0;
			shiftReg[1] <= 0;
			shiftReg[2] <= 0;
			shiftReg[3] <= 0;
			shiftReg[4] <= 0;
			shiftReg[5] <= 0;
			shiftReg[6] <= 0;
		end

		else if(shiftEnable) begin
			shiftReg[0] <= rgbInputRow;
			shiftReg[1] <= shiftReg[0];
			shiftReg[2] <= shiftReg[1];
			shiftReg[3] <= shiftReg[2];
			shiftReg[4] <= shiftReg[3];
			shiftReg[5] <= shiftReg[4];
			shiftReg[6] <= shiftReg[5];
		end
	end

	//Calculate the count of each row
	wire [$clog2(ROW_WIDTH+1)-1:0] OneCounts [ROW_WIDTH-1:0];

	genvar idx;
	generate
		for(idx = 0; idx < ROW_WIDTH; idx=idx+1)begin:RowProcessors
			RowPixelConverterAndCounter rowProc(.threeChannelInputRow(shiftReg[idx]), .filteredRowCount(OneCounts[idx]));
		end
	endgenerate

	//Finally, threshold the data
	wire[7:0] count = OneCounts[0] +
					 OneCounts[1] +
					 OneCounts[2] +
					 OneCounts[3] +
					 OneCounts[4] +
					 OneCounts[5] +
					 OneCounts[6];
	assign boxPassesThreshold = (count > ACCEPTANCE_THRESHOLD);
endmodule