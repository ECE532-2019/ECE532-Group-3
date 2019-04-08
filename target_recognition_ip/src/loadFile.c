/*
 * FileLoader.c
 *
 *  Created on: Mar 6, 2019
 *      Author: elee1
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "loadFile.h"
#include "Pixel.h"
#include "CONSTANTS.h"

Pixel* loadFile(char* fileName){
	//Attempt to open the file
	FILE *fp;
	fp = fopen(fileName, "r");

	if(fp == NULL){
		printf("Unable to open %s\n", fileName);
		return NULL;
	}


	//Allocate memory for the image data
	Pixel* pixelData = (Pixel*)malloc(HEIGHT*WIDTH*sizeof(Pixel));

	if(pixelData == NULL){
		return NULL;
	}

	//TODO: Error check the first three lines to see file format is correct
	//They should match image format (P3), dimension (640x480), and max value (255)
	char line [255];

	//Discard header
	for(unsigned i = 0; i < 3; i++){
		fgets(line, sizeof(line), fp);
	}

	//Iterate over each pixel in the image and save it to the data structure
	for (unsigned row = 0; row < HEIGHT; row++){
		for(unsigned col = 0; col < WIDTH; col++){
			for(unsigned channel = 0; channel <= 2; channel++){
				uint8_t data;

				//Read line and convert to a uint
				fgets(line, sizeof(line), fp);
				data = strtoumax(line, NULL, 10);

				//Data is stored as 8 bit (max 255), while we can only store as 4 bit (max 15)
				//Simple conversion is to bit shift so that only the 4 MSB are saved

				unsigned storeEntry;

				//Most likely data will be stored in the same way (i.e. R/G/B), but you have the freedom to change this
				switch(channel){
								case 0: storeEntry = R;
								break;

								case 1: storeEntry = G;
								break;

								case 2: storeEntry = B;
								break;
				}
				pixelData[getIndex(row,col)].data[storeEntry] = (data >> 4);
			}
		}
	}


	fclose(fp);
	return pixelData;
}

