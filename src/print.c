#include "../inc/print.h"
#include "../inc/glob.h"
#include "../inc/lex/lex.h"
#include "../inc/lex/enum.h"
#include "../inc/asm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <math.h>

// local structs
struct lexv_s { // graphical data for lex_s
	long tknvi;
	long tknvc;
	char tknrowv[48 * (8 + 64)];
	long newtknrowi;};

struct lexfv_s { // graphical data for lexf_s
	struct lexf_s* lexf;
	
	long strvi;
	long strvlinei;
	long strvlinec;
	char wrowv[48 * (8 + 64)];
	char w[64];
	long wi;};

struct lexav_s { // graphical data for lexa_s
	char* ostr;};

// local vars
static struct lexfv_s* lexfvv = 0; static long lexfvc = 0;
static struct lexv_s lexv;
static struct lexav_s lexav;
static long lexfvicache = -1;

// global funcs
void errprint(char* msg) {
	char* line = lexf->str + lexf->stri;
	for (; line != lexf->str && line[-1] != '\n'; --line);
	long linelen = 0;
	for (char* ch = line; *ch != '\n' && *ch != 0; ++ch, ++linelen);
	//long linei = 0;
	//for (char* ch = lexf->str; ch != line; ++ch) if (*ch == '\n') ++linei;
	
	char linen[9];
	long linenc = snprintf(linen, 8, "%ld", ++lexv.tknvc);
	memmove(linen + 7 - linenc, linen, linenc);
	memset(linen, ' ', 7 - linenc);
	linen[7] = ' ';
	linen[8] = 0;

	printf("\033[1;31merror:\t\033[0m%s\n%s%.*s\n", msg, linen, (int)linelen, line);}

void printinit() {
	memset(&lexv, 0, sizeof(struct lexv_s));
	memset(&lexav, 0, sizeof(struct lexav_s));
	free(lexfvv);
	lexfvv = 0;
	lexfvc = 0;
	lexfvicache = -1;}
	
void lexprint(struct lexf_s* lexf) {
	struct lexfv_s* lexfv = 0;

	if (lexfvicache >= 0 && lexfvv[lexfvicache].lexf == lexf)
		lexfv = lexfvv + lexfvicache; 
	else {
		// make/get visual lexf struct
		long lexvi = 0;
		for (; lexvi < lexfvc; ++lexvi)
			if (lexfvv[lexvi].lexf == lexf) {
				lexfv = lexfvv + lexvi;
				break;}
		if (!lexvi) memset(lexv.tknrowv, ' ', 48 * (8 + 64));
		if (!lexfv) {
			lexfvv = realloc(lexfvv, sizeof(struct lexfv_s) * ++lexfvc);
			lexfv = lexfvv + (lexfvc - 1);
			memset(lexfv, 0, sizeof(struct lexfv_s));
			lexfv->lexf = lexf;
			memset(lexfv->wrowv, ' ', 48 * (8 + 64));}
		lexfvicache = lexvi;}

	long newline = 0;
	long newtoken = 0;
	
	while (lexf->stri > lexfv->strvi) {
		switch (lexf->str[lexfv->strvi]) {
		case '\n':
			++lexfv->strvlinec;
			
			// ding code line number
			char linen[8];
			long linenc = snprintf(linen, 8, "%ld", lexfv->strvlinec);
			memmove(linen + 7 - linenc, linen, linenc);
			memset(linen, ' ', 7 - linenc);
			linen[7] = ' ';
			
			// append row
			memmove(lexfv->wrowv + (8 + 64), lexfv->wrowv, 47 * (8 + 64));
			memcpy(lexfv->wrowv, linen, 8);
			memcpy(lexfv->wrowv + 8, lexfv->w, 64);
			memset(lexfv->wrowv + 8 + lexfv->wi, ' ', 64 - lexfv->wi);
			
			// reset ding code buf
			memset(lexfv->w, 0, 64);
			lexfv->wi = 0;
			
		       	lexfv->strvlinei = lexfv->strvi + 1;
			newline = 1;
			break;
		case '\t':
			// ding code, append 8 spaces (shortened if overflow)
			if (lexfv->wi > 63) break;
			long spacec = abs(((int)lexfv->wi % 8) - 8);
			long overflow = lexfv->wi + spacec > 63;
			spacec = overflow ? 64 - lexfv->wi : spacec;
			memset(lexfv->w + lexfv->wi, ' ', spacec);
			lexfv->wi = overflow ? 63 : lexfv->wi + spacec;
			break;
		default:
			// ding code, append char
			if (lexfv->wi > 63) break;
			lexfv->w[lexfv->wi++] = lexf->str[lexfv->strvi];
			break;}
		++lexfv->strvi;}
	
	while (lexer.tknc - 1 >= lexv.tknvi) { 
		char tkn[65]; long tknc = 0;
		switch (lexer.tknv[lexv.tknvi]) {
		case ASSEMBLY: {
			struct asm_s* v = lexer.asmv + lexer.tknv[lexv.tknvi + 1];
			
			// format asm str to replace '\n' with "\n" and '\t' with "\t"
			char* fv = 0;
			for (long vi = 0, fvi = 0; lexer.strv[v->stri + vi] != 0; ++vi) {
				if (lexer.strv[v->stri + vi] == '\n') {
					fvi += 2;
					fv = realloc(fv, fvi + 1);
					fv[fvi - 2] = '\\';
					fv[fvi - 1] = 'n';
					fv[fvi] = 0;}
				else if (lexer.strv[v->stri + vi] == '\t') {
					fvi += 2;
					fv = realloc(fv, fvi + 1);
					fv[fvi - 2] = '\\';
					fv[fvi - 1] = 't';
					fv[fvi] = 0;}
				else {
					++fvi;
					fv = realloc(fv, fvi + 1);
					fv[fvi - 1] = lexer.strv[v->stri + vi];
					fv[fvi] = 0;}}

			tknc = snprintf(tkn, 64, "asm     %s", fv);
			break;}	
		case VARDEF: { 
			struct var_s* v = lexer.varv + lexer.tknv[lexv.tknvi + 1];
			tknc = snprintf(tkn, 64, "vardef  %s", v->namei >= 0 ? lexer.strv + v->namei : "\033[33manon\033[0m");
			break;}
		case VARLIT: { 
			struct var_s* v = lexer.varv + lexer.tknv[lexv.tknvi + 1];
			tknc = snprintf(tkn, 64, "varlit  %s", lexer.strv + v->namei);
			break;}
		case VAR: { 
			struct var_s* v = lexer.varv + lexer.tknv[lexv.tknvi + 1];
			tknc = snprintf(tkn, 64, "var     %s", lexer.strv + v->namei);
			break;}
		case NUMLIT: {
			struct numlit_s* v = lexer.numlitv + lexer.tknv[lexv.tknvi + 1];
			tknc = snprintf(tkn, 64, "numlit  %ld", v->num);
			break;}
		case NUM: {
			tknc = snprintf(tkn, 64, "num     %ld", lexer.tknv[lexv.tknvi + 1]);
			break;}
		case MATH: {
			struct math_s* v = lexer.mathv + lexer.tknv[lexv.tknvi + 1];
			
			char* op = 0;
			switch (v->op) {
			case ADD: op = strdup("+="); break;
			case SUB: op = strdup("-="); break;
			case MUL: op = strdup("*="); break;
			case DIV: op = strdup("/="); break;
			case SET: op = strdup("="); break;}
			
			switch (v->type) {
			case LIT: tknc = snprintf(tkn, 64, "math    %s %s %ld", regitstr(v->regi), op, v->val); break;
			case VARI: tknc = snprintf(tkn, 64, "math    %s %s %s", regitstr(v->regi), op, lexer.strv + lexer.varv[v->val].namei); break;
			case REGI: tknc = snprintf(tkn, 64, "math    %s %s %s", regitstr(v->regi), op, regitstr(v->val));}
			
			break;}
		case ZEROLIT:
			tknc = snprintf(tkn, 64, "zerolit %ld", lexer.tknv[lexv.tknvi + 1]);
			break;
		case MOV: {
			struct mov_s* v = lexer.movv + lexer.tknv[lexv.tknvi + 1];

			switch (v->desttype) {
			case REGI: tknc = snprintf(tkn, 64, "mov     %s = ", regitstr(v->dest)); break;
			case VARI: tknc = snprintf(tkn, 64, "mov     %s = ", lexer.strv + lexer.varv[v->dest].namei); break;
			default: tknc = snprintf(tkn, 64, "mov     ? (%ld) = ", v->dest); break;}
			 
			switch (v->valtype) {
			case REGI: tknc += snprintf(tkn + tknc, 64 - tknc, "%s", regitstr(v->val)); break;
			case VARI: tknc += snprintf(tkn + tknc, 64 - tknc, "%s", lexer.strv + lexer.varv[v->val].namei); break;
			case LIT: tknc += snprintf(tkn + tknc, 64 - tknc, "%ld", v->val); break;
			default: tknc += snprintf(tkn + tknc, 64 - tknc, "? (%ld)", v->val); break;}

			break;}
		default:
			tknc = snprintf(tkn, 64, "? (%ld)", lexer.tknv[lexv.tknvi]);}
		lexv.tknvi += 2;

		// tkn count number
		char linen[8];
		long linenc = snprintf(linen, 8, "%ld", ++lexv.tknvc);
		memmove(linen + 7 - linenc, linen, linenc);
		memset(linen, ' ', 7 - linenc);
		linen[7] = ' ';
		
		// append row
		tknc = tknc > 64 ? 64 : tknc;
		memmove(lexv.tknrowv + (8 + 64), lexv.tknrowv, 47 * (8 + 64));
		memcpy(lexv.tknrowv, linen, 8);
		memcpy(lexv.tknrowv + 8, tkn, 64);
		memset(lexv.tknrowv + 8 + tknc, ' ', 64 - tknc);
		++lexv.newtknrowi;
		newtoken = 1;}

	if (newline || newtoken) {
		printf("\033[s");	
		for (long i = 0; i < 48; ++i) {
			if (i == 0 && newline) printf("\033[47m\033[30m");
			printf("\033[%d;1H%.*s\033[0m", abs((int)i - 48), 8 + 64, lexfv->wrowv + (i * (8 + 64)));
			if (i < lexv.newtknrowi) printf("\033[47m\033[30m");
			printf("%.*s\033[0m\033[K", 8 + 64, lexv.tknrowv + (i * (8 + 64)));}
		printf("\033[u");
		fflush(stdout);
		newtoken = 0;

		static struct timespec ts = {0};
		ts.tv_sec = floor(delay);
		ts.tv_nsec = (delay - floor(delay)) * 1000000000;
		nanosleep(&ts, 0);}
	
	if (newline) {
		newline = 0;
		lexv.newtknrowi = 0;}}
