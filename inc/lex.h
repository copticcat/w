#ifndef DING_LEX
#define DING_LEX
#include <stdio.h>

struct var_s {
	long namei;
	long scopei;
	long* sectiv; long sectic;// indexes of sectv
	long typei;		// index of typev
	long structi;}; 	// index of structv

struct asm_s {
	long stri;	// index of strv
	long scopei;};	// index of tknv

struct numlit_s {
	long num;
	long scopei; // index of tknv
	long typei;};

struct strlit_s {
	long stri;	// index of strv
	long scopei;};	// index of tknv

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

struct def_s {
	char* name;
	char* scope;
	long keywi;		// index of keywv
	long* sectiv; long sectic;// indexes of sectv
	long typei;		// index of typev
	long structi;}; 	// index of structv

struct lexf_s {
	long fi;

	char* str;
	long stri;
	long strtkni;
	long strvlinei;
	long strvlinec;
	
	long tkn;	
	long tknpi;
	long tknvardefi;

	struct def_s def;
	long* defiv; long defic; // indexes of tknv
	struct def_s funcdef;
	long* argiv; long argic; // indexes of tknv
	long flgdef;
	long flgargdef;	
	long flgassembly;
	long flgjump; long jumpfi;};

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
	struct inline_s* inlinev; long inlinec;};

enum {
	// ids
	TYPE,
	KEYWORD,
	DIRECTIVE,
	ASSEMBLY,
	SECTDEF,
	SECT,
	STRUCTDEF,
	STRUCT,
	INLINEDEF,
	INLINE,
	VARDEF,
	VAR,
	INCLUDE,
	// literals
	STR,
	NUMLIT,
	// math operators
	ADD,		//+
	SUB,		//-
	MUL,		//*
	DIV,		///
	MOD,		//%
	// bitwise operators
	AND,		//&
	OR,		//|
	XOR,		//^
	NOT,		//~
	// type operators
	CAST,		//?
	// assignment operators
	MOV,		//:
	CASTMOV,	//?:
	ADDMOV,		//+:
	SUBMOV,		//-:
	MULMOV,		//*:
	DIVMOV,		///:
	MODMOV,		//%:
	ANDMOV,		//&:
	ORMOV,		//|:
	XORMOV,		//^:
	NOTMOV,		//~:
	EQUALMOV,	//=:
	NOTEQUALMOV,	//~=:
	GREATEREQUALMOV,//>=:
	LESSEREQUALMOV,	//<=:
	ANDBOOLMOV,	//&&:
	ORBOOLMOV,	//||:
	NOTBOOLMOV,	//~~:
	GREATERMOV,	//>:
	LESSERMOV,	//<:
	ADDRMOV,	//$:
	DEREFMOV,	//@:
	// boolean operators
	EQUAL,		//=
	NOTEQUAL,	//~=
	GREATEREQUAL,	//>=
	LESSEREQUAL,	//<=
	ANDBOOL,	//&&
	ORBOOL,		//||
	NOTBOOL,	//~~
	GREATER,	//>
	LESSER,		//<
	// memory operators
	ADDR,		//$
	DEREF,		//@
	DOT,		//.
	ODEREF,		//[
	CDEREF,		//]
	// flow operators
	JUMP,		//\	/
	CALL,		//!
	// function operators
	OFUNC,		//(
	CFUNC,		//)
	// literal operators	
	COMMA,		//,
	OLIST,		//{
	CLIST,		//}
	DQUOTE,		//"
	SQUOTE,		//'
	// precedence operators
	TERM,		//;
	OBRACK,		//(
	CBRACK};	//)

extern struct lexer_s lexer;

extern void lex();
extern void lexfree();
extern void lexread();

#endif
