/*
 * Build identification. The Makefile compiles this file afresh for every
 * link with AMIGA_BUILD set to the next number from build_number.txt, so
 * each executable says which build it is (in the log and to the Version
 * command), and test reports can be matched to the code.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef AMIGA_BUILD
#define AMIGA_BUILD 0
#endif

#define STR2(x) #x
#define STR(x) STR2(x)

/* Release name; test builds keep their 0.1.N build number next to it. */
#define AMIGA_RELEASE "1.0 beta 1"

const char amiga_buildstring[] = AMIGA_RELEASE ", build 0.1." STR(AMIGA_BUILD) " (" __DATE__ " " __TIME__ ")";

const char verstring[] = "$VER: jk2sp 1.0 (08.10.2026) " AMIGA_RELEASE ", build 0.1." STR(AMIGA_BUILD) ", Jedi Outcast SP for AmigaOS 3.x/MiniGL";
