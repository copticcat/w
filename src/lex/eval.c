#include "../../inc/lex/eval.h"
#include "../../inc/lex/lex.h"
#include "../../inc/lex/new.h"
#include "../../inc/lex/enum.h"
#include "../../inc/print.h"
#include "../../inc/asm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// local vars
struct lexe_s lexe = {0};

// local funcs
static void pushbranch();
static struct math_s* pushstack();
static void popbranch();
static void opprint(char*);

// global func defs
void plugl(long n) {
	if (!lexe.hanging) {
		// push op growth
		*(pushstack()) = (struct math_s){n, LIT, -1, -1, -1};
		return;}
	// set hanged op n
	lexe.mathv[lexe.mathc - 1] = (struct math_s){n, LIT, -1, lexe.mathv[lexe.mathc - 1].op, -1};
	lexe.hanging = 0;}

void plugvar(long vari) {
	if (!lexe.hanging) {
		// push op growth
		*(pushstack()) = (struct math_s){vari, VARI, -1, -1, -1};
		return;}
	// set hanged op n
	lexe.mathv[lexe.mathc - 1] = (struct math_s){vari, VARI, -1, lexe.mathv[lexe.mathc - 1].op, -1};
	lexe.hanging = 0;}

void plugop(long op) {
	switch (op) {
	case OB: pushbranch(); return;
	case CB: popbranch(); return;}

	// if branch has growth
	if ((lexe.mathbranchc > 0 ? lexe.mathbranchv[lexe.mathbranchc - 1] : 0) < lexe.mathstackc) {
		// if this op supercedes the previous op, invalidate last value in lexe.mathv, and carry it to push op
		if ((lexe.mathstackv[lexe.mathstackc - 1]->op == ADD || lexe.mathstackv[lexe.mathstackc - 1]->op == SUB) && (op == MUL || op == DIV)) {
			lexe.mathv[lexe.mathc - 1].type = -1;
			pushbranch();
			*(pushstack()) = (struct math_s){lexe.mathv[lexe.mathc - 3].val, LIT, -1, -1, -1};}
		// else if this op cedes the previous op, place closebrack & carry
		else if (lexe.mathbranchc > 0 && (lexe.mathstackv[lexe.mathstackc - 1]->op == MUL || lexe.mathstackv[lexe.mathstackc - 1]->op == DIV) && (op == ADD || op == SUB)) popbranch();}

	// push hanged stack
	pushstack()->op = op;	
	lexe.hanging = 1;}

static long* reservedregv = 0;

void increg(long reservedregi, long* regi) {
	++*regi;
	if (*regi > 15) *regi = 0;
	for (;;) {
		long taken = 0;
		for (int i = 0; i < reservedregi; ++i)	
			if ((taken = reservedregv[i] == *regi)) break;
		if (!taken) break;
		else ++*regi;}}

struct eval_s eval(long reservedregi) {
	struct eval_s reteval = {0, -1};
	if (!lexe.mathv) return reteval;

	// if a void type is present, throw error
	for (int i = 0; i < lexe.mathc; ++i) if (lexe.mathv[i].type == VARI && lexer.varv[lexe.mathv[i].val].typei == -1) {
		errprint("attempt to evaluate an untyped variable");
		goto end;}

	// close remaining brackets
	for (long i = 0; i < lexe.mathbranchc; ++i) {
		lexe.mathv = realloc(lexe.mathv, sizeof(struct math_s) * ++lexe.mathc);
		lexe.mathv[lexe.mathc - 1] = (struct math_s){0, -1, -1, CB, -1};}
	
	// reserve registers
	long regi = -1;
	increg(reservedregi, &regi);
	long ansi = 0;
	
	// if only one math item... just skip everything
	if (lexe.mathc == 1) goto eval;
	
	long h = 0;
	char* iterstr;
	
	iter:
	asprintf(&iterstr, "%ld", h++);
	opprint(iterstr);
	
	// exit loop if finished
	for (long i = 0; i < lexe.mathc; ++i) {
		if (lexe.mathv[i].op != -1) break;
		if (lexe.mathv[i].type != -1) ansi = i;
		if (i == lexe.mathc - 1) goto eval;}

	// apply operations where possible
	for (long i = 0, lasti = -1; i < lexe.mathc; ++i) {
		if (lexe.mathv[i].op == OB || lexe.mathv[i].op == CB) lasti = -1;
		else if (lexe.mathv[i].type == -1) continue;
		else if (lexe.mathv[i].op == -1 && (lexe.mathv[i].type == LIT || lexe.mathv[i].type == VARI || lexe.mathv[i].type == REGI)) lasti = i;
		else if (lasti != -1) {
			if (lexe.mathv[i].type == LIT && lexe.mathv[lasti].type == LIT) 
				switch (lexe.mathv[i].op) {
				case ADD: lexe.mathv[i].val += lexe.mathv[lasti].val; break;
				case SUB: lexe.mathv[i].val = lexe.mathv[lasti].val - lexe.mathv[i].val; break;
				case MUL: lexe.mathv[i].val *= lexe.mathv[lasti].val; break;
				case DIV: lexe.mathv[i].val = lexe.mathv[lasti].val / lexe.mathv[i].val;}
			else if ((lexe.mathv[i].type == VARI && lexe.mathv[lasti].type == VARI) || (lexe.mathv[i].type == LIT && lexe.mathv[lasti].type == VARI) || (lexe.mathv[i].type == VARI && lexe.mathv[lasti].type == LIT)) { // new reg on vari type
				newmath(lexe.mathv[lasti].val, lexe.mathv[lasti].type, regi, SET);
				newmath(lexe.mathv[i].val, lexe.mathv[i].type, regi, lexe.mathv[i].op);
				lexe.mathv[i].val = regi;	
				increg(reservedregi,  &regi);
				lexe.mathv[i].type = REGI;}
			else if (lexe.mathv[lasti].type == REGI && lexe.mathv[i].type == VARI) {
				newmath(lexe.mathv[i].val, VARI, lexe.mathv[lasti].val, lexe.mathv[i].op);
				lexe.mathv[i].val = lexe.mathv[lasti].val;
				lexe.mathv[i].type = REGI;}
			else if (lexe.mathv[lasti].type == VARI && lexe.mathv[i].type == REGI) {
				switch (lexe.mathv[i].op) {
				case ADD: case MUL: newmath(lexe.mathv[lasti].val, VARI, lexe.mathv[i].val, lexe.mathv[i].op); break;
				case SUB: case DIV:
					newmath(lexe.mathv[lasti].val, VARI, regi, SET);
					newmath(lexe.mathv[i].val, REGI, regi, lexe.mathv[i].op);
					lexe.mathv[i].val = regi;
					increg(reservedregi,  &regi);
					lexe.mathv[i].type = REGI;
					break;}}
			else if (lexe.mathv[lasti].type == REGI && lexe.mathv[i].type == REGI) {
				newmath(lexe.mathv[i].val, REGI, lexe.mathv[lasti].val, lexe.mathv[i].op);
				lexe.mathv[i].val = lexe.mathv[lasti].val;}

			lexe.mathv[i].op = -1;
			lexe.mathv[lasti] = (struct math_s){0, -1, -1, -1, -1};
			lasti = i;}}
	
	// deconstruct brackets with no containing operators
	for (long i = 0; i < lexe.mathc;) {
		long obi = -1;
		for (; i < lexe.mathc; ++i) {
			if (lexe.mathv[i].op == OB) {
				obi = i++;
				break;}}
		if (obi == -1) continue;
		long cbi = -1;
		for (; i < lexe.mathc && (lexe.mathv[i].op == -1 || lexe.mathv[i].op == CB); ++i) 
			if (lexe.mathv[i].op == CB) {
				cbi = i++;
				break;}
		if (cbi == -1) continue;
		lexe.mathv[obi] = (struct math_s){0, -1, -1, -1, -1};
		lexe.mathv[cbi] = (struct math_s){0, -1, -1, -1, -1};}

	// pull numbers back to valid operator
	for (long i = 0; i < lexe.mathc; ++i) {
		if (lexe.mathv[i].type == -1 && (lexe.mathv[i].op == ADD || lexe.mathv[i].op == SUB || lexe.mathv[i].op == MUL || lexe.mathv[i].op == DIV)) {
			long j;
			for (j = i + 1; j < lexe.mathc; ++j) {
				if (lexe.mathv[j].op == -1) {
					if (lexe.mathv[j].type == -1) continue;
					lexe.mathv[i].val = lexe.mathv[j].val;
					lexe.mathv[i].type = lexe.mathv[j].type;
					lexe.mathv[j] = (struct math_s){0, -1, -1, -1, -1};
					break;}
				else break;}
			i = j - 1;}}

	goto iter;
	eval:
	reteval.val = lexe.mathv[ansi].val;
	reteval.type = lexe.mathv[ansi].type;
	switch (lexe.mathv[ansi].type) {
	case REGI: {
		reservedregv = realloc(reservedregv, sizeof(long) * (reservedregi + 1));
		reservedregv[reservedregi] = lexe.mathv[ansi].val;
		break;}
	case LIT: {
		int typei = 0;
		for (; strcmp(lexer.strv + lexer.typev[typei].namei, "long") != 0; ++typei);
		if (!lexf->flgmov) {
			newtkn(NUMLIT, lexer.numlitc);
			lexer.numlitv = realloc(lexer.numlitv, sizeof(struct numlit_s) * ++lexer.numlitc);
			long zeroed = 0;
			for (int i = 0; i < lexf->defiv[lexf->defic - 1].sectic; ++i) if ((zeroed = strcmp("zero", lexer.strv + lexf->defiv[lexf->defic - 1].sectiv[i]) == 0)) break;
			lexer.numlitv[lexer.numlitc - 1] = (struct numlit_s){lexe.mathv[ansi].val, lexf->defic ? lexf->defiv[lexf->defic - 1].scopei : -1, typei, zeroed};
			break;}
		break;}
	case VARI: if (!lexf->flgmov) newtkn(VARLIT, lexe.mathv[ansi].val); break;}
	end:
	
	// free & reset vars
	free(lexe.mathv);
	free(lexe.mathstackv);
	free(lexe.mathbranchv);
	memset(&lexe, 0, sizeof(struct lexe_s));
	
	return reteval;}

// local func defs
static void pushbranch() {
	// push op
	lexe.mathv = realloc(lexe.mathv, sizeof(struct math_s) * ++lexe.mathc);
	lexe.mathv[lexe.mathc - 1] = (struct math_s){0, -1, -1, OB, -1};
	
	// push op branch
	lexe.mathbranchv = realloc(lexe.mathbranchv, sizeof(int) * ++lexe.mathbranchc);
	lexe.mathbranchv[lexe.mathbranchc - 1] = lexe.mathstackc;	

	// unhang op
	lexe.hanging = 0;}

static struct math_s* pushstack() {
	// push op
	lexe.mathv = realloc(lexe.mathv, sizeof(struct math_s) * ++lexe.mathc);
	lexe.mathv[lexe.mathc - 1] = (struct math_s){0, -1, -1, -1, -1};	
	
	// push op twig
	lexe.mathstackv = realloc(lexe.mathstackv, sizeof(struct math_s*) * ++lexe.mathstackc);
	return lexe.mathstackv[lexe.mathstackc - 1] = lexe.mathv + lexe.mathc - 1;}

static void popbranch() {
	// pop stack
	if (lexe.mathbranchv && (lexe.mathstackc -= (lexe.mathstackc - lexe.mathbranchv[lexe.mathbranchc - 1]))) lexe.mathstackv = realloc(lexe.mathstackv, sizeof(struct math_s*) * lexe.mathstackc);
	else if (lexe.mathstackv) {
		free(lexe.mathstackv);
		lexe.mathstackv = 0;}

	// pop branch
	if (--lexe.mathbranchc) {
		lexe.mathbranchv = realloc(lexe.mathbranchv, sizeof(int) * --lexe.mathbranchc);}
	else if (lexe.mathbranchv) {
		free(lexe.mathbranchv);
		lexe.mathbranchv = 0;}
	
	// push math
	lexe.mathv = realloc(lexe.mathv, sizeof(struct math_s) * ++lexe.mathc);
	lexe.mathv[lexe.mathc - 1] = (struct math_s){0, -1, -1, CB, -1};

	// unhang op
	lexe.hanging = 0;}

static void opprint(char* msg) {
	printf("\033[32m%s\033[0m\t", msg);
	for (long i = 0; i < lexe.mathc; ++i) {
		printf("\033[31m%ld\033[0m ", i);
		if (lexe.mathv[i].op >= 0) {
			char c;
			switch (lexe.mathv[i].op) {
			case ADD: c = '+'; break;
			case SUB: c = '-'; break;
			case MUL: c = '*'; break;
			case DIV: c = '/'; break;
			case OB: c = '('; break;
			case CB: c = ')'; break;}
			printf("\033[1;32m%c\033[0m", c);}
		if (lexe.mathv[i].type == LIT) printf("\033[1m%ld\033[0m", lexe.mathv[i].val);
		else if (lexe.mathv[i].type == VARI) printf("\033[1m%s\033[0m", lexer.strv + lexer.varv[lexe.mathv[i].val].namei);
		else if (lexe.mathv[i].type == REGI) printf("\033[1m%s\033[0m", regitstr(lexe.mathv[i].val));
		printf("\t");}
	printf("\n");}

