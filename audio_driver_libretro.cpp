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

#include "libretro.h"
#include "audio_driver.h"

extern retro_audio_sample_t audio_cb;

static bool audio_driver_libretro_init()
{
}

static void audio_driver_libretro_flush()
{
}

static void audio_driver_libretro_push(uint8_t value)
{
	int16_t sample = 128 * (((int16_t)value) - 128);
	audio_cb(sample, sample);
}

audio_driver_t audio_driver_libretro = {
    .init = audio_driver_libretro_init,
	.flush = audio_driver_libretro_flush,
	.push = audio_driver_libretro_push
};
