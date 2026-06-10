#include "../inc/asm.h"
#include "../inc/glob.h"
#include "../inc/print.h"
#include "../inc/lex/lex.h"
#include "../inc/lex/enum.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

// local funcdefs
void asminit();
void asmprog();
void asmwrite();

// global vars
struct lexa_s lexa;

// global funcs
void asm_() {
	printinit();
	asminit();
	lexread();

	printf("\033[36mentry\033[0m %s\033[K\n", fv[lexfi].dir);
	while (lexa.tkni < lexer.tknc) asmprog();
	asmwrite();
	printf("\033[36mfinished\033[0m %s (%ldB)\033[K\n", fv[asmfi].dir, ftell(fv[asmfi].f));
	
	lexfree();}

char* regitstr(long regi) {
	char* reg = 0;
	switch (regi) {
	case 0: reg = strdup("rax"); break;
	case 1: reg = strdup("rbx"); break;
	case 2: reg = strdup("rcx"); break;
	case 3: reg = strdup("rdx"); break;
	case 4: reg = strdup("rsi"); break;
	case 5: reg = strdup("rdi"); break;
	case 6: reg = strdup("r8"); break;
	case 7: reg = strdup("r9"); break;
	case 8: reg = strdup("r10"); break;
	case 9: reg = strdup("r11"); break;
	case 10: reg = strdup("r12"); break;
	case 11: reg = strdup("r13"); break;
	case 12: reg = strdup("r14"); break;
	case 13: reg = strdup("r15");}
	return reg;}

// local funcs
void asminit() {
	lexa.tkni = 0;}

struct asmsect_s* scopeitosect(long scopei) {
	if (scopei == -1) return 0;

	long tkni = -1;
	for (int i = 0; i < lexa.tkni; i += 2)
		if (lexer.tknv[i] == VARDEF)
			if (strcmp(lexer.strv + lexer.varv[lexer.tknv[i + 1]].scopei, lexer.strv + scopei) == 0) {
				tkni = i;
				break;}

	if (tkni == -1) return 0;

	for (int i = 0; i < lexa.sectc; ++i) 
		for (int j = 0; j < lexa.sectv[i].tknic; ++j)
			if (tkni == lexa.sectv[i].tkniv[j]) 
				return lexa.sectv + i;
	
	return 0;}

void asmprog() {
	switch (lexer.tknv[lexa.tkni]) {
	case VARDEF: {
		struct var_s* var = lexer.varv + lexer.tknv[lexa.tkni + 1];
		struct asmsect_s* sect = 0;
		
		// check sectiv combo already exists, if so, set sect
		for (int i = 0; i < lexa.sectc; ++i) {
			if (lexa.sectv[i].sectic != var->sectic) continue;
			int matchc = 0;
			for (int j = 0; j < lexa.sectv[i].sectic; ++j) 
				for (int k = 0; k < var->sectic; ++k) { 
					if (lexa.sectv[i].sectiv[j] != var->sectiv[k]) break;
					++matchc;}
			if (matchc == var->sectic) {
				sect = lexa.sectv + i;
				break;}}
		
		// if no sect found, create new
		if (!sect) {
			// create sect
			lexa.sectv = realloc(lexa.sectv, sizeof(struct asmsect_s) * ++lexa.sectc);
			sect = lexa.sectv + lexa.sectc - 1;
			*sect = (struct asmsect_s){var->sectiv, var->sectic, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0};
			sect->tkniv = realloc(sect->tkniv, sizeof(long) * sect->tknic);
			sect->tkniv[0] = lexa.tkni;

			// set sect flgs
			for (int i = 0; i < var->sectic; ++i) {
				if (!sect->w && strcmp(lexer.strv + lexer.sectv[var->sectiv[i]], "write") == 0) sect->w = 1;
				else if (!sect->x && strcmp(lexer.strv + lexer.sectv[var->sectiv[i]], "exec") == 0) sect->x = 1;
				else if (!sect->nob && strcmp(lexer.strv + lexer.sectv[var->sectiv[i]], "zero") == 0) {sect->progb = 0; sect->nob = 1;}}}
		else { // add tkn entry for converting tkns to sects
			sect->tkniv = realloc(sect->tkniv, sizeof(long) * ++sect->tknic);
			sect->tkniv[0] = lexa.tkni;}

		// add ".global _start" and "_start:" lines if main
		if (strcmp(lexer.strv + var->namei, "main") == 0) {
			sect->v = realloc(sect->v, sizeof(char*) * (sect->c += 2));
			asprintf(sect->v + sect->c - 2, ".global _start\n");
			asprintf(sect->v + sect->c - 1, "_start:\n");}
		else if (var->scopei >= 0) {
			// add label line (i.e. str:)
			sect->v = realloc(sect->v, sizeof(char*) * ++sect->c);
			asprintf(sect->v + sect->c - 1, "%s:\n", lexer.strv + var->scopei);}

		break;}
	case ASSEMBLY: {
		struct asm_s* asms = lexer.asmv + lexer.tknv[lexa.tkni + 1];
		struct asmsect_s* sect = scopeitosect(asms->scopei);

		// add asm code to sect
		sect->v = realloc(sect->v, sizeof(char*) * ++sect->c);
		sect->v[sect->c - 1] = malloc(1);
		sect->v[sect->c - 1][0] = '\t';
		int len = 1;
		for (int i = asms->stri; lexer.strv[i] != 0; ++i) {
			sect->v[sect->c - 1] = realloc(sect->v[sect->c - 1], ++len);
			sect->v[sect->c - 1][len - 1] = lexer.strv[i];
			if (lexer.strv[i] == '\n' && lexer.strv[i + 1] != '\0') { 
				sect->v[sect->c - 1] = realloc(sect->v[sect->c - 1], ++len);
				sect->v[sect->c - 1][len - 1] = '\t';}}
		sect->v[sect->c - 1] = realloc(sect->v[sect->c - 1], ++len);
		sect->v[sect->c - 1][len - 1] = '\0';

		break;}
	case NUMLIT: {
		struct numlit_s* numlit = lexer.numlitv + lexer.tknv[lexa.tkni + 1];
		struct asmsect_s* sect = scopeitosect(numlit->scopei);
		char*** v = sect ? &sect->v : &lexa.v;
		long* c = sect ? &sect->c : &lexa.c;

		char* type = numlit->typei >= 0 ? lexer.strv + lexer.typev[numlit->typei].namei : 0;
		
		// add asm code to sect
		if (sect && sect->nob) {
			int width = 8;
			if (type) {
				if (strcmp(type, "char") == 0 || strcmp(type, "schar") == 0) width = 1;
				else if (strcmp(type, "short") == 0 || strcmp(type, "sshort") == 0) width = 2;
				else if (strcmp(type, "int") == 0 || strcmp(type, "sint") == 0) width = 4;}
			*v = realloc(*v, sizeof(char*) * ++*c);
			asprintf(*v + *c - 1, "\t.zero %d\n", width);
			break;}

		*v = realloc(*v, sizeof(char*) * ++*c);
		(*v)[*c - 1] = 0;
	
		if (type) {
			if (strcmp(type, "char") == 0) asprintf(*v + *c - 1, "\t.byte %u\n", (unsigned char)numlit->num);
			else if (strcmp(type, "short") == 0) asprintf(*v + *c - 1, "\t.short %u\n", (unsigned short)numlit->num);
			else if (strcmp(type, "int") == 0) asprintf(*v + *c - 1, "\t.long %u\n", (unsigned int)numlit->num);
			else if (strcmp(type, "schar") == 0) asprintf(*v + *c - 1, "\t.byte %d\n", (char)numlit->num);
			else if (strcmp(type, "sshort") == 0) asprintf(*v + *c - 1, "\t.short %d\n", (short)numlit->num);
			else if (strcmp(type, "sint") == 0) asprintf(*v + *c - 1, "\t.long %d\n", (int)numlit->num);
			else if (strcmp(type, "slong") == 0) asprintf(*v + *c - 1, "\t.quad %ld\n", (long)numlit->num);}
		if (!(*v)[*c - 1]) asprintf(*v + *c - 1, "\t.quad %lu\n", (unsigned long)numlit->num);
		
		break;}
	case MATH: {
		struct math_s* math = lexer.mathv + lexer.tknv[lexa.tkni + 1];
		printf("scope %s scopei %ld\n", lexer.strv + math->scopei, math->scopei);
	
		struct asmsect_s* sect = scopeitosect(math->scopei);
		char*** v = sect ? &sect->v : &lexa.v;
		long* c = sect ? &sect->c : &lexa.c;
	
		*v = realloc(*v, sizeof(char*) * ++*c);
		(*v)[*c - 1] = 0;
		
		switch (math->op) {
		case SET:
			switch (math->type) {
			case VARI: asprintf(*v + *c - 1, "\tmovq %s(%%rip), %%%s\n", lexer.strv + lexer.varv[math->val].scopei, regitstr(math->regi)); break;
			case REGI: asprintf(*v + *c - 1, "\tmovq %%%s, %%%s\n", regitstr(math->val), regitstr(math->regi)); break;}
			break;}

		asprintf(*v + *c - 1, "\t...\n");
		   
		break;}}

	lexa.tkni += 2;}

void asmwrite() {
	if (asmfi < 0) return;
	FILE* f = fv[asmfi].f;
	rewind(f);
	ftruncate(fileno(f), 0);

	for (int i = 0; i < lexa.c; ++i) 
		fprintf(f, "%s", lexa.v[i]);

	for (int i = 0; i < lexa.sectc; ++i) {
		struct asmsect_s* sect = lexa.sectv + i;

		if (sect->a && sect->r && !sect->w && sect->x && sect->progb && !sect->nob) fprintf(f, ".section .text\n");
		else if (sect->a && sect->r && sect->w && !sect->x && sect->progb && !sect->nob) fprintf(f, ".section .data\n");
		else if (sect->a && sect->r && !sect->w && !sect->x && sect->progb && !sect->nob) fprintf(f, ".section .rodata\n");
		else if (sect->a && sect->r && sect->w && !sect->x && !sect->progb && sect->nob) fprintf(f, ".section .bss\n");
	
		for (int j = 0; j < sect->c; ++j) 
			fprintf(f, "%s", sect->v[j]);}}
