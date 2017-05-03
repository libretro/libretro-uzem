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

#define MAX_JOYSTICKS 2

extern input_driver_t input_driver_sdl;

static const char* joystickFile = NULL;

static const char* joySettingsFilename = "joystick-settings";

struct joyAxis { int axis; uint8_t bits; };
struct joystickState {
	int handle;
	struct joyButton *buttons;
	struct joyAxis axes[MAX_JOYSTICK_AXES];
	uint32_t hats; // 4 bits per hat (1 for each direction)
};

struct joyMapSettings {
	int jstate, jiter, jindex, jaxis;
};

joystickState joysticks[MAX_JOYSTICKS];
joyMapSettings jmap;

struct keymap { uint32_t key; uint8_t player, bit; };
#define END_OF_MAP { 0,0,0 }
keymap nes_one_player[] =
{
	{ SDLK_a, 0, NES_A }, { SDLK_s, 0, NES_B }, { SDLK_TAB, 0, PAD_SELECT }, { SDLK_RETURN, 0, PAD_START },
	{ SDLK_UP, 0, PAD_UP }, { SDLK_DOWN, 0, PAD_DOWN }, { SDLK_LEFT, 0, PAD_LEFT }, { SDLK_RIGHT, 0, PAD_RIGHT },
	END_OF_MAP
};
keymap snes_one_player[] =
{
	{ SDLK_s, 0, SNES_B }, { SDLK_z, 0, SNES_Y }, { SDLK_TAB, 0, PAD_SELECT }, { SDLK_RETURN, 0, PAD_START },
	{ SDLK_UP, 0, PAD_UP }, { SDLK_DOWN, 0, PAD_DOWN }, { SDLK_LEFT, 0, PAD_LEFT }, { SDLK_RIGHT, 0, PAD_RIGHT },
	{ SDLK_a, 0, SNES_A }, { SDLK_x, 0, SNES_X }, { SDLK_LSHIFT, 0, SNES_LSH }, { SDLK_RSHIFT, 0, SNES_RSH },
	END_OF_MAP
};

keymap snes_two_players[] =
{
   // P1
   { SDLK_a, 0, PAD_LEFT }, { SDLK_s, 0, PAD_DOWN }, { SDLK_d, 0, PAD_RIGHT }, { SDLK_w, 0, PAD_UP },
   { SDLK_q, 0, SNES_LSH }, { SDLK_e, 0, SNES_RSH }, { SDLK_r, 0, SNES_Y }, { SDLK_t, 0, SNES_X },
   { SDLK_f, 0, SNES_B }, { SDLK_g, 0, SNES_A }, { SDLK_z, 0, PAD_START }, { SDLK_x, 0, PAD_SELECT },
   // P2
   { SDLK_j, 1, PAD_LEFT }, { SDLK_k, 1, PAD_DOWN }, { SDLK_l, 1, PAD_RIGHT }, { SDLK_i, 1, PAD_UP },
   { SDLK_u, 1, SNES_LSH }, { SDLK_o, 1, SNES_RSH }, { SDLK_p, 1, SNES_Y }, { SDLK_LEFTBRACKET, 1, SNES_X },
   { SDLK_SEMICOLON, 1, SNES_B }, { SDLK_QUOTE, 1, SNES_A }, { SDLK_n, 1, PAD_START }, { SDLK_m, 1, PAD_SELECT },
   END_OF_MAP
};

keymap snes_mouse[] =
{
	END_OF_MAP
};

keymap *keymaps[] = { nes_one_player, snes_one_player, snes_two_players, snes_mouse };

struct joyButton { uint8_t button; uint8_t bit; };

// Joysticks
struct joyButton joy_btns_p1[] =
{
	{ JOY_SNES_START, PAD_START }, { JOY_SNES_SELECT, PAD_SELECT },
	{ JOY_SNES_A, SNES_A }, { JOY_SNES_B, SNES_B },
	{ JOY_SNES_X, SNES_X }, { JOY_SNES_Y, SNES_Y },
	{ JOY_SNES_LSH, SNES_LSH }, { JOY_SNES_RSH, SNES_RSH }
};

struct joyButton joy_btns_p2[] =
{
	{ JOY_SNES_START, PAD_START }, { JOY_SNES_SELECT, PAD_SELECT },
	{ JOY_SNES_A, SNES_A }, { JOY_SNES_B, SNES_B },
	{ JOY_SNES_X, SNES_X }, { JOY_SNES_Y, SNES_Y },
	{ JOY_SNES_LSH, SNES_LSH }, { JOY_SNES_RSH, SNES_RSH }
};

struct joyButton *joyButtons[] =  { joy_btns_p1, joy_btns_p2 };

typedef struct input_driver_sdl_js_data {
	SDL_Joystick *device;
} input_driver_sdl_js_data_t;

static input_driver_sdl_js_data_t jsdata[MAX_JOYSTICKS];

static void update_buttons(int key,bool down)
{
	keymap *k = keymaps[input_driver_sdl.pad_mode];
	while (k->key)
	{
		if (key == k->key)
		{
			if (down)
				input_driver_sdl.buttons[k->player] &= ~(1<<k->bit);
			else
				input_driver_sdl.buttons[k->player] |= (1<<k->bit);
			break;
		}
		++k;
	}
}

static uint8_t encode_delta(int d)
{
	uint8_t result;
	if (d < 0)
	{
		result = 0;
		d = -d;
	}
	else
		result = 1;
	if (d > 127)
		d = 127;
	if (!(d & 64))
		result |= 2;
	if (!(d & 32))
		result |= 4;
	if (!(d & 16))
		result |= 8;
	if (!(d & 8))
		result |= 16;
	if (!(d & 4))
		result |= 32;
	if (!(d & 2))
		result |= 64;
	if (!(d & 1))
		result |= 128;
	return result;
}

static void update_mouse(SDL_Event *event)
{
	if (input_driver_sdl.pad_mode == SNES_MOUSE) {
		// http://www.repairfaq.org/REPAIR/F_SNES.html
		// we always report "low sensitivity"
		int mouse_dx, mouse_dy;
		uint8_t mouse_buttons = input_driver_sdl.get_mouse_state(&mouse_dx, &mouse_dy);
		mouse_dx >>= input_driver_sdl.mouse_scale;
		mouse_dy >>= input_driver_sdl.mouse_scale;
		// clear high bit so we know it's the mouse
		input_driver_sdl.buttons[0] = (encode_delta(mouse_dx) << 24)
			| (encode_delta(mouse_dy) << 16) | 0x7FFF;
		if (mouse_buttons & SDL_BUTTON_LMASK)
			input_driver_sdl.buttons[0] &= ~(1 << 9);
		if (mouse_buttons & SDL_BUTTON_RMASK)
			input_driver_sdl.buttons[0] &= ~(1 << 8);
		// keep mouse centered so it doesn't get stuck on edge of screen.
		// ...and immediately consume the bogus motion event it generated.
//TODO
//		if (fullscreen) {
//			input_driver_sdl.update_mouse();
//		}
	} else {
		input_driver_sdl.buttons[0] |= 0xFFFF8000;
	}
}

static void set_jmap_state(int state)
{
	switch (state) {
		case JMAP_INIT:
			printf("Press START on the joystick you wish to re-map...");
			fflush(stdout);
			break;
		case JMAP_BUTTONS:
			jmap.jiter = 0;
			jmap.jaxis = JOY_AXIS_UNUSED;

			for (int i= 0; i < MAX_JOYSTICK_AXES; ++i)
				joysticks[jmap.jindex].axes[i].axis = JOY_AXIS_UNUSED;
			break;
		case JMAP_AXES:
			jmap.jiter = NUM_JOYSTICK_BUTTONS;
			jmap.jaxis = JOY_AXIS_UNUSED;
			printf("\nPress Left on axial input (hats are mapped automatically and won't register)...");
			fflush(stdout);
			break;
		case JMAP_MORE_AXES:
			jmap.jiter = NUM_JOYSTICK_BUTTONS+2;
			printf("\nMap an axial input (y/n)? ");
			fflush(stdout);
			break;
		case JMAP_DONE:
			jmap.jstate = JMAP_IDLE;
			joystickFile = joySettingsFilename;	// Save on exit
			printf("\nJoystick mappings complete.\n");
			fflush(stdout);
			state = JMAP_IDLE;
			break;
		default:
			break;
	}
	jmap.jstate = state;
}

static void map_joysticks(SDL_Event *ev)
{
	if (jmap.jstate == JMAP_MORE_AXES) {
		return;
	} else if (jmap.jstate == JMAP_INIT) { // Initialize mapping settings
		jmap.jindex = ev->jbutton.which;
		set_jmap_state(JMAP_BUTTONS);
	} else if (ev->jbutton.which != jmap.jindex) {
		return; // Ignore input from all joysticks but the one being mapped
	}

	if (jmap.jstate == JMAP_BUTTONS) {
		if (ev->jbutton.type == SDL_JOYBUTTONDOWN)
				joyButtons[ev->jbutton.which][jmap.jiter].button = ev->jbutton.button;
		else
			return;
	} else if (jmap.jstate == JMAP_AXES && ev->type == SDL_JOYAXISMOTION) {
		// Find index to place new axes
		if (jmap.jaxis == JOY_AXIS_UNUSED) {
			for (int i = 0; i < MAX_JOYSTICK_AXES; i+=2) {
				if (joysticks[jmap.jindex].axes[i].axis == JOY_AXIS_UNUSED || joysticks[jmap.jindex].axes[i].axis == ev->jaxis.axis) {
					joysticks[jmap.jindex].axes[i].axis = JOY_AXIS_UNUSED;
					joysticks[jmap.jindex].axes[i+1].axis = JOY_AXIS_UNUSED;
					jmap.jaxis = i;
					break;
				}
			}
		}

		if (joysticks[jmap.jindex].axes[jmap.jaxis].axis == JOY_AXIS_UNUSED) {
			if (ev->jaxis.value < -(2*JOY_ANALOG_DEADZONE))
				joysticks[jmap.jindex].axes[jmap.jaxis].axis = ev->jaxis.axis;
			else
				return;
		} else if (joysticks[jmap.jindex].axes[jmap.jaxis].axis != ev->jaxis.axis && ev->jaxis.value < -(2*JOY_ANALOG_DEADZONE)) {
			joysticks[jmap.jindex].axes[jmap.jaxis+1].axis = ev->jaxis.axis;
		} else {
			return;
		}
	} else {
		return;
	}

	if (++jmap.jiter == NUM_JOYSTICK_BUTTONS) {
		if (SDL_JoystickNumAxes(jsdata[jmap.jindex].device) == 0) {
			set_jmap_state(JMAP_DONE);
			return;
		} else {
			set_jmap_state(JMAP_MORE_AXES);
		}
	} else if (jmap.jiter == (NUM_JOYSTICK_BUTTONS+2)) {
		set_jmap_state(JMAP_MORE_AXES);
	}

	switch (jmap.jiter) {
		case 1: printf("\nPress SELECT..."); break;
		case 2: printf("\nPress A..."); break;
		case 3: printf("\nPress B..."); break;
		case 4: printf("\nPress X..."); break;
		case 5: printf("\nPress Y..."); break;
		case 6: printf("\nPress LShldr..."); break;
		case 7: printf("\nPress RShldr..."); break;
		case 9: printf("\nPress Up on axial input..."); break;
		default: break;
	}
	fflush(stdout);
}

static void update_joysticks(SDL_Event *ev)
{
	uint8_t axisBits = 0;

	if (ev->type == SDL_JOYAXISMOTION) {
		for (int i = 0; i < MAX_JOYSTICK_AXES; ++i) {
			if (joysticks[ev->jaxis.which].axes[i].axis == -1)
				break;
			if (joysticks[ev->jaxis.which].axes[i].axis != ev->jaxis.axis) {
				axisBits |= joysticks[ev->jaxis.which].axes[i].bits;
				continue;
			}
			if (i&1) {
				if (ev->jaxis.value < -JOY_ANALOG_DEADZONE) { // UP
					joysticks[ev->jaxis.which].axes[i].bits |= JOY_DIR_UP;
					joysticks[ev->jaxis.which].axes[i].bits &= ~JOY_DIR_DOWN;
				} else if (ev->jaxis.value > JOY_ANALOG_DEADZONE) { // DOWN
					joysticks[ev->jaxis.which].axes[i].bits |= JOY_DIR_DOWN;
					joysticks[ev->jaxis.which].axes[i].bits &= ~JOY_DIR_UP;
				} else {
					joysticks[ev->jaxis.which].axes[i].bits &= ~(JOY_DIR_UP|JOY_DIR_DOWN);
				}
			} else {
				if (ev->jaxis.value < -JOY_ANALOG_DEADZONE) { // LEFT
					joysticks[ev->jaxis.which].axes[i].bits |= JOY_DIR_LEFT;
					joysticks[ev->jaxis.which].axes[i].bits &= ~JOY_DIR_RIGHT;
				} else if (ev->jaxis.value > JOY_ANALOG_DEADZONE) { // RIGHT
					joysticks[ev->jaxis.which].axes[i].bits |= JOY_DIR_RIGHT;
					joysticks[ev->jaxis.which].axes[i].bits &= ~JOY_DIR_LEFT;
				} else {
					joysticks[ev->jaxis.which].axes[i].bits &= ~(JOY_DIR_LEFT|JOY_DIR_RIGHT);
				}
			}
			axisBits |= joysticks[ev->jaxis.which].axes[i].bits;
		}
	} else if (ev->type == SDL_JOYHATMOTION) {
		joysticks[ev->jhat.which].hats &= ~(0xf<<ev->jhat.hat);
		joysticks[ev->jhat.which].hats |= (ev->jhat.value<<ev->jhat.hat);
	} else if (ev->type == SDL_JOYBUTTONDOWN || ev->type == SDL_JOYBUTTONUP) {
		struct joyButton *j = joysticks[ev->jbutton.which].buttons;

		for (int i = 0; i < NUM_JOYSTICK_BUTTONS; ++i, ++j) {
			if (ev->jbutton.button == j->button) {
				if (ev->jbutton.type == SDL_JOYBUTTONUP)
					input_driver_sdl.buttons[ev->jaxis.which] |= (1<<j->bit);
				else
					input_driver_sdl.buttons[ev->jaxis.which] &= ~(1<<j->bit);
				break;
			}
		}
	}

	if (ev->type == SDL_JOYAXISMOTION || ev->type == SDL_JOYHATMOTION) {
		for (uint32_t i = JOY_MASK_UP, bit = PAD_UP; bit; i<<=1) {
			if ((axisBits&i) || (joysticks[ev->jaxis.which].hats&i))
				input_driver_sdl.buttons[ev->jaxis.which] &= ~(1<<bit);
			else
				input_driver_sdl.buttons[ev->jaxis.which] |= (1<<bit);
			if (bit == PAD_UP)
				bit = PAD_RIGHT;
			else if (bit == PAD_RIGHT)
				bit = PAD_DOWN;
			else if (bit == PAD_DOWN)
				bit = PAD_LEFT;
			else
				bit = 0;
		}
	}
}

static void handle_event(SDL_Event *event)
{
	static const char *pad_mode_strings[4] = {"NES pad.","SNES pad.","SNES 2p pad.","SNES mouse."};

	switch (event->type) {
	case SDL_KEYDOWN:
		if (jmap.jstate == JMAP_IDLE)
			update_buttons(event->key.keysym.sym, true);
		switch (event->key.keysym.sym) {
			case SDLK_5:
				if (input_driver_sdl.pad_mode == NES_PAD)
					input_driver_sdl.pad_mode = SNES_PAD;
				else if (input_driver_sdl.pad_mode == SNES_PAD)
					input_driver_sdl.pad_mode = SNES_PAD2;
				else if (input_driver_sdl.pad_mode == SNES_PAD2)
					input_driver_sdl.pad_mode = SNES_MOUSE;
				else
					input_driver_sdl.pad_mode = NES_PAD;
				puts(pad_mode_strings[input_driver_sdl.pad_mode]);
				break;
			case SDLK_7:
				if (jmap.jstate == JMAP_IDLE)
					set_jmap_state(JMAP_INIT);
				break;
			case SDLK_n:
				if (jmap.jstate == JMAP_MORE_AXES)
					set_jmap_state(JMAP_DONE);
				break;
			case SDLK_y:
				if (jmap.jstate == JMAP_MORE_AXES)
					set_jmap_state(JMAP_AXES);
				break;
		}
		break;
	case SDL_KEYUP:
		update_buttons(event->key.keysym.sym,false);
		break;
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
	case SDL_JOYAXISMOTION:
	case SDL_JOYHATMOTION:
	case SDL_JOYBALLMOTION:
		if (jmap.jstate != JMAP_IDLE)
			map_joysticks(event);
		else
			update_joysticks(event);
		break;
	}

	update_mouse(event);
}

static void load_joystick_file(const char* filename)
{
	bool validFile = true;
	FILE* f = fopen(filename,"rb");

	if (f) {
		size_t btnsSize = MAX_JOYSTICKS*NUM_JOYSTICK_BUTTONS*sizeof(struct joyButton);
		size_t axesSize = MAX_JOYSTICKS*MAX_JOYSTICK_AXES*sizeof(struct joyAxis);
		fseek(f,0,SEEK_END);

		size_t size = ftell(f);

		if (size < (btnsSize+axesSize)) {
			validFile = false;
		} else {
			fseek(f,0,SEEK_SET);
			size_t result;

			for (int i = 0; i < MAX_JOYSTICKS; ++i) {
				if (!(result = fread(joyButtons[i],NUM_JOYSTICK_BUTTONS*sizeof(struct joyButton),1,f)) ||
						!(result = fread(joysticks[i].axes,MAX_JOYSTICK_AXES*sizeof(struct joyAxis),1,f)))
					validFile = false;
			}
		}
		fclose(f);

		if (!validFile)
			printf("Warning: Invalid Joystick settings file.\n");
	}
}

static int input_driver_sdl_poll(SDL_Event *event)
{
	int ret = SDL_PollEvent(event);
	if (ret) {
		handle_event(event);
	}
	return ret;
}

static int input_driver_sdl_wait(SDL_Event *event)
{
	int ret = SDL_WaitEvent(event);
	if (ret) {
		handle_event(event);
	}
	return ret;
}

static int input_driver_sdl_get_mouse_state(int *mouse_dx, int *mouse_dy)
{
	return SDL_GetRelativeMouseState(mouse_dx, mouse_dy);
}

static bool input_driver_sdl_joystick_init()
{
	for (int a = 0;a < NR_OF_BUTTONS;a++)
	{
		input_driver_sdl.buttons[a] = ~0;
	}
	for (int a = 0;a < MAX_JOYSTICKS;a++)
	{
		jsdata[a].device = NULL;
	}
	
	if ((SDL_JoystickEventState(SDL_QUERY) != SDL_ENABLE) || (SDL_JoystickEventState(SDL_ENABLE) != SDL_ENABLE)) {
		return false;
	}

	jmap.jstate = JMAP_IDLE;

	for (int i = 0; i < MAX_JOYSTICKS; ++i) {
		for (int j= 0; j < MAX_JOYSTICK_AXES; ++j)
			joysticks[i].axes[j].axis = JOY_AXIS_UNUSED;
	}

	load_joystick_file(joySettingsFilename);

	for (int i = 0; i < MAX_JOYSTICKS; ++i) {
		if ((joysticks[i].handle = input_driver_sdl.joystick_open(i)) < 0)
			printf("P%i joystick not found.\n", i+1);
		else {
			joysticks[i].buttons = joyButtons[i];
			joysticks[i].hats = 0;
			printf("P%i joystick: %s.\n", i+1, SDL_JoystickName(jsdata[i].device));
		}

		for (int j= 0; j < MAX_JOYSTICK_AXES; ++j)
			joysticks[i].axes[j].bits = 0;
	}

	return true;
}

static int input_driver_sdl_joystick_open(int idx)
{
	jsdata[idx].device = SDL_JoystickOpen(idx);
	return jsdata[idx].device == NULL ? -1 : idx;
}

static void input_driver_sdl_joystick_close(int idx)
{
	SDL_JoystickClose(jsdata[idx].device);
}

static void input_driver_sdl_joystick_shutdown()
{
	if (joystickFile) {
		FILE* f = fopen(joystickFile,"wb");

        if(f) {
			for (int i = 0; i < MAX_JOYSTICKS; ++i) {
				fwrite(joyButtons[i],sizeof(struct joyButton),NUM_JOYSTICK_BUTTONS,f);
				fwrite(joysticks[i].axes,sizeof(struct joyAxis),MAX_JOYSTICK_AXES,f);

				if (joysticks[i].handle >= 0)
					input_driver_sdl_joystick_close(joysticks[i].handle);
			}
            fclose(f);
        }
	}
}

input_driver_t input_driver_sdl = {
	.buttons = { 0 },
	.mouse_scale = 1,
	.pad_mode = SNES_PAD,
    .poll = input_driver_sdl_poll,
	.wait = input_driver_sdl_wait,
	.get_mouse_state = input_driver_sdl_get_mouse_state,
	.joystick_init = input_driver_sdl_joystick_init,
	.joystick_shutdown = input_driver_sdl_joystick_shutdown,
	.joystick_open = input_driver_sdl_joystick_open,
	.joystick_close = input_driver_sdl_joystick_close,
};

