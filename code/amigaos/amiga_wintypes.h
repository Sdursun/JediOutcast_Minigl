/*
 * The few Windows types and calls the portable parts of the Jedi Outcast
 * code still use (ICARUS's tokenizer, the navigation code, timing).
 * Included from q_shared.h and icarus/Tokenizer.h on AmigaOS.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef AMIGA_WINTYPES_H
#define AMIGA_WINTYPES_H

/* The NDK's exec/types.h has its own BYTE, WORD, LONG and BOOL (BYTE signed,
   unlike Windows'). The engine includes it through minigl.library's headers
   and uses none of these; only the game module (navigation code) does, and
   it never sees the NDK. */
#if !defined(_JK2EXE) && !defined(EXEC_TYPES_H)
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef long			LONG;
typedef int				BOOL;
#endif

typedef unsigned long	DWORD;
typedef unsigned int	UINT;
typedef void			*HANDLE;
typedef void			*LPVOID;
typedef char			TCHAR;
typedef char			*LPSTR;
typedef char			*LPTSTR;
typedef const char		*LPCSTR;
typedef const char		*LPCTSTR;
typedef DWORD			COLORREF;

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#define OutputDebugString(s)	((void)0)

/* MSVC names; libnix has stricmp and strnicmp. */
#define strcmpi		stricmp
#define _stricmp	stricmp
#define _strnicmp	strnicmp
#define _isnan		isnan

#define __cdecl

/* Milliseconds since some fixed point, as winmm's timeGetTime. */
int Sys_Milliseconds( void );
#define timeGetTime()	((DWORD)Sys_Milliseconds())

#endif
