#include "../inc/print.h"
#include "../inc/glob.h"
#include "../inc/lex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <math.h>

struct lexfv_s* lexfvv = 0; int lexfvc = 0;
struct lexv_s lexv;
struct lexav_s lexav;

static int lexfvicache = -1;

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
		int lexvi = 0;
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

	int newline = 0;
	int newtoken = 0;
	
	while (lexf->stri > lexfv->strvi) {
		switch (lexf->str[lexfv->strvi]) {
		case '\n':
			++lexf->strvlinec;
			
			// ding code line number
			char linen[8];
			int linenc = snprintf(linen, 8, "%ld", lexf->strvlinec);
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
			
		       	lexf->strvlinei = lexfv->strvi + 1;
			newline = 1;
			break;
		case '\t':
			// ding code, append 8 spaces (shortened if overflow)
			if (lexfv->wi > 63) break;
			int spacec = abs((lexfv->wi % 8) - 8);
			int overflow = lexfv->wi + spacec > 63;
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
		char tkn[65]; int tknc = 0;
		switch (lexer.tknv[lexv.tknvi]) {
		case ASSEMBLY: {
			struct asm_s* v = lexer.asmv + lexer.tknv[lexv.tknvi + 1];
			
			// format asm str to replace '\n' with "\n" and '\t' with "\t"
			char* fv = 0;
			for (int vi = 0, fvi = 0; lexer.strv[v->stri + vi] != 0; ++vi) {
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

			tknc = snprintf(tkn, 64, "asm    %s", fv);
			break;}	
		case VARDEF: { 
			struct var_s* v = lexer.varv + lexer.tknv[lexv.tknvi + 1];
			tknc = snprintf(tkn, 64, "vardef %s", lexer.strv + v->namei);
			break;}
		case NUMLIT: {
			struct numlit_s* v = lexer.numlitv + lexer.tknv[lexv.tknvi + 1];
			tknc = snprintf(tkn, 64, "numlit %ld", v->num);
			break;}}
		lexv.tknvi += 2;

		// tkn count number
		char linen[8];
		int linenc = snprintf(linen, 8, "%d", ++lexv.tknvc);
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
		for (int i = 0; i < 48; ++i) {
			if (i == 0 && newline) printf("\033[47m\033[30m");
			printf("\033[%d;1H%.*s\033[0m", abs(i - 48), 8 + 64, lexfv->wrowv + (i * (8 + 64)));
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

void asmprint() {
	int newline = 0;
	int newtoken = 0;
	
	while (lexer.tknc - 1 >= lexv.tknvi) { 
		char tkn[65]; int tknc = 0;
		switch (lexer.tknv[lexv.tknvi]) {
		case ASSEMBLY: {
			struct asm_s* v = lexer.asmv + lexer.tknv[lexv.tknvi + 1];
			
			// format asm str to replace '\n' with "\n" and '\t' with "\t"
			char* fv = 0;
			for (int vi = 0, fvi = 0; lexer.strv[v->stri + vi] != 0; ++vi) {
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

			tknc = snprintf(tkn, 64, "asm    %s", fv);
			break;}	
		case VARDEF: { 
			struct var_s* v = lexer.varv + lexer.tknv[lexv.tknvi + 1];
			tknc = snprintf(tkn, 64, "vardef %s", lexer.strv + v->namei);
			break;}
		case NUMLIT: {
			struct numlit_s* v = lexer.numlitv + lexer.tknv[lexv.tknvi + 1];
			tknc = snprintf(tkn, 64, "numlit %ld", v->num);
			break;}}
		lexv.tknvi += 2;

		// tkn count number
		char linen[8];
		int linenc = snprintf(linen, 8, "%d", ++lexv.tknvc);
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
		for (int i = 0; i < 48; ++i) {
			if (i == 0 && newline) printf("\033[47m\033[30m");
			//printf("\033[%d;1H%.*s\033[0m", abs(i - 48), 8 + 64, lexfv->wrowv + (i * (8 + 64)));
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
