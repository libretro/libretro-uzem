#ifndef SDL_EVENTS_DUMMY
#define SDL_EVENTS_DUMMY

typedef struct SDL_Event {
	int type;
	struct {
		struct {
			int sym;
		} keysym;
	} key;
} SDL_Event;

#define SDL_KEYDOWN 1
#define SDL_KEYUP 2
#define SDL_QUIT 3

#define SDLK_0			(0x0001)
#define SDLK_1			(0x0002)
#define SDLK_2			(0x0003)
#define SDLK_3			(0x0004)
#define SDLK_4			(0x0005)
#define SDLK_5			(0x0006)
#define SDLK_6			(0x0007)
#define SDLK_7			(0x0008)
#define SDLK_8			(0x0009)
#define SDLK_9			(0x0010)
#define SDLK_a			(0x0011)
#define SDLK_b			(0x0012)
#define SDLK_BACKQUOTE		(0x0013)
#define SDLK_BACKSLASH		(0x0014)
#define SDLK_BACKSPACE		(0x0015)
#define SDLK_c			(0x0016)
#define SDLK_COMMA		(0x0017)
#define SDLK_d			(0x0018)
#define SDLK_e			(0x0019)
#define SDLK_EQUALS		(0x0020)
#define SDLK_ESCAPE		(0x0021)
#define SDLK_f			(0x0022)
#define SDLK_F1			(0x0023)
#define SDLK_g			(0x0024)
#define SDLK_h			(0x0025)
#define SDLK_i			(0x0026)
#define SDLK_j			(0x0027)
#define SDLK_k			(0x0028)
#define SDLK_KP_0		(0x0029)
#define SDLK_KP_1		(0x0030)
#define SDLK_KP_2		(0x0031)
#define SDLK_KP_3		(0x0032)
#define SDLK_KP_4		(0x0033)
#define SDLK_KP_5		(0x0034)
#define SDLK_KP_6		(0x0035)
#define SDLK_KP_7		(0x0036)
#define SDLK_KP_8		(0x0037)
#define SDLK_KP_9		(0x0038)
#define SDLK_KP_MINUS		(0x0039)
#define SDLK_KP_MULTIPLY	(0x0040)
#define SDLK_KP_PERIOD		(0x0041)
#define SDLK_KP_PLUS		(0x0042)
#define SDLK_l			(0x0043)
#define SDLK_LEFTBRACKET	(0x0044)
#define SDLK_LSHIFT		(0x0045)
#define SDLK_m			(0x0046)
#define SDLK_MINUS		(0x0047)
#define SDLK_n			(0x0048)
#define SDLK_o			(0x0049)
#define SDLK_p			(0x0050)
#define SDLK_PERIOD		(0x0051)
#define SDLK_PRINTSCREEN	(0x0052)
#define SDLK_q			(0x0053)
#define SDLK_QUOTE		(0x0054)
#define SDLK_r			(0x0055)
#define SDLK_RETURN		(0x0056)
#define SDLK_RIGHTBRACKET	(0x0057)
#define SDLK_RSHIFT		(0x0058)
#define SDLK_s			(0x0059)
#define SDLK_SEMICOLON		(0x0060)
#define SDLK_SLASH		(0x0061)
#define SDLK_SPACE		(0x0062)
#define SDLK_t			(0x0063)
#define SDLK_TAB		(0x0064)
#define SDLK_u			(0x0065)
#define SDLK_v			(0x0066)
#define SDLK_w			(0x0067)
#define SDLK_x			(0x0068)
#define SDLK_y			(0x0069)
#define SDLK_z			(0x0070)

#endif
