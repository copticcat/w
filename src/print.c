#include "../inc/print.h"
#include "../inc/lex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>

struct lexv_s* lexvv; int lexvc;

struct lexv_s {
	struct lexf_s* lexf;

	int strvi;
	char wrowv[48 * (8 + 64)];
	char w[64];
	int wi;};
	
static int tknvi;
static int tknvc;
static char tknrowv[48 * (8 + 64)];
static int newtknrowi;

void lexprint(struct lexf_s* lexf) {
	static int lexvicache = -1;
	struct lexv_s* lexv = 0;

	if (lexvicache >= 0 && lexvv[lexvicache].lexf == lexf)
		lexv = lexvv + lexvicache; 
	else {
		// make/get visual lex struct
		int lexvi = 0;
		for (; lexvi < lexvc; ++lexvi)
			if (lexvv[lexvi].lexf == lexf) {
				lexv = lexvv + lexvi;
				break;}
		if (!lexvi) memset(tknrowv, ' ', 48 * (8 + 64));
		if (!lexv) {
			lexvv = realloc(lexvv, sizeof(struct lexv_s) * ++lexvc);
			lexv = lexvv + (lexvc - 1);
			memset(lexv, 0, sizeof(struct lexv_s));
			lexv->lexf = lexf;
			memset(lexv->wrowv, ' ', 48 * (8 + 64));}
		lexvicache = lexvi;}

	int newline = 0;
	int newtoken = 0;
	
	while (lexf->stri > lexv->strvi) {
		switch (lexf->str[lexv->strvi]) {
		case '\n':
			++lexf->strvlinec;
			
			// .w code line number
			char linen[8];
			int linenc = snprintf(linen, 8, "%d", lexf->strvlinec);
			memmove(linen + 7 - linenc, linen, linenc);
			memset(linen, ' ', 7 - linenc);
			linen[7] = ' ';
			
			// .w code
			lexv->w[63] = ' ';
			
			// append row
			memmove(lexv->wrowv + (8 + 64), lexv->wrowv, 47 * (8 + 64));
			memcpy(lexv->wrowv, linen, 8);
			memcpy(lexv->wrowv + 8, lexv->w, 64);
			memset(lexv->wrowv + 8 + lexv->wi, ' ', 64 - lexv->wi);
			
			// reset .w code buf
			memset(lexv->w, 0, 64);
			lexv->wi = 0;
			
		       	lexf->strvlinei = lexv->strvi + 1;
			newline = 1;
			break;
		case '\t':
			// .w code, append 8 spaces (shortened if overflow)
			if (lexv->wi > 63) break;
			int spacec = abs((lexv->wi % 8) - 8);
			int overflow = lexv->wi + spacec > 63;
			spacec = overflow ? 64 - lexv->wi : spacec;
			memset(lexv->w + lexv->wi, ' ', spacec);
			lexv->wi = overflow ? 63 : lexv->wi + spacec;
			break;
		default:
			// .w code, append char
			if (lexv->wi > 63) break;
			lexv->w[lexv->wi++] = lexf->str[lexv->strvi];
			break;}
		++lexv->strvi;}
	
	while (lexer.tknc - 1 >= tknvi) { 
		char tkn[65]; int tknc = 0;
		char* k; char* v;
		switch (lexer.tknv[tknvi]) {
		case TYPE: 
			k = "TYPE"; 
			v = lexer.typev[*(long*)(lexer.tknv + tknvi + 1)];
			goto tknkv;
		case KEYWORD: 
			k = "KEYWORD"; 
			v = lexer.keywv[*(long*)(lexer.tknv + tknvi + 1)];
			goto tknkv;
		case DIRECTIVE: 
			k = "DIRECTIVE"; 
			v = lexer.dirv[*(long*)(lexer.tknv + tknvi + 1)];
			goto tknkv;
		case ASSEMBLY: 
			k = "ASSEMBLY";
			v = lexer.asmv[*(long*)(lexer.tknv + tknvi + 1)];
			goto tknkv;
		case SECTDEF:
		       	k = "SECTDEF";
			v = lexer.sectv[*(long*)(lexer.tknv + tknvi + 1)];
			goto tknkv;
		case SECT:
		       	k = "SECT";
			v = lexer.sectv[*(long*)(lexer.tknv + tknvi + 1)];
			goto tknkv;
		case STRUCTDEF:
		       	k = "STRUCTDEF";
			v = lexer.structv[*(long*)(lexer.tknv + tknvi + 1)].name;
			goto tknkv;
		case STRUCT: 
			k = "STRUCT";
			v = lexer.structv[*(long*)(lexer.tknv + tknvi + 1)].name;
			goto tknkv;
		case INLINEDEF:
		       	k = "INLINEDEF";
			v = lexer.inlinev[*(long*)(lexer.tknv + tknvi + 1)];
			goto tknkv;
		case INLINE: 
			k = "INLINE";
			v = lexer.inlinev[*(long*)(lexer.tknv + tknvi + 1)];
			goto tknkv;
		case VARDEF: 
			k = "VARDEF";
			v = lexer.varv[*(long*)(lexer.tknv + tknvi + 1)].name;
			goto tknkv;
		case VAR: 
			k = "VAR";
			v = lexer.varv[*(long*)(lexer.tknv + tknvi + 1)].name;
			goto tknkv;
		case INCLUDE:
			k = "INCLUDE";
			v = lexer.fv[*(long*)(lexer.tknv + tknvi + 1)];
			goto tknkv;
		case STR: 
			k = "STR";
			v = lexer.strv[*(long*)(lexer.tknv + tknvi + 1)];
			goto tknkv;
		case LONG: 
			k = "LONG";
			static char str[32];
			memset(str, 0, 32);
			snprintf(str, 32, "%ld", lexer.longv[*(long*)(lexer.tknv + tknvi + 1)]);
			v = str;
			goto tknkv;
		case INT: k = "INT"; v = ""; goto tknkv;
		case SHORT: k = "SHORT"; v = ""; goto tknkv;
		case CHAR: k = "CHAR"; v = ""; goto tknkv;
		case SLONG: k = "SLONG"; v = ""; goto tknkv;
		case SINT: k = "SINT"; v = ""; goto tknkv;
		case SSHORT: k = "SSHORT"; v = ""; goto tknkv;
		case SCHAR: k = "SCHAR"; v = ""; goto tknkv;
		tknkv: 
			// tkn
			tknc = snprintf(tkn, 65, "[%s:%s]", k, v);
			
			tknvi += 9;
			break;
 		case (ADD): k = "+"; goto tknk;
		case (SUB): k = "-"; goto tknk;
		case (MUL): k = "*"; goto tknk;
		case (DIV): k = "/"; goto tknk;
		case (MOD): k = "%"; goto tknk;
		case (AND): k = "&"; goto tknk;
		case (OR): k = "|"; goto tknk;
		case (XOR): k = "^"; goto tknk;
		case (NOT): k = "~"; goto tknk;
		case (CAST): k = "?"; goto tknk;
		case (MOV): k = ":"; goto tknk;
		case (CASTMOV): k = "?:"; goto tknk;
		case (ADDMOV): k = "+:"; goto tknk;
		case (SUBMOV): k = "-:"; goto tknk;
		case (MULMOV): k = "*:"; goto tknk;
		case (DIVMOV): k = "/:"; goto tknk;
		case (MODMOV): k = "%:"; goto tknk;
		case (ANDMOV): k = "&:"; goto tknk;
		case (ORMOV): k = "|:"; goto tknk;
		case (XORMOV): k = "^:"; goto tknk;
		case (NOTMOV): k = "~:"; goto tknk;
		case (EQUALMOV): k = "=:"; goto tknk;
		case (NOTEQUALMOV): k = "~=:"; goto tknk;
		case (GREATEREQUALMOV): k = ">=:"; goto tknk;
		case (LESSEREQUALMOV): k = "<=:"; goto tknk;
		case (ANDBOOLMOV): k = "&&:"; goto tknk;
		case (ORBOOLMOV): k = "||:"; goto tknk;
		case (NOTBOOLMOV): k = "~~:"; goto tknk;
		case (GREATERMOV): k = ">:"; goto tknk;
		case (LESSERMOV): k = "<:"; goto tknk;
		case (ADDRMOV): k = "$:"; goto tknk;
		case (DEREFMOV): k = "@:"; goto tknk;
		case (EQUAL): k = "="; goto tknk;
		case (NOTEQUAL): k = "~="; goto tknk;
		case (GREATEREQUAL): k = ">="; goto tknk;
		case (LESSEREQUAL): k = "<="; goto tknk;
		case (ANDBOOL): k = "&&"; goto tknk;
		case (ORBOOL): k = "||"; goto tknk;
		case (NOTBOOL): k = "~~"; goto tknk;
		case (GREATER): k = ">"; goto tknk;
		case (LESSER): k = "<"; goto tknk;
		case (ADDR): k = "$"; goto tknk;
		case (DEREF): k = "@"; goto tknk;
		case (DOT): k = "."; goto tknk;
		case (ODEREF): k = "["; goto tknk;
		case (CDEREF): k = "]"; goto tknk;
		case (JUMP): k = "\\"; goto tknk;
		case (CALL): k = "!"; goto tknk;
		case (OFUNC): k = "("; goto tknk;
		case (CFUNC): k = ")"; goto tknk;
		case (COMMA): k = ","; goto tknk;
		case (OLIST): k = "{"; goto tknk;
		case (CLIST): k = "}"; goto tknk;
		case (DQUOTE): k = "\""; goto tknk;
		case (SQUOTE): k = "'"; goto tknk;
		case (TERM): k = ";"; goto tknk;
		case (OBRACK): k = "("; goto tknk;
		case (CBRACK): k = ")"; goto tknk;
		tknk: 
			// tkn
			tknc = snprintf(tkn, 65, "[%s]", k);
			
			++tknvi;
			break;
		default:
			++tknvi;
			++tknvc;
			break;}

		if (!tknc) continue;

		// tkn count number
		char linen[8];
		int linenc = snprintf(linen, 8, "%d", ++tknvc);
		memmove(linen + 7 - linenc, linen, linenc);
		memset(linen, ' ', 7 - linenc);
		linen[7] = ' ';
		
		// append row
		memmove(tknrowv + (8 + 64), tknrowv, 47 * (8 + 64));
		memcpy(tknrowv, linen, 8);
		memcpy(tknrowv + 8, tkn, 64);
		memset(tknrowv + 8 + tknc, ' ', 64 - tknc);
		++newtknrowi;
		newtoken = 1;}

	if (newline || newtoken) {
		printf("\033[s");	
		for (int i = 0; i < 48; ++i) {
			if (i == 0 && newline) printf("\033[47m\033[30m");
			printf("\033[%d;1H%.*s\033[0m", abs(i - 48), 8 + 64, lexv->wrowv + (i * (8 + 64)));
			if (i < newtknrowi) printf("\033[47m\033[30m");
			printf("%.*s\033[0m\033[K", 8 + 64, tknrowv + (i * (8 + 64)));}
		printf("\033[u");
		fflush(stdout);
		newtoken = 0;

		static struct timespec ts = {0, 1000000000/100};
		nanosleep(&ts, 0);}
	
	if (newline) {
		newline = 0;
		newtknrowi = 0;}}
