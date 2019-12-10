#include "antialiasing.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <omp.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define NUM_THREADS 4

int resize_factor;
image *aux_in, *aux_out;

unsigned char** allocMatrix(int n, int m) {

    unsigned char** matrix;

    matrix = (unsigned char**) malloc(sizeof(unsigned char *) * n);
    if(matrix == NULL) {
        exit(1);
    }

    int i;

    for(i = 0; i < n; i++) {
        matrix[i] = (unsigned char*) malloc(sizeof(unsigned char) * m);
        if(matrix[i] == NULL) {
            exit(1);
        }
    }

    return(matrix);
}


void readInput(const char * fileName, image *img) {

    FILE *filePointer;

    char line[50];
    char *token;
    int linepixelnum;

    filePointer = fopen(fileName, "rb");
    if(filePointer == NULL){
        printf("File not found!\n");
        exit(1);
    }
    if(fgets(line, sizeof(line), filePointer) == NULL)
        exit(1);
    if(line[1] - '0' == 5)
        img -> colored = 0;
    if(line[1] - '0' == 6)
        img -> colored = 1;

    if(fgets(line, sizeof(line), filePointer) == NULL)
        exit(1);
    if( (token = strtok(line, " \n")) != NULL ) {
        img -> width = atoi(token);
        token = strtok(NULL, " \n");
        img -> height = atoi(token);
    }

    do{
        if(fgets(line, sizeof(line), filePointer) == NULL)
            exit(1);
    }while(!isdigit(line[0]));
        img -> max_value = atoi(line); 

    if(img -> colored){
        linepixelnum = img -> width * 3;
        img -> pixels = allocMatrix(img -> height, linepixelnum);
        for(int i = 0; i < img -> height; i++){
            fread(img -> pixels[i], sizeof(unsigned char), linepixelnum, filePointer);
        }

    } else {
        linepixelnum = img -> width;
        img -> pixels = allocMatrix(img -> height, linepixelnum);
        for(int i = 0; i < img -> height; i++){
            fread(img -> pixels[i], sizeof(unsigned char), linepixelnum, filePointer);
        }
    }
    
    fclose(filePointer);
}

void writeData(const char * fileName, image *img) {

    FILE *filePointer;
    int linepixelnum;
    
    filePointer = fopen(fileName, "wb");
    if(filePointer == NULL){
        printf("Cannot create file!\n");
        exit(1);
    }

    if(img -> colored){
        linepixelnum = img -> width * 3;
        
        fprintf(filePointer, "P6\n");
        fprintf(filePointer, "%d %d\n", img -> width, img -> height);
        fprintf(filePointer, "%d\n", img -> max_value);

        for(int i = 0; i <  img -> height; i++){
            fwrite(img -> pixels[i], sizeof(unsigned char), linepixelnum, filePointer);
        }

    } else {
        linepixelnum = img -> width;

        fprintf(filePointer, "P5\n");
        fprintf(filePointer, "%d %d\n", img -> width, img -> height);
        fprintf(filePointer, "%d\n", img -> max_value);

        for(int i = 0; i < img -> height; i++){
            fwrite(img -> pixels[i], sizeof(unsigned char), linepixelnum, filePointer);
        }
    }

    fclose(filePointer);
}

void* antialiasing()
{
    int i, j, x, k, q, z, y, w;
    int new_pixel = 0, red = 0, green = 0, blue = 0;
    int square_resize = resize_factor * resize_factor;
    int GaussianKernel[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};

    omp_set_num_threads(NUM_THREADS);

    double start = 0, end = aux_out -> height;

    if(resize_factor == 0){

    } else if(resize_factor == 3){

        if(aux_out -> colored == 0) { //grayscale

            k = start;
            #pragma omp task private(x,y,z,i,j,new_pixel,q,w) firstprivate(k)
            for(x = start * resize_factor; x < (long)end * resize_factor; x += resize_factor) {
                for(y = 0, z = 0; y < aux_in -> width && z < aux_out -> width; y += resize_factor, z++){
                    new_pixel = 0;
                   
                    for(i = x, q = 0; i < x + resize_factor && q < 3; i++, q++)
                        for(j = y, w = 0; j < y + resize_factor && w < 3; j++, w++){
                            new_pixel += aux_in -> pixels[i][j] * GaussianKernel[q][w];
                        }
                    aux_out -> pixels[k][z] = new_pixel / 16;
                }

            #pragma omp critical
            {
                if (k == end) {
                   #pragma omp taskwait
                   x = end * resize_factor;
                }
                else k++;
            }

            }
                       
        } else { //color
            
                k = start;
                #pragma omp task private(x,y,z,i,j,red,green,blue,q,w) firstprivate(k)
                for(x = start * resize_factor; x < (long)end * resize_factor; x+=resize_factor) {
                    for(y = 0, z = 0; y < aux_in -> width * 3 && z < aux_out -> width * 3; y += 3 * resize_factor, z+= 3){
                        red = 0;
                        green = 0;
                        blue = 0;
                        for(i = x, q = 0; i < x + resize_factor && q < 3; i++, q++)
                            for(j = y, w = 0; j < y + 3 * resize_factor && w < 3; j+= 3, w++){
                                red += aux_in -> pixels[i][j] * GaussianKernel[q][w];
                              
                            }
                       
                        for(i = x, q = 0; i < x + resize_factor && q < 3; i++, q++)
                            for(j = y + 1, w = 0; j < y + 3 * resize_factor && w < 3; j+= 3, w++){
                                green += aux_in -> pixels[i][j] * GaussianKernel[q][w];
                              
                            }
                        
                        for(i = x, q = 0; i < x + resize_factor && q < 3; i++, q++)
                            for(j = y + 2, w = 0; j < y + 3 * resize_factor && w < 3; j+= 3, w++){
                                blue += aux_in -> pixels[i][j] * GaussianKernel[q][w];
                                
                            }
                        
                        
                       aux_out -> pixels[k][z] = red / 16;
                       aux_out -> pixels[k][z + 1] = green / 16;
                       aux_out -> pixels[k][z + 2] = blue / 16;
                    }

                    #pragma omp critical
                    {
                    if (k == end){ 
                        #pragma omp taskwait
                        x = end * resize_factor;
                    }
                    else k++;
                    }
            }
        }

    } else if(resize_factor % 2 == 0){
        
        if(aux_out -> colored == 0) { //grayscale

            k = start;
            #pragma omp task private(x,y,z,i,j,new_pixel) firstprivate(k)
            for(x = start * resize_factor; x < (long)end * resize_factor; x += resize_factor) {
                for(y = 0, z = 0; y < aux_in -> width && z < aux_out -> width; y += resize_factor, z++){
                    new_pixel = 0;
                    
                    for(i = x; i < x + resize_factor; i++)
                        for(j = y; j < y + resize_factor; j++){
                            new_pixel += aux_in -> pixels[i][j];
                        }
                    aux_out -> pixels[k][z] = new_pixel / square_resize;
                }

                #pragma omp critical
                {
                    if (k == end){
                        #pragma omp taskwait
                        x = end * resize_factor;
                    }
                    else k++;
                }
            }
                       
        } else { //color
            k = start;
            #pragma omp task private(x,y,z,i,j,red,green,blue,q,w) firstprivate(k)
            for(x = start * resize_factor; x < (long)end * resize_factor; x += resize_factor) {
                for(y = 0, z = 0; y < aux_in -> width * 3 && z < aux_out -> width * 3; y += 3 * resize_factor, z+= 3){
                    red = 0;
                    green = 0;
                    blue = 0;
                    for(i = x; i < x + resize_factor; i++)
                        for(j = y; j < y + 3 * resize_factor; j+= 3){
                            red += aux_in -> pixels[i][j];
                            
                        }
                    
                    for(i = x; i < x + resize_factor; i++)
                        for(j = y + 1; j < y + 3 * resize_factor; j+= 3){
                            green += aux_in -> pixels[i][j];
                            
                        }
                    
                    for(i = x; i < x + resize_factor; i++)
                        for(j = y + 2; j < y + 3 * resize_factor; j+= 3){
                            blue += aux_in -> pixels[i][j];
                           
                        }
                    aux_out -> pixels[k][z] = red / square_resize;
                    aux_out -> pixels[k][z + 1] = green / square_resize;
                    aux_out -> pixels[k][z + 2] = blue / square_resize;
                }

                #pragma omp critical
                {
                    if (k == end){
                        #pragma omp taskwait
                        x = end * resize_factor;
                    }
                    else k++;
                }
            }
        }
                
    } else {
        exit(1);
    }

    return NULL;
}

void resize(image *in, image *out) { 

    int i, j;
    aux_in = in;
    aux_out = out;
        
    aux_out -> colored = aux_in -> colored;
    aux_out -> max_value = aux_in -> max_value; 

    if(resize_factor == 0){
        aux_out -> width = in -> width;
        aux_out -> height = in -> height;
        aux_out -> pixels = allocMatrix( aux_out -> height, aux_out -> width);

        for(i = 0; i < aux_in -> height; i++)
            for(j = 0; j < aux_in -> width; j++)
                aux_out -> pixels[i][j] = aux_in -> pixels[i][j];

    } else if(resize_factor == 3){

        if(aux_out -> colored == 0) { //grayscale
            aux_out -> width = aux_in -> width / resize_factor;
            aux_out -> height = aux_in -> height / resize_factor;
            aux_out -> pixels = allocMatrix( aux_out -> height, aux_out -> width);
                       
        } else { //color
            aux_out -> width = aux_in -> width / resize_factor;
            aux_out -> height = aux_in -> height / resize_factor;
            aux_out -> pixels = allocMatrix( aux_out -> height, aux_out -> width * 3);
            
        }

    } else if(resize_factor % 2 == 0){
        
        if(aux_out -> colored == 0) { //grayscale
            aux_out -> width = aux_in -> width / resize_factor;
            aux_out -> height = aux_in -> height / resize_factor;
            aux_out -> pixels = allocMatrix( aux_out -> height, aux_out -> width);
                       
        } else { //color
            aux_out -> width = aux_in -> width / resize_factor;
            aux_out -> height = aux_in -> height / resize_factor;
            aux_out -> pixels = allocMatrix( aux_out -> height, aux_out -> width * 3);
        }
                
    } else {
        exit(1);
    }

    antialiasing();
    
}