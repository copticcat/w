CC = gcc
CFLAGS = -g -Iinc -std=gnu99 -Wall -Wextra -Wpedantic

SRC = src/main.c src/lex.c src/print.c
OBJ = $(SRC:.c=.o)
BIN = w

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN)

run: $(BIN)
	./$(BIN) demo/hw.w -s hw.s

clean:
	rm -f $(OBJ) $(BIN)
	rm -f hw.s 
