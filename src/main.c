#include "../inc/glob.h"
#include "../inc/lex.h"
#include "../inc/asm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>

// globals
struct f_s* fv = 0; int fc = 0;
long asmfi = -1;
long startfi = -1;
long lexfi = -1;

double delay = 0;

// locals
char** historyv = 0; int historyc = 0;

// local funcs
static void allocf(char*, char*, int, int, int, int);
static void freef(char*);

// global funcs
int main() {
	printf("\033[32mwelcome to ding\033[0m\n");

	// set new term settings
	struct termios ot, nt;
	tcgetattr(STDIN_FILENO, &ot);
	nt = ot;                      
	nt.c_lflag &= ~(ICANON | ECHO); // disable line buffering and echo
	nt.c_cc[VMIN] = 1;              // read returns after 1 char
	nt.c_cc[VTIME] = 0;             // no timeout, block until char arrives
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &nt);
	
	// read history
	FILE* historyf = fopen("data/history", "a+");
	char* line = 0;
	size_t thing = 0;
	while (getline(&line, &thing, historyf) != -1) {
		historyv = realloc(historyv, sizeof(char*) * ++historyc);
		historyv[historyc - 1] = strdup(line);}
	free(line);

	// include path
	DIR *path;
	struct dirent *ent;
	path = opendir("std");
	while ((ent = readdir(path)) != 0) {
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
			continue;
		char dir[1024];
		snprintf(dir, sizeof(dir), "std/%s", ent->d_name);
		allocf(dir, ent->d_name, 0, 0, 0, 0);}

	// cli loop
	int quit = 0;
	while (!quit) {
		printf("ding$ ");
		line = 0;
		int linelen = 0;
		char ch = 0;
		int historyi = historyc;
		int linei = 0;
		while ((ch = getchar())) {
			if (ch == 127) {
				if (linei == 0) continue;
				--linei;
				del:
				for (int i = linei; i < linelen - 1; ++i) line[i] = line[i + 1];
				line = realloc(line, --linelen);}
			else if (ch == '\033') {
				if (getchar() != '[') continue;
				if ((ch = getchar()) == 'A') {
					if (historyi == 0) continue;
					--historyi;}
				else if (ch == 'B') {
					if (historyi == historyc) continue;
					++historyi;
					if (historyi == historyc) { 
						free(line);
						line = 0;
						linelen = 0;
						linei = 0;
						goto print;}}
				else if (ch == 'C') {
					if (linei == linelen) continue;
					++linei;
					goto print;}
				else if (ch == 'D') {
					if (linei == 0) continue;
					--linei;
					goto print;}
				else if (ch == '3' && getchar() == '~') {
					if (linei == linelen) continue;
					goto del;}
				else continue;
				free(line);
				linelen = strlen(historyv[historyi]) - 1;
				line = malloc(linelen);
				linei = linelen;
				memcpy(line, historyv[historyi], linelen);}
			else if (ch == '\n') break;
			else if (ch >= 32 && ch <= 126) {
				line = realloc(line, ++linelen);
				for (int i = linelen - 1; i > linei; --i) line[i] = line[i - 1];
				line[linei++] = ch;}
			else continue;
			print:
			printf("\rding$ %.*s\033[J\r\033[%dC", linelen, line ? line : "", 6 + linei);}
		line = realloc(line, linelen + 2);
		memcpy(line + linelen++, "\n", 2);
		printf("\n");

		char** lineargv = 0; int lineargc = 0;
		char* arg = 0;
		for (int i = 0; line[i] != 0; ++i) {
			switch (line[i]) {
			case ' ':
			case '\n':
			case '\t':
				if (!arg) break;
				lineargv = realloc(lineargv, sizeof(char*) * ++lineargc);
				lineargv[lineargc - 1] = strdup(arg);
				free(arg);
				arg = 0;
				break;
			default: {
				int len = arg ? strlen(arg) : 0;
				arg = realloc(arg, len + 2);
				arg[len] = line[i];
				arg[len + 1] = 0;
				break;}}}

		if (lineargc < 1) continue;

		if (strcmp(lineargv[0], "tldr") == 0) {
			if (lineargc == 1) {
				printf(
					"\033[36mtldr\033[0m\n"
					"\033[36mquit\033[0m/\033[36mexit\033[0m\n"
					"\033[36mlink \033[0m[\033[1mding code\033[0m] <\033[1malias\033[0m>\n"
					"\033[36mstart \033[0m[\033[1mding code\033[0m] <\033[1malias\033[0m>\n"
					"\033[36munlink \033[0m[\033[1mding code\033[0m]\n"
					"\033[36mlex \033[0m[\033[1mout lex code\033[0m]\n"
					"\033[36masm \033[0m[\033[1min lex code\033[0m] [\033[1mout asm code\033[0m]\n"
					"\033[36mdelay \033[0m[\033[1mseconds\033[0m]\n");
			goto allocfhistory;}
			else goto invalidargc;}
		else if (strcmp(lineargv[0], "quit") == 0 || strcmp(lineargv[0], "exit") == 0) {
			if (lineargc == 1) {
				printf("\033[32mgoodbye friend\033[0m\n");
				quit = 1;
				goto allocfhistory;}
			else goto invalidargc;}
		else if (strcmp(lineargv[0], "link") == 0) {
			if (lineargc == 2) {
				allocf(lineargv[1], lineargv[1], 0, 0, 0, 0);
				goto allocfhistory;}
			else if (lineargc == 3) {
				allocf(lineargv[1], lineargv[2], 0, 0, 0, 0);
				goto allocfhistory;}
			else goto invalidargc;}
		else if (strcmp(lineargv[0], "start") == 0) {
			if (lineargc == 2) {
				allocf(lineargv[1], lineargv[1], 0, 1, 0, 0);
				goto allocfhistory;}
			else if (lineargc == 3) {
				allocf(lineargv[1], lineargv[2], 0, 1, 0, 0);
				goto allocfhistory;}
			else goto invalidargc;}
		else if (strcmp(lineargv[0], "asm") == 0) {
			if (lineargc == 2) {
				allocf(lineargv[1], lineargv[1], 1, 0, 1, 0);
				asm_();
				goto allocfhistory;}
			else goto invalidargc;}
		else if (strcmp(lineargv[0], "unlink") == 0) {
			if (lineargc == 2) {
				freef(lineargv[1]);
				goto allocfhistory;}
			else goto invalidargc;}
		else if (strcmp(lineargv[0], "lex") == 0) {
			if (lineargc == 2) {
				allocf(lineargv[1], lineargv[1], 1, 0, 0, 1);
				lex();
				goto allocfhistory;}
			else goto invalidargc;}
		else if (strcmp(lineargv[0], "delay") == 0) {
			if (lineargc == 2) {
				char* last;
				double d = strtod(lineargv[1], &last);
				if (last == lineargv[1]) printf("\033[31margument must be a number\033[0m %s\n", lineargv[1]);
				else {
					printf("\033[32mdelay set\033[0m %g\n", d);
					delay = d;}
				goto allocfhistory;}
			else goto invalidargc;}
		else {
			printf("\033[31minvalid command\033[0m %s\n", lineargv[0]);
			goto end;}
		invalidargc:
		printf("\033[31minvalid argument count\033[0m %.*s\n", linelen - 1, line);
		goto end;
		allocfhistory:
		if (historyc && strcmp(historyv[historyc - 1], line) == 0) goto end;
		historyv = realloc(historyv, sizeof(char*) * ++historyc);
		historyv[historyc - 1] = strdup(line);
		fseek(historyf, 0, SEEK_END);
		fprintf(historyf, "%s", line);
		end:
		free(line);}

	// free	
	for (int i = 0; i < fc; ++i) {
		fclose(fv[i].f);
		free(fv[i].dir);
		free(fv[i].alias);}
	for (int i = 0; i < historyc; ++i)
		free(historyv[i]);
	free(historyv);
	free(fv);
	
	// restore old term settings
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &ot);

	return 0;}

static void allocf(char* dir, char* alias, int write, int start, int asm_, int lex_) {
	for (int i = 0; i < fc; ++i)
		if (strcmp(fv[i].dir, dir) == 0) {
			freef(dir);
			break;}

	FILE* f = write ? fopen(dir, "w+") : fopen(dir, "a+");
	if (f) {
		fv = realloc(fv, sizeof(struct f_s) * ++fc);
		fv[fc - 1].f = f;
		fv[fc - 1].dir = strdup(dir);
		fv[fc - 1].alias = strdup(alias);
		
		if (start) {
			startfi = fc - 1;
			if (strcmp(dir, alias) == 0) printf("\033[32mallocated start ding file\033[0m %s\n", dir);
			else printf("\033[32mallocated start ding file\033[0m %s \"%s\"\n", dir, alias);
			return;}
		else if (asm_) {
			asmfi = fc - 1;
			if (strcmp(dir, alias) == 0) printf("\033[32mallocated asm file\033[0m %s\n", dir);
			else printf("\033[32mallocated asm file\033[0m %s \"%s\"\n", dir, alias);
			return;}
		else if (lex_) {
			lexfi = fc - 1;
			if (strcmp(dir, alias) == 0) printf("\033[32mallocated lex file\033[0m %s\n", dir);
			else printf("\033[32mallocated lex file\033[0m %s \"%s\"\n", dir, alias);
			return;}
		else {
			if (strcmp(dir, alias) == 0) printf("\033[32mallocated ding file\033[0m %s\n", dir);
			else printf("\033[32mallocated ding file\033[0m %s \"%s\"\n", dir, alias);
			return;}}
	else 
		printf("\033[31minvalid directory\033[0m %s\n", dir);}

static void freef(char* dir) {
	int fi = -1;
	for (int i = 0; i < fc; ++i) 
		if (strcmp(fv[i].dir, dir) == 0) {
			fi = i;
			break;}
	if (fi < 0) {
		printf("\033[31mfile isn't allocated\033[0m %s\n", dir);
		return;}
	
	if (startfi == fi) startfi = -1;
	if (lexfi == fi) lexfi = -1;
	if (asmfi == fi) asmfi = -1;

	if (strcmp(dir, fv[fi].alias) == 0) printf("\033[32mfreed file\033[0m %s\n", dir);
	else printf("\033[32mfreed file\033[0m %s \"%s\"\n", dir, fv[fi].alias);

	fclose(fv[fi].f);
	free(fv[fi].dir);
	free(fv[fi].alias);

	for (int i = fi; i < fc - 1; ++i)
		fv[i] = fv[i + 1];
	
	if (startfi > fi) --startfi;
	if (lexfi > fi) --lexfi;
	if (asmfi > fi) --asmfi;
	--fc;}
