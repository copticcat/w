#ifndef DING_ENUM
#define DING_ENUM

// token types
enum {
	STR,
	
	// compile time directives
	ASSEMBLY,
	INCLUDE,

	// memory allocation
	NUMLIT,
	VARLIT,
	ZEROLIT,

	// memory management
	VARDEF,

	// memory assignment
	MOV,
	NUM,
	VAR,

	// math	
	MATH};

// internal types
enum {
	LIT,
	VARI,
	REGI};

// operators
enum {
	OB,
	CB,
	ADD,
	SUB,
	MUL,
	DIV,
	SET};

#endif
