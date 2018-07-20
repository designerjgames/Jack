#include "CompilerTokenizer.h"

#include <regex>

using namespace std;

CompilerTokenizer::CompilerTokenizer(const string& xSource, bool bIsString /*= false*/)
: Tokenizer(xSource, bIsString)
{

}

CompilerTokenizer::CompilerTokenizer(const CompilerTokenizer& xOther)
	: Tokenizer(xOther)
{

}

CompilerTokenizer::TOKEN_TYPE CompilerTokenizer::FindTokenType(const string& xToken) const
{
	TOKEN_TYPE eTokenType = Tokenizer::FindTokenType(xToken);
	if (eTokenType != TOKEN_UNKNOWN)
		return eTokenType;

	// "\\" means loose its meaning for the reg
	std::regex xRgxS("\\{|\\}|\\(|\\)|\\[|\\]|\\.|,|;|\\+|-|\\*|/|&|\\||<|>|=|~|:|\\!");
	if (std::regex_match(xToken, xRgxS))
	{
		return TOKEN_SYMBOL;
	}

	//Check for symbols
	if (xToken.size() == 1)
	{
		if (xToken == "#")
		{
			return TOKEN_KEYWORD;
		}
	}
	else
	{
		//Check for keywords
		std::regex xRgxK("class|constructor|function|method|field|static|var|int|char|boolean|void|true|false|"
			"null|this|let|do|if|else|while|return|cast|virtual|destructor|asm");
		if (std::regex_match(xToken, xRgxK))
		{
			return TOKEN_KEYWORD;
		}
	}

	return TOKEN_UNKNOWN;
}

CompilerTokenizer::KEYWORD_TYPE CompilerTokenizer::GetKeywordType(const string& xKeyword)
{
	if (xKeyword == "class") return KEYWORD_CLASS;
	if (xKeyword == "method") return KEYWORD_METHOD;
	if (xKeyword == "function") return KEYWORD_FUNCTION;
	if (xKeyword == "constructor") return KEYWORD_CONSTRUCTOR;
	if (xKeyword == "int") return KEYWORD_INT;
	if (xKeyword == "boolean") return KEYWORD_BOOLEAN;
	if (xKeyword == "char") return KEYWORD_CHAR;
	if (xKeyword == "void") return KEYWORD_VOID;
	if (xKeyword == "var") return KEYWORD_VAR;
	if (xKeyword == "static") return KEYWORD_STATIC;
	if (xKeyword == "field") return KEYWORD_FIELD;
	if (xKeyword == "let") return KEYWORD_LET;
	if (xKeyword == "do") return KEYWORD_DO;
	if (xKeyword == "if") return KEYWORD_IF;
	if (xKeyword == "else") return KEYWORD_ELSE;
	if (xKeyword == "while") return KEYWORD_WHILE;
	if (xKeyword == "return") return KEYWORD_RETURN;
	if (xKeyword == "true") return KEYWORD_TRUE;
	if (xKeyword == "false") return KEYWORD_FALSE;
	if (xKeyword == "null") return KEYWORD_NULL;
	if (xKeyword == "this") return KEYWORD_THIS;
	if (xKeyword == "cast") return KEYWORD_CAST;
	if (xKeyword == "virtual") return KEYWORD_VIRTUAL;
	if (xKeyword == "#") return KEYWORD_ADDRESS;
	if (xKeyword == "destructor") return KEYWORD_DESTRUCTOR;
	if (xKeyword == "asm") return KEYWORD_ASM;

	return KEYWORD_INVALID;
}

const char* CompilerTokenizer::GetKeywordString(KEYWORD_TYPE eKeywordType)
{
	if (eKeywordType == KEYWORD_CLASS) return "class";
	if (eKeywordType == KEYWORD_METHOD) return "method";
	if (eKeywordType == KEYWORD_FUNCTION) return "function";
	if (eKeywordType == KEYWORD_CONSTRUCTOR) return "constructor";
	if (eKeywordType == KEYWORD_INT) return "int";
	if (eKeywordType == KEYWORD_BOOLEAN) return "boolean";
	if (eKeywordType == KEYWORD_CHAR) return "char";
	if (eKeywordType == KEYWORD_VOID) return "void";
	if (eKeywordType == KEYWORD_VAR) return "var";
	if (eKeywordType == KEYWORD_STATIC) return "static";
	if (eKeywordType == KEYWORD_FIELD) return "field";
	if (eKeywordType == KEYWORD_LET) return "let";
	if (eKeywordType == KEYWORD_DO) return "do";
	if (eKeywordType == KEYWORD_IF) return "if";
	if (eKeywordType == KEYWORD_ELSE) return "else";
	if (eKeywordType == KEYWORD_WHILE) return "while";
	if (eKeywordType == KEYWORD_RETURN) return "return";
	if (eKeywordType == KEYWORD_TRUE) return "true";
	if (eKeywordType == KEYWORD_FALSE) return "false";
	if (eKeywordType == KEYWORD_NULL) return "null";
	if (eKeywordType == KEYWORD_THIS) return "this";
	if (eKeywordType == KEYWORD_CAST) return "cast";
	if (eKeywordType == KEYWORD_VIRTUAL) return "virtual";
	if (eKeywordType == KEYWORD_ADDRESS) return "#";
	if (eKeywordType == KEYWORD_DESTRUCTOR) return "destructor";
	if (eKeywordType == KEYWORD_ASM) return "asm";

	return "invalid keyword";
}