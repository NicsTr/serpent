#!/bin/bash

gcc -Wall -c -o serpent.o serpent.c
gcc -Wall -c -o main.o main.c
gcc -Wall -o main main.o serpent.o
