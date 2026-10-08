/*
 * Toolchain check for the Jedi Outcast AmigaOS port: the C++ features the
 * engine relies on (STL containers, exceptions thrown as const char *,
 * global constructors), float math through libnix and rand(). Run on the
 * Amiga; every line should end in "ok".
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <set>
#include <algorithm>

static int failures;

static void check(const char *what, bool ok)
{
	printf("%-40s %s\n", what, ok ? "ok" : "FAILED");
	if (!ok)
		failures++;
}

struct GlobalCtor
{
	int value;
	GlobalCtor() : value(1234) {}
};
static GlobalCtor globalCtor;
static std::string globalString("jedi");

static void thrower(int n)
{
	if (n > 2)
		throw ("DISCONNECTED\n");
	thrower(n + 1);
}

int main()
{
	check("global constructor", globalCtor.value == 1234);
	check("global std::string", globalString == "jedi");

	std::map<std::string, int> m;
	m["kyle"] = 1; m["jan"] = 2; m["desann"] = 3;
	check("std::map<string,int>", m.size() == 3 && m["jan"] == 2);

	std::vector<int> v;
	for (int i = 0; i < 1000; i++) v.push_back(999 - i);
	std::sort(v.begin(), v.end());
	check("std::vector + std::sort", v[0] == 0 && v[999] == 999);

	std::list<int> l;
	l.push_back(1); l.push_front(0); l.remove(1);
	check("std::list", l.size() == 1 && l.front() == 0);

	std::set<int> s; s.insert(5); s.insert(5); s.insert(3);
	check("std::set", s.size() == 2);

	bool caught = false;
	try { thrower(0); }
	catch (const char *reason) { caught = strcmp(reason, "DISCONNECTED\n") == 0; }
	check("throw/catch const char *", caught);

	volatile float f = 2.0f;
	check("sqrtf", fabs(sqrtf(f) - 1.41421356f) < 1e-5f);
	check("cosf(0)", cosf(0.0f) == 1.0f);
	check("atan2f", fabs(atan2f(1.0f, 1.0f) - 0.78539816f) < 1e-5f);

	char buf[64];
	sprintf(buf, "%f", 0.9999999);
	check("sprintf %f rounding", strcmp(buf, "1.000000") == 0);

	/* amiga_libfix.c: MSVC's rand(), 0..0x7fff, which the game expects */
	int rmax = 0;
	srand(1);
	for (int i = 0; i < 10000; i++)
	{
		int r = rand();
		if (r > rmax)
			rmax = r;
	}
	srand(1);
	check("rand() range 0..0x7fff", rmax <= 0x7fff && rmax > 0x7000 && rand() == 41);

	printf(failures ? "%d FAILED\n" : "all ok\n", failures);
	return failures ? 20 : 0;
}
