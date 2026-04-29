#include "../inc/glob.h"
#include "../inc/lex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

// globals
FILE** infv = 0; char** infnamev = 0; int infc = 0;

// locals
static FILE* sf = 0;

// global funcs
int main(int argc, char** argv) {
	// parse args
	if (argc < 2) {
		printf("Hello... I am the w compiler.\n");
		return 0;}
	for (int i = 1; i < argc; ++i) {
		char* arg = argv[i];
		if (strcmp(arg, "-s") == 0) {
			sf = fopen(argv[++i], "w");
			if (sf) continue;
			else {
				printf("\033[31minvalid out asm file: %s\033[0m\n", argv[i]);
				return 1;}}
		FILE* f = fopen(argv[i], "r");
		if (f) {
			infv = realloc(infv, sizeof(FILE*) * ++infc);
			infv[infc - 1] = f;
			infnamev = realloc(infnamev, sizeof(char*) * infc);
			infnamev[infc - 1] = strdup(argv[i]);
			continue;}
		else {
			printf("\033[31minvalid in file: %s\033[0m\n", argv[i]);
			return 1;}
		printf("\033[31minvalid argument: %s\033[0m\n", argv[i]);
		return 1;}

	// include path
	DIR *path;
	struct dirent *ent;
	path = opendir("path");
	while ((ent = readdir(path)) != 0) {
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
			continue;
		char filepath[1024];
		snprintf(filepath, sizeof(filepath), "path/%s", ent->d_name);
		
		FILE* f = fopen(filepath, "r");
		infv = realloc(infv, sizeof(FILE*) * ++infc);
		infv[infc - 1] = f;
		infnamev = realloc(infnamev, sizeof(char*) * infc);
		infnamev[infc - 1] = strdup(ent->d_name);}

	for (int i = 0; i < infc; ++i)
		printf("%s\n", infnamev[i]);

	// prepare display
	setvbuf(stdout, 0, 0, 0); // disable buffering
	printf("\033[49;1H\033[2J");
	fflush(stdout);

	// tokenize in files
	lex();

	// free	
	for (int i = 0; i < infc; ++i) {
		free(infnamev[i]);
		fclose(infv[i]);}
	if (sf) fclose(sf);

	// exit display
	printf("\033[2k\r");
	fflush(stdout);

	return 0;}

