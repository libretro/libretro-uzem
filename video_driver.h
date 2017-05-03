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

#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <stdio.h>
#include <stdint.h>

typedef struct video_driver {
	unsigned int width;
	unsigned int height;
	uint32_t *framebuffer;
	uint32_t stride;
	bool (*init)(const char *caption, bool fullscreen, int sdl_flags);
	uint32_t (*map_rgb)(uint8_t red, uint8_t green, uint8_t blue);
	void (*render_line)(unsigned int scanline, uint8_t *src, unsigned int spos, uint32_t *palette);
	void (*update_frame)();
	void (*record_frame)(FILE *avconv_video);
	void (*screenshot)(char *filename);
	void (*update_mouse)();
	void (*set_title)(char *title);
} video_driver_t;

video_driver_t * video_driver_get_default();

#endif
