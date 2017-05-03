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

#include <stdio.h>
#include "input_driver.h"

#include "libretro.h"

extern input_driver_t input_driver_libretro;

extern retro_input_poll_t input_poll_cb;
extern retro_input_state_t input_state_cb;

typedef struct {
	unsigned retro_id;
	uint32_t button_bit;
} button_lookup_t;

static button_lookup_t button_lookup[] = {
	{ RETRO_DEVICE_ID_JOYPAD_LEFT,   1 << PAD_LEFT },
	{ RETRO_DEVICE_ID_JOYPAD_RIGHT,  1 << PAD_RIGHT },
	{ RETRO_DEVICE_ID_JOYPAD_UP,     1 << PAD_UP },
	{ RETRO_DEVICE_ID_JOYPAD_DOWN,   1 << PAD_DOWN },
	{ RETRO_DEVICE_ID_JOYPAD_SELECT, 1 << PAD_SELECT },
	{ RETRO_DEVICE_ID_JOYPAD_START,  1 << PAD_START },
	{ RETRO_DEVICE_ID_JOYPAD_A,      1 << SNES_A },
	{ RETRO_DEVICE_ID_JOYPAD_B,      1 << SNES_B },
	{ RETRO_DEVICE_ID_JOYPAD_X,      1 << SNES_X },
	{ RETRO_DEVICE_ID_JOYPAD_Y,      1 << SNES_Y },
	{ RETRO_DEVICE_ID_JOYPAD_L,      1 << SNES_LSH },
	{ RETRO_DEVICE_ID_JOYPAD_R,      1 << SNES_RSH },
	{ 0, 0 },
};

static void poll_input()
{
	input_poll_cb();

	input_driver_libretro.buttons[0] = 0;
	for (int a = 0;button_lookup[a].button_bit != 0;a++) {
		if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, button_lookup[a].retro_id)) {
			input_driver_libretro.buttons[0] |= button_lookup[a].button_bit;
		}
	}
	input_driver_libretro.buttons[0] = ~input_driver_libretro.buttons[0];
}

static int input_driver_libretro_poll(SDL_Event *event)
{
	poll_input();
	return 0;
}

static int input_driver_libretro_wait(SDL_Event *event)
{
	poll_input();
	return 0;
}

static int input_driver_libretro_get_mouse_state(int *mouse_dx, int *mouse_dy)
{
	return 0;
}

static bool input_driver_libretro_joystick_init()
{
	return true;
}

static void input_driver_libretro_joystick_shutdown()
{
}

static int input_driver_libretro_joystick_open(int idx)
{
	return idx;
}

static void input_driver_libretro_joystick_close(int idx)
{
}

input_driver_t input_driver_libretro = {
	.buttons = { 0 },
	.mouse_scale = 1,
	.pad_mode = SNES_PAD,
    .poll = input_driver_libretro_poll,
	.wait = input_driver_libretro_wait,
	.get_mouse_state = input_driver_libretro_get_mouse_state,
	.joystick_init = input_driver_libretro_joystick_init,
	.joystick_shutdown = input_driver_libretro_joystick_shutdown,
	.joystick_open = input_driver_libretro_joystick_open,
	.joystick_close = input_driver_libretro_joystick_close,
};
