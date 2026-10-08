/*
 * Hang finder for test builds (-synclog).
 *
 * A separate process watches the console output. When the game has printed
 * nothing for WATCH_SECONDS, it scans the main task's stack for addresses
 * inside the executable's code and appends them to the sync log as offsets
 * from the start of the code hunk. Looked up with nm on objects/jk2sp.db
 * (whose .text starts at 0), they give the function the game is stuck in
 * and the chain of callers: the saved context (when the task was switched
 * out) holds the current PC, the stack the return addresses.
 *
 * Only DOS calls here, no libnix stdio: the main task may be stuck inside
 * it.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <exec/exec.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <stdlib.h>
#include <string.h>

/* Bumped by Sys_Print for every console line. */
volatile unsigned long amiga_printcount;

/* libnix's malloc lock and the engine's last .bss object before libnix's
   own variables (nm: _PCM_Buffer, ___stdfilesize ... ___memsema). The main
   task was found waiting on a semaphore inside free(). */
extern "C" struct SignalSemaphore *__memsema;
extern "C" char PCM_Buffer[];

#define WATCH_SECONDS	10
#define MAX_HITS		80
#define MAX_SEGS		8

static struct Task *maintask;
static const char *logname;
static volatile int watchquit;
static volatile int watchrunning;

static ULONG segstart[MAX_SEGS], segend[MAX_SEGS];
static int numsegs;

/* Small formatter, no stdio: appends "s" or hex numbers to a buffer. */
static char *AddStr(char *p, const char *s)
{
	while (*s)
		*p++ = *s++;
	return p;
}

static char *AddHex(char *p, ULONG v, int digits)
{
	int i;

	for (i = digits - 1; i >= 0; i--)
	{
		int n = (v >> (i * 4)) & 15;
		*p++ = n < 10 ? '0' + n : 'a' + n - 10;
	}
	return p;
}

static void LogLine(const char *line)
{
	BPTR fh = Open((STRPTR)logname, MODE_READWRITE);

	if (fh)
	{
		Seek(fh, 0, OFFSET_END);
		Write(fh, (APTR)line, strlen(line));
		Close(fh);
	}
}

static void LogFrames(ULONG fp, ULONG lower, ULONG upper);

/* Which code segment an address falls in, or -1. */
static int SegOf(ULONG a)
{
	int i;

	for (i = 0; i < numsegs; i++)
	{
		if (a >= segstart[i] && a < segend[i])
			return i;
	}
	return -1;
}

static void DumpMainTask(unsigned int seconds)
{
	char line[256], *p;
	UBYTE *sp, *upper, *lower;
	ULONG sigwait, sigrecvd;
	UBYTE state;
	int hits = 0, n;

	Forbid();
	sp = (UBYTE *)maintask->tc_SPReg;
	upper = (UBYTE *)maintask->tc_SPUpper;
	lower = (UBYTE *)maintask->tc_SPLower;
	state = maintask->tc_State;
	sigwait = maintask->tc_SigWait;
	sigrecvd = maintask->tc_SigRecvd;
	Permit();

	/* state 4 = waiting (sigwait: what for; 0x10 = semaphore), 3 = ready */
	p = AddStr(line, "watchdog: no output for ");
	p = AddHex(p, seconds, 4);
	p = AddStr(p, "(hex) s. state ");
	p = AddHex(p, state, 2);
	p = AddStr(p, " sigwait ");
	p = AddHex(p, sigwait, 8);
	p = AddStr(p, " sigrecvd ");
	p = AddHex(p, sigrecvd, 8);
	p = AddStr(p, " sp ");
	p = AddHex(p, (ULONG)sp, 8);
	p = AddStr(p, " stack ");
	p = AddHex(p, (ULONG)lower, 8);
	p = AddStr(p, "-");
	p = AddHex(p, (ULONG)upper, 8);
	p = AddStr(p, " code ");
	p = AddHex(p, segstart[0], 8);
	p = AddStr(p, "\n");
	*p = 0;
	LogLine(line);

	/* libnix's malloc semaphore: whose is it? (the main task is above as
	   "task") A garbage pointer or owner means it was overwritten. */
	{
		struct SignalSemaphore *ss = __memsema;

		p = AddStr(line, "watchdog: task ");
		p = AddHex(p, (ULONG)maintask, 8);
		p = AddStr(p, " __memsema ");
		p = AddHex(p, (ULONG)ss, 8);
		if (ss && !((ULONG)ss & 1))
		{
			p = AddStr(p, " owner ");
			p = AddHex(p, (ULONG)ss->ss_Owner, 8);
			p = AddStr(p, " nest ");
			p = AddHex(p, (ULONG)(UWORD)ss->ss_NestCount, 4);
			p = AddStr(p, " queue ");
			p = AddHex(p, (ULONG)(UWORD)ss->ss_QueueCount, 4);
		}
		p = AddStr(p, "\n");
		*p = 0;
		LogLine(line);

		/* the end of PCM_Buffer (60000 bytes) and the libnix variables
		   that follow it in .bss */
		p = AddStr(line, "watchdog: PCM_Buffer end:");
		for (n = 60000 - 8; n < 60000 + 40; n += 4)
		{
			p = AddStr(p, " ");
			p = AddHex(p, *(ULONG *)(PCM_Buffer + n), 8);
		}
		p = AddStr(p, "\n");
		*p = 0;
		LogLine(line);
	}

	/* The live callers: exec saved the task's registers at sp, a5 among
	   them; the first word there that starts a frame chain is taken. */
	for (n = 0; n < 0x80; n += 2)
	{
		ULONG fp = *(ULONG *)(sp + n);

		if (fp > (ULONG)sp && fp + 8 <= (ULONG)upper && !(fp & 1)
			&& SegOf(*(ULONG *)(fp + 4)) == 0)
		{
			p = AddStr(line, "watchdog: frames from sp+");
			p = AddHex(p, n, 2);
			p = AddStr(p, "\n");
			*p = 0;
			LogLine(line);
			LogFrames(fp, (ULONG)lower, (ULONG)upper);
			break;
		}
	}

	/* Code addresses on the stack, read at every word: the 68k stack is
	   only word aligned, so return addresses can sit at 2 mod 4. The first
	   ones are the current PC (saved context) and the innermost callers;
	   map the offsets with nm -n on symbols/jk2sp-0.1.N.db (.text). */
	for (n = 0; sp + n + 4 <= upper && n < 65536 && hits < MAX_HITS; n += 2)
	{
		ULONG v = *(ULONG *)(sp + n);

		if (v < segstart[0] || v >= segend[0] || (v & 1))
			continue;

		p = AddStr(line, "watchdog:   sp+");
		p = AddHex(p, n, 5);
		p = AddStr(p, " -> code+");
		p = AddHex(p, v - segstart[0], 6);
		p = AddStr(p, "\n");
		*p = 0;
		LogLine(line);
		hits++;
	}
}

/* Walks the a5 frame chain (all our code keeps a5 as frame pointer: link
   a5 / unlk a5) from fp: unlike the flat scan above, every entry is a live
   caller, no stale values. */
static void LogFrames(ULONG fp, ULONG lower, ULONG upper)
{
	char line[128], *p;
	int depth;

	for (depth = 0; depth < 40; depth++)
	{
		ULONG ret;

		if (fp < lower || fp + 8 > upper || (fp & 1))
			break;
		ret = *(ULONG *)(fp + 4);
		if (SegOf(ret) != 0)
			break;

		p = AddStr(line, "  frame ");
		p = AddHex(p, fp, 8);
		p = AddStr(p, " return to code+");
		p = AddHex(p, ret - segstart[0], 6);
		p = AddStr(p, "\n");
		*p = 0;
		LogLine(line);

		if (*(ULONG *)fp <= fp)
			break;	/* the chain only goes up the stack */
		fp = *(ULONG *)fp;
	}
}

/*
 * CPU exceptions (illegal instruction, address error, ...) in the main task.
 *
 * By default dos.library puts up its "Software Failure" requester on the
 * Workbench screen, behind the game's screen, and the task waits for it
 * forever: the game just hangs. Our handler instead notes where it
 * happened, makes the exception return to Amiga_TrapStub in user mode, and
 * that writes the details to the sync log and ends the game through
 * Com_Error.
 *
 * Exec calls tc_TrapCode in supervisor mode with the exception number on
 * top of the exception frame (SR.w, PC.l, format/vector.w, ...).
 */
extern "C" {
ULONG amiga_trapnum, amiga_trappc, amiga_trapusp, amiga_trapa5;
ULONG amiga_trapaddr[2];
UWORD amiga_trapsr, amiga_trapfmt;
void Amiga_TrapHandler(void);
void Amiga_TrapReport(void);
}

asm(
"	.text\n"
"	.even\n"
"	.globl	_Amiga_TrapHandler\n"
"_Amiga_TrapHandler:\n"
"	move.l	(sp)+,_amiga_trapnum\n"
"	move.w	(sp),_amiga_trapsr\n"
"	move.l	2(sp),_amiga_trappc\n"
"	move.w	6(sp),_amiga_trapfmt\n"
/* access fault address: frame +8 (format 2) or +0x14 (68040 format 7) */
"	move.l	8(sp),_amiga_trapaddr\n"
"	move.l	20(sp),_amiga_trapaddr+4\n"
"	move.l	a5,_amiga_trapa5\n"
"	move.l	a0,-(sp)\n"
"	move.l	usp,a0\n"
"	move.l	a0,_amiga_trapusp\n"
"	move.l	(sp)+,a0\n"
"	move.l	#_Amiga_TrapStub,2(sp)\n"
"	rte\n"
"_Amiga_TrapStub:\n"
"	jsr	_Amiga_TrapReport\n"
"	bra.s	_Amiga_TrapStub\n"
);

static APTR oldtrapcode;
static int trapinstalled;

static const char *TrapName(ULONG n)
{
	switch (n)
	{
	case 2: return "bus/access error";
	case 3: return "address error";
	case 4: return "illegal instruction";
	case 5: return "division by zero";
	case 6: return "CHK";
	case 7: return "TRAPV";
	case 8: return "privilege violation";
	case 10: return "line A";
	case 11: return "line F";
	}
	if (n >= 32 && n < 48) return "TRAP #n";
	if (n >= 48 && n < 56) return "FPU";
	return "?";
}

void Com_Error(int code, const char *fmt, ...);

extern "C" void Amiga_TrapReport(void)
{
	static int reported;
	char line[256], *p;
	struct Task *me = FindTask(NULL);
	ULONG pc = amiga_trappc;
	int n;

	if (reported++)
		Wait(0);	/* failed again while reporting: stay put */

	p = AddStr(line, "crash: CPU exception ");
	p = AddHex(p, amiga_trapnum, 2);
	p = AddStr(p, " (");
	p = AddStr(p, TrapName(amiga_trapnum));
	p = AddStr(p, ") at ");
	p = AddHex(p, pc, 8);
	if (SegOf(pc) >= 0)
	{
		p = AddStr(p, " = code+");
		p = AddHex(p, pc - segstart[SegOf(pc)], 6);
		if (SegOf(pc))
			p = AddStr(p, " (not the first segment)");
	}
	p = AddStr(p, " sr ");
	p = AddHex(p, amiga_trapsr, 4);
	p = AddStr(p, " frame ");
	p = AddHex(p, amiga_trapfmt, 4);
	p = AddStr(p, " addr ");
	p = AddHex(p, amiga_trapaddr[0], 8);
	p = AddStr(p, " ");
	p = AddHex(p, amiga_trapaddr[1], 8);
	p = AddStr(p, " usp ");
	p = AddHex(p, amiga_trapusp, 8);
	p = AddStr(p, " a5 ");
	p = AddHex(p, amiga_trapa5, 8);
	p = AddStr(p, " code ");
	p = AddHex(p, segstart[0], 8);
	p = AddStr(p, "\n");
	*p = 0;
	LogLine(line);

	/* the code around the PC, in case it is not ours (a jump into data) */
	if (!(pc & 1))
	{
		p = AddStr(line, "crash: at pc-8:");
		for (n = -8; n < 16; n += 2)
		{
			p = AddStr(p, " ");
			p = AddHex(p, *(UWORD *)(pc + n), 4);
		}
		p = AddStr(p, "\n");
		*p = 0;
		LogLine(line);
	}

	LogFrames(amiga_trapa5, (ULONG)me->tc_SPLower, (ULONG)me->tc_SPUpper);

	/* the stack too, from where it was: catches a return through a
	   smashed address, which leaves no frame to walk */
	for (n = 0; n < 0x200 && amiga_trapusp + n + 4 <= (ULONG)me->tc_SPUpper; n += 2)
	{
		ULONG v = *(ULONG *)(amiga_trapusp + n);

		if (SegOf(v) != 0 || (v & 1))
			continue;
		p = AddStr(line, "crash:   usp+");
		p = AddHex(p, n, 4);
		p = AddStr(p, " -> code+");
		p = AddHex(p, v - segstart[0], 6);
		p = AddStr(p, "\n");
		*p = 0;
		LogLine(line);
	}

	Com_Error(0 /* ERR_FATAL */, "CPU exception %d (%s) at code+%06x, see jk2sync.log",
		(int)amiga_trapnum, TrapName(amiga_trapnum), (unsigned)(pc - segstart[0]));
	Wait(0);
}

static void RemoveTrapHandler(void)
{
	if (trapinstalled)
	{
		maintask->tc_TrapCode = oldtrapcode;
		trapinstalled = 0;
	}
}

static void WatchdogMain(void)
{
	unsigned long last = amiga_printcount;
	unsigned int quiet = 0, nextdump = WATCH_SECONDS;

	watchrunning = 1;

	while (!watchquit)
	{
		Delay(50);	/* 1 s */

		if (amiga_printcount != last)
		{
			last = amiga_printcount;
			quiet = 0;
			nextdump = WATCH_SECONDS;
			continue;
		}

		quiet++;
		if (quiet >= nextdump)
		{
			DumpMainTask(quiet);
			nextdump = quiet + 30;	/* again later: does it move? */
		}
	}

	watchrunning = 0;
}

void Amiga_StartWatchdog(const char *logfile)
{
	struct Process *pr = (struct Process *)FindTask(NULL);
	struct CommandLineInterface *cli;
	BPTR seg;

	if (!pr->pr_CLI)
		return;	/* Workbench start: no seglist at hand */

	cli = (struct CommandLineInterface *)BADDR(pr->pr_CLI);
	for (seg = cli->cli_Module; seg && numsegs < MAX_SEGS; seg = *(BPTR *)BADDR(seg))
	{
		ULONG *s = (ULONG *)BADDR(seg);

		/* the segment's memory is AllocMem'd with its size in front */
		segstart[numsegs] = (ULONG)(s + 1);
		segend[numsegs] = (ULONG)s + s[-1] - 4;
		numsegs++;
	}

	maintask = &pr->pr_Task;
	logname = logfile;

	/* our CPU exception handler, see Amiga_TrapReport */
	Forbid();
	oldtrapcode = maintask->tc_TrapCode;
	maintask->tc_TrapCode = (APTR)Amiga_TrapHandler;
	trapinstalled = 1;
	Permit();
	atexit(RemoveTrapHandler);

	CreateNewProcTags(NP_Entry, (ULONG)WatchdogMain,
	                  NP_Name, (ULONG)"jk2sp watchdog",
	                  NP_Priority, 1,
	                  NP_StackSize, 16384,
	                  TAG_END);
}

void Amiga_StopWatchdog(void)
{
	int i;

	RemoveTrapHandler();
	watchquit = 1;
	for (i = 0; i < 50 && watchrunning; i++)
		Delay(5);
}
