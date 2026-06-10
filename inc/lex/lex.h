#ifndef DING_LEX
#define DING_LEX
#include "eval.h"
#include <stdio.h>

struct var_s {
	long namei;
	long scopei;
	long* sectiv; long sectic;// indexes of sectv
	long typei;		// index of typev
	long structi;}; 		// index of structv

struct asm_s {
	long stri;	
	long scopei;};	

struct numlit_s {
	long num;
	long scopei; 
	long typei;
	long zeroed;};

struct strlit_s {
	long stri;
	long scopei;};	

struct struct_s {
	long namei;
	long scopei;
	long* tkniv; long tknic;}; // indexes of tknv

struct inline_s {
	long namei;
	long scopei;
	long* argiv; long argic;}; // indexes of varv

struct type_s {
	long namei;
	long width;};

struct mov_s {
	long dest;
	long desttype;
	long val;
	long valtype;};

struct def_s {
	char* name;
	long namei;
	char* scope;
	long scopei;
	long keywi;		// index of keywv
	long* sectiv; long sectic;// indexes of sectv
	long typei;		// index of typev
	long structi;}; 	// index of structv

struct lexf_s {
	long fi;

	char* str;
	long stri;
	long strtkni;
	
	long tkn;	

	struct def_s def;
	struct def_s* defiv; long defic;
	long flgdef;
	long flgassembly;
	long flgmov;
	long flgjump; long jumpfi;
	
	char** aliasv; long aliasc; long* aliasiv; long aliasic;}; // aliasiv is a list of indexes of aliasv that pops and pushes on new scopes

struct lexer_s {
	char* strv; long strc;
	
	long* tknv; long tknc;
	
	long* keywv; long keywc;
	long* sectv; long sectc;
	struct type_s* typev; long typec;
	struct asm_s* asmv; long asmc;	
	struct strlit_s* strlitv; long strlitc;
	struct numlit_s* numlitv; long numlitc;
	struct var_s* varv; long varc;
	struct struct_s* structv; long structc;
	struct inline_s* inlinev; long inlinec;
	struct math_s* mathv; long mathc;
	struct mov_s* movv; long movc;};

extern struct lexer_s lexer;
extern struct lexf_s* lexf;

extern void lex();
extern void lexfree();
extern void lexread();

#endif
