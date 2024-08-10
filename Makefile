CPP=g++
CPPFLAGS=-O0 -I. -g3 -Wall -Wextra
# CPPFLAGS+=-fsanitize=undefined
# CPPFLAGS=-O3 -I.
OBJ=main.cpp lex.cpp parse.cpp
OUTPUT=jasm

%.o: %.cpp
	@$(CPP) -c -o $@ $< $(CPPFLAGS)

build: $(OBJ)
	@$(CPP) -o $(OUTPUT) $^ $(CPPFLAGS)

