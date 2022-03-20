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

extern bool done_rendering;
extern bool half_width;
extern video_driver_t video_driver_libretro;

static bool video_driver_libretro_init(const char *caption, bool fullscreen, int sdl_flags) { return true; }

#ifdef USE_RGB565
typedef uint16_t pixel_t;
#else
typedef uint32_t pixel_t;
#endif

static uint32_t video_driver_libretro_map_rgb(uint8_t red, uint8_t green, uint8_t blue)
{
#if defined(ABGR1555)
        return ((blue & 0xf8) << 7) | ((green & 0xf8) << 2) | ((red & 0xf8) >> 3);
#elif defined(USE_RGB565)
        return ((red & 0xf8) << 8) | ((green & 0xfc) << 3) | ((blue & 0xf8) >> 3);
#else
        return ((uint32_t)red) << 16 | ((uint32_t)green) << 8 | ((uint32_t)blue) ;
#endif
}

// Renders a line into a 32 bit output buffer.
// Performs a shrink by 2
static void video_driver_libretro_render_line(unsigned int scanline, uint8_t *src, unsigned int spos, uint32_t *palette)
{
	if (half_width)
   {
      unsigned int i;
      pixel_t *dest = (pixel_t *)((uint8_t *)video_driver_libretro.framebuffer + scanline * video_driver_libretro.stride * sizeof(pixel_t));
      for (i = 0; i < VIDEO_DISP_WIDTH / 2; i++)
         dest[i] = palette[src[((i << 2) + spos) & 0x7FFU]];
   }
   else
   {
      unsigned int i;
      pixel_t *dest = (pixel_t *)((uint8_t *)video_driver_libretro.framebuffer + scanline * video_driver_libretro.stride * sizeof(pixel_t));
      for (i = 0; i < VIDEO_DISP_WIDTH; i++)
         dest[i] = palette[src[((i << 1) + spos) & 0x7FFU]];
   }
}

static void video_driver_libretro_update_frame(void) { done_rendering = true; }
static void video_driver_libretro_update_mouse(void) { }
static void video_driver_libretro_record_frame(FILE* avconv_video) { }
static void video_driver_libretro_screenshot(char *filename) { }

video_driver_t video_driver_libretro = {
   720,                               /* width */
   448,                               /* height */
   NULL,                              /* framebuffer */
   0,                                 /* stride */
   video_driver_libretro_init,        /* init */
   video_driver_libretro_map_rgb,     /* map_rgb */
	video_driver_libretro_render_line, /* render_line */
	video_driver_libretro_update_frame,/* update_frame */
	video_driver_libretro_record_frame,/* record_frame */
	video_driver_libretro_screenshot,  /* screenshot */
	video_driver_libretro_update_mouse,/* update_mouse */
   NULL                               /* set_title */
};
