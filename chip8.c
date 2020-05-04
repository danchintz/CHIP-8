#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <SDL2/SDL.h>

const int AMPLITUDE = 2800;
const int SAMPLE_RATE = 44100;

const int SCALE = 8;
const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

unsigned char MEMORY[4096] = {
	0xF0,0x90,0x90,0x90,0xF0,
	0x20,0x60,0x20,0x20,0x70,
	0xF0,0x10,0xF0,0x80,0xF0,
	0xF0,0x10,0xF0,0x10,0xF0,
	0x90,0x90,0xF0,0x10,0x10,
	0xF0,0x80,0xF0,0x10,0xF0,
	0xF0,0x80,0xF0,0x90,0xF0,
	0xF0,0x10,0x20,0x40,0x40,
	0xF0,0x90,0xF0,0x90,0xF0,
	0xF0,0x90,0xF0,0x10,0xF0,
	0xF0,0x90,0xF0,0x90,0x90,
	0xE0,0x90,0xE0,0x90,0xE0,
	0xF0,0x80,0x80,0x80,0xF0,
	0xE0,0x90,0x90,0x90,0xE0,
	0xF0,0x80,0xF0,0x80,0xF0,
	0xF0,0x80,0xF0,0x80,0x80,
};

unsigned char V[16];

unsigned char KEYS[16];
unsigned char SPRITES[16]={0,1*5,2*5,3*5,4*5,5*5,6*5,7*5,8*5,9*5,10*5,11*5,12*5,13*5,14*5,15*5};

unsigned short I;
unsigned int PC=0x200;

SDL_Window  *window;
SDL_Surface *windowSurface;
SDL_Surface *surface;

unsigned int stack[64] = {0};
unsigned int sp = 0;

unsigned char delay, sound;

SDL_Window *init() {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		exit(1);
	}

	return SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH*SCALE, SCREEN_HEIGHT*SCALE, SDL_WINDOW_SHOWN);
}

void audio_callback(void *user_data, Uint8 *raw_buffer, int bytes)
{
	Sint16 *buffer = (Sint16*)raw_buffer;
	int length = bytes / 2; // 2 bytes per sample for AUDIO_S16SYS
	int *sample_nr = ((int*)user_data);

	for(int i = 0; i < length; i++, sample_nr++)
	{
		double time = (double)*sample_nr / (double)SAMPLE_RATE;
		buffer[i] = (Sint16)(AMPLITUDE * sin(2.0f * M_PI * 441.0f * time)); // render 441 HZ sine wave
	}
}
void push(unsigned int val) {
	stack[sp++] = val;
}

unsigned int pop() {
	return stack[--sp];
}


void draw(unsigned char X, unsigned char Y, unsigned char H) {
	unsigned char *start = MEMORY+I;
	V[0xF] = 0;
	for(int y0 = 0;y0<H;y0++) {
		char current = *(start++);
		/*if(y0+Y > 31)break;*/
		for(int x0 = 0;x0<8;x0++) {
			int screenIndex = ((7-x0+X))+((y0+Y))*SCREEN_WIDTH;
			/*if(x0+X > 63)break;*/
			unsigned int *pixel = (unsigned int *)(surface->pixels)+screenIndex;
			if((current >> x0) & 1){
				if(*pixel) V[0xF] = 1;
				*pixel ^= 0xffffffff;
			}
		}
	}
}

void loop() {

	SDL_Event event;
	unsigned int opcode;
	unsigned int X,Y;
	unsigned int N, NN, NNN;

	char blocking = 0;
	char key = 0;

	int i =0;

	while (1) {
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP){
				int state = event.key.state;
				int sym = event.key.keysym.sym;
				if(sym == '0') KEYS[0] = state;
				if(sym == '1') KEYS[1] = state;
				if(sym == '2') KEYS[2] = state;
				if(sym == '3') KEYS[3] = state;
				if(sym == '4') KEYS[4] = state;
				if(sym == '5') KEYS[5] = state;
				if(sym == '6') KEYS[6] = state;
				if(sym == '7') KEYS[7] = state;
				if(sym == '8') KEYS[8] = state;
				if(sym == '9') KEYS[9] = state;
				if(sym == 'a') KEYS[0xA] = state;
				if(sym == 'b') KEYS[0xB] = state;
				if(sym == 'c') KEYS[0xC] = state;
				if(sym == 'd') KEYS[0xD] = state;
				if(sym == 'e') KEYS[0xE] = state;
				if(sym == 'f') KEYS[0xF] = state;
			}
			if(event.type == SDL_QUIT)exit(0);
		}

		opcode = MEMORY[PC]<<8|MEMORY[PC+1];


		X = opcode & 0x0F00;
		Y = opcode & 0x00F0;
		N = opcode & 0x000F;
		NN = Y|N;
		NNN = X|NN;

		Y >>= 4;
		X >>= 8;


		/*printf("Processing Instruction %x, switch %x, X=%x, Y=%x, N=%x, NN=%x, NNN=%x\n", opcode, opcode>>12, X,Y,N,NN,NNN);*/

		PC+=2;
		switch(opcode >> 12) {
			case 0x0:
				switch(NN) {
					case 0xE0:
						for(int i =0;i<surface->w*surface->h;i++)
							((unsigned int *)surface->pixels)[i]=0;
						break;
					case 0xEE:
						PC = pop();
						break;
				}
				break;
			case 0x1:
				PC = NNN;
				break;
			case 0x2:
				push(PC);
				PC = NNN;
				break;
			case 0x3:
				if(V[X] == NN) PC+=2;
				break;
			case 0x4:
				if(V[X] != NN) PC+=2;
				break;
			case 0x5:
				if(V[X] == V[Y]) PC+=2;
				break;
			case 0x6:
				V[X] = NN;
				break;
			case 0x7:
				V[X] += NN;
				break;
			case 0x8:
				switch(N) {
					case 0:
						V[X] = V[Y];
						break;
					case 1:
						V[X] = V[X] | V[Y];
						break;
					case 2:
						V[X] = V[X] & V[Y];
						break;
					case 3:
						V[X] = V[X] ^ V[Y];
						break;
					case 4:
						V[0xF] = V[X]+V[Y] < V[X];
						V[X] += V[Y];
						break;
					case 5:
						V[0xF] = V[X] > V[Y];
						V[X]-=V[Y];
						break;
					case 6:
						V[0xF] = V[Y] & 1;
						V[X] = V[Y] >> 1;
						break;
					case 7:
						V[0xF] = V[Y] > V[X];
						V[X]=V[Y]-V[X];
						break;
					case 0xE:
						V[0xF] = (V[Y] >> 7) & 1;
						V[X] = V[Y] << 1;
						break;

				}
				break;
			case 0x9:
				if(V[X]!=V[Y]) PC += 2;
				break;
			case 0xA:
				I = NNN;
				break;
			case 0xB:
				PC = V[0]+NNN;
				break;
			case 0xC:
				V[X]=(rand())&NN;
				break;
			case 0xD:
				draw(V[X], V[Y], N);
				break;
			case 0xE:
				switch(NN) {
					case 0x9E:
						if(KEYS[V[X]]) PC += 2;
						break;
					case 0xA1:
						if(!KEYS[V[X]]) PC += 2;
						break;
				}
				break;
			case 0xF:
				switch(NN) {
					case 0x07:
						V[X] = delay;
						break;
					case 0x0A:
						//TODO: Actually implement blocking
						/*blocking = 1;*/
						/*if(!key) PC -= 2;*/
						/*else {*/
							/*V[X] = key;*/
							/*blocking = key = 0;*/
						/*}*/
						break;
					case 0x15:
						delay = V[X];
						break;
					case 0x18:
						sound = V[X];
					case 0x1E:
						/*if(I + V[X] < I) V[0xF] = 1; else V[0xF] = 0;*/
						I += V[X];
						break;
					case 0x29:
						I = SPRITES[V[X]];
						break;
					case 0x33:
						MEMORY[I] = (V[X]/100) % 10;
						MEMORY[I+1] = (V[X]/10) % 10;
						MEMORY[I+2] = V[X]%10;
						break;
					case 0x55:
						for(;I<=X;I++) MEMORY[I]=V[I];
						break;
					case 0x64:
						for(;I<=X;I++) V[I] = MEMORY[I];
						break;
				}
				break;

		}
		if(delay>0)delay--;
		if(sound>0) {
			SDL_PauseAudio(0);
			sound--;
		}else SDL_PauseAudio(1);
		SDL_BlitScaled(surface, NULL, windowSurface, NULL);
		SDL_UpdateWindowSurface(window);
	}
}


int main(int argc, char **argv) {
	if(argc != 2) {
		fprintf(stderr, "usage %s <rom file>", argv[0]);
		exit(69);
	}

	int fd = open(argv[1], O_RDONLY);
	read(fd, MEMORY+0x200, 4096-0x200);
	close(fd);

	Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	window = init();

	int sample_nr = 0;

	SDL_AudioSpec want;
	want.freq = SAMPLE_RATE; // number of samples per second
	want.format = AUDIO_S16SYS; // sample type (here: signed short i.e. 16 bit)
	want.channels = 1; // only one channel
	want.samples = 2048; // buffer-size
	want.callback = audio_callback; // function SDL calls periodically to refill the buffer
	want.userdata = &sample_nr; // counter, keeping track of current sample number

	SDL_AudioSpec have;
	if(SDL_OpenAudio(&want, &have) != 0) SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to open audio: %s", SDL_GetError());
	if(want.format != have.format) SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to get the desired AudioSpec");
	windowSurface = SDL_GetWindowSurface(window);
	surface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
			rmask, gmask, bmask, amask);
	loop();
}
