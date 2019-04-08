/*
 * writeFile.h
 *
 *  Created on: Mar 6, 2019
 *      Author: elee1
 */

#ifndef WRITEFILE_H_
#define WRITEFILE_H_
#include "Pixel.h"
#include <stdbool.h>

//Write a file at the desired path
void writeFile(char* fileName, Pixel* imageData);

//Write a binary B/W file at the desired path
void writeBWFile(char* fileName, bool* imageData);

//Write a variable size B/W file at the desired path
void writeVariableSizeBWFile(char* fileNAme, bool* imageData, unsigned int width, unsigned int height);

#endif /* WRITEFILE_H_ */
