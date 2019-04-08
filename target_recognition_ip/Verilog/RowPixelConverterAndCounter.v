
module PixelConverter(
	R, 
	G, 
	B, 
	binaryFilteredPix
);
	localparam channelWidth = 4;

	//Input: data for the red/green/blue colour channels
	input [channelWidth-1:0] R, G, B;	

	//Output: A converted and filtered binary result of the colour channels. Corresponds to skin filter. 
	output binaryFilteredPix;

	//Convert to colourspace - leave an extra bit for signed
	wire signed [2*channelWidth:0] u = R - G; 
	wire signed [2*channelWidth:0] v = B - G;

	wire u_in_range = (u < 6) & (u > 3);
	wire v_in_range = (v < 1) & (v > -2);

	assign binaryFilteredPix = u_in_range & v_in_range;
	

endmodule 