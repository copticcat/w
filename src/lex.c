#include "../inc/lex.h"
#include "../inc/glob.h"
#include "../inc/print.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// local func defs
static char* ftostr(FILE*);
static void lexinit();
static void lexkill();
static struct lexf_s* lexfinit(int);
static struct lexf_s* lexfkill(struct lexf_s*);
static void lexfprog(struct lexf_s*);

// global vars
struct lexer_s lexer = {0};

// local vars
static struct lexf_s* lexfv = 0; static int lexfc = 0;	

// global funcs
void lex() {
	// create lex struct
	lexinit();

	// create file lex struct
	struct lexf_s* lexf = lexfinit(0);	
	printf("\r[lex] entry:   \"%s\"\n", infnamev[lexf->fi]);
	fflush(stdout);

	// execute until no lexfs left	
	while (lexf) {
		while (lexf->str[lexf->stri]) {
			// progress to next token
			lexfprog(lexf);

			// if include token, promote new file lex struct
			if (lexf->tkn == INCLUDE) {
				int fi = -1;
				for (int i = 0; i < infc; ++i)  
					if (strcmp(infnamev[i], lexer.fv[*(long*)(lexer.tknv + lexf->tknpi + 1)]) == 0) {
						fi = i;
						break;}
				
				lexf = lexfinit(fi);
				printf("\r[lex] promote: \"%s\"\n", infnamev[lexf->fi]);
				fflush(stdout);
				continue;}}
		
		// kill used up lexf
		lexf = lexfkill(lexf);
		if (lexf != 0) {
			printf("\r[lex] demote:  \"%s\"\n", infnamev[lexf->fi]);
			fflush(stdout);}}
	
	printf("\r[lex] exit\n");
	fflush(stdout);

	// free lex struct
	lexkill();}

// local funcs
static char* ftostr(FILE* f) {
	char* str = 0;
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	rewind(f);
	str = malloc(len + 1);
	fread(str, 1, len, f);
	str[len] = 0;
	rewind(f);
	str = strdup(str);
	return str;}

static void lexinit() {
	memset(&lexer, 0, sizeof(struct lexer_s));
	
	// create lex struct
	lexer.dirv = malloc(sizeof(char*) * 3); lexer.dirc = 3;
	lexer.keywv = malloc(sizeof(char*) * 2); lexer.keywc = 2;
	lexer.sectv = malloc(sizeof(char*) * 5); lexer.sectc = 5;
	lexer.typev = malloc(sizeof(char*) * 8); lexer.typec = 8;
	lexer.varv = malloc(sizeof(struct var_s) * 2); lexer.varc = 2;

	// create directives
	lexer.dirv[0] = "#if";
	lexer.dirv[1] = "#asm";
	lexer.dirv[2] = "#include";

	// create keywords
	lexer.keywv[0] = "inline";
	lexer.keywv[1] = "struct";

	// create sects
	lexer.sectv[0] = "text";
	lexer.sectv[1] = "data";
	lexer.sectv[2] = "rodata";
	lexer.sectv[3] = "bss";
	lexer.sectv[4] = "stack";

	// create types
	lexer.typev[0] = "char";
	lexer.typev[1] = "short";
	lexer.typev[2] = "int";
	lexer.typev[3] = "long";
	lexer.typev[4] = "schar";
	lexer.typev[5] = "sshort";
	lexer.typev[6] = "sint";
	lexer.typev[7] = "slong";

	// create vars (dynamic, temporarily static)
	lexer.varv[0] = (struct var_s){"varg", 0, 0, 0};
	lexer.varv[1] = (struct var_s){"vargw", 0, 0, 0};}

static void lexkill() {
	free(lexer.dirv);
	free(lexer.keywv);
	free(lexer.sectv);
	free(lexer.typev);

	memset(&lexer, 0, sizeof(struct lexer_s));}

static struct lexf_s* lexfinit(int fi) {
	// allocate new lexf
	lexfv = realloc(lexfv, sizeof(struct lexf_s) * ++lexfc);
	struct lexf_s* lexf = lexfv + (lexfc - 1);

	// clear lexf
	memset(lexf, 0, sizeof(struct lexf_s));

	// create str
	lexf->fi = fi;
	lexf->str = ftostr(infv[fi]);

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
	prog:
	lexf->tkn = -1;
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
	case '-':
		if (lexf->str[lexf->stri + 1] == ':') {
			++lexf->stri;
			lexf->tkn = SUBMOV;
			goto settkn;}
		else {
			lexf->tkn = SUB;
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
				goto settkn;}
			goto settkn;}
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
				goto settkn;}
			goto settkn;}
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
	case '{':
		lexf->tkn = OLIST;
		goto settkn;
	case '}':
		lexf->tkn = CLIST;
		goto settkn;
	case '(':
		lexf->tkn = OBRACK;
		goto settkn;
	case ')':
		lexf->tkn = CBRACK;
		goto settkn;
	settkn:
		//lexf->tknpi = lexer.tknc;
		//lexer.tknv = realloc(lexer.tknv, ++lexer.tknc);
		//lexer.tknv[lexer.tknc - 1] = lexf->tkn;
		++lexf->stri;
		lexf->strtkni = lexf->stri;
		break;
	case ':':
		lexf->flgdef = 0;
		++lexf->stri;
		lexf->strtkni = lexf->stri;
		goto vardef;
	case ';':
		lexf->flgdef = lexf->flgassembly = lexf->flginclude = 0;
		++lexf->stri;
		lexf->strtkni = lexf->stri;
		goto vardef;
	vardef:
		if (!lexf->def.name) break;
		
		long tknvali = 0;
		if (lexf->def.keyw && strcmp(*lexf->def.keyw, "inline") == 0) {
			lexf->tkn = INLINEDEF;
			tknvali = lexer.inlinec;
			lexer.inlinev = realloc(lexer.inlinev, sizeof(char*) * ++lexer.inlinec);
			lexer.inlinev[tknvali] = lexf->def.name;}
		else if (lexf->def.keyw && strcmp(*lexf->def.keyw, "struct") == 0) {
			lexf->tkn = STRUCTDEF;
			tknvali = lexer.structc;
			lexer.structv = realloc(lexer.structv, sizeof(struct struct_s) * ++lexer.structc);
		       	lexer.structv[tknvali] = (struct struct_s){lexf->def.name, 0, 0};}
		else {
			lexf->tkn = VARDEF;
			tknvali = lexer.varc;
			lexer.varv = realloc(lexer.varv, sizeof(struct var_s) * ++lexer.varc);
			lexer.varv[tknvali] = (struct var_s){strdup(lexf->def.name), lexf->def.sect, lexf->def.type, lexf->def.struct_};}
		
		lexf->tknpi = lexer.tknc;
		lexer.tknv = realloc(lexer.tknv, lexer.tknc += 1 + sizeof(long));
		memcpy(lexer.tknv + lexer.tknc - sizeof(long), &tknvali, sizeof(long));
		lexer.tknv[lexer.tknc - 9] = lexf->tkn;
		
		memset(&lexf->def, 0, sizeof(struct def_s));
		
		break;
	default: {
		// set tkn to relevant enum value
		lexf->tkn = 
		// from flags
			lexf->flgassembly ? ASSEMBLY : (
			lexf->flginclude ? INCLUDE : (
		// from character
			lexf->str[lexf->strtkni] == '\'' || (lexf->str[lexf->strtkni] >= '0' && lexf->str[lexf->strtkni] <= '9') ? LONG : (
			lexf->str[lexf->strtkni] == '\"' ? STR : -1)));
		
		// kill flags
		lexf->flgassembly = lexf->flginclude = 0;

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
		// if long or uninitialized tkn, select while
		case LONG:
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
		if (lexf->tkn == STR) {
			char* str = malloc(tknstrlen - 1);
			memcpy(str, tknstr + 1, tknstrlen - 2);
			str[tknstrlen] = 0;
			
			tknvali = lexer.strc;
			lexer.strv = realloc(lexer.strv, sizeof(char*) * ++lexer.strc);
			lexer.strv[lexer.strc - 1] = str;}
		else if (lexf->tkn == LONG) {
			long l;
			if (tknstr[0] == '\'') {
				l = (long)tknstr[1];}
			else {
				int base = tknstr[0] != '0' ? 10 : (
					tknstr[1] == 'b' ? 2 : (
					tknstr[1] == 'x' ? 16 : 
					8));
				char* strnum = tknstr[0] != '0' ? tknstr : tknstr + 2;
				l = strtol(strnum, 0, base);}
			
			tknvali = lexer.longc;
			lexer.longv = realloc(lexer.longv, sizeof(long) * ++lexer.longc);
			lexer.longv[lexer.longc - 1] = l;}
		else if (lexf->tkn == ASSEMBLY) {
			tknvali = lexer.asmc;
			lexer.asmv = realloc(lexer.asmv, sizeof(char*) * ++lexer.asmc);
			lexer.asmv[lexer.asmc - 1] = strdup(tknstr);}
		else if (lexf->tkn == INCLUDE) {
			tknvali = lexer.fc;
			lexer.fv = realloc(lexer.fv, sizeof(char*) * ++lexer.fc);
			lexer.fv[lexer.fc - 1] = strdup(tknstr);}
		
		// append data structures from uninitialized tkn
		else if (lexf->tkn == -1) {
			// if not preparing for a definition (marked by flgdef),
			if (!lexf->flgdef) {
				// dont tokenize, and set relevant flags if strtkn is a certain string (good for skipping unnecessary logic)
				if (strcmp(tknstr, "#asm") == 0) {
					lexf->flgassembly = 1;
					goto disctkn;}
				else if (strcmp(tknstr, "#include") == 0) {
					lexf->flginclude = 1;
					goto disctkn;}
				
				// tokenize, and set tkn to relevant enum value from tknstr
				for (int i = 0; i < lexer.dirc; ++i)
					if (strcmp(lexer.dirv[i], tknstr) == 0) {
						lexf->tkn = DIRECTIVE;
						tknvali = i;
						goto keeptkn;}
				for (int i = 0; i < lexer.varc; ++i)
					if (lexer.varv[i].name && strcmp(lexer.varv[i].name, tknstr) == 0) {
						tknvali = i;
						lexf->tkn = VAR;
						goto keeptkn;}
				for (int i = 0; i < lexer.inlinec; ++i)
					if (strcmp(lexer.inlinev[i], tknstr) == 0) {
						tknvali = i;
						lexf->tkn = INLINE;
						goto keeptkn;}
				
				// if none of the previous logic was able to set tkn from tknstr, create a def object that prepares for a new structdef, vardef, inlinedef, etc..
				lexf->def = (struct def_s){0, 0, 0, 0, 0};
				lexf->flgdef = 1;}
			
			// always discards the token beyond this point for the philosophy of preparing the def... the def finally gets defined upon a ; or a : of course.
			// if preparing for a definition, append relevant data structures from tknstr
			for (int i = 0; i < lexer.structc; ++i)
				if (strcmp(lexer.structv[i].name, tknstr) == 0) {
					lexf->def.struct_ = &lexer.structv[i];
					goto disctkn;}
			for (int i = 0; i < lexer.sectc; ++i)
				if (strcmp(lexer.sectv[i], tknstr) == 0) {
					lexf->def.sect = &lexer.sectv[i];
					goto disctkn;}
			for (int i = 0; i < lexer.typec; ++i)
				if (strcmp(lexer.typev[i], tknstr) == 0) {
					lexf->def.type = &lexer.typev[i];
					goto disctkn;}
			for (int i = 0; i < lexer.keywc; ++i)
				if (strcmp(lexer.keywv[i], tknstr) == 0) {
					lexf->def.keyw = &lexer.keywv[i];
					goto disctkn;}

			// if none of the previous logic was able to append a data structre from tknstr, set the defs name
			free(lexf->def.name);
			lexf->def.name = strdup(tknstr);
			goto disctkn;}
		
		// appends the tkn data structure	
		keeptkn:
		lexf->tknpi = lexer.tknc;
		lexer.tknv = realloc(lexer.tknv, lexer.tknc += 1 + sizeof(long));
		memcpy(lexer.tknv + lexer.tknc - sizeof(long), &tknvali, sizeof(long));
		lexer.tknv[lexer.tknc - sizeof(long) - 1] = lexf->tkn;
		
		// doesnt append the tkn data structure, and moves onto the next relevant character
		disctkn:
		lexf->stri = lexf->tkn == STR ? strnewtkni + 1 : strnewtkni;
		lexf->strtkni = lexf->stri;
		free(tknstr);
		break;}}

	if (lexf->tkn == -1) goto prog;}
