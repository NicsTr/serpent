OBJECTS := serpent_algo.o main.o
CC := gcc
FLAGS := -Wall


.PHONY: default all clean

default: main
all: default

clean:
	rm -rf $(OBJECTS) main

%.o: %.c %.h
	${CC} ${FLAGS} -c -o $@ $<

main: $(OBJECTS)
	${CC} ${FLAGS} -o $@ $^
