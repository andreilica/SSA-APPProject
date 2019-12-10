all: antialiasing

antialiasing: antialiasing.c antialiasing.h
	mpicc -o antialiasing antialiasing.c -lm -Wall

.PHONY: clean
clean:
	rm -f antialiasing
	rm -rf out/*
