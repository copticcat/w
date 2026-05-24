#ifndef DING_GLOB
#define DING_GLOB
#include <stdio.h>

struct f_s {
	FILE* f;
	char* dir;
	char* alias;};

extern struct f_s* fv; extern int fc;
extern long asmfi;
extern long startfi;
extern long lexfi;

extern double delay;

#endif
