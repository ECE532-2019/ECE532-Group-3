/*
 * drawTarget.c
 *
 *  Created on: Mar 17, 2019
 *      Author: elee1
 */

#include "drawTarget.h"
#include "CONSTANTS.h"
int max(int a, int b){
	return (a > b) ? a : b;
}

int min (int a, int b){
	return (a < b) ? a : b;
}

void drawTarget(int x, int y, Pixel* data){
	const int lowerX = max(0, x - TARGET_DIMENSION/2);
	const int upperX = min(WIDTH-1, x + TARGET_DIMENSION/2);

	const int lowerY = max(0, y - TARGET_DIMENSION/2);
	const int upperY = min(HEIGHT-1, y + TARGET_DIMENSION/2);

	for(unsigned curX = lowerX; curX <= upperX; curX++){
		for (unsigned curY = lowerY; curY <= upperY; curY++){
			unsigned index = getIndex(curY, curX);

			data[index].data[R] = 15;
			data[index].data[G] = 0;
			data[index].data[B] = 0;
		}
	}

	return;
}

