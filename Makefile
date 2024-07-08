CC = $(shell which clang++)
CC ?= $(shell which g++) # fallback compiler

CPP_SRC = $(wildcard src/*.cpp)
HEADERS = $(wildcard include/*.hpp)
OBJ = $(patsubst src/%.cpp, obj/%.o, $(CPP_SRC))

CFLAGS = -g -std=c++20 -Wall -pedantic -Iinclude

run: compile
	./bin/nimble

compile: bin/nimble

bin/nimble: $(OBJ) | bin
	$(CC) -o $@ $(OBJ)

bin:
	mkdir -p bin

obj/%.o: src/%.cpp $(HEADERS) | obj
	${CC} $(CFLAGS) -c $< -o $@

obj:
	mkdir -p obj

clean:
	rm -f bin/* obj/*.o

ast:
	mv src/ast_printer_driver.tmp src/ast_printer_driver.cpp
	$(CC) $(CFLAGS) -c src/token.cpp -o obj/token.o
	$(CC) $(CFLAGS) -c src/expr.cpp -o obj/expr.o
	$(CC) $(CFLAGS) -c src/ast_printer.cpp -o obj/ast_printer.o
	$(CC) $(CFLAGS) -c src/ast_printer_driver.cpp -o obj/ast_printer_driver.o
	$(CC) -o bin/ast obj/token.o obj/expr.o obj/ast_printer.o obj/ast_printer_driver.o
	mv src/ast_printer_driver.cpp src/ast_printer_driver.tmp

.PHONY: run compile clean
