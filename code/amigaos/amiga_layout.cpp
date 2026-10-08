/*
 * Struct layout cross-check.
 *
 * The game code is compiled with -malign-int (data files read straight
 * into structures are laid out with 4-byte aligned ints), while the
 * amiga_*.cpp files that talk to the operating system are not, because the
 * NDK structures (Window, InputEvent, ExecBase, ...) assume 2-byte
 * alignment. The structures the two sides share must come out the same
 * either way. The Makefile compiles this file once with each set of
 * flags, and main() compares the results at startup.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "../renderer/tr_local.h"
#include "../client/snd_local.h"

#define AMIGA_LAYOUT_SIZES \
	X(sysEvent_t) \
	X(cvar_t) \
	X(glconfig_t) \
	X(glstate_t) \
	X(dma_t)

#define X(t) sizeof(t),
static const int sizes[] = { AMIGA_LAYOUT_SIZES 0 };
#undef X

#define X(t) #t,
static const char *names[] = { AMIGA_LAYOUT_SIZES 0 };
#undef X

#ifdef AMIGA_LAYOUT_ENGINE
const int *amiga_layout_engine(const char ***n)
{
	*n = names;
	return sizes;
}
#else
const int *amiga_layout_os(const char ***n)
{
	*n = names;
	return sizes;
}
#endif
