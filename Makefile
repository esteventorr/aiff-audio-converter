# Compile a simple C program

main: src/dmc.c
	gcc -g src/dmc.c src/selectView.c -o dmc 

ui: src/uiTest.c
	gcc -o ui src/progressBar.c src/progressBarList.c src/selectView.c src/uiTest.c src/userInterface.c 

clean:
	rm -f main

.PHONY: all

all: clean main
