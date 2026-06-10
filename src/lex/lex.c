#include "../../inc/lex/lex.h"
#include "../../inc/lex/eval.h"
#include "../../inc/lex/new.h"
#include "../../inc/lex/enum.h"
#include "../../inc/glob.h"
#include "../../inc/print.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

// local func defs
static void lexinit();
static void lexwrite();
static void lexfinit(long);
static void lexfkill();
static void lexfprog();

// global vars
struct lexer_s lexer = {0};
struct lexf_s* lexf = {0};

// local vars
static struct lexf_s* lexfv = 0; static long lexfc = 0;	

// global funcs
void lex() {
	printinit();
	lexinit();

	// create file lex struct
	lexfinit(startfi >= 0 ? startfi : 0);	
	if (strcmp(fv[lexf->fi].dir, fv[lexf->fi].alias) == 0) printf("\033[36mentry\033[0m %s\033[K\n", fv[lexf->fi].dir);
	else printf("\033[36mentry\033[0m %s \"%s\"\033[K\n", fv[lexf->fi].dir, fv[lexf->fi].alias);

	// execute until no lexfs left	
	while (lexf) {
		while (lexf->str[lexf->stri]) {
			// progress to next token
			lexfprog();

			// if include token, promote new file lex struct
			if (lexf->jumpfi >= 0) {
				lexfinit(lexf->jumpfi);
				if (strcmp(fv[lexf->fi].dir, fv[lexf->fi].alias) == 0) printf("\033[36mjump\033[0m %s\033[K\n", fv[lexf->fi].dir);
				else printf("\033[36mjump\033[0m %s \"%s\"\033[K\n", fv[lexf->fi].dir, fv[lexf->fi].alias);
				continue;}}
		
		// kill used up lexf
		lexfkill();
		if (lexf != 0) {
			if (strcmp(fv[lexf->fi].dir, fv[lexf->fi].alias) == 0) printf("\033[36mreturn\033[0m %s\033[K\n", fv[lexf->fi].dir);
			else printf("\033[36mreturn\033[0m %s \"%s\"\033[K\n", fv[lexf->fi].dir, fv[lexf->fi].alias);}}
	
	lexwrite();
	printf("\033[36mfinished\033[0m %s (%ldB)\033[K\n", fv[lexfi].dir, ftell(fv[lexfi].f));

	lexfree();}

void lexfree() {
	memset(&lexer, 0, sizeof(struct lexer_s));}

void lexread() {
	FILE* f = fv[lexfi].f;
	rewind(f);

	lexinit();
	
	fread(&lexer.strc, sizeof(long), 1, f);
	lexer.strv = malloc(lexer.strc);	
	fread(lexer.strv, 1, lexer.strc, f);
	
	fread(&lexer.tknc, sizeof(long), 1, f);
	lexer.tknv = malloc(lexer.tknc * sizeof(long));
	fread(lexer.tknv, sizeof(long), lexer.tknc, f);
	
	fread(&lexer.keywc, sizeof(long), 1, f);
	lexer.keywv = malloc(lexer.keywc * sizeof(long));
	fread(lexer.keywv, sizeof(long), lexer.keywc, f);
	
	fread(&lexer.sectc, sizeof(long), 1, f);
	lexer.sectv = malloc(lexer.sectc * sizeof(long));
	fread(lexer.sectv, sizeof(long), lexer.sectc, f);
	
	fread(&lexer.typec, sizeof(long), 1, f);
	lexer.typev = malloc(lexer.typec * sizeof(struct type_s));
	fread(lexer.typev, sizeof(struct type_s), lexer.typec, f);
	
	fread(&lexer.asmc, sizeof(long), 1, f);
	lexer.asmv = malloc(lexer.asmc * sizeof(long));
	fread(lexer.asmv, sizeof(struct asm_s), lexer.asmc, f);
	
	fread(&lexer.strlitc, sizeof(long), 1, f);
	lexer.strlitv = malloc(lexer.strlitc * sizeof(struct strlit_s));
	fread(lexer.strlitv, sizeof(struct strlit_s), lexer.strlitc, f);
	
	fread(&lexer.numlitc, sizeof(long), 1, f);
	lexer.numlitv = malloc(lexer.numlitc * sizeof(struct numlit_s));
	fread(lexer.numlitv, sizeof(struct numlit_s), lexer.numlitc, f);
	
	fread(&lexer.varc, sizeof(long), 1, f);
	lexer.varv = malloc(lexer.varc * sizeof(struct var_s));
	fread(lexer.varv, sizeof(struct var_s), lexer.varc, f);
	for (long i = 0; i < lexer.varc; ++i) {
		lexer.varv[i].sectiv = malloc(lexer.varv[i].sectic * sizeof(long));
		fread(lexer.varv[i].sectiv, sizeof(long), lexer.varv[i].sectic, f);}

	fread(&lexer.structc, sizeof(long), 1, f);
	fread(lexer.structv, sizeof(struct struct_s), lexer.structc, f);
	for (long i = 0; i < lexer.structc; ++i) { 
		lexer.structv[i].tkniv = malloc(lexer.structv[i].tknic * sizeof(long));
		fread(lexer.structv[i].tkniv, sizeof(long), lexer.structv[i].tknic, f);}
	
	fread(&lexer.inlinec, sizeof(long), 1, f);
	fread(lexer.inlinev, sizeof(struct inline_s), lexer.inlinec, f);
	for (long i = 0; i < lexer.inlinec; ++i) {
		lexer.inlinev[i].argiv = malloc(lexer.inlinev[i].argic * sizeof(long));
		fread(lexer.inlinev[i].argiv, sizeof(long), lexer.inlinev[i].argic, f);}

	fread(&lexer.mathc, sizeof(long), 1, f);
	lexer.mathv = malloc(lexer.mathc * sizeof(struct math_s));
	fread(lexer.mathv, sizeof(struct math_s), lexer.mathc, f);
	
	fread(&lexer.movc, sizeof(long), 1, f);
	lexer.movv = malloc(lexer.movc * sizeof(struct mov_s));
	fread(lexer.movv, sizeof(struct mov_s), lexer.movc, f);}

// local funcs
static void lexinit() {
	memset(&lexer, 0, sizeof(struct lexer_s));
	
	// create lex struct
	lexer.keywv = 0; lexer.keywc = 0;
	lexer.sectv = 0; lexer.sectc = 0;
	lexer.typev = 0; lexer.typec = 0;
	lexer.varv = 0; lexer.varc = 0;

	// create sects
	newsect("write");
	newsect("exec");
	newsect("zero");

	// create types
	newtype("char", 1);
	newtype("short", 2);
	newtype("int", 4);
	newtype("long", 8);
	newtype("schar", 1);
	newtype("sshort", 2);
	newtype("sint", 4);
	newtype("long", 8);}

static void lexwrite() {
	if (lexfi < 0) return;
	FILE* f = fv[lexfi].f;
	rewind(f);
	ftruncate(fileno(f), 0);

	fwrite(&lexer.strc, sizeof(long), 1, f);
	fwrite(lexer.strv, 1, lexer.strc, f);
	
	fwrite(&lexer.tknc, sizeof(long), 1, f);
	fwrite(lexer.tknv, sizeof(long), lexer.tknc, f);
	
	fwrite(&lexer.keywc, sizeof(long), 1, f);
	fwrite(lexer.keywv, sizeof(long), lexer.keywc, f);
	
	fwrite(&lexer.sectc, sizeof(long), 1, f);
	fwrite(lexer.sectv, sizeof(long), lexer.sectc, f);

	fwrite(&lexer.typec, sizeof(long), 1, f);
	fwrite(lexer.typev, sizeof(struct type_s), lexer.typec, f);
	
	fwrite(&lexer.asmc, sizeof(long), 1, f);
	fwrite(lexer.asmv, sizeof(struct asm_s), lexer.asmc, f);
	
	fwrite(&lexer.strlitc, sizeof(long), 1, f);
	fwrite(lexer.strlitv, sizeof(struct strlit_s), lexer.strlitc, f);
	
	fwrite(&lexer.numlitc, sizeof(long), 1, f);
	fwrite(lexer.numlitv, sizeof(struct numlit_s), lexer.numlitc, f);
	
	fwrite(&lexer.varc, sizeof(long), 1, f);
	fwrite(lexer.varv, sizeof(struct var_s), lexer.varc, f);
	for (long i = 0; i < lexer.varc; ++i) fwrite(lexer.varv[i].sectiv, sizeof(long), lexer.varv[i].sectic, f);

	fwrite(&lexer.structc, sizeof(long), 1, f);
	fwrite(lexer.structv, sizeof(struct struct_s), lexer.structc, f);
	for (long i = 0; i < lexer.structc; ++i) fwrite(lexer.structv[i].tkniv, sizeof(long), lexer.structv[i].tknic, f);
	
	fwrite(&lexer.inlinec, sizeof(long), 1, f);
	fwrite(lexer.inlinev, sizeof(struct inline_s), lexer.inlinec, f);
	for (long i = 0; i < lexer.inlinec; ++i) fwrite(lexer.inlinev[i].argiv, sizeof(long), lexer.inlinev[i].argic, f);

	fwrite(&lexer.mathc, sizeof(long), 1, f);
	fwrite(lexer.mathv, sizeof(struct math_s), lexer.mathc, f);
	
	fwrite(&lexer.movc, sizeof(long), 1, f);
	fwrite(lexer.movv, sizeof(struct mov_s), lexer.movc, f);}

static void lexfinit(long fi) {
	// allocate new lexf
	lexfv = realloc(lexfv, sizeof(struct lexf_s) * ++lexfc);
	lexf = lexfv + (lexfc - 1);

	// clear lexf
	memset(lexf, 0, sizeof(struct lexf_s));

	// create str from file
	lexf->fi = fi;
	FILE* f = fv[lexf->fi].f;

	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	rewind(f);
	lexf->str = malloc(len + 1);
	fread(lexf->str, 1, len, f);
	lexf->str[len] = 0;
	rewind(f);
	lexf->str = strdup(lexf->str);}

static void lexfkill() {
	// free lexf members
	free(lexf->str);
	
	// clear lexf
	memset(lexf, 0, sizeof(struct lexf_s));

	// reached last token, kill lexf and demote to previously promoted lexf
	if (--lexfc > 0) {
		lexfv = realloc(lexfv, sizeof(struct lexf_s) * lexfc);
		lexf = lexfv + (lexfc - 1);}
	// no lexf to demote to? free the array of lexfs and leave loop
	else {
		free(lexfv);
		lexfv = 0;
		lexf = 0;}}

static void vardef() {
	lexf->flgdef = 0;
	
	long tknval = 0;
	long namei = -1, scopei = -1;	
	
	if (lexf->def.name) {
		// push aliasv
		lexf->aliasv = realloc(lexf->aliasv, sizeof(char*) * ++lexf->aliasc);
		lexf->aliasv[lexf->aliasc - 1] = strdup(lexf->def.name);

		// push aliasiv
		if (lexf->str[lexf->stri - 1] == '{') {
			lexf->aliasiv = realloc(lexf->aliasiv, sizeof(long) * ++lexf->aliasic);
			lexf->aliasiv[lexf->aliasic - 1] = lexf->aliasc;}
	
		namei = lexer.strc;
		newstr(lexf->def.name);
		scopei = lexer.strc;
		newstr(lexf->def.scope);}
		
	// if latest definition in the definition hierarchy is a struct,
	lexf->tkn = VARDEF;
	tknval = lexer.varc;
	lexer.varv = realloc(lexer.varv, sizeof(struct var_s) * ++lexer.varc);
	lexer.varv[tknval] = (struct var_s){namei, scopei, lexf->def.sectiv, lexf->def.sectic, lexf->def.typei, lexf->def.structi};

	// if { or (, push definition on to the definition hierachy
	if (lexf->str[lexf->stri - 1] == '{') {
		lexf->defiv = realloc(lexf->defiv, sizeof(struct def_s) * ++lexf->defic);
		lexf->defiv[lexf->defic - 1] = lexf->def;
		lexf->defiv[lexf->defic - 1].name = lexf->defiv[lexf->defic -1].scope = 0;
		lexf->defiv[lexf->defic - 1].namei = namei;
		lexf->defiv[lexf->defic - 1].scopei = scopei;
		printf("scope %s scopei %ld\n", lexer.strv + lexf->defiv[lexf->defic -1].scopei, lexf->defiv[lexf->defic - 1].scopei);}

	newtkn(lexf->tkn, tknval);	
	
	free(lexf->def.name);
	free(lexf->def.scope);
	lexf->def = (struct def_s){0, -1, 0, -1, -1, 0, 0, -1, -1};}

static void lexfprog() {
	// clear data left by previous cycle
	lexf->jumpfi = -1;
	lexf->tkn = -1;

	prog:
	lexprint(lexf);
	if (!lexf->str[lexf->stri]) return;
	
	switch (lexf->str[lexf->stri]) {
	// ignore
	case '\n':
	case ' ':
	case '\t':
		lexf->strtkni = ++lexf->stri;
		break;
	// single char tokens
	case '+': // add op
		lexf->strtkni = ++lexf->stri;
		plugop(ADD);
		break;
	case '-': // sub op
		lexf->strtkni = ++lexf->stri;
		plugop (SUB);
		break;
	case '*': // mul op
		lexf->strtkni = ++lexf->stri;
		plugop(MUL);
		break;
	case '/': // comments & div op
		if (lexf->str[lexf->stri + 1] == '/') {
			while (lexf->str[lexf->stri + 3] != '\n' && !(lexf->str[lexf->stri + 2] == '/' && lexf->str[lexf->stri + 1] == '/' && lexf->str[lexf->stri] != '\\')) ++lexf->stri;
			lexf->stri += 3;
			break;}
		else {
			lexf->strtkni = ++lexf->stri;
			plugop(DIV);
			break;}
	case '(': // for marking the beginning of making or parsing arguments
		lexf->strtkni = ++lexf->stri;
		
		plugop(OB);

		break;
	
		// if defining, and the definition has inline keyword
		if (!lexf->def.name) break;
		if (!(lexf->def.keywi >= 0 && strcmp(lexer.strv + lexer.keywv[lexf->def.keywi], "inline") == 0)) break;
		
		break;
	case ')': // for marking the end of making or parsing arguments
		lexf->strtkni = ++lexf->stri;

		plugop(CB);

		break;
	case '{': // for marking the beginning of setting a variables value
		lexf->strtkni = ++lexf->stri;

		// define
		vardef();
		break;
	case '}': // for marking the end of setting a variables value
		lexf->strtkni = ++lexf->stri;

		// pop aliasiv & aliasv
		if (--lexf->aliasic) {
			if (lexf->aliasc -= (lexf->aliasc - lexf->aliasiv[lexf->aliasic])) lexf->aliasv = realloc(lexf->aliasv, sizeof(char*) * lexf->aliasc);
			else {
				free(lexf->aliasv);
				lexf->aliasv = 0;}
			lexf->aliasiv = realloc(lexf->aliasiv, sizeof(long) * lexf->aliasic);}
		else {
			free(lexf->aliasiv);
			lexf->aliasiv = 0;}
		
		// pop defiv
		if (--lexf->defic) lexf->defiv = realloc(lexf->defiv, sizeof(long) * lexf->defic);
		else {
			free(lexf->defiv);
			lexf->defiv = 0;}
		break;
	case '=': // for mov operations
		lexf->strtkni = ++lexf->stri;
		lexf->flgmov = 1;

		struct eval_s evals = eval(0);
		lexer.movv = realloc(lexer.movv, sizeof(struct mov_s) * ++lexer.movc);
		lexer.movv[lexer.movc - 1] = (struct mov_s){evals.val, evals.type, 0, -1};
		
		break;
	case ';': // for marking the end of a statement
		lexf->strtkni = ++lexf->stri;
		lexf->flgassembly = 0;

		if (lexf->flgmov) {
			struct eval_s evals = eval(lexer.movv[lexer.movc - 1].desttype == REGI ? 1 : 0);
			lexer.movv[lexer.movc - 1].val = evals.val;
			lexer.movv[lexer.movc - 1].valtype = evals.type;
			
			newtkn(MOV, lexer.movc - 1);
			lexf->flgmov = 0;

			break;}
		
		if (lexf->flgdef) vardef(); 
		eval(0);
		break;
	default: {
		// set tkn to relevant enum value
		lexf->tkn = 
		// from flags
			lexf->flgassembly ? ASSEMBLY : (
			lexf->flgjump ? INCLUDE : (
		// from character
			lexf->str[lexf->strtkni] == '\'' || (lexf->str[lexf->strtkni] >= '0' && lexf->str[lexf->strtkni] <= '9') || lexf->str[lexf->strtkni] == '-' ? NUMLIT : (
			lexf->str[lexf->strtkni] == '\"' ? STR : -1)));
		
		// kill flags
		lexf->flgassembly = lexf->flgjump = 0;


		// get the index of the new token, thus marking a boundary in str which is relevant for the next tokenization
		long strnewtkni = lexf->strtkni;
		switch (lexf->tkn) {
		// if assembly or include, select until semicolon
		case ASSEMBLY:
		case INCLUDE:
			while (lexf->str[strnewtkni] != ';') ++strnewtkni;
			break;
		// if str, select until next dquote, unless a backslash precedes the dquote
		case STR:
			while (strnewtkni <= lexf->strtkni + 2 || lexf->str[strnewtkni - 1] != '"' || (strnewtkni > 0 && lexf->str[strnewtkni - 2] == '\\')) ++strnewtkni;
			break;
		// if long, select
		case NUMLIT: {
			long hex = lexf->str[strnewtkni] == '0' && lexf->str[strnewtkni + 1] == 'x';
			if (lexf->str[strnewtkni] == '-') ++strnewtkni;
			if (lexf->str[strnewtkni] == '0' && (lexf->str[strnewtkni + 1] == 'b' || lexf->str[strnewtkni + 1] == 'x')) strnewtkni += 2;
			else if (lexf->str[strnewtkni] == '\'') ++strnewtkni;
			if (hex) while ((lexf->str[strnewtkni] >= 'A' && lexf->str[strnewtkni] <= 'F') || (lexf->str[strnewtkni] >= 'a' && lexf->str[strnewtkni] <= 'f') || (lexf->str[strnewtkni] >= '0' && lexf->str[strnewtkni] <= '9')) ++strnewtkni;
			else while (lexf->str[strnewtkni] >= '0' && lexf->str[strnewtkni] <= '9') ++strnewtkni;
			break;}
		// if uninitialized tkn, select while
		case -1:
		// # if its the first character OR
			while ((lexf->str[strnewtkni] == '#' && lexf->strtkni == strnewtkni)
		// underscore OR
				|| lexf->str[strnewtkni] == '_' 
		// alphanumeric
				|| (lexf->str[strnewtkni] >= 'a' && lexf->str[strnewtkni] <= 'z') 
				|| (lexf->str[strnewtkni] >= 'A' && lexf->str[strnewtkni] <= 'Z') 
				|| (lexf->str[strnewtkni] >= '0' && lexf->str[strnewtkni] <= '9')) ++strnewtkni;}
		
		// invalid? just ignore it...
		if (lexf->strtkni == strnewtkni) {
			++lexf->stri;
			lexf->strtkni = lexf->stri;
			break;}
		
		// alloc tknstr from strnewtkni and strtkni (old token index)
		long tknstrlen = strnewtkni - lexf->strtkni;
		char* tknstr = malloc(tknstrlen + 1);
		memcpy(tknstr, lexf->str + lexf->strtkni, tknstrlen);
		tknstr[tknstrlen] = 0;
		
		// append data structures from initialized tkn
		long tknval; // the index of the relevant value in the tokens relevant data structure
		switch (lexf->tkn) {
		case STR: {
			tknval = lexer.strlitc;
			lexer.strlitv = realloc(lexer.strlitv, sizeof(struct strlit_s) * ++lexer.strlitc);
			lexer.strlitv[lexer.strlitc - 1] = (struct strlit_s){lexer.strc, lexf->defic ? lexf->defiv[lexf->defic - 1].scopei : -1};
			
			newnstr(tknstr + 1, tknstrlen - 2);
			
			goto disctkn;}
		case NUMLIT: {
			// get value
			long l;
			if (tknstr[0] == '\'') l = (long)tknstr[1];
			else {
				long base = tknstr[0] != '0' ? 10 : (
					tknstr[1] == 'b' ? 2 : (
					tknstr[1] == 'x' ? 16 : 
					8));
				char* strnum = base == 10 ? tknstr : 
					(base == 8 ? tknstr + 1 : 
					tknstr + 2);
				l = strtol(strnum, 0, base);}

			plugl(l);
			goto disctkn;}
		case ASSEMBLY: {
			tknval = lexer.asmc;
			lexer.asmv = realloc(lexer.asmv, sizeof(struct asm_s) * ++lexer.asmc);
			lexer.asmv[lexer.asmc - 1] = (struct asm_s){lexer.strc, lexf->defic ? lexf->defiv[lexf->defic - 1].scopei : -1};
			
			char* asmstr = 0;
			long asmstrlen = 0;
			
			for (long i = 0; i < tknstrlen + 1; ++i)
				if (tknstr[i] != '\t') {
					asmstr = realloc(asmstr, ++asmstrlen);
					asmstr[asmstrlen - 1] = tknstr[i];}
			if (asmstr[asmstrlen - 2] != '\n') {
				asmstr = realloc(asmstr, ++asmstrlen);
				asmstr[asmstrlen - 2] = '\n';
				asmstr[asmstrlen - 1] = 0;}

			newstr(asmstr);
			break;}
		case INCLUDE: {
			lexf->jumpfi = -1;
			for (long i = 0; i < fc; ++i)  
				if (strcmp(fv[i].alias, tknstr) == 0) {
					lexf->jumpfi = i;
					break;}
			goto disctkn;}
		
		// append data structures from uninitialized tkn
		case -1: {
			// if not preparing for a definition (marked by flgdef),
			if (!lexf->flgdef) {
				// dont tokenize, and set relevant flags if strtkn is a certain string (good for skipping unnecessary logic)
				if (strcmp(tknstr, "#asm") == 0) {
					lexf->flgassembly = 1;
					goto disctkn;}
				else if (strcmp(tknstr, "#include") == 0) {
					lexf->flgjump = 1;
					goto disctkn;}

				// if name of a var's name in current scope (such are held in aliasv), plug it
				for (long i = 0; i < lexf->aliasc; ++i) {
					if (strcmp(lexf->aliasv[i], tknstr) != 0) continue;
					for (long j = 0; j < lexer.varc; ++j) {
						if (strcmp(lexer.strv + lexer.varv[j].namei, tknstr) != 0) continue;
						// found var.
						
						plugvar(j);
						goto disctkn;}}}
				
			// make a scoped version of tknstr for var names that are changed via being scoped
			char* tknstrscoped = strdup(tknstr);
			if (lexf->defic) {
				char* scope = 0;
				for (long i = 0; i < lexf->defic; ++i) {
					char* name = lexer.strv + lexf->defiv[i].namei;
					if (scope) {
						char* old = strdup(scope);
						scope = realloc(scope, strlen(scope) + 1 + strlen(name) + 1);
						sprintf(scope, "%s.%s", old, name);}
					else {
						scope = strdup(name);}}
				if (scope) {
					char* old = strdup(scope);
					tknstrscoped = realloc(tknstrscoped, strlen(scope) + 1 + tknstrlen + 1);
					sprintf(tknstrscoped, "%s.%s", old, tknstr);}}
			
			// if not preparing for a definition (marked by flgdef),
			if (!lexf->flgdef) {
				// if none of the previous logic diverted flow, create a def object that prepares for a new structdef, vardef, inlinedef, etc..
				lexf->def = (struct def_s){0, -1, 0, -1, -1, 0, 0, -1, -1};
				lexf->flgdef = 1;}
			
			// always discards the token beyond this polong to prepare the def... the def finally gets defined upon a ; or a : of course
			// if preparing for a definition, append relevant data structures from tknstr
			for (long i = 0; i < lexer.structc; ++i)
				if (strcmp(lexer.strv + lexer.structv[i].scopei, tknstrscoped) == 0) {
					lexf->def.structi = i;
					goto disctkn;}
			for (long i = 0; i < lexer.sectc; ++i)
				if (strcmp(lexer.strv + lexer.sectv[i], tknstr) == 0) {
					lexf->def.sectiv = realloc(lexf->def.sectiv, sizeof(long) * ++lexf->def.sectic);
					lexf->def.sectiv[lexf->def.sectic - 1] = i;
					goto disctkn;}
			for (long i = 0; i < lexer.typec; ++i)
				if (strcmp(lexer.strv + lexer.typev[i].namei, tknstr) == 0) {
					lexf->def.typei = i;
					goto disctkn;}
			for (long i = 0; i < lexer.keywc; ++i)
				if (strcmp(lexer.strv + lexer.keywv[i], tknstr) == 0) {
					lexf->def.keywi = i;
					goto disctkn;}

			// if none of the previous logic was able to append a data structre from tknstr, set the defs name
			free(lexf->def.name);
			free(lexf->def.scope);
			lexf->def.name = strdup(tknstr);
			lexf->def.scope = tknstrscoped;
			goto disctkn;}}
		
		// appends the tkn data structure
		newtkn(lexf->tkn, tknval);
		
		// doesnt append the tkn data structure, and moves onto the next relevant character
		disctkn:
		lexf->stri = lexf->tkn == STR ? strnewtkni + 1 : strnewtkni;
		lexf->strtkni = lexf->stri;
		free(tknstr);
		break;}}

	if (lexf->tkn == -1) goto prog;}
