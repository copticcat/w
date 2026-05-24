CC = gcc
CFLAGS = -g -Iinc -std=gnu99 -Wall -Wextra -Wpedantic -lm

SRC = src/main.c src/lex.c src/print.c src/asm.c
OBJ = $(patsubst src/%.c,obj/%.o,$(SRC))
BIN = ding

all: $(BIN)

obj:
	mkdir -p obj

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(CFLAGS)

obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	{ printf 'start demo/var.d\nlex var.l\nasm var.s\nquit\n'; } | ./$(BIN)
	as var.s -o var.o
	ld var.o -o var
	./var

asm: $(BIN)
	as var.s -o var.o
	ld var.o -o var
	./var

slow: $(BIN)
	{ printf 'start demo/var.d\ndelay 1\nlex var.l\nasm var.s\nquit\n'; } | ./$(BIN)
	as var.s -o var.o
	ld var.o -o var
	./var

clean:
	rm -rf obj $(BIN) var.s var.l var.o var
