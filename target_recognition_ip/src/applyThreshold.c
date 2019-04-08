/*
 * applyThreshold.c
 *
 *  Created on: Mar 6, 2019
 *      Author: elee1
 */

#include "Pixel.h"
#include <stdbool.h>
#include <stdlib.h>

bool* applyThreshold(Pixel* imageData){
	bool* uData = (bool*)malloc(WIDTH*HEIGHT*sizeof(bool));
	//Iterate over each pixel in the image
	for (unsigned row = 0; row < HEIGHT; row++){
		for(unsigned col = 0; col < WIDTH; col++){
			uint8_t r,g,b, y, u, v;

			Pixel* curPx = &imageData[getIndex(row,col)];
			y = curPx->data[Y];
			u = curPx->data[U];
			v = curPx->data[V];

//			curPx->data[Y] = 0;
//			curPx->data[U] = (u > U_MAX | u < U_MIN) ? 0 : u;
//			curPx->data[V] = 0;
			//curPx->data[V] = (v > V_MAX | v < V_MIN) ? 0 : v;
			bool u_in_range = (u > U_MAX | u < U_MIN) ? 0 : 1;
			bool v_in_range = 1 - (v > V_MAX | v < V_MIN) ? 0 : 1;
			uData[WIDTH*row+col] = u_in_range & v_in_range;
		}
	}

	return uData;
}
