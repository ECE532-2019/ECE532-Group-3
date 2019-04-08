/*
 * applyDilationFilter.c
 *
 *  Created on: Mar 6, 2019
 *      Author: elee1
 */

#include <stdio.h>
#include <stdbool.h>
#include "CONSTANTS.h"
#include <stdlib.h>
#include "../include/applySpatialFilter.h"
#include "../include/Pixel.h"
#include "../include/writeFile.h"

bool* applySpatialFilter(bool* imageData){
	bool* newImage = (bool*)malloc(HEIGHT*WIDTH*sizeof(bool));

	//Iterate every pixel
	const unsigned ACCEPTANCE_THRESHOLD = (2*SPATIAL_HEIGHT)*(2*SPATIAL_WIDTH)*SPATIAL_THRESHOLD;

	unsigned sum_of_box = 0;

	//Initialize the first box
	for(unsigned y = 0; y <= 2*SPATIAL_HEIGHT; y++){
		//Add row - in Verilog this will just be done by reading the bus
		for(unsigned x = 0; x <= 2*SPATIAL_WIDTH; x++){
			sum_of_box += imageData[getIndex(y,x)];
		}
	}


	//Boundaries which can sweep the full box
	const unsigned min_x = SPATIAL_WIDTH;
	const unsigned max_x = (WIDTH-1) - SPATIAL_WIDTH;

	const unsigned min_y = SPATIAL_HEIGHT;
	const unsigned max_y = (HEIGHT-1) - SPATIAL_HEIGHT;

	//The current central point of the box
	unsigned curX = min_x;
	unsigned curY = min_y;
	unsigned curArrayIndex = getIndex(curY, curX);

	bool isGoingDown = true;
	bool isMovingRight = false;

	newImage[curArrayIndex] = (sum_of_box >= ACCEPTANCE_THRESHOLD);

	//Writing using while loop to more closely resemble Verilog implementation
	while(true){
		//FSM STATE 1: Move to next pixel
		isMovingRight = false;
		if(isGoingDown){
			//Check if bottom reached
			if(curY == max_y){
				//The final pixel was processed last iteration
				if(curX == max_x){
					free(imageData);
					return newImage;
				}

				//Move right and prepare to go back up
				else{
					curX++;
					curArrayIndex++;

					isMovingRight = true;
				}
			}

			///Can proceed normal movement
			else{
				curY++;
				curArrayIndex += WIDTH;
			}
		}

		//Going up
		else{
			if(curY == min_y){
				//The final pixel was processed last iteration
				if(curX == max_x){
					free(imageData);
					return newImage;
				}

				//Move right and prepare to go back down
				else{
					curX++;
					curArrayIndex++;

					isMovingRight = true;
				}
			}

			//Normal movement
			else{
				curY--;
				curArrayIndex -= WIDTH;
			}
		}

		//FSM STATE 2 & 3: ADD+REMOVE rows
		//Get the index in the array corresponding to where we should remove the old data and add the new data
		unsigned addRowY, subRowY;

		//Add the new row, and remove the old row from the running sum

		//Need to add/sub the edge pixels
		unsigned amountToAdd = 0, amountToSub = 0;

		if(isMovingRight){
			unsigned leftArrayIdx = curArrayIndex - SPATIAL_HEIGHT*WIDTH - SPATIAL_WIDTH - 1;
			unsigned rightArrayIdx = curArrayIndex - SPATIAL_HEIGHT*WIDTH + SPATIAL_WIDTH;

			//Subtract left edge, Add right edge
			for(unsigned i = 0; i <= 2*SPATIAL_HEIGHT; i++){
				amountToAdd += imageData[rightArrayIdx];
				amountToSub += imageData[leftArrayIdx];

				leftArrayIdx += WIDTH;
				rightArrayIdx += WIDTH;
			}
		}

		//Normal add/sub of going up/down
		else{
			unsigned addRow_beginIndex;
			unsigned subRow_beginIndex;

			//Get indices in the array of the rows being added/removed
			if(isGoingDown){
				addRow_beginIndex = (curArrayIndex + SPATIAL_HEIGHT*WIDTH) - SPATIAL_WIDTH;
				subRow_beginIndex = (curArrayIndex - (SPATIAL_HEIGHT+1)*WIDTH) - SPATIAL_WIDTH;
			}

			else{
				addRow_beginIndex = (curArrayIndex - SPATIAL_HEIGHT*WIDTH) - SPATIAL_WIDTH;
				subRow_beginIndex = (curArrayIndex + (SPATIAL_HEIGHT+1)*WIDTH) - SPATIAL_WIDTH;
			}

			//Add up the values of the rows being added/removed
			for(unsigned i = 0; i <= 2*SPATIAL_WIDTH; i++){ //one width on each side of pixel + the pixel's column itself
				amountToAdd += imageData[addRow_beginIndex + i];

				amountToSub += imageData[subRow_beginIndex + i];
			}
		}


		sum_of_box += amountToAdd;
		sum_of_box -= amountToSub;

		//Need to flip directions if we are at an edge
		if(isMovingRight){
			isGoingDown = !isGoingDown;
		}

		//Threshold and save the pixel
		newImage[curArrayIndex] = (sum_of_box >= ACCEPTANCE_THRESHOLD);
	}
}
