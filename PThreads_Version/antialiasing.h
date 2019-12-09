#ifndef HOMEWORK_H
#define HOMEWORK_H

typedef struct {
    int colored, width, height, max_value;
    unsigned char** pixels; 
}image;


void readInput(const char * fileName, image *img);

void writeData(const char * fileName, image *img);

void resize(image *in, image * out);

#endif /* HOMEWORK_H */