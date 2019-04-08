module Counter(
	clk,
	reset,
	data,
	increment);
	
	parameter WIDTH = 8;
	parameter RESET_VAL = 0;

	input clk;
	input reset;
	input increment;

	output reg [WIDTH-1:0] data;


	always@(posedge clk) begin
		if(reset) begin
			data <= RESET_VAL;
		end

		else if(increment) begin
			data <= data + 1;
		end

		else begin
			data <= data;
		end
	end
endmodule