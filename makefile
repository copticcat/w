CC = gcc
CFLAGS = -g -Iinc -std=gnu99 -Wall -Wextra -Wpedantic -lm
SRC = src/main.c src/lex/lex.c src/print.c src/asm.c src/lex/eval.c src/lex/new.c
OBJ = $(patsubst src/%.c,obj/%.o,$(SRC))
BIN = ding

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(CFLAGS)

obj/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	{ printf 'start demo/var.d\nlex var.l\nasm var.s\nquit\n'; } | ./$(BIN)

asm: $(BIN)
	as var.s -o var.o
	ld var.o -o var
	./var

mid: $(BIN)
	{ printf 'start demo/var.d\ndelay 0.3\nlex var.l\nasm var.s\nquit\n'; } | ./$(BIN)

slow: $(BIN)
	{ printf 'start demo/var.d\ndelay 1\nlex var.l\nasm var.s\nquit\n'; } | ./$(BIN)

clean:
	rm -rf obj $(BIN) var.s var.l var.o var
