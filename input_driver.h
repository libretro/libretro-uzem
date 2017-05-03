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

#ifndef INPUT_DRIVER_H
#define INPUT_DRIVER_H

#include <stdint.h>
#include <SDL2/SDL_events.h>

#define MAX_JOYSTICKS 2
#define NUM_JOYSTICK_BUTTONS 8
#define MAX_JOYSTICK_AXES 8
#define MAX_JOYSTICK_HATS 8

#define JOY_SNES_X 0
#define JOY_SNES_A 1
#define JOY_SNES_B 2
#define JOY_SNES_Y 3
#define JOY_SNES_LSH 6
#define JOY_SNES_RSH 7
#define JOY_SNES_SELECT 8
#define JOY_SNES_START 9

#define JOY_DIR_UP 1
#define JOY_DIR_RIGHT 2
#define JOY_DIR_DOWN 4
#define JOY_DIR_LEFT 8
#define JOY_DIR_COUNT 4
#define JOY_AXIS_UNUSED -1

#define JOY_MASK_UP 0x11111111
#define JOY_MASK_RIGHT 0x22222222
#define JOY_MASK_DOWN 0x44444444
#define JOY_MASK_LEFT 0x88888888

#ifndef JOY_ANALOG_DEADZONE
	#define JOY_ANALOG_DEADZONE 4096
#endif

#define NR_OF_BUTTONS 2

enum { JMAP_IDLE, JMAP_INIT, JMAP_BUTTONS, JMAP_AXES, JMAP_MORE_AXES, JMAP_DONE };

enum { NES_A, NES_B, PAD_SELECT, PAD_START, PAD_UP, PAD_DOWN, PAD_LEFT, PAD_RIGHT };
enum { SNES_B, SNES_Y, SNES_A=8, SNES_X, SNES_LSH, SNES_RSH };

typedef enum { NES_PAD, SNES_PAD, SNES_PAD2, SNES_MOUSE } pad_mode_t;

typedef struct input_driver {
	uint32_t buttons[NR_OF_BUTTONS];
	int mouse_scale;
	pad_mode_t pad_mode;
	int (*poll)(SDL_Event *event);
	int (*wait)(SDL_Event *event);
	int (*get_mouse_state)(int *mouse_dx, int *mouse_dy);
	bool (*joystick_init)();
	void (*joystick_shutdown)();
	int (*joystick_open)(int idx);
	void (*joystick_close)(int idx);
} input_driver_t;

input_driver_t * input_driver_get_default();

#endif
