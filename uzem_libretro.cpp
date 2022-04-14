/*
(The MIT License)

Copyright (c) 2008-2016 by
David Etherton, Eric Anderton, Alec Bourque (Uze), Filipe Rinaldi,
Sandor Zsuga (Jubatian), Matt Pandina (Artcfox), Torsten Paul

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

#include "uzem.h"
#include "avr8.h"
#include "uzerom.h"
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <retro_dirent.h>
#include <vfs/vfs_implementation.h>
#include <streams/file_stream.h>
#include <retro_endianness.h>

avr8 uzebox;
static char sd_path[4096];

extern audio_driver_t audio_driver_libretro;
extern input_driver_t input_driver_libretro;
extern video_driver_t video_driver_libretro;

static uint32_t *framebuffer = NULL;
bool done_rendering;
bool half_width;
struct retro_vfs_interface *vfs_interface;

static float last_aspect;
static float last_sample_rate;

#ifdef USE_RGB565
typedef uint16_t pixel_t;
#define PIXEL_FORMAT RETRO_PIXEL_FORMAT_RGB565
#define PIXEL_SIZE_SHIFT 1
#else
typedef uint32_t pixel_t;
#define PIXEL_FORMAT RETRO_PIXEL_FORMAT_XRGB8888
#define PIXEL_SIZE_SHIFT 2
#endif

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
	(void) level;
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
}

static retro_log_printf_t log_cb = fallback_log;

unsigned retro_api_version(void)
{
	return RETRO_API_VERSION;
}

void retro_init(void)
{
	uzebox.adrv = &audio_driver_libretro;
	uzebox.idrv = &input_driver_libretro;
	uzebox.vdrv = &video_driver_libretro;

   //uzebox.vdrv->framebuffer = framebuffer;
   uzebox.vdrv->stride = 720;
   
	uzebox.init_gui();
	uzebox.init_joysticks();
}

void retro_deinit(void)
{
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
}

unsigned retro_get_region(void)
{
	return RETRO_REGION_NTSC;
}

void retro_get_system_info(struct retro_system_info *info)
{
	memset(info, 0, sizeof(*info));
	info->library_name = "Uzem";
	info->library_version = VERSION;
	info->need_fullpath = false;
	info->valid_extensions = "uze";
}

static retro_video_refresh_t video_cb;
retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
retro_input_poll_t input_poll_cb;
retro_input_state_t input_state_cb;

typedef struct cpu_state_buf {
	uint16_t pc, currentPc;
	unsigned cycleCounter, elapsedCycles,prevCyclesCounter,elapsedCyclesSleep,lastCyclesSleep;
	unsigned prevPortB, prevWDR, watchdogTimer;
	int scanline_top;
	unsigned left_edge_cycle, left_edge;
} cpu_state_buf_t;

static cpu_state_buf_t cpu_state_buf;

void retro_get_system_av_info(struct retro_system_av_info *info)
{
	info->timing.fps = 2 * 29.97;
	info->timing.sample_rate = AUDIO_FREQUENCY;

	info->geometry.base_width = half_width ? 360U : 720U;
	info->geometry.base_height = half_width ? 224U : 448U;
	info->geometry.max_width = half_width ? 360U : 720U;
	info->geometry.max_height = half_width ? 224U : 448U;
	info->geometry.aspect_ratio = 630.0/448.0;
}

void retro_set_environment(retro_environment_t cb)
{
	struct retro_log_callback logging;
	struct retro_vfs_interface_info vfs_interface_info;

	environ_cb = cb;

	if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
		log_cb = logging.log;

	vfs_interface_info.required_interface_version = DIRENT_REQUIRED_VFS_VERSION;
	vfs_interface_info.iface = NULL;
	if(cb(RETRO_ENVIRONMENT_GET_VFS_INTERFACE, &vfs_interface_info)) {
		vfs_interface = vfs_interface_info.iface;
		dirent_vfs_init(&vfs_interface_info);
		filestream_vfs_init(&vfs_interface_info);
	}


	bool no_content = false;
	cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);

	static const struct retro_variable vars[] = {
		{ NULL, NULL},
	};

	cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*) vars);

	static const struct retro_controller_description controllers[] = {
		{ "RetroPad", RETRO_DEVICE_JOYPAD }
	};

	static const struct retro_controller_info ports[] = {
		{ controllers, 1 },
		{ NULL, 0},
	};

	cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*) ports);

	char *dir = NULL;
	if (cb(RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY, &dir)) {
		if (dir != NULL) {
			strncpy(sd_path, dir, sizeof(sd_path));
			uzebox.SDpath = &sd_path[0];
			uzebox.init_sd();
		}
	}
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
	audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
	audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
	input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
	input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
	video_cb = cb;
}

void retro_reset(void)
{
}

static void audio_set_state(bool enable)
{
	(void) enable;
}

bool retro_load_game(const struct retro_game_info *info)
{
	struct retro_input_descriptor desc[] = {
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left"},
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up"},
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down"},
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right"},
		{ 0},
	};

	// TODO: make this configurable
#if defined (_PSP) || defined (_3DS)
	half_width = true;
#else
	half_width = false;
#endif

	environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

	enum retro_pixel_format fmt = PIXEL_FORMAT;
	if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
		log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
		return false;
	}

	if (info->size <= HEADER_SIZE) {
		return false;
	}

	if (memcmp("UZEBOX", info->data, 6) != 0) {
		return false;
	}

	RomHeader *header = (RomHeader *)info->data;
	if (info->size != HEADER_SIZE + retro_le_to_cpu32(header->progSize)
		|| retro_le_to_cpu32(header->progSize) > sizeof (uzebox.progmem)) {
		return false;
	}

	if (header->mouse){
		uzebox.idrv->pad_mode = SNES_MOUSE;
		printf("Mouse support enabled\n");
	}

	uint8_t *buffer = (uint8_t *)info->data;
#if RETRO_IS_LITTLE_ENDIAN
	memcpy((unsigned char*)(uzebox.progmem), buffer + HEADER_SIZE, retro_le_to_cpu32(header->progSize));
#elif RETRO_IS_BIG_ENDIAN
	{
		uint16_t *outptr = uzebox.progmem;
		const uint16_t *inptr = (const uint16_t *) (buffer + HEADER_SIZE);
		int size = retro_le_to_cpu32(header->progSize) / 2;
		while (size--)
			*outptr++ = retro_le_to_cpu16(*inptr++);
	}
#else
#error Wrong endianness headers
#endif

	framebuffer = (uint32_t *)malloc(sizeof(uint32_t) * 720 * 224);

	uzebox.decodeFlash();
	strncpy(uzebox.romName, "ROM", sizeof(uzebox.romName));

	uzebox.enableSound = true;
	uzebox.randomSeed=time(NULL);
	srand(uzebox.randomSeed);	//used for the watchdog timer entropy

	return true;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
	return false;
}

void retro_run(void)
{
	unsigned width = half_width ? 360 : 720;
	unsigned height = 224;

	/* Try rendering straight into VRAM if we can. */
   
   
	struct retro_framebuffer fb = {0};
	fb.width = width;
	fb.height = height;
	fb.access_flags = RETRO_MEMORY_ACCESS_WRITE;
	if (environ_cb(RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER, &fb) && fb.format == PIXEL_FORMAT) {
		uzebox.vdrv->framebuffer = (uint32_t *) fb.data;
		uzebox.vdrv->stride = fb.pitch >> PIXEL_SIZE_SHIFT;
	} else {
		uzebox.vdrv->framebuffer = framebuffer;
		uzebox.vdrv->stride = width;
	}
   
    
	while (uzebox.scanline_count == -999)
		uzebox.exec();
	while (uzebox.scanline_count != -999)
		uzebox.exec();
  
   /*
   done_rendering = false;
   while (!done_rendering)
      uzebox.exec();
   */
   

	video_cb(uzebox.vdrv->framebuffer, width, height, uzebox.vdrv->stride << PIXEL_SIZE_SHIFT);
}

void retro_unload_game(void)
{
	last_aspect = 0.0f;
	last_sample_rate = 0.0f;
	free(framebuffer);
	framebuffer = NULL;
}

size_t retro_serialize_size(void)
{
	return sizeof(cpu_state_buf) + sizeof(uzebox.r) + sizeof(uzebox.io) + sizeof(uzebox.sram);
}

bool retro_serialize(void *data, size_t size)
{
	cpu_state_buf.pc = uzebox.pc;
	cpu_state_buf.currentPc = uzebox.currentPc;
	cpu_state_buf.scanline_top = uzebox.scanline_top;
	cpu_state_buf.left_edge_cycle = uzebox.left_edge_cycle;
	cpu_state_buf.left_edge = uzebox.left_edge;

	uint8_t *buf = (uint8_t *)data;
	int len = sizeof(cpu_state_buf) + sizeof(uzebox.r) + sizeof(uzebox.io) + sizeof(uzebox.sram);
	if (size >= len) {
		memcpy(buf, &cpu_state_buf, sizeof(cpu_state_buf));
		buf += sizeof(cpu_state_buf);
		memcpy(buf, &uzebox.r[0], sizeof(uzebox.r));
		buf += sizeof(uzebox.r);
		memcpy(buf, &uzebox.io[0], sizeof(uzebox.io));
		buf += sizeof(uzebox.io);
		memcpy(buf, &uzebox.sram[0], sizeof(uzebox.sram));

		return true;
	}

	return false;
}

bool retro_unserialize(const void *data, size_t size)
{
	uint8_t *buf = (uint8_t *)data;
	int len = sizeof(cpu_state_buf) + sizeof(uzebox.r) + sizeof(uzebox.io) + sizeof(uzebox.sram);

	if (size >= len) {
		memcpy(&cpu_state_buf, buf, sizeof(cpu_state_buf));
		buf += sizeof(cpu_state_buf);
		memcpy(&uzebox.r[0], buf, sizeof(uzebox.r));
		buf += sizeof(uzebox.r);
		memcpy(&uzebox.io[0], buf, sizeof(uzebox.io));
		buf += sizeof(uzebox.io);
		memcpy(&uzebox.sram[0], buf, sizeof(uzebox.sram));

		uzebox.pc = cpu_state_buf.pc;
		uzebox.currentPc = cpu_state_buf.currentPc;
		uzebox.scanline_top = cpu_state_buf.scanline_top;
		uzebox.left_edge_cycle = cpu_state_buf.left_edge_cycle;
		uzebox.left_edge = cpu_state_buf.left_edge;

		return true;
	}
	return false;
}

void *retro_get_memory_data(unsigned id)
{
   if (id != RETRO_MEMORY_SAVE_RAM)
      return NULL;

   return uzebox.eeprom;
}

size_t retro_get_memory_size(unsigned id)
{
   if (id != RETRO_MEMORY_SAVE_RAM)
      return 0;

   return eepromSize;
}

void retro_cheat_reset(void)
{
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
	(void) index;
	(void) enabled;
	(void) code;
}

