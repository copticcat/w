#include "../../inc/lex/lex.h"
#include "../../inc/lex/enum.h"
#include <stdlib.h>
#include <string.h>

void newtkn(long tkn, long vali) {
	lexer.tknv = realloc(lexer.tknv, sizeof(long) * (lexer.tknc += 2));
	lexer.tknv[lexer.tknc - 2] = tkn;
	lexer.tknv[lexer.tknc - 1] = vali;}

void newstr(char* str) {
	long len = strlen(str) + 1;
	lexer.strv = realloc(lexer.strv, lexer.strc += len);
	memcpy(lexer.strv + lexer.strc - len, str, len);}

void newnstr(char* str, long len) {
	char* nstr = strndup(str, len);
	long nlen = strlen(nstr) + 1;
	lexer.strv = realloc(lexer.strv, lexer.strc += nlen);
	memcpy(lexer.strv + lexer.strc - nlen, nstr, nlen);
	free(nstr);}

void newtype(char* str, int width) {
	lexer.typev = realloc(lexer.typev, sizeof(struct type_s) * ++lexer.typec);
	lexer.typev[lexer.typec - 1] = (struct type_s){lexer.strc, width};
	newstr(str);}

void newsect(char* str) {
	lexer.sectv = realloc(lexer.sectv, sizeof(long) * ++lexer.sectc);
	lexer.sectv[lexer.sectc - 1] = lexer.strc;
	newstr(str);}

void newmath(long val, long valtype, long regi, long op) {
	newtkn(MATH, lexer.mathc);
	lexer.mathv = realloc(lexer.mathv, sizeof(struct math_s) * ++lexer.mathc);
	lexer.mathv[lexer.mathc - 1] = (struct math_s){val, valtype, regi, op, lexf->defic ? lexf->defiv[lexf->defic - 1].scopei : -1};}
