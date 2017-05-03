/*
(The MIT License)

Copyright (c) 2016 by Torsten Paul

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "audio_driver.h"

// If you're building from the command line or on a non-MS compiler you'll need
// -lSDL or somesuch.
#include <SDL2/SDL.h>
#if defined (_MSC_VER)
	#pragma comment(lib, "SDL.lib")
	#pragma comment(lib, "SDLmain.lib")
#endif

class ringBuffer
{
public:
	ringBuffer(int s) : head(0), tail(0), avail(s), size(s),last(127)
	{
		buffer = new uint8_t[size];
	}
	~ringBuffer()
	{
		delete[] buffer;
	}
	bool isFull() const
	{
		return avail == 0;
	}
	void push(uint8_t data)
	{
		if (avail)
		{
			buffer[head++] = data;
			if (head == size)
				head = 0;
			--avail;
		}
	}
	bool isEmpty() const
	{
		return avail == size;
	}
	int getUsed() const
	{
		return size - avail;
	}
	uint8_t pop()
	{
		if (avail != size)
		{
			++avail;
			uint8_t result = buffer[tail++];
			last=result;
			if (tail == size) tail = 0;
			return result;
		}
		else
			return last;//128;
	}
private:
	int head, tail, size, avail;
	uint8_t *buffer;
	uint8_t last;
};

static ringBuffer audio_buffer(1024);

typedef struct audio_driver_sdl_data {	
} audio_driver_sdl_data_t;

static audio_driver_sdl_data_t data;

// SDL callback
static void audio_callback(void *userdata, Uint8 *stream, int len)
{
	// printf("want %d bytes (have %d)\n",len,audioRing.getUsed());
	while (len--) {
		*stream++ = audio_buffer.pop();
	}
}

bool audio_driver_sdl_init()
{
	// Open audio driver
	SDL_AudioSpec desired;
	memset(&desired, 0, sizeof(desired));
	desired.freq = AUDIO_FREQUENCY;
	desired.format = AUDIO_U8;
	desired.callback = audio_callback;
	desired.userdata = 0;
	desired.channels = 1;
	desired.samples = 512;

	if (SDL_OpenAudio(&desired, NULL) < 0)
	{
		fprintf(stderr, "Unable to open audio device, no sound will play.\n");
		return false;
	}

	SDL_PauseAudio(0);
	return true;
}

static void audio_driver_sdl_flush()
{
	while (audio_buffer.isFull())
	{
		SDL_Delay(1);
	}
}

static void audio_driver_sdl_push(uint8_t value)
{
	SDL_LockAudio();
	audio_buffer.push(value);
	SDL_UnlockAudio();
}

audio_driver_t audio_driver_sdl = {
    .init = audio_driver_sdl_init,
	.flush = audio_driver_sdl_flush,
	.push = audio_driver_sdl_push
};
