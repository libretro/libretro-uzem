#ifndef _LIBRETRO_COMPILE_H
#define _LIBRETRO_COMPILE_H
#if defined (__GNUC__) && __GNUC__ <= 4 && defined (__cplusplus)
#define INIT_FIELD(field, val) field: val
#else
#define INIT_FIELD(field, val) .field = val
#endif
#endif
