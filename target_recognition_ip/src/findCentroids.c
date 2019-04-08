/*
 * findCentroids.c
 *
 *  Created on: Mar 7, 2019
 *      Author: elee1
 */

#include "findCentroids.h"
#include "CONSTANTS.h"
#include <math.h>

void add_target(int* x_list, int* y_list, int* c_num_list, int x, int y, int num_lit){

    for(int i = 0; i < NUM_TO_TARGET; i++){
        if(num_lit > c_num_list[i]){
            x_list[i] = x;
            y_list[i] = y;
            c_num_list[i] = num_lit;
            return;
        }
    }

    return;
}

bool* AND_reduction(bool* imageData, int reduction_level){

    int divisor = (int) pow(2.0, (double) reduction_level);
    int scale = (int) pow(2.0, (double) reduction_level-1);

    int r_height = HEIGHT/divisor;
    int r_width = WIDTH/divisor;

    int height = HEIGHT/(scale);
    int width = WIDTH/(scale);

    int reduced_size = (int) (r_height * r_width);

    bool* reduced = malloc(reduced_size * sizeof(bool));
    int reduced_index = 0;

    for(int row = 0; row < height; row = row + 2){
        for(int col = 0; col < width; col = col + 2){
            
            int cur_row_ind = (row * width) + col;
            int n_row_ind = ((row+1)*width) + col;

            bool gate_output = imageData[cur_row_ind] | imageData[cur_row_ind+1] | imageData[n_row_ind] | imageData[n_row_ind+1];
            reduced[reduced_index] = gate_output;

            reduced_index = reduced_index + 1;
       }
    }   

    return reduced;
}


void findCentroids(bool* imageData, int* x, int* y){



    //Layer 1 AND gate Reduction
    bool* reduced = AND_reduction(imageData, 1);
    unsigned int tmp1 = WIDTH / (int) pow(2.0, (double) 1);
    unsigned int tmp2 = HEIGHT / (int) pow(2.0, (double) 1);
    writeVariableSizeBWFile("../Images/Stage41_Centroid.ppm", reduced, tmp1, tmp2);    


    //Layer 2 AND gate Reduction
    bool* reduced_2 = AND_reduction(reduced, 2);
    free(reduced);
    tmp1 = WIDTH / (int) pow(2.0, (double) 2);
    tmp2 = HEIGHT / (int) pow(2.0, (double) 2);
    writeVariableSizeBWFile("../Images/Stage42_Centroid.ppm", reduced_2, tmp1, tmp2);    

    //Layer 3 AND gate Reduction
    bool* reduced_3 = AND_reduction(reduced_2, 3);
    free(reduced_2);
    tmp1 = WIDTH / (int) pow(2.0, (double) 3);
    tmp2 = HEIGHT / (int) pow(2.0, (double) 3);
    writeVariableSizeBWFile("../Images/Stage43_Centroid.ppm", reduced_3, tmp1, tmp2);    

     
    //Layer 4 AND gate Reduction
    bool* reduced_4 = AND_reduction(reduced_3, 4);
    free(reduced_3);

    unsigned int r_height = HEIGHT / (int) pow(2.0, (double) 4);
    unsigned int r_width  = WIDTH / (int) pow(2.0, (double) 4);
    writeVariableSizeBWFile("../Images/Stage44_Centroid.ppm", reduced_4, r_width, r_height);
    

    //Find the Largest Filled Region
    r_height = (unsigned int) r_height;
    r_width = (unsigned int) r_width;
    printf("height: %d width: %d\n", r_height, r_width);
    int center_num_lit = 0;
    *x = 0;
    *y = 0;    

    //Let's return the top 4 options
    int x_s[NUM_TO_TARGET];
    int y_s[NUM_TO_TARGET];
    int cnum_lit[NUM_TO_TARGET];
    for(int i = 0; i < NUM_TO_TARGET; i++) cnum_lit[i] = 0;


    for(int row = 1; row < r_height-1; row = row + 3){
        for(int col = 1; col < r_width-1; col = col + 3){
            
            int num_lit_pix = 0;

            int p_row_ind   = ((row-1) * r_width) + col;
            int cur_row_ind = (row * r_width) + col;
            int n_row_ind   = ((row+1) * r_width) + col;

            if(reduced_4[p_row_ind-1])    num_lit_pix += 1;
            if(reduced_4[p_row_ind])      num_lit_pix += 1;
            if(reduced_4[p_row_ind+1])    num_lit_pix += 1;

            if(reduced_4[cur_row_ind-1])    num_lit_pix += 1;
            if(reduced_4[cur_row_ind])      num_lit_pix += 1;
            if(reduced_4[cur_row_ind+1])    num_lit_pix += 1;

            if(reduced_4[n_row_ind-1])    num_lit_pix += 1;
            if(reduced_4[n_row_ind])      num_lit_pix += 1;
            if(reduced_4[n_row_ind+1])    num_lit_pix += 1;

            if(num_lit_pix > center_num_lit){
                printf("update x: %d  y: %d   num_lit: %d  cen_num_lit: %d\n", col, row, num_lit_pix, center_num_lit);
                center_num_lit = num_lit_pix;
                add_target(x_s, y_s, cnum_lit, col, row, num_lit_pix);
            }
        }
    }    


    //Rescale x and y
    for(int i = 0; i < NUM_TO_TARGET; i++){
        x_s[i] = x_s[i] * 2 * 2 * 2 * 2;           
        y_s[i] = y_s[i] * 2 * 2 * 2 * 2;
    }
    
    *x = x_s[0];
    *y = y_s[0];
    return;
}
