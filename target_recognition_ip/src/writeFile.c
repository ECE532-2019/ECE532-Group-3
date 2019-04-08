/*
 * writeFile.c
 *
 *  Created on: Mar 6, 2019
 *      Author: elee1
 */

#include "writeFile.h"
#include "Pixel.h"
#include "CONSTANTS.h"

#include <stdint.h>
#include <stdio.h>

void writeFile(char* fileName, Pixel* imageData){
	//Attempt to open the file
	FILE *fp;
	fp = fopen(fileName, "w");

	if(fp == NULL){
		printf("Unable to open %s\n", fileName);
		return;
	}

	//Write header
	fprintf(fp, "P3\n");
	fprintf(fp, "%d %d\n", WIDTH, HEIGHT);
	fprintf(fp, "%d\n", MAX_INTENSITY);

	//Write all the pixel data
	//Iterate over each pixel in the image and save it to the data structure
	for (unsigned row = 0; row < HEIGHT; row++){
		for(unsigned col = 0; col < WIDTH; col++){
			for(unsigned channel = 0; channel <= 2; channel++){
				unsigned currentColour;

				switch(channel){
				case 0: currentColour = R;
				break;

				case 1: currentColour = G;
				break;

				case 2: currentColour = B;
				break;
				}

				uint8_t curChannelData = imageData[getIndex(row,col)].data[currentColour];
				fprintf(fp, "%u\n", curChannelData);
			}
		}
	}

	fclose(fp);
}

void writeBWFile(char* fileName, bool* imageData){
	//Attempt to open the file
	FILE *fp;
	fp = fopen(fileName, "w");

	if(fp == NULL){
		printf("Unable to open %s\n", fileName);
		return;
	}

	//Write header
	fprintf(fp, "P2\n");
	fprintf(fp, "%d %d\n", WIDTH, HEIGHT);
	fprintf(fp, "%d\n", 1);

	//Write all the pixel data
	//Iterate over each pixel in the image and save it to the data structure
	for (unsigned row = 0; row < HEIGHT; row++){
		for(unsigned col = 0; col < WIDTH; col++){
			unsigned currentColour;
			uint8_t curChannelData = imageData[row*WIDTH+col];
			fprintf(fp, "%u\n", curChannelData);
		}
	}

	fclose(fp);
}

void writeVariableSizeBWFile(char* fileName, bool* imageData, unsigned int width, unsigned int height){
	//Attempt to open the file
	FILE *fp;
	fp = fopen(fileName, "w");

	if(fp == NULL){
		printf("Unable to open %s\n", fileName);
		return;
	}

	//Write header
	fprintf(fp, "P2\n");
	fprintf(fp, "%d %d\n", width, height);
	fprintf(fp, "%d\n", 1);

	//Write all the pixel data
	//Iterate over each pixel in the image and save it to the data structure
	for (unsigned row = 0; row < height; row++){
		for(unsigned col = 0; col < width; col++){
			unsigned currentColour;
			uint8_t curChannelData = imageData[row*width+col];
			fprintf(fp, "%u\n", curChannelData);
		}
	}

	fclose(fp);
}
