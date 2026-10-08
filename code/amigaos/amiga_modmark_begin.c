/*
 * Linked first among a game module's objects (see the Makefile). The
 * Makefile renames these to <module>_mod_data_begin/_bss_begin, so they
 * mark where that module's .data and .bss start in the executable.
 * Must be built with -fno-common so the bss marker lands in .bss.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

int mod_data_begin = 1;
int mod_bss_begin;
