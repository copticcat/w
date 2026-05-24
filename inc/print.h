#ifndef DING_PRINT
#define DING_PRINT
#include "lex.h"
#include "asm.h"
#include <stdio.h>

struct lexv_s { // graphical data for lex_s
	int tknvi;
	int tknvc;
	char tknrowv[48 * (8 + 64)];
	int newtknrowi;};

struct lexfv_s { // graphical data for lexf_s
	struct lexf_s* lexf;

	int strvi;
	char wrowv[48 * (8 + 64)];
	char w[64];
	int wi;};

struct lexav_s { // graphical data for lexa_s
	char* ostr;};

extern struct lexfv_s* lexfvv; extern int lexfvc;
extern struct lexv_s lexv;
extern struct lexav_s lexav;

extern void printinit();
extern void lexprint(struct lexf_s*);
extern void asmprint();

#endif
