// Assembler.cpp : Defines the entry point for the console application.
//
#include "Code.h"
#include "Parser.h"

int main(/*int argc, char* argv[]*/)
{
	Parser parser("program.asm");
	Code   code("program.asm");

	code.AddLabelsToSymbolTable( parser );
	code.Generate( parser );

	return 0;
}

