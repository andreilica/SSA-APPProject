all: antialiasing

antialiasing: antialiasing.c antialiasing.h main.c
	gcc -o antialiasing antialiasing.c main.c -fopenmp -Wall -lm

.PHONY: clean
clean:
	rm -f antialiasing
	rm -rf out/*
