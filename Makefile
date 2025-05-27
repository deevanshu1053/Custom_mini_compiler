# Makefile for mini compiler

# Compiler and flags
CC = gcc
CFLAGS = -Wall -g
LEX = flex
YACC = bison -d

# Targets
TARGET = compiler
OBJS = ast.o main.o
SRC = main.c ast.c parser.y lexer.l

# Default rule
all: $(TARGET)

# Build the final executable
$(TARGET): parser.tab.c lex.yy.c $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) parser.tab.c lex.yy.c $(OBJS) -lfl

# Bison generates parser.tab.c and parser.tab.h
parser.tab.c parser.tab.h: parser.y
	$(YACC) parser.y

# Flex generates lex.yy.c and depends on parser.tab.h
lex.yy.c: lexer.l parser.tab.h
	$(LEX) lexer.l

# Compile object files
main.o: main.c ast.h
	$(CC) $(CFLAGS) -c main.c

ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c

# Clean generated files
clean:
	rm -f $(TARGET) *.o parser.tab.* lex.yy.c ui_temp_input.txt
