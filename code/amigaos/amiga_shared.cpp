/*
 * AmigaOS 3.x: time, directories and paths. Based on unix/unix_shared.c.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>

#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"

//===============================================================================

static int sys_timeBase;

int Sys_Milliseconds (void)
{
	struct timeval tp;

	gettimeofday(&tp, NULL);

	if (!sys_timeBase)
	{
		sys_timeBase = tp.tv_sec;
		return tp.tv_usec/1000;
	}

	return (tp.tv_sec - sys_timeBase)*1000 + tp.tv_usec/1000;
}

void Sys_Mkdir( const char *path )
{
	mkdir (path, 0777);
}

//============================================

#define	MAX_FOUND_FILES	0x1000

char **Sys_ListFiles( const char *directory, const char *extension, int *numfiles, qboolean wantsubs )
{
	struct dirent *d;
	DIR		*fdir;
	qboolean dironly = wantsubs;
	char		search[MAX_OSPATH];
	int			nfiles;
	char		**listCopy;
	char		*list[MAX_FOUND_FILES];
	int			i;
	struct stat st;

	if ( !extension)
		extension = "";

	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		dironly = qtrue;
	}

	nfiles = 0;

	if ((fdir = opendir(directory)) == NULL) {
		*numfiles = 0;
		return NULL;
	}

	while ((d = readdir(fdir)) != NULL) {
		Com_sprintf(search, sizeof(search), "%s/%s", directory, d->d_name);
		if (stat(search, &st) == -1)
			continue;
		if ((dironly && !S_ISDIR(st.st_mode)) ||
			(!dironly && S_ISDIR(st.st_mode)))
			continue;

		if (*extension) {
			if ( strlen( d->d_name ) < strlen( extension ) ||
				Q_stricmp(
					d->d_name + strlen( d->d_name ) - strlen( extension ),
					extension ) ) {
				continue; // didn't match
			}
		}

		if ( nfiles == MAX_FOUND_FILES - 1 )
			break;
		list[ nfiles ] = CopyString( d->d_name );
		nfiles++;
	}

	list[ nfiles ] = 0;

	closedir(fdir);

	*numfiles = nfiles;

	if ( !nfiles ) {
		return NULL;
	}

	listCopy = (char **)Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ), TAG_LISTFILES, qfalse );
	for ( i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	return listCopy;
}

void Sys_FreeFileList( char **list )
{
	int		i;

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}

char *Sys_Cwd( void )
{
	static char cwd[MAX_OSPATH];

	getcwd( cwd, sizeof( cwd ) - 1 );
	cwd[MAX_OSPATH-1] = 0;

	return cwd;
}

/* No CD path: every file the game cannot find would be probed there as
   well (the RTCW port lost seconds per map to that). */
char *Sys_DefaultCDPath(void)
{
	return "";
}

/* main() changes to PROGDIR:, so the current directory is where the
   executable and its base/ directory are. */
char *Sys_DefaultBasePath(void)
{
	return Sys_Cwd();
}

//============================================

int Sys_GetProcessorId( void )
{
	return CPUID_GENERIC;
}

void Sys_ShowConsole( int visLevel, qboolean quitOnClose )
{
}

char *Sys_GetCurrentUser( void )
{
	return "player";
}

void Sys_SetErrorText( const char *text )
{
}

void Sys_DisplaySystemConsole( qboolean show )
{
}

void Sys_EndProfiling( void )
{
}

//============================================
// Force feedback (Immersion/SideWinder joysticks on Windows): none here.

#include "../client/fffx.h"

void		FF_Init( qboolean bTryMouseFirst ) {}
void		FF_Shutdown( void ) {}
qboolean	FF_IsAvailable( void ) { return qfalse; }
qboolean	FF_IsMouse( void ) { return qfalse; }
qboolean	FF_SetTension( int iTension ) { return qfalse; }
qboolean	FF_SetSpring( long lSpring ) { return qfalse; }
void		FF_Play( ffFX_e fffx ) {}
void		FF_EnsurePlaying( ffFX_e fffx ) {}
void		FF_Stop( ffFX_e fffx ) {}
void		FF_StopAll( void ) {}
