CC = gcc
CFLAGS = -g -Iinc -std=gnu99 -Wall -Wextra -Wpedantic

SRC = src/main.c src/lex.c src/print.c
OBJ = $(SRC:.c=.o)
BIN = w

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN)

run: $(BIN)
	./$(BIN) demo/0.w -s 0.s

clean:
	rm -f $(OBJ) $(BIN)
	rm -f 0.s 
