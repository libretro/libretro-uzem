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

// If you're building from the command line or on a non-MS compiler you'll need
// -lSDL or somesuch.
#include <SDL2/SDL.h>
#if defined (_MSC_VER)
	#pragma comment(lib, "SDL.lib")
	#pragma comment(lib, "SDLmain.lib")
#endif

#include "avr8.h"
#include "video_driver.h"
#include "logo.h"

typedef struct video_driver_sdl_data {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Surface *surface;
	SDL_Texture *texture;
} video_driver_sdl_data_t;

static video_driver_sdl_data_t data;

static bool video_driver_sdl_init(const char *caption, bool fullscreen, int sdl_flags)
{
	if ( SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0 )
	{
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		return false;
	}

	atexit(SDL_Quit);

	data.window = SDL_CreateWindow(caption,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,630,448,fullscreen?SDL_WINDOW_FULLSCREEN:SDL_WINDOW_RESIZABLE);
	if (!data.window){
		fprintf(stderr, "CreateWindow failed: %s\n", SDL_GetError());
		return false;
	}
	data.renderer = SDL_CreateRenderer(data.window, -1, sdl_flags);
	if (!data.renderer){
		SDL_DestroyWindow(data.window);
		fprintf(stderr, "CreateRenderer failed: %s\n", SDL_GetError());
		return false;
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	SDL_RenderSetLogicalSize(data.renderer, 630, 448);

	data.surface = SDL_CreateRGBSurface(0, VIDEO_DISP_WIDTH, 224, 32, 0, 0, 0, 0);
	if(!data.surface){
		fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
		return false;
	}

	data.texture = SDL_CreateTexture(data.renderer,data.surface->format->format,SDL_TEXTUREACCESS_STATIC,data.surface->w,data.surface->h);
	if (!data.texture){
		SDL_DestroyRenderer(data.renderer);
		SDL_DestroyWindow(data.window);
		fprintf(stderr, "CreateTexture failed: %s\n", SDL_GetError());
		return false;
	}

	SDL_RenderClear(data.renderer);
	SDL_RenderCopy(data.renderer, data.texture, NULL, NULL);
	SDL_RenderPresent(data.renderer);

	if (fullscreen)
	{
		SDL_ShowCursor(0);
	}

	//Set window icon
	SDL_Surface *slogo;
	slogo = SDL_CreateRGBSurfaceFrom((void *)&logo,32,32,32,32*4,0xFF,0xff00,0xff0000,0xff000000);
	SDL_SetWindowIcon(data.window,slogo);
	SDL_FreeSurface(slogo);
	return true;
}

static uint32_t video_driver_sdl_map_rgb(uint8_t red, uint8_t green, uint8_t blue)
{
	return SDL_MapRGB(data.surface->format, red, green, blue);
}

// Renders a line into a 32 bit output buffer.
// Performs a shrink by 2
static void video_driver_sdl_render_line(unsigned int scanline, uint8_t *src, unsigned int spos, uint32_t *palette)
{
	uint32_t *dest = (uint32_t *)((uint8_t *)data.surface->pixels + scanline * data.surface->pitch);
	for (unsigned int i = 0; i < VIDEO_DISP_WIDTH; i++)
		dest[i] = palette[src[((i << 1) + spos) & 0x7FFU]];
}

static void video_driver_sdl_update_frame()
{
	SDL_UpdateTexture(data.texture, NULL, data.surface->pixels, data.surface->pitch);
	SDL_RenderClear(data.renderer);
	SDL_RenderCopy(data.renderer, data.texture, NULL, NULL);
	SDL_RenderPresent(data.renderer);
}

static void video_driver_sdl_update_mouse()
{
	int tmp;
	SDL_WarpMouseInWindow(data.window, 400, 300);
	SDL_GetRelativeMouseState(&tmp, &tmp);
}

static void video_driver_sdl_record_frame(FILE* avconv_video)
{
	fwrite(data.surface->pixels, VIDEO_DISP_WIDTH*224*4, 1, avconv_video);
}

static void video_driver_sdl_screenshot(char *filename)
{
	SDL_Surface *surfBMP;
	const Uint8 *kbstate = SDL_GetKeyboardState(NULL);
	if (kbstate[SDL_SCANCODE_LSHIFT] || kbstate[SDL_SCANCODE_RSHIFT]) {
		surfBMP = SDL_CreateRGBSurface(0, 240, 224, 32, 0, 0, 0, 0);
	} else {
		surfBMP = SDL_CreateRGBSurface(0, 630, 448, 32, 0, 0, 0, 0);
	}

	if (!surfBMP) {
		fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
	} else {
		if (SDL_BlitScaled(data.surface, NULL, surfBMP, NULL) < 0) {
			fprintf(stderr, "BlitScaled failed: %s\n", SDL_GetError());
			fprintf(stderr, "There was a problem rescaling the screenshot, saving the unscaled version.\n");
			SDL_SaveBMP(data.surface, filename); // at least save the weirdly scaled one
		} else {
			SDL_SaveBMP(surfBMP, filename);
		}
		SDL_FreeSurface(surfBMP);
	}
}

static void video_driver_sdl_set_title(char *title)
{
	SDL_SetWindowTitle(data.window, title);
}

video_driver_t video_driver_sdl = {
    .width = 720,
    .height = 448,
    .framebuffer = 0,
    .stride = 0,
    .init = video_driver_sdl_init,
    .map_rgb = video_driver_sdl_map_rgb,
	.render_line = video_driver_sdl_render_line,
	.update_frame = video_driver_sdl_update_frame,
	.record_frame = video_driver_sdl_record_frame,
	.screenshot = video_driver_sdl_screenshot,
	.update_mouse = video_driver_sdl_update_mouse,
	.set_title = video_driver_sdl_set_title
};
