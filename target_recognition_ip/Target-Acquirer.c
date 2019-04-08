#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//File IO
#include "loadFile.h"
#include "writeFile.h"
#include "include/drawTarget.h"

//Core
#include "convertYUV.h"
#include "applyThreshold.h"
#include "Pixel.h"
#include "CONSTANTS.h"
#include "include/applySpatialFilter.h"
#include "findCentroids.h"



int main(int argc, char **argv) {
	if (argc < 2){
		printf ("Give argument for image to load - expect ppm of dimensions %d x %d\n", WIDTH, HEIGHT);
		return 0;
	}

	char* fileName = argv[1];

	//Load pixel data
	Pixel* pixelData = loadFile(fileName);

	if(pixelData == NULL)
		return 0;

	Pixel* originalCopy = (Pixel*) malloc(WIDTH*HEIGHT*sizeof(Pixel));
	memcpy (originalCopy, pixelData, WIDTH*HEIGHT*sizeof(Pixel));

	if(pixelData == NULL){
		printf("Failed to read image\n");
		return 0;
	}

	//Colourspace conversion
	convertRgbYuv(pixelData);
	writeFile("../Images/Stage1_YUV.ppm", pixelData);

	//Thresholding
	bool* uData = applyThreshold(pixelData);
	writeBWFile("../Images/Stage2_Threshold.ppm", uData);

	//Dilation Filter
	uData = applySpatialFilter(uData);
	writeBWFile("../Images/Stage3_Spatial.ppm", uData);

	//Find Centroids
    int x = -1;
    int y = -1;
    findCentroids(uData, &x, &y);
    printf("x: %d  y: %d \n", x, y);

	//Write output and close
    drawTarget(x,y,originalCopy);
	writeFile("../Images/Final_Output.ppm", originalCopy);

	free(originalCopy);
	free(pixelData);
	free(uData);

	return 1;
}
