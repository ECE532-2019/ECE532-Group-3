/*
 * convertYUV.h
 *
 *  Created on: Mar 6, 2019
 *      Author: elee1
 */

#ifndef CONVERTYUV_H_
#define CONVERTYUV_H_

#include "Pixel.h"

//Accepts an array of pixel data corresponding to dimensions on CONSTANTS.h
//Returns an array of pixel data of the same size, but now YUV on the channels instead of RGB
void convertRgbYuv(Pixel* rgbPixelArray);

#endif /* CONVERTYUV_H_ */
