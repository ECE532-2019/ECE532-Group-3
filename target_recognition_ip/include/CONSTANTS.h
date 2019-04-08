/*
 * CONSTANTS.h
 *
 *  Created on: Mar 6, 2019
 *      Author: elee1
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

//Image dimensions
#define WIDTH 640
#define HEIGHT 480
#define MAX_INTENSITY 15

//Thresholding limits
#define U_MAX 6
#define U_MIN 3
#define V_MAX 1
#define V_MIN -2

//Spatial box definition
#define SPATIAL_WIDTH 3
#define SPATIAL_HEIGHT 3
#define SPATIAL_THRESHOLD 0.8

//Target definition
#define TARGET_DIMENSION 30

//Number of Targets to Return
#define NUM_TO_TARGET 4

#endif /* CONSTANTS_H_ */
