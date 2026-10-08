/*
 * What the game module's source files saw on Windows before their own
 * includes: they were built with the precompiled header made from
 * game/g_headers.cpp, which pulls in more than g_headers.h itself, and
 * many of them rely on that (NPC_*.cpp use b_local.h without including
 * it). The Makefile force-includes this file into every module C++ file.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "../game/g_headers.h"
#include "../icarus/icarus.h"
#include "../game/npc_headers.h"
#include "../cgame/cg_headers.h"
