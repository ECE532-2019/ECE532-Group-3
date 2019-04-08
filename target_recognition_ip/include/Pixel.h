/*
 * Pixel.h
 *
 *  Created on: Mar 6, 2019
 *      Author: elee1
 */

#ifndef PIXEL_H_
#define PIXEL_H_

#include <stdint.h>
#include "CONSTANTS.h"

#define R 0
#define Y 0

#define G 1
#define U 1

#define B 2
#define V 2


typedef struct PixelStruct{
	int8_t data[3]; //only need 4 bits, not a whole Byte
} Pixel;

unsigned  getIndex(unsigned row, unsigned col);

#endif /* PIXEL_H_ */
