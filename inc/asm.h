#ifndef DING_ASM
#define DING_ASM

struct asmsect_s {
	long* sectiv; long sectic;
	long* tkniv; long tknic;
	char** v; long c;
	
	long a; long r; long w; long x;
	long progb; long nob;};

struct lexa_s {
	long tkni;
	struct asmsect_s* sectv; long sectc;
	char** v; long c;};

extern struct lexa_s lexa;

extern void asm_();
extern char* regitstr(long);

#endif
