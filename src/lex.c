#include "../inc/lex.h"
#include "../inc/glob.h"
#include "../inc/print.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

// local func defs
static void lexinit();
static void lexwrite();
static struct lexf_s* lexfinit(int);
static struct lexf_s* lexfkill(struct lexf_s*);
static void lexfprog(struct lexf_s*);

// global vars
struct lexer_s lexer = {0};

// local vars
static struct lexf_s* lexfv = 0; static int lexfc = 0;	

// global funcs
void lex() {
	printinit();
	lexinit();

	// create file lex struct
	struct lexf_s* lexf = lexfinit(startfi >= 0 ? startfi : 0);	
	if (strcmp(fv[lexf->fi].dir, fv[lexf->fi].alias) == 0) printf("\033[36mentry\033[0m %s\033[K\n", fv[lexf->fi].dir);
	else printf("\033[36mentry\033[0m %s \"%s\"\033[K\n", fv[lexf->fi].dir, fv[lexf->fi].alias);

	// execute until no lexfs left	
	while (lexf) {
		while (lexf->str[lexf->stri]) {
			// progress to next token
			lexfprog(lexf);

			// if include token, promote new file lex struct
			if (lexf->jumpfi >= 0) {
				lexf = lexfinit(lexf->jumpfi);
				if (strcmp(fv[lexf->fi].dir, fv[lexf->fi].alias) == 0) printf("\033[36mjump\033[0m %s\033[K\n", fv[lexf->fi].dir);
				else printf("\033[36mjump\033[0m %s \"%s\"\033[K\n", fv[lexf->fi].dir, fv[lexf->fi].alias);
				continue;}}
		
		// kill used up lexf
		lexf = lexfkill(lexf);
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
	for (int i = 0; i < lexer.varc; ++i) {
		lexer.varv[i].sectiv = malloc(lexer.varv[i].sectic * sizeof(long));
		fread(lexer.varv[i].sectiv, sizeof(long), lexer.varv[i].sectic, f);}

	fread(&lexer.structc, sizeof(long), 1, f);
	fread(lexer.structv, sizeof(struct struct_s), lexer.structc, f);
	for (int i = 0; i < lexer.structc; ++i) { 
		lexer.structv[i].tkniv = malloc(lexer.structv[i].tknic * sizeof(long));
		fread(lexer.structv[i].tkniv, sizeof(long), lexer.structv[i].tknic, f);}
	
	fread(&lexer.inlinec, sizeof(long), 1, f);
	fread(lexer.inlinev, sizeof(struct inline_s), lexer.inlinec, f);
	for (int i = 0; i < lexer.inlinec; ++i) {
		lexer.inlinev[i].argiv = malloc(lexer.inlinev[i].argic * sizeof(long));
		fread(lexer.inlinev[i].argiv, sizeof(long), lexer.inlinev[i].argic, f);}}

// local funcs
static void newtype(char* str, int width) {
	lexer.typev = realloc(lexer.typev, sizeof(struct type_s) * ++lexer.typec);
	lexer.typev[lexer.typec - 1] = (struct type_s){lexer.strc, width};
	long len = strlen(str);
	lexer.strv = realloc(lexer.strv, lexer.strc += len + 1);
	memcpy(lexer.strv + lexer.strc - len - 1, str, len + 1);}

static void newsect(char* str) {
	lexer.sectv = realloc(lexer.sectv, sizeof(long) * ++lexer.sectc);
	lexer.sectv[lexer.sectc - 1] = lexer.strc;
	long len = strlen(str);
	lexer.strv = realloc(lexer.strv, lexer.strc += len + 1);
	memcpy(lexer.strv + lexer.strc - len - 1, str, len + 1);}

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
	for (int i = 0; i < lexer.varc; ++i) fwrite(lexer.varv[i].sectiv, sizeof(long), lexer.varv[i].sectic, f);

	fwrite(&lexer.structc, sizeof(long), 1, f);
	fwrite(lexer.structv, sizeof(struct struct_s), lexer.structc, f);
	for (int i = 0; i < lexer.structc; ++i) fwrite(lexer.structv[i].tkniv, sizeof(long), lexer.structv[i].tknic, f);
	
	fwrite(&lexer.inlinec, sizeof(long), 1, f);
	fwrite(lexer.inlinev, sizeof(struct inline_s), lexer.inlinec, f);
	for (int i = 0; i < lexer.inlinec; ++i) fwrite(lexer.inlinev[i].argiv, sizeof(long), lexer.inlinev[i].argic, f);}

static struct lexf_s* lexfinit(int fi) {
	// allocate new lexf
	lexfv = realloc(lexfv, sizeof(struct lexf_s) * ++lexfc);
	struct lexf_s* lexf = lexfv + (lexfc - 1);

	// clear lexf
	memset(lexf, 0, sizeof(struct lexf_s));

	// create str from file
	lexf->fi = fi;
	FILE* f = fv[lexf->fi].f;

	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	rewind(f);
	lexf->str = malloc(len + 1);
	fread(lexf->str, 1, len, f);
	lexf->str[len] = 0;
	rewind(f);
	lexf->str = strdup(lexf->str);

	return lexf;}

static struct lexf_s* lexfkill(struct lexf_s* lexf) {
	// declare maintained variables
	int tknpi = lexf->tknpi;

	// free lexf members
	free(lexf->str);
	
	// clear lexf
	memset(lexf, 0, sizeof(struct lexf_s));

	// reached last token, kill lexf and demote to previously promoted lexf
	if (--lexfc > 0) {
		lexfv = realloc(lexfv, sizeof(struct lexf_s) * lexfc);
		lexf = lexfv + (lexfc - 1);
		
		// maintain variables
		lexf->tknpi = tknpi;

		return lexf;}
	// no lexf to demote to? free the array of lexfs and leave loop
	else {
		free(lexfv);
		lexfv = 0;
		return 0;}}

static void lexfprog(struct lexf_s* lexf) {
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
		++lexf->stri;
		lexf->strtkni = lexf->stri;
		break;
	// single char tokens
	case '+':
		if (lexf->str[lexf->stri + 1] == ':') {
			++lexf->stri;
			lexf->tkn = ADDMOV;
			goto settkn;}
		else {
			lexf->tkn = ADD;
			goto settkn;}
	case '*':
		if (lexf->str[lexf->stri + 1] == ':') {
			++lexf->stri;
			lexf->tkn = MULMOV;
			goto settkn;}
		else {
			lexf->tkn = MUL;
			goto settkn;}
	case '/':
		if (lexf->str[lexf->stri + 1] == ':') {
			++lexf->stri;
			lexf->tkn = DIVMOV;
			goto settkn;}
		else if (lexf->str[lexf->stri + 1] == '/') {
			while (lexf->str[lexf->stri + 3] != '\n' && !(lexf->str[lexf->stri + 2] == '/' && lexf->str[lexf->stri + 1] == '/' && lexf->str[lexf->stri] != '\\')) ++lexf->stri;
			lexf->stri += 3;
			break;}
		else {
			lexf->tkn = DIV;
			goto settkn;}
	case '%':
		if (lexf->str[lexf->stri + 1] == ':') {
			++lexf->stri;
			lexf->tkn = MODMOV;
			goto settkn;}
		else {
			lexf->tkn = MOD;
			goto settkn;}
	case '&':
		if (lexf->str[lexf->stri + 1] == ':') {
			++lexf->stri;
			lexf->tkn = ANDMOV;
			goto settkn;}
		else if (lexf->str[lexf->stri + 1] == '&') {
			++lexf->stri;
			if (lexf->str[lexf->stri + 1] == ':') {
				++lexf->stri;
				lexf->tkn = ANDBOOLMOV;
				goto settkn;}
			else {
				lexf->tkn = ANDBOOL;
				goto settkn;}}
		else {
			lexf->tkn = AND;
			goto settkn;}
	case '|':
		if (lexf->str[lexf->stri + 1] == ':') {
			++lexf->stri;
			lexf->tkn = ORMOV;
			goto settkn;}
		else if (lexf->str[lexf->stri + 1] == '|') {
			++lexf->stri;
			if (lexf->str[lexf->stri + 1] == ':') {
				++lexf->stri;
				lexf->tkn = ORBOOLMOV;
				goto settkn;}
			else {
				lexf->tkn = ORBOOL;
				goto settkn;}}
		else {
			lexf->tkn = OR;
			goto settkn;}
	case '^':
		if (lexf->str[lexf->stri + 1] == ':') {
			++lexf->stri;
			lexf->tkn = XORMOV;
			goto settkn;}
		else {
			lexf->tkn = XOR;
			goto settkn;}
	case '~':
		if (lexf->str[lexf->stri + 1] == ':') {
			++lexf->stri;
			lexf->tkn = NOTMOV;
			goto settkn;}
		else if (lexf->str[lexf->stri + 1] == '~') {
			++lexf->stri;
			if (lexf->str[lexf->stri + 1] == ':') {
				++lexf->stri;
				lexf->tkn = NOTBOOLMOV;
				goto settkn;}
			else {
				lexf->tkn = NOTBOOL;
				goto settkn;}
			goto settkn;}
		else if (lexf->str[lexf->stri + 1] == '=') {
			++lexf->stri;
			if (lexf->str[lexf->stri + 1] == ':') {
				++lexf->stri;
				lexf->tkn = NOTEQUALMOV;
				goto settkn;}
			else {
				lexf->tkn = NOTEQUAL;
				goto settkn;}
			goto settkn;}
		else {
			lexf->tkn = NOT;
			goto settkn;}
	case '?':
		if (lexf->str[lexf->stri + 1] == ':') {
			++lexf->stri;
			lexf->tkn = CASTMOV;
			goto settkn;}
		else {
			lexf->tkn = CAST;
			goto settkn;}
	case '=':
		if (lexf->str[lexf->stri + 1] == ':') {
			++lexf->stri;
			lexf->tkn = EQUALMOV;
			goto settkn;}
		else {
			lexf->tkn = EQUAL;
			goto settkn;}
	case '>':
		if (lexf->str[lexf->stri + 1] == ':') {
			++lexf->stri;
			lexf->tkn = GREATERMOV;
			goto settkn;}
		else if (lexf->str[lexf->stri + 1] == '=') {
			++lexf->stri;
			if (lexf->str[lexf->stri + 1] == ':') {
				++lexf->stri;
				lexf->tkn = GREATEREQUALMOV;
				goto settkn;}
			else {
				lexf->tkn = GREATEREQUAL;
				goto settkn;}}
		else {
			lexf->tkn = GREATER;
			goto settkn;}
	case '<':
		if (lexf->str[lexf->stri + 1] == ':') {
			++lexf->stri;
			lexf->tkn = LESSERMOV;
			goto settkn;}
		else if (lexf->str[lexf->stri + 1] == '=') {
			++lexf->stri;
			if (lexf->str[lexf->stri + 1] == ':') {
				++lexf->stri;
				lexf->tkn = LESSEREQUALMOV;
				goto settkn;}
			else {
				lexf->tkn = LESSEREQUAL;
				goto settkn;}}
		else {
			lexf->tkn = LESSER;
			goto settkn;}
	case '$':
		if (lexf->str[lexf->stri + 1] == ':') {
			++lexf->stri;
			lexf->tkn = ADDRMOV;
			goto settkn;}
		else {
			lexf->tkn = ADDR;
			goto settkn;}
	case '@':
		if (lexf->str[lexf->stri + 1] == ':') {
			++lexf->stri;
			lexf->tkn = DEREFMOV;
			goto settkn;}
		else {
			lexf->tkn = DEREF;
			goto settkn;}
	case '\\':
		lexf->tkn = JUMP;
		goto settkn;
	case '!':
		lexf->tkn = CALL;
		goto settkn;
	case '.':
		lexf->tkn = DOT;
		goto settkn;
	case '[':
		lexf->tkn = ODEREF;
		goto settkn;
	case ']':
		lexf->tkn = CDEREF;
		goto settkn;
	settkn:
		++lexf->stri;
		lexf->strtkni = lexf->stri;
		break;

	case '(': // for marking the beginning of making or parsing arguments
		++lexf->stri;
		lexf->strtkni = lexf->stri;
	
		// if defining, and the definition has inline keyword
		if (!lexf->def.name) break;
		if (!(lexf->def.keywi >= 0 && strcmp(lexer.strv + lexer.keywv[lexf->def.keywi], "inline") == 0)) break;
		
		// mark flgargdef so next definitions are known to be args of the inline
		lexf->flgargdef = 1;
		
		// move def to funcdef to be defined later
		lexf->funcdef = lexf->def;
		lexf->def = (struct def_s){0, 0, -1, 0, 0, -1, -1};

		break;
	case ')': // for marking the end of making or parsing arguments
		++lexf->stri;
		lexf->strtkni = lexf->stri;

		// stop defining args
		lexf->flgargdef = 0;
		
		// if defining a func thats def has been moved to funcdef to support arg defs, move funcdef to def to define the func
		if (lexf->funcdef.name) lexf->def = lexf->funcdef;
		break;
	case '{': // for marking the beginning of setting a variables value
		++lexf->stri;
		lexf->strtkni = lexf->stri;
		
		// define
		goto vardef;
	case '}': // for marking the end of setting a variables value
		++lexf->stri;
		lexf->strtkni = lexf->stri;
		
		// pop definition hierarchy
		if (--lexf->defic) lexf->defiv = realloc(lexf->defiv, sizeof(long) * lexf->defic);
		else {
			free(lexf->defiv);
			lexf->defiv = 0;}
		break;
	case ';': // for marking the end of a statement
		++lexf->stri;
		lexf->strtkni = lexf->stri;
		
		// kill statement flags
		lexf->flgassembly = 0;
		
		// if defining, define	
		if (lexf->flgdef) goto vardef;
		break;
	vardef: // for defining a variable
		lexf->flgdef = 0;
		if (!lexf->def.name) break;

		// if latest definition in the definition hierarchy is a struct,
		if (lexf->defiv && lexer.tknv[lexf->defiv[lexf->defic - 1]] == STRUCTDEF) {
			// add the token index of the to be defined defition to the struct 
			struct struct_s* struct_ = &lexer.structv[lexer.tknv[lexf->defiv[lexf->defic - 1] + 1]];
			struct_->tkniv = realloc(struct_->tkniv, sizeof(long) * ++struct_->tknic);
			struct_->tkniv[struct_->tknic - 1] = lexer.tknc;}

		// if { or (, push definition on to the definition hierachy
		if (lexf->str[lexf->stri - 1] == '{') {
			lexf->defiv = realloc(lexf->defiv, sizeof(long) * ++lexf->defic);
			lexf->defiv[lexf->defic - 1] = lexer.tknc;}	
		
		long tknvali = 0;
		long namelen = strlen(lexf->def.name);
		long scopelen = strlen(lexf->def.scope);
		lexer.strv = realloc(lexer.strv, lexer.strc += namelen + 1 + scopelen + 1);
		long namei = lexer.strc - scopelen - 1 - namelen - 1;
		memcpy(lexer.strv + lexer.strc - namelen - 1 - scopelen - 1, lexf->def.name, namelen + 1);
		long scopei = lexer.strc - scopelen - 1;
		memcpy(lexer.strv + lexer.strc - scopelen - 1, lexf->def.scope, scopelen + 1);
		
		if (lexf->def.keywi >= 0 && strcmp(lexer.strv + lexer.keywv[lexf->def.keywi], "inline") == 0) {
			lexf->tkn = INLINEDEF;
			tknvali = lexer.inlinec;
			
			lexer.inlinev = realloc(lexer.inlinev, sizeof(struct inline_s) * ++lexer.inlinec);
			lexer.inlinev[tknvali] = (struct inline_s){namei, scopei, lexf->argiv, lexf->argic};
			lexf->argiv = 0;
			lexf->argic = 0;}
		else if (lexf->def.keywi >= 0 && strcmp(lexer.strv + lexer.keywv[lexf->def.keywi], "struct") == 0) {
			lexf->tkn = STRUCTDEF;
			tknvali = lexer.structc;
			lexer.structv = realloc(lexer.structv, sizeof(struct struct_s) * ++lexer.structc);
		       	lexer.structv[tknvali] = (struct struct_s){namei, scopei, 0, 0};}
		else {
			lexf->tkn = VARDEF;
			tknvali = lexer.varc;
			
			lexer.varv = realloc(lexer.varv, sizeof(struct var_s) * ++lexer.varc);
			lexer.varv[tknvali] = (struct var_s){namei, scopei, lexf->def.sectiv, lexf->def.sectic, lexf->def.typei, lexf->def.structi};
		
			if (lexf->flgargdef) {
				lexf->argiv = realloc(lexf->argiv, sizeof(long) * ++lexf->argic);
				lexf->argiv[lexf->argic - 1] = tknvali;}}
		
		lexf->tknpi = lexer.tknc;
		lexer.tknv = realloc(lexer.tknv, sizeof(long) * (lexer.tknc += 2));
		lexer.tknv[lexer.tknc - 1] = tknvali;
		lexer.tknv[lexer.tknc - 2] = lexf->tkn;
		
		lexf->def = (struct def_s){0, 0, -1, 0, 0, -1, -1};
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
		int strnewtkni = lexf->strtkni;
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
			int hex = lexf->str[strnewtkni] == '0' && lexf->str[strnewtkni + 1] == 'x';
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
		int tknstrlen = strnewtkni - lexf->strtkni;
		char* tknstr = malloc(tknstrlen + 1);
		memcpy(tknstr, lexf->str + lexf->strtkni, tknstrlen);
		tknstr[tknstrlen] = 0;
		
		// append data structures from initialized tkn
		long tknvali; // the index of the relevant value in the tokens relevant data structure
		switch (lexf->tkn) {
		case STR: {
			tknvali = lexer.strlitc;
			lexer.strlitv = realloc(lexer.strlitv, sizeof(struct strlit_s) * ++lexer.strlitc);
			lexer.strlitv[lexer.strlitc - 1] = (struct strlit_s){lexer.strc, lexf->defic ? lexf->defiv[lexf->defic - 1] : -1};
			
			lexer.strv = realloc(lexer.strv, lexer.strc += tknstrlen - 1);
			memcpy(lexer.strv + lexer.strc - tknstrlen - 1, tknstr + 1, tknstrlen - 2);
			lexer.strv[lexer.strc - 1] = 0;
			
			goto disctkn;}
		case NUMLIT: {
			int neg = 0;
			if (tknstr[0] == '-') {
				neg = 1;
				++tknstr;}

			// get value
			long l;
			if (tknstr[0] == '\'') {
				l = (long)tknstr[1];}
			else {
				int base = tknstr[0] != '0' ? 10 : (
					tknstr[1] == 'b' ? 2 : (
					tknstr[1] == 'x' ? 16 : 
					8));
				char* strnum = base == 10 ? tknstr : 
					(base == 8 ? tknstr + 1 : 
					tknstr + 2);
				if (!neg) l = strtol(strnum, 0, base);
				else l = -strtol(strnum, 0, base);}

			tknvali = lexer.numlitc;
			lexer.numlitv = realloc(lexer.numlitv, sizeof(struct numlit_s) * ++lexer.numlitc);
			lexer.numlitv[lexer.numlitc - 1] = (struct numlit_s){l, lexf->defic ? lexf->defiv[lexf->defic - 1] : -1, lexf->def.typei};
			if (neg) --tknstr;
			
			break;}
		case ASSEMBLY: {
			tknvali = lexer.asmc;
			lexer.asmv = realloc(lexer.asmv, sizeof(struct asm_s) * ++lexer.asmc);
			lexer.asmv[lexer.asmc - 1] = (struct asm_s){lexer.strc, lexf->defic ? lexf->defiv[lexf->defic - 1] : -1};
			for (int i = 0; i < tknstrlen + 1; ++i)
				if (tknstr[i] != '\t') {
					lexer.strv = realloc(lexer.strv, ++lexer.strc);
					lexer.strv[lexer.strc - 1] = tknstr[i];}
			if (lexer.strv[lexer.strc - 2] != '\n') {
				lexer.strv = realloc(lexer.strv, ++lexer.strc);
				lexer.strv[lexer.strc - 2] = '\n';
				lexer.strv[lexer.strc - 1] = '\0';}
			break;}
		case INCLUDE: {
			lexf->jumpfi = -1;
			for (int i = 0; i < fc; ++i)  
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
					goto disctkn;}}
				
			// make a scoped version of tknstr for var names that are changed via being scoped
			char* tknstrscoped = strdup(tknstr);
			char* scope = 0;
			for (int i = 0; i < lexf->defic; ++i) {
				char* name = 0;
				if (lexf->defic > 0 && lexer.tknv[lexf->defiv[i]] == STRUCTDEF) 
					name = lexer.strv + lexer.structv[lexer.tknv[lexf->defiv[i] + 1]].namei;
				else if (lexf->defic > 0 && lexer.tknv[lexf->defiv[i]] == VARDEF) 
					name = lexer.strv + lexer.varv[lexer.tknv[lexf->defiv[i] + 1]].namei;
				if (!name) continue;
				if (scope) {
					char* old = strdup(scope);
					scope = realloc(scope, strlen(scope) + 1 + strlen(name) + 1);
					sprintf(scope, "%s.%s", old, name);}
				else {
					scope = strdup(name);}}
			if (lexf->flgargdef) {
				char* name = lexf->funcdef.name;
				if (scope) {
					char* old = strdup(scope);
					scope = realloc(scope, strlen(scope) + 1 + strlen(name) + 1);
					sprintf(scope, "%s.%s", old, name);}
				else {
					scope = strdup(name);}}
			if (scope) {
				char* old = strdup(scope);
				tknstrscoped = realloc(tknstrscoped, strlen(scope) + 1 + tknstrlen + 1);
				sprintf(tknstrscoped, "%s.%s", old, tknstr);}
			
			// if not preparing for a definition (marked by flgdef),
			if (!lexf->flgdef) {
				// if name of a var, tokenize var
				for (int i = 0; i < lexer.varc; ++i) {
					// check tknstr == varname
					char* accessname = lexer.strv + lexer.varv[i].namei;
					if (strcmp(tknstr, accessname) != 0) continue;

					// check inscope, if so, tokenize
					char* accessscope = lexer.strv + lexer.varv[i].scopei;
					if (strlen(accessscope) >= strlen(tknstrscoped)) continue;
					
					char* accessparent = strdup(accessscope);
					char* lastdot = strrchr(accessparent, '.');
					if (lastdot) {
						*lastdot = '\0';
						if (memcmp(accessparent, tknstrscoped, strlen(accessparent)) != 0) continue;}
					goto disctkn;}
				
				// if none of the previous logic diverted flow, create a def object that prepares for a new structdef, vardef, inlinedef, etc..
				lexf->def = (struct def_s){0, 0, -1, 0, 0, -1, -1};
				lexf->flgdef = 1;}
			
			// always discards the token beyond this point for the philosophy of preparing the def... the def finally gets defined upon a ; or a : of course.
			// if preparing for a definition, append relevant data structures from tknstr
			for (int i = 0; i < lexer.structc; ++i)
				if (strcmp(lexer.strv + lexer.structv[i].scopei, tknstrscoped) == 0) {
					lexf->def.structi = i;
					goto disctkn;}
			for (int i = 0; i < lexer.sectc; ++i)
				if (strcmp(lexer.strv + lexer.sectv[i], tknstr) == 0) {
					lexf->def.sectiv = realloc(lexf->def.sectiv, sizeof(long) * ++lexf->def.sectic);
					lexf->def.sectiv[lexf->def.sectic - 1] = i;
					goto disctkn;}
			for (int i = 0; i < lexer.typec; ++i)
				if (strcmp(lexer.strv + lexer.typev[i].namei, tknstr) == 0) {
					lexf->def.typei = i;
					goto disctkn;}
			for (int i = 0; i < lexer.keywc; ++i)
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
		lexf->tknpi = lexer.tknc;
		lexer.tknv = realloc(lexer.tknv, sizeof(long) * (lexer.tknc += 2));
		lexer.tknv[lexer.tknc - 1] = tknvali;
		lexer.tknv[lexer.tknc - 2] = lexf->tkn;
		
		// doesnt append the tkn data structure, and moves onto the next relevant character
		disctkn:
		lexf->stri = lexf->tkn == STR ? strnewtkni + 1 : strnewtkni;
		lexf->strtkni = lexf->stri;
		free(tknstr);
		break;}}

	if (lexf->tkn == -1) goto prog;}
