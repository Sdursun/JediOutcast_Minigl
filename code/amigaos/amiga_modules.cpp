/*
 * The statically linked game module for AmigaOS 3.x.
 *
 * On Windows the game (with cgame and ICARUS) is jk2gamex86.dll. AmigaOS
 * 3.x has no shared objects, so it is linked into the executable; the
 * Makefile prefixes every global symbol it defines with "game_" (its own
 * q_shared.cpp, vmMain, dllEntry...) and brackets its .data and .bss with
 * marker symbols.
 *
 * The engine unloads and reloads the DLL on every map change (it expects
 * the module's globals to start afresh each time). So the first load
 * takes a copy of the module's .data and .bss and every later load puts it
 * back. The .bss is restored rather than cleared: the module is C++, and
 * the constructors of its global objects (the STL containers in ICARUS,
 * the navigator, the effects system...) ran before main() and wrote into
 * .bss; zeroes would leave those objects broken.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <stdlib.h>
#include <string.h>

#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"

/* The module's entry points, by their prefixed C++ symbol names. */
extern void *game_GetGameAPI( void *import ) __asm__( "_game__Z10GetGameAPIP13game_import_t" );
extern int game_vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7 ) __asm__( "_game__Z6vmMainiiiiiiiii" );
extern void game_dllEntry( int (*syscallptr)( int arg, ... ) ) __asm__( "_game__Z8dllEntryPFiizE" );

/* amiga_modmark_begin.c / _end.c, renamed by the Makefile. */
extern "C" int game_mod_data_begin, game_mod_data_end;
extern "C" int game_mod_bss_begin, game_mod_bss_end;

static void		*datacopy;
static void		*bsscopy;
static qboolean	loaded;

static void ResetModule( void )
{
	char *data = (char *)&game_mod_data_begin;
	char *bss = (char *)&game_mod_bss_begin;
	unsigned int datasize = (char *)&game_mod_data_end - data;
	unsigned int bsssize = (char *)&game_mod_bss_end - bss;

	if ( datacopy == NULL ) {
		/* First load: only the constructors have run in the module. */
		datacopy = malloc( datasize );
		bsscopy = malloc( bsssize );
		if ( datacopy == NULL || bsscopy == NULL ) {
			Com_Error( ERR_FATAL, "Sys_GetGameAPI: out of memory for the game module's copy" );
		}
		memcpy( datacopy, data, datasize );
		memcpy( bsscopy, bss, bsssize );
	} else {
		memcpy( data, datacopy, datasize );
		memcpy( bss, bsscopy, bsssize );
	}

	Com_DPrintf( "Sys_GetGameAPI: built-in game module, %u bytes data, %u bytes bss\n", datasize, bsssize );
}

void Sys_UnloadGame( void )
{
	loaded = qfalse;
}

void *Sys_GetGameAPI( void *parms )
{
	if ( loaded ) {
		Com_Error( ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadingGame" );
	}

	ResetModule();
	loaded = qtrue;

	Amiga_Trace( "Sys_GetGameAPI: calling GetGameAPI" );
	return game_GetGameAPI( parms );
}

/* cgame lives in the same module; hooked up after Sys_GetGameAPI. */
void *Sys_LoadCgame( int (**entryPoint)(int, ...), int (*systemcalls)(int, ...) )
{
	*entryPoint = (int (*)(int, ...))game_vmMain;
	Amiga_Trace( "Sys_LoadCgame: dllEntry" );
	game_dllEntry( systemcalls );
	Amiga_Trace( "Sys_LoadCgame: done" );

	return (void *)game_GetGameAPI;
}
