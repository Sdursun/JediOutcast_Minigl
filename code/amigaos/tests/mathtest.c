/*
 * Checks the math functions the game relies on, built with the game's
 * compiler flags, against known values. Run it from a Shell on the
 * target machine; every line should say "ok".
 *
 * In the first hardware test the 3D view came out turned by 90 degrees
 * and the player moved as if walls were floors, which is what swapped or
 * wrong sin()/cos() results would produce (AngleVectors).
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>

/* volatile keeps gcc from folding the calls at compile time */
static volatile double zero = 0.0;

static int failures;

static void check(const char *what, double got, double want)
{
	double diff = got - want;
	int ok = diff < 1e-6 && diff > -1e-6;

	printf("%-28s = %12.6f  (want %12.6f)  %s\n", what, got, want, ok ? "ok" : "WRONG");
	if (!ok)
		failures++;
}

static void checkstr(const char *what, char *buf, const char *got, const char *want)
{
	int ok = strcmp(got, want) == 0;

	(void)buf;
	printf("%-28s = \"%s\"  (want \"%s\")  %s\n", what, got, want, ok ? "ok" : "WRONG");
	if (!ok)
		failures++;
}

/* The engine's AngleVectors, verbatim apart from the name. */
static void AngleVectors(const float angles[3], float forward[3], float right[3], float up[3])
{
	float angle;
	static float sr, sp, sy, cr, cp, cy;

	angle = angles[1] * (M_PI * 2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[0] * (M_PI * 2 / 360);
	sp = sin(angle);
	cp = cos(angle);
	angle = angles[2] * (M_PI * 2 / 360);
	sr = sin(angle);
	cr = cos(angle);

	forward[0] = cp * cy;
	forward[1] = cp * sy;
	forward[2] = -sp;
	right[0] = (-1 * sr * sp * cy + -1 * cr * -sy);
	right[1] = (-1 * sr * sp * sy + -1 * cr * cy);
	right[2] = -1 * sr * cp;
	up[0] = (cr * sp * cy + -sr * -sy);
	up[1] = (cr * sp * sy + -sr * cy);
	up[2] = cr * cp;
}

int main(void)
{
	double z = zero;
	float angles[3], f[3], r[3], u[3];
	volatile float fl = -2.7f;

	printf("RTCW Amiga math test\n\n");

	check("sin(0)", sin(z), 0.0);
	check("cos(0)", cos(z), 1.0);
	check("sin(pi/2)", sin(z + M_PI / 2), 1.0);
	check("cos(pi/2)", cos(z + M_PI / 2), 0.0);
	check("sin(pi/6)", sin(z + M_PI / 6), 0.5);
	check("cos(pi/3)", cos(z + M_PI / 3), 0.5);
	check("tan(pi/4)", tan(z + M_PI / 4), 1.0);
	check("atan2(1,1)", atan2(z + 1, 1), M_PI / 4);
	check("atan2(1,-1)", atan2(z + 1, -1), 3 * M_PI / 4);
	check("atan2(-1,0)", atan2(z - 1, 0), -M_PI / 2);
	check("atan(1)", atan(z + 1), M_PI / 4);
	check("acos(0.5)", acos(z + 0.5), M_PI / 3);
	check("asin(0.5)", asin(z + 0.5), M_PI / 6);
	check("sqrt(2)", sqrt(z + 2), 1.41421356237);
	check("floor(2.7)", floor(z + 2.7), 2.0);
	check("floor(-2.2)", floor(z - 2.2), -3.0);
	check("ceil(2.2)", ceil(z + 2.2), 3.0);
	check("rint(2.4)", rint(z + 2.4), 2.0);
	check("rint(-2.6)", rint(z - 2.6), -3.0);
	check("fabs(-3)", fabs(z - 3), 3.0);
	check("pow(2,10)", pow(z + 2, 10), 1024.0);
	check("exp(1)", exp(z + 1), 2.718281828);
	check("log(e)", log(z + 2.718281828459045), 1.0);
	check("fmod(7.5,2)", fmod(z + 7.5, 2), 1.5);
	check("(int)-2.7f", (int)fl, -2.0);
	check("sqrtf(9)", sqrtf((float)(z + 9)), 3.0);
	check("sinf(pi/2)", sinf((float)(z + M_PI / 2)), 1.0);
	check("cosf(0)", cosf((float)z), 1.0);

	printf("\nAngleVectors( 0 0 0 ): forward x1 y0 z0, right x0 y-1 z0, up x0 y0 z1\n");
	angles[0] = angles[1] = angles[2] = (float)z;
	AngleVectors(angles, f, r, u);
	check("forward.x", f[0], 1); check("forward.y", f[1], 0); check("forward.z", f[2], 0);
	check("right.x", r[0], 0);   check("right.y", r[1], -1);  check("right.z", r[2], 0);
	check("up.x", u[0], 0);      check("up.y", u[1], 0);      check("up.z", u[2], 1);

	printf("\nAngleVectors( pitch 0, yaw 90, roll 0 ): forward x0 y1 z0\n");
	angles[1] = (float)(z + 90);
	AngleVectors(angles, f, r, u);
	check("forward.x", f[0], 0); check("forward.y", f[1], 1); check("forward.z", f[2], 0);
	check("up.z", u[2], 1);

	printf("\nString formatting (amiga_libfix.c)\n");
	{
		char buf[64];

		checkstr("%f of 0.99999999", buf, snprintf(buf, sizeof(buf), "%f", z + 0.99999999) >= 0 ? buf : "", "1.000000");
		checkstr("%.2f of 9.999", buf, (sprintf(buf, "%.2f", z + 9.999), buf), "10.00");
		checkstr("%g of 0.25", buf, (sprintf(buf, "%g", z + 0.25), buf), "0.25");
		strcpy(buf, "stale");
		checkstr("empty result terminated", buf, (sprintf(buf, "%s", ""), buf), "");
		checkstr("%5i|%-4s|", buf, (sprintf(buf, "%5i|%-4s|", 42, "ab"), buf), "   42|ab  |");
	}

	printf("\n%s: %d wrong\n", failures ? "FAILED" : "PASSED", failures);

	return failures ? 5 : 0;
}
