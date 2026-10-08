/*
 * Workarounds for bugs in the C library (libnix, bebbo's amiga-gcc).
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <stdarg.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_STATIC
#define STB_SPRINTF_NOUNALIGNED
#include "thirdparty/stb_sprintf.h"

/*
 * String formatting. libnix gets two things wrong that the engine, which
 * formats everything through vsprintf/vsnprintf (Com_sprintf, va,
 * Com_Printf, cvar values), trips over:
 *
 * - vsprintf does not terminate the string when the output is empty: it
 *   writes the terminator with fputc() after setting the buffer's space to
 *   the number of characters written, which is 0, so the fputc() finds the
 *   buffer full and drops it. PS_SetBaseFolder("") left stale stack bytes
 *   in botlib's basefolder and ui/menus.txt was looked up as
 *   "         /ui/menus.txt".
 *
 * - %f of a value that rounds up to the next power of ten from below 1
 *   comes out wrong: 0.9999999 prints as "0.1000000", not "1.000000"
 *   (tests/mathtest.c). Cvar_SetValue formats floats with %f, so such a
 *   value would be stored as 0.1.
 *
 * Both are replaced with stb_sprintf. libnix's sprintf and snprintf call
 * these two, so they are covered as well; printf/fprintf to files still
 * use libnix's formatter, which the engine only hands plain strings.
 */
int vsprintf(char *s, const char *format, va_list args)
{
	return stbsp_vsprintf(s, format, args);
}

int vsnprintf(char *s, size_t size, const char *format, va_list args)
{
	if (size > INT_MAX)
		size = INT_MAX;

	return stbsp_vsnprintf(s, (int)size, format, args);
}

/*
 * rand() and srand() as in Microsoft's C library, which the game was written
 * against: it assumes rand() returns 0..0x7fff (Q_irand shifts by 15,
 * random() divides by 0x7fff), while libnix's goes up to 0x7fffffff.
 * q_shared.h sets RAND_MAX to match.
 */
static unsigned long randseed = 1;

void srand(unsigned int seed)
{
	randseed = seed;
}

int rand(void)
{
	randseed = randseed * 214013UL + 2531011UL;
	return (int)((randseed >> 16) & 0x7fff);
}

/*
 * libnix's float math functions (sqrtf, atan2f, asinf, rintf, cosf, ...)
 * come from its 68000 multilib even with -m68040: -m68040/-m68060 select
 * no FPU multilib. They are soft-float fdlibm code that returns the
 * result in d0, while code built for the FPU reads it from fp0 and gets
 * whatever was left there. gcc also turns float-typed calls like
 * "float y = sqrt(x)" into sqrtf, so the engine hit this all over
 * (VectorNormalize, vectoangles, ...): in the first hardware test the 3D
 * view came out rotated by 90 degrees and the player fell to death.
 * tests/mathtest.c showed cosf(0) returning 0.
 *
 * These replacements go through the double functions, which call the
 * system's mathieeedoub*.library and return in fp0. This file is built
 * with -fno-builtin, or gcc would turn "(float)sqrt(x)" straight back
 * into a call to sqrtf, i.e. into infinite recursion.
 */
float sqrtf(float x)          { return (float)__builtin_sqrt((double)x); }
float sinf(float x)           { return (float)sin(x); }
float cosf(float x)           { return (float)cos(x); }
float tanf(float x)           { return (float)tan(x); }
float asinf(float x)          { return (float)asin(x); }
float acosf(float x)          { return (float)acos(x); }
float atanf(float x)          { return (float)atan(x); }
float atan2f(float y, float x){ return (float)atan2(y, x); }
float floorf(float x)         { return (float)floor(x); }
float ceilf(float x)          { return (float)ceil(x); }
float rintf(float x)          { return (float)rint(x); }
float fabsf(float x)          { return x < 0 ? -x : x; }
float fmodf(float x, float y) { return (float)fmod(x, y); }
float powf(float x, float y)  { return (float)pow(x, y); }
float expf(float x)           { return (float)exp(x); }
float logf(float x)           { return (float)log(x); }
float log10f(float x)         { return (float)log10(x); }

/*
 * libnix's qsort compares its middle element with itself (when the scan
 * reaches it). The game's comparators assume, as MSVC's qsort guarantees,
 * that an element is never compared with itself: SV_QsortEntityNumbers
 * calls Com_Error("duplicated entity") on equal numbers, which ended the
 * first map right after it had loaded (build 0.1.8). Same algorithm as
 * libnix's, minus the self-comparison.
 */
static int QCompare(int (*compar)(const void *, const void *), const char *p, const char *q)
{
	return p == q ? 0 : compar(p, q);
}

void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
	char *base2 = (char *)base;
	size_t i, a, b, c;

	while (nmemb > 1)
	{
		a = 0;
		b = nmemb - 1;
		c = (a + b) / 2;	/* middle element */
		for (;;)
		{
			while (QCompare(compar, &base2[size * c], &base2[size * a]) > 0)
				a++;
			while (QCompare(compar, &base2[size * c], &base2[size * b]) < 0)
				b--;
			if (a >= b)
				break;
			for (i = 0; i < size; i++)
			{
				char tmp = base2[size * a + i];
				base2[size * a + i] = base2[size * b + i];
				base2[size * b + i] = tmp;
			}
			if (c == a)	/* keep track of the middle element */
				c = b;
			else if (c == b)
				c = a;
			a++;
			b--;
		}
		b++;
		/* recurse into the smaller part, loop on the larger */
		if (b < nmemb - b)
		{
			qsort(base2, b, size, compar);
			base2 = &base2[size * b];
			nmemb = nmemb - b;
		}
		else
		{
			qsort(&base2[size * b], nmemb - b, size, compar);
			nmemb = b;
		}
	}
}

/*
 * gcc merges sin(x) and cos(x) of the same x into one cexp/cexpf call
 * (sincos via exp(i*x)), even with -fno-builtin-sin/-cos, and libnix's
 * cexp/cexpf return their result where gcc does not look for it. The game
 * module's AngleVectors did this (build 0.1.14): view axes came out about
 * 12 times too long, so the world was drawn magnified (blurred walls, a
 * white screen in the ship cockpit). Built here with -fno-builtin, so the
 * sin/cos calls below stay separate.
 */
double _Complex cexp(double _Complex z)
{
	double e = exp(__real__ z);
	double _Complex r;

	__real__ r = e * cos(__imag__ z);
	__imag__ r = e * sin(__imag__ z);
	return r;
}

float _Complex cexpf(float _Complex z)
{
	double e = exp((double)__real__ z);
	float _Complex r;

	__real__ r = (float)(e * cos((double)__imag__ z));
	__imag__ r = (float)(e * sin((double)__imag__ z));
	return r;
}
