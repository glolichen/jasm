CPP=g++
CPPFLAGS=-O0 -I. --debug -g
OBJ=main.cpp lex.cpp parse.cpp firstpass.cpp
OUTPUT=jasm

%.o: %.cpp
	@$(CPP) -c -o $@ $< $(CPPFLAGS)

build: $(OBJ)
	@$(CPP) -o $(OUTPUT) $^ $(CPPFLAGS)

run: build
	@./$(OUTPUT)

