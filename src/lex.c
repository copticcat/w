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
	lexer.typev[7] = "slong";}

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
	if (!lexf->str[lexf->stri]) return;
	
	prog:
	lexf->tkn = -1;
	lexprint(lexf);
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
		lexf->tknpi = lexer.tknc;
		lexer.tknv = realloc(lexer.tknv, ++lexer.tknc);
		lexer.tknv[lexer.tknc - 1] = lexf->tkn;
		++lexf->stri;
		lexf->strtkni = lexf->stri;
		break;
	case ':':
		lexf->flgvariable = 0;
		++lexf->stri;
		lexf->strtkni = lexf->stri;
		goto vardef;
	case ';':
		lexf->flgvariable = lexf->flgassembly = lexf->flginclude = 0;
		++lexf->stri;
		lexf->strtkni = lexf->stri;
		goto vardef;
	vardef:
		if (!lexf->variable || !lexf->variable->keyw) break;
	
		long tknvi = 0;
		if (strcmp(*lexf->variable->keyw, "inline") == 0) {
			lexf->tkn = INLINEDEF;
			tknvi = lexer.inlinec;
			lexer.inlinev = realloc(lexer.inlinev, sizeof(char*) * ++lexer.inlinec);
			lexer.inlinev[tknvi] = lexf->variable->name;}
		else if (strcmp(*lexf->variable->keyw, "struct") == 0) {
			lexf->tkn = STRUCTDEF;
			tknvi = lexer.structc;
			lexer.structv = realloc(lexer.structv, sizeof(char*) * ++lexer.structc);
		       	lexer.structv[tknvi] = (struct struct_s){lexf->variable->name, 0, 0};}
		else {
			lexf->tkn = VARDEF;
			tknvi = lexer.varc;
			lexer.varv = realloc(lexer.varv, sizeof(struct var_s) * ++lexer.varc);
			lexer.varv[tknvi] = *lexf->variable;}
		
		lexf->tknpi = lexer.tknc;
		lexer.tknv = realloc(lexer.tknv, lexer.tknc += 1 + sizeof(void*));
		memcpy(lexer.tknv + lexer.tknc - 8, &tknvi, sizeof(void*));
		lexer.tknv[lexer.tknc - 9] = lexf->tkn;
		
		lexf->variable = 0;
		
		break;
	case '\'':
		lexf->tkn = CHAR;
		goto settknstr;
	case '"': 
		lexf->tkn = STR;
		goto settknstr;
	default:
	settknstr: {
		// if previous token is asm directive (doesnt get tokenized), tokenize rest until ; as [ASSEMBLY:literal string]
		lexf->tkn = lexf->flgassembly ? ASSEMBLY : (
		// if previous token is asm directive, tokenize rest until ; as [DIRECTORY:literal string]
			lexf->flginclude ? INCLUDE : lexf->tkn);
		// kill flags
		lexf->flgassembly = lexf->flginclude = 0;

		// if current char is numeric, tokenize [LONG:literal long]
		lexf->tkn = lexf->tkn == -1 && lexf->str[lexf->strtkni] >= '0' && lexf->str[lexf->strtkni] <= '9' ? LONG : lexf->tkn;
		// if squote, tokenize [CHAR:literal char]
		// if dquote, tokenize [STR:literal string]
		// if hashtag, tokenize [DIRECTIVE:literal string]
		
		// skip to relevant part
		if (lexf->tkn == CHAR || lexf->tkn == STR) ++lexf->strtkni;
		
		int ntkni = lexf->strtkni;
		while (((lexf->tkn == ASSEMBLY || lexf->tkn == INCLUDE) && lexf->str[ntkni] != ';') || 
			(lexf->tkn == CHAR && ntkni == lexf->strtkni) || 
			(lexf->tkn == STR && (lexf->str[ntkni] != '"' || (ntkni > 0 && lexf->str[ntkni - 1] == '\\'))) || 
			((lexf->tkn == LONG || lexf->tkn == -1) 
				&& ((lexf->str[ntkni] == '#' && lexf->strtkni == ntkni) 
				|| (lexf->str[ntkni] == '_' || (lexf->str[ntkni] >= 'a' && lexf->str[ntkni] <= 'z') || (lexf->str[ntkni] >= 'A' && lexf->str[ntkni] <= 'Z') || (lexf->str[ntkni] >= '0' && lexf->str[ntkni] <= '9'))))) 
			++ntkni;
		
		// invalid?
		if (lexf->strtkni == ntkni) {
			++lexf->stri;
			lexf->strtkni = lexf->stri;
			break;}
		
		int tknstrlen = ntkni - lexf->strtkni;
		char* tknstr = malloc(tknstrlen + 1);
		memcpy(tknstr, lexf->str + lexf->strtkni, tknstrlen);
		tknstr[tknstrlen] = 0;
		
		// append data structures
		long tknvi;
		if (lexf->tkn == STR) {
			lexer.strv = realloc(lexer.strv, sizeof(char*) * ++lexer.strc);
			lexer.strv[lexer.strc - 1] = strdup(tknstr);
			tknvi = lexer.strc - 1;}
		else if (lexf->tkn == LONG) {
			int base = lexf->str[lexf->stri] != '0' ? 10 : (
				lexf->str[lexf->stri + 1] == 'b' ? 2 : (
				lexf->str[lexf->stri + 1] == 'x' ? 16 : 
				8));
			char* strnum = lexf->str[lexf->stri] != '0' ? lexf->str + lexf->stri : lexf->str + lexf->stri + 2;
			lexer.longv = realloc(lexer.longv, sizeof(long) * ++lexer.longc);
			lexer.longv[lexer.longc - 1] = strtol(strnum, NULL, base);
			tknvi = lexer.longc - 1;}
		else if (lexf->tkn == ASSEMBLY) {
			lexer.asmv = realloc(lexer.asmv, sizeof(char*) * ++lexer.asmc);
			lexer.asmv[lexer.asmc - 1] = strdup(tknstr);
			tknvi = lexer.asmc - 1;}
		else if (lexf->tkn == INCLUDE) {
			lexer.fv = realloc(lexer.fv, sizeof(char*) * ++lexer.fc);
			lexer.fv[lexer.fc - 1] = strdup(tknstr);
			tknvi = lexer.fc - 1;}
		else if (lexf->tkn == -1) {
			if (strcmp(tknstr, "#asm") == 0) {
				lexf->flgassembly = 1;
				goto disctkn;}
			else if (strcmp(tknstr, "#include") == 0) {
				lexf->flginclude = 1;
				goto disctkn;}
			
			for (int i = 0; i < lexer.inlinec; ++i)
				if (strcmp(lexer.inlinev[i], tknstr) == 0) {
					tknvi = i;
					lexf->tkn = INLINE;
					goto keeptkn;}
			for (int i = 0; i < lexer.dirc; ++i)
				if (strcmp(lexer.dirv[i], tknstr) == 0) {
					tknvi = i;
					lexf->tkn = DIRECTIVE;
					goto keeptkn;}
			for (int i = 0; i < lexer.varc; ++i)
				if (lexer.varv[i].name && strcmp(lexer.varv[i].name, tknstr) == 0) {
					tknvi = i;
					lexf->tkn = VAR;
					goto keeptkn;}
			
			if (!lexf->flgvariable) {
				lexf->variable = malloc(sizeof(struct var_s));
				*lexf->variable = (struct var_s){strdup(""), 0, 0, 0, 0};
				lexf->flgvariable = 1;}

			for (int i = 0; i < lexer.structc; ++i)
				if (strcmp(lexer.structv[i].name, tknstr) == 0) {
					lexf->variable->struct_ = &lexer.structv[i];
					goto disctkn;}
			for (int i = 0; i < lexer.sectc; ++i)
				if (strcmp(lexer.sectv[i], tknstr) == 0) {
					lexf->variable->sect = &lexer.sectv[i];
					goto disctkn;}
			for (int i = 0; i < lexer.typec; ++i)
				if (strcmp(lexer.typev[i], tknstr) == 0) {
					lexf->variable->type = &lexer.typev[i];
					goto disctkn;}
			for (int i = 0; i < lexer.keywc; ++i)
				if (strcmp(lexer.keywv[i], tknstr) == 0) {
					lexf->variable->keyw = &lexer.keywv[i];
					goto disctkn;}
		
			free(lexf->variable->name);
			lexf->variable->name = strdup(tknstr);
			goto disctkn;}
			
		keeptkn:
		lexf->tknpi = lexer.tknc;
		lexer.tknv = realloc(lexer.tknv, lexer.tknc += 1 + sizeof(void*));
		memcpy(lexer.tknv + lexer.tknc - 8, &tknvi, sizeof(void*));
		lexer.tknv[lexer.tknc - 9] = lexf->tkn;
		disctkn:
		lexf->stri = lexf->tkn == STR ? ntkni + 1 : ntkni;
		lexf->strtkni = lexf->stri;
		free(tknstr);
		break;}}

	if (lexf->tkn == -1) goto prog;}
