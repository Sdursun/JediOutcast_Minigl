/*
 * Part of the AmigaOS 3.x port of Jedi Outcast SP.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

struct Window;

/* The window Intuition delivers input to while a GL context is open, or
   0. In fullscreen MiniGL opens a borderless backdrop window for this. */
extern struct Window *amiga_inputwindow;

/* Non-zero while the mouse pointer is shown (console open, windowed). */
extern int mousevisible;

void GLimp_Frame(void);

/* amiga_main.c: prints a shutdown step with developer 1 (see there). */
void Amiga_Trace(const char *msg);

/* amiga_main.c: while non-zero, console output is held back and written
   by Amiga_FlushConsole() when the display is unlocked (see Sys_Print). */
extern int amiga_holdconsole;
void Amiga_FlushConsole(void);

/* r_amigaspeeds: per-frame split of the renderer back end into time in
   glDrawElements (counted by tr_shade.c while amiga_speeds is set) and
   in mglSwitchDisplay, printed by GLimp_EndFrame. */
extern int amiga_speeds;
extern unsigned int amiga_drawMicros, amiga_drawCalls, amiga_drawIndexes;
unsigned int Amiga_Micros(void);
