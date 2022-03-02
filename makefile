asembler: main.o lex.yy.o
	g++ -o asembler main.o lex.yy.o

main.o: main.cpp tableType.h define.h relocationType.h
	g++ -c main.cpp

lex.yy.o: lex.yy.cc
	g++ -c lex.yy.cc

lex.yy.cc: lexer_process.l
	flex++ lexer_process.l
	