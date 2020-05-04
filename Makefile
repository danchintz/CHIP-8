all: chip8

chip8: chip8.c
	gcc -o chip8 chip8.c -lSDL2 -lm

debug: clean
	gcc -o chip8 -g chip8.c -lSDL2 -lm

clean:
	rm -f chip8
