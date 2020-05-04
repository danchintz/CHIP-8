# CHIP-8 Intepreter

## Compiling

This depends on

- SDL2

git clone git@github.com:danchintz/CHIP-8.git
cd CHIP-8
make

## Usage

```
./chip8 <rom file>
```


### TODO
 - Add better timing for delay and sound timers (Not even sure what the clock rate of the CHIP-8 was
 - Add key blocking op code, still trying to think of a good way to do it
 - Write a standalone assembler for it
