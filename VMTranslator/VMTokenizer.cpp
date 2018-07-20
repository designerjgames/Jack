#include "VMTokenizer.h"

#include <regex>

using namespace std;

VMTokenizer::VMTokenizer(const string& xSource, bool bIsString /*= false*/)
: Tokenizer(xSource, bIsString)
{

}

VMTokenizer::TOKEN_TYPE VMTokenizer::FindTokenType(const string& xToken) const
{
	TOKEN_TYPE eTokenType = Tokenizer::FindTokenType(xToken);
	if (eTokenType != TOKEN_UNKNOWN)
		return eTokenType;

	std::regex xRgxS("\\{|\\}|\\;|\\/");
	if (std::regex_match(xToken, xRgxS))
	{
		return TOKEN_SYMBOL;
	}

	//Check for keywords
	std::regex xRgxK("eq|gt|lt|ge|le|or|add|sub|neg|and|not|pop|push|pusha|pushf|goto|label|if-goto|function|call|callv|return"
					 "|static|this|local|argument|that|constant|pointer|temp|asm|memory");
	if (std::regex_match(xToken, xRgxK))
	{
		return TOKEN_KEYWORD;
	}

	return TOKEN_UNKNOWN;
}

VMTokenizer::KEYWORD_TYPE VMTokenizer::GetKeywordType(const string& xKeyword)
{
	if (xKeyword == "eq") return COMMAND_ARITHMETIC_EQ;
	if (xKeyword == "gt") return COMMAND_ARITHMETIC_GT;
	if (xKeyword == "lt") return COMMAND_ARITHMETIC_LT;
	if (xKeyword == "ge") return COMMAND_ARITHMETIC_GE;
	if (xKeyword == "le") return COMMAND_ARITHMETIC_LE;
	if (xKeyword == "or") return COMMAND_ARITHMETIC_OR;
	if (xKeyword == "add") return COMMAND_ARITHMETIC_ADD;
	if (xKeyword == "sub") return COMMAND_ARITHMETIC_SUB;
	if (xKeyword == "neg") return COMMAND_ARITHMETIC_NEG;
	if (xKeyword == "and") return COMMAND_ARITHMETIC_AND;
	if (xKeyword == "not") return COMMAND_ARITHMETIC_NOT;
	if (xKeyword == "push") return COMMAND_PUSH;
	if (xKeyword == "pusha") return COMMAND_PUSH_ADDRESS;
	if (xKeyword == "pushf") return COMMAND_PUSH_FUNCTION;
	if (xKeyword == "pop") return COMMAND_POP;
	if (xKeyword == "label") return COMMAND_LABEL;
	if (xKeyword == "goto") return COMMAND_GOTO;

	if (xKeyword == "if-goto") return COMMAND_IF;
	if (xKeyword == "function") return COMMAND_FUNCTION;
	if (xKeyword == "call") return COMMAND_CALL;
	if (xKeyword == "callv") return COMMAND_CALL_VIRTUAL;
	if (xKeyword == "return") return COMMAND_RETURN;
	if (xKeyword == "asm") return COMMAND_ASM;

	if (xKeyword == "static") return RAM_SEGMENT_STATIC;
	if (xKeyword == "this") return RAM_SEGMENT_THIS;
	if (xKeyword == "local") return RAM_SEGMENT_LOCAL;
	if (xKeyword == "argument") return RAM_SEGMENT_ARGUMENT;
	if (xKeyword == "that") return RAM_SEGMENT_THAT;
	if (xKeyword == "constant") return RAM_SEGMENT_CONSTANT;
	if (xKeyword == "pointer") return RAM_SEGMENT_POINTER;
	if (xKeyword == "temp") return RAM_SEGMENT_TEMP;
	if (xKeyword == "memory") return COMMAND_MEMORY;


	return KEYWORD_INVALID;
}


