#include "antialiasing.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
double MAX_VAL = 2147483647;
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int resize_factor, rank, nProcesses;
image *aux_in, *aux_out;
image input, output;

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

    int start = rank * ceil((double)aux_out -> height/nProcesses);
	int end = MIN(aux_out -> height, (rank + 1) * ceil((double)aux_out -> height/nProcesses) );

    if(resize_factor == 0){

    } else if(resize_factor == 3){

        if(aux_out -> colored == 0) { //grayscale

            for(x = start * resize_factor, k = start; x < end * resize_factor && k < end; x += resize_factor, k++) {
                for(y = 0, z = 0; y < aux_in -> width && z < aux_out -> width; y += resize_factor, z++){
                    new_pixel = 0;
                   
                    for(i = x, q = 0; i < x + resize_factor && q < 3; i++, q++)
                        for(j = y, w = 0; j < y + resize_factor && w < 3; j++, w++){
                            new_pixel += aux_in -> pixels[i][j] * GaussianKernel[q][w];
                        }
                    aux_out -> pixels[k][z] = new_pixel / 16;
                }
            }
                       
        } else { //color

            for(x = start * resize_factor, k = start; x < end * resize_factor && k < end; x += resize_factor, k++) {
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
            }
        }

    } else if(resize_factor % 2 == 0){
        
        if(aux_out -> colored == 0) { //grayscale
            
            for(x = start * resize_factor, k = start ; x < end * resize_factor && k < end; x += resize_factor, k++) {
                for(y = 0, z = 0; y < aux_in -> width && z < aux_out -> width; y += resize_factor, z++){
                    new_pixel = 0;
                    
                    for(i = x; i < x + resize_factor; i++)
                        for(j = y; j < y + resize_factor; j++){
                            new_pixel += aux_in -> pixels[i][j];
                        }
                    aux_out -> pixels[k][z] = new_pixel / square_resize;
                }
            }
                       
        } else { //color
            
            for(x = start * resize_factor, k = start; x < end * resize_factor && k < end; x += resize_factor, k++) {
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


int main(int argc, char * argv[]) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    const int root = 0;
    unsigned char *pixelArray;

    //argv[1] input
	//argv[2] output
	//argv[3] resize_factor
	if(argc < 4) {
		printf("Incorrect number of arguments\n");
		exit(-1);
	}

	resize_factor = atoi(argv[3]);
    if (rank == root)
	    readInput(argv[1], &input);
    MPI_Bcast(&input.colored, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&input.width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&input.height, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&input.max_value, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(input.colored == 0){
        pixelArray = (unsigned char*) malloc(sizeof(unsigned char) * input.width * input.height);
    }
    else {
        pixelArray = (unsigned char*) malloc(sizeof(unsigned char) * input.width * input.height * 3);
    }

    if(rank == root){
        if(input.colored == 0){
            for(int i = 0; i < input.height; i++)
                for(int j = 0; j < input.width; j++)
                    pixelArray[i * input.width + j] = input.pixels[i][j];
        }
        else {
            for(int i = 0; i < input.height; i++)
                for(int j = 0; j < 3 * input.width; j++)
                    pixelArray[i * (3 * input.width) + j] = input.pixels[i][j];
        }
    }else {
        if(input.colored == 0)
            input.pixels = allocMatrix(input.height, input.width);
        else
            input.pixels = allocMatrix(input.height, 3 * input.width);
    }

    if(input.colored == 0)
        MPI_Bcast(pixelArray, input.width * input.height, MPI_UNSIGNED_CHAR, root, MPI_COMM_WORLD);
    else{
        int count, k;
        double picture_size = 3 * (double)input.width * (double)input.height;
        double total = 0;
        if (picture_size > MAX_VAL){
            count = picture_size / MAX_VAL;
            for(k = 0; k < count; k++){
                printf("De la: %lf pana la %lf\n", (double)k * MAX_VAL, (double)k * MAX_VAL + MAX_VAL - 1);
                MPI_Bcast(pixelArray + k * (int)MAX_VAL, (int)MAX_VAL, MPI_UNSIGNED_CHAR, root, MPI_COMM_WORLD);
		total += MAX_VAL;
            }
            printf("De la: %lf pana la %lf\n",  k * MAX_VAL, k * MAX_VAL + (picture_size - count * MAX_VAL) - 1);
            MPI_Bcast(pixelArray + count * (int)MAX_VAL, (int)(picture_size - count * MAX_VAL), MPI_UNSIGNED_CHAR, root, MPI_COMM_WORLD);
	    total += picture_size - count * MAX_VAL;
	    printf("verif: %lf, %lf\n", total, 3 * (double)input.width * (double)input.height);
        } else {
            MPI_Bcast(pixelArray, 3 * input.width * input.height, MPI_UNSIGNED_CHAR, root, MPI_COMM_WORLD);
        }
    }

    if(rank != root){
        if(input.colored == 0){
            for(int i = 0; i < input.height; i++)
                for(int j = 0; j < input.width; j++)
                    input.pixels[i][j] = pixelArray[i * input.width + j];
                    
        }
        else {
            for(long i = 0; i < input.height; i++)
                for(long j = 0; j < 3 * input.width; j++){
		    if (rank == 1){
		    	printf("%ld\n", i * (3 * input.width) + j);
                    }
                    input.pixels[i][j] = pixelArray[i * (3 * input.width) + j];
        	}
	}
    }

	struct timespec startTime, finishTime;
	double elapsed;

	clock_gettime(CLOCK_MONOTONIC, &startTime);
	resize(&input, &output);
	clock_gettime(CLOCK_MONOTONIC, &finishTime);

	elapsed = (finishTime.tv_sec - startTime.tv_sec);
	elapsed += (finishTime.tv_nsec - startTime.tv_nsec) / 1000000000.0;

	printf("%lf\n", elapsed);
    int start, end;
    if(rank != root){
        start = rank * ceil((double)aux_out -> height/nProcesses);
	    end = MIN(aux_out -> height, (rank + 1) * ceil((double)aux_out -> height/nProcesses) );

        if(input.colored == 0){
            for(int i = start; i < end; i++)
                for(int j = 0; j < aux_out -> width; j++)
                    pixelArray[i * aux_out -> width + j] = output.pixels[i][j];
        }
        else {
            for(int i = start; i < end; i++)
                for(int j = 0; j < 3 * aux_out -> width; j++)
                    pixelArray[i * (3 * aux_out -> width) + j] = output.pixels[i][j];
        }

        if(input.colored == 0){
            MPI_Send(pixelArray + (aux_out -> width * start), (end - start) * aux_out -> width, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
        }
        else{
            MPI_Send(pixelArray + (3 * aux_out -> width * start), (end - start) * (3*aux_out -> width), MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
        }
    } else {
        start = rank * ceil((double)aux_out -> height/nProcesses);
	    end = MIN(aux_out -> height, (rank + 1) * ceil((double)aux_out -> height/nProcesses) ); 
        if(input.colored == 0){
            for(int i = start; i < end; i++)
                for(int j = 0; j < aux_out -> width; j++)
                    pixelArray[i * aux_out -> width + j] = output.pixels[i][j];
        }
        else {
            for(int i = start; i < end; i++)
                for(int j = 0; j < 3 * aux_out -> width; j++)
                    pixelArray[i * (3 * aux_out -> width) + j] = output.pixels[i][j];
        }
        

        for(int i = 1; i < nProcesses; i++){
            start = i * ceil((double)aux_out -> height/nProcesses);
	        end = MIN(aux_out -> height, (i + 1) * ceil((double)aux_out -> height/nProcesses) );
            if(input.colored == 0)
                MPI_Recv(pixelArray + (output.width * start), (end - start) * output.width, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            else
                MPI_Recv(pixelArray + (3 * output.width * start), (end - start) * (3*output.width), MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        if(input.colored == 0){
            for(int i = 0; i < output.height; i++)
                for(int j = 0; j < output.width; j++){
                    output.pixels[i][j] = pixelArray[i * output.width + j];
                }
                    
        }
        else {
            for(int i = 0; i < output.height; i++)
                for(int j = 0; j < 3 * output.width; j++){
                    output.pixels[i][j] = pixelArray[i * (3 * output.width) + j];
                }
        }     
    }

    if(rank == root)
	    writeData(argv[2], &output);
    MPI_Finalize();
	return 0;
}
