all: chip8

chip8: chip8.o
	gcc -o chip8 chip8.o -lSDL2 -lm

chip8.o: chip8.c
	gcc -c chip8.c

clean:
	rm -f chip8.o chip8
