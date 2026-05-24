#ifndef DING_ASM
#define DING_ASM

struct asmsect_s {
	long* sectiv; long sectic;
	long* scopeiv; long scopeic;
	long a; long r; long w; long x;
	long progb; long nob;
	char** v; long c;};

struct lexa_s {
	long tkni;
	struct asmsect_s* sectv; long sectc;
	char** v; long c;};

extern void asm_();

extern struct lexa_s lexa;

#endif
