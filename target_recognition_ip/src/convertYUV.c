/*
 * convertYUV.c
 *
 *  Created on: Mar 6, 2019
 *      Author: elee1
 */


#include "convertYUV.h"
#include "Pixel.h"

void convertRgbYuv(Pixel* rgbPixelArray){
	//Iterate over each pixel in the image
	for (unsigned row = 0; row < HEIGHT; row++){
		for(unsigned col = 0; col < WIDTH; col++){
			uint8_t r,g,b, y, u, v;

			Pixel* curPx = &rgbPixelArray[getIndex(row,col)];
			r = curPx->data[R];
			g = curPx->data[G];
			b = curPx->data[B];

			curPx->data[Y] = (r + 2*g + b)/4;
			curPx->data[U] = r - g;
			curPx->data[V] = b - g;
		}
	}
}
