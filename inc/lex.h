#ifndef W_LEX
#define W_LEX
#include <stdio.h>

struct var_s {
	char* name;
	char** keyw;
	char** sect;
	char** type;
	struct struct_s* struct_;};

struct struct_s {
	char* name;
	struct var_s* memv; int memc;};

struct lexf_s {
	int fi;

	char* str;
	int stri;
	int strtkni;
	int strvlinei;
	int strvlinec;
	
	int tkn;	
	int tknpi;
	int tknvardefi;

	int flgvariable;	
	struct var_s* variable;
	int flgassembly;
	int flginclude;};

struct lexer_s {
	char* tknv; int tknc;

	char** asmv; int asmc;	
	char** fv; int fc;	
	char** dirv; int dirc;
	char** keywv; int keywc;
	char** sectv; int sectc;
	char** inlinev; int inlinec;
	char** typev; int typec;
	char** strv; int strc;
	long* longv; int longc;
	struct var_s* varv; int varc;
	struct struct_s* structv; int structc;};

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
	LONG,
	INT,
	SHORT,
	CHAR,
	SLONG,
	SINT,
	SSHORT,
	SCHAR,
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

#endif
