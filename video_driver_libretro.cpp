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

#include "avr8.h"
#include "video_driver.h"
#include "compiler.h"

extern bool done_rendering;
extern bool half_width;
extern video_driver_t video_driver_libretro;

static bool video_driver_libretro_init(const char *caption, bool fullscreen, int sdl_flags)
{
   return true;
}

static uint32_t video_driver_libretro_map_rgb(uint8_t red, uint8_t green, uint8_t blue)
{
	uint32_t col = ((uint32_t)red) << 16 | ((uint32_t)green) << 8 | ((uint32_t)blue) ;
	return col;
}

// Renders a line into a 32 bit output buffer.
// Performs a shrink by 2
static void video_driver_libretro_render_line(unsigned int scanline, uint8_t *src, unsigned int spos, uint32_t *palette)
{
	if (half_width) {
		uint32_t *dest = (uint32_t *)((uint8_t *)video_driver_libretro.framebuffer + scanline * video_driver_libretro.stride * 4);
		for (unsigned int i = 0; i < VIDEO_DISP_WIDTH / 2; i++)
			dest[i] = palette[src[((i << 2) + spos) & 0x7FFU]];
	} else {
		uint32_t *dest = (uint32_t *)((uint8_t *)video_driver_libretro.framebuffer + scanline * video_driver_libretro.stride * 4);
		for (unsigned int i = 0; i < VIDEO_DISP_WIDTH; i++)
			dest[i] = palette[src[((i << 1) + spos) & 0x7FFU]];
	}
}

static void video_driver_libretro_update_frame()
{
   done_rendering = true;
}

static void video_driver_libretro_update_mouse()
{
}

static void video_driver_libretro_record_frame(FILE* avconv_video)
{
}

static void video_driver_libretro_screenshot(char *filename)
{
}

video_driver_t video_driver_libretro = {
	INIT_FIELD(width, 720),
	INIT_FIELD(height, 448),
	INIT_FIELD(framebuffer, NULL),
	INIT_FIELD(stride, 0),
	INIT_FIELD(init, video_driver_libretro_init),
	INIT_FIELD(map_rgb, video_driver_libretro_map_rgb),
	INIT_FIELD(render_line, video_driver_libretro_render_line),
	INIT_FIELD(update_frame, video_driver_libretro_update_frame),
	INIT_FIELD(record_frame, video_driver_libretro_record_frame),
	INIT_FIELD(screenshot, video_driver_libretro_screenshot),
	INIT_FIELD(update_mouse, video_driver_libretro_update_mouse)
};
