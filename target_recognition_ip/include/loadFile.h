#ifndef LOADFILE_H_
#define LOADFILE_H_

#include <stdio.h>
#include "Pixel.h"
//Loads image of our fixed size, and returns pointer
//Output array will be row-major, R/G/B data
//Returns pointer to 4-bit image of size defined in CONSTANTS.h
//Or NULL on failure
Pixel* loadFile(char* fileName);


#endif /* LOADFILE_H_ */
