# Compile a simple C program

main: src/dmc.c
	gcc src/dmc.c -o dmc

clean:
	rm -f main

.PHONY: all

all: clean main