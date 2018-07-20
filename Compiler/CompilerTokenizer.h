#ifndef _COMPILER_TOKENIZER_
#define _COMPILER_TOKENIZER_

#include "Tokenizer.h"

class CompilerTokenizer : public Tokenizer
{
public:
	enum KEYWORD_TYPE
	{
		KEYWORD_INVALID = 1 << 0,
		KEYWORD_CLASS = 1 << 1,
		KEYWORD_METHOD = 1 << 2,
		KEYWORD_FUNCTION = 1 << 3,
		KEYWORD_CONSTRUCTOR = 1 << 4,
		KEYWORD_INT = 1 << 5,
		KEYWORD_BOOLEAN = 1 << 6,
		KEYWORD_CHAR = 1 << 7,
		KEYWORD_VOID = 1 << 8,
		KEYWORD_VAR = 1 << 9,
		KEYWORD_STATIC = 1 << 10,
		KEYWORD_FIELD = 1 << 11,
		KEYWORD_LET = 1 << 12,
		KEYWORD_DO = 1 << 13,
		KEYWORD_IF = 1 << 14,
		KEYWORD_ELSE = 1 << 15,
		KEYWORD_WHILE = 1 << 16,
		KEYWORD_RETURN = 1 << 17,
		KEYWORD_TRUE = 1 << 18,
		KEYWORD_FALSE = 1 << 19,
		KEYWORD_NULL = 1 << 20,
		KEYWORD_THIS = 1 << 21,
		KEYWORD_CAST = 1 << 22,
		KEYWORD_VIRTUAL = 1 << 23,
		KEYWORD_ADDRESS = 1 << 24,
		KEYWORD_DESTRUCTOR = 1 << 25,
		KEYWORD_ASM        = 1 << 26
	};

	CompilerTokenizer(const std::string& xSource, bool bIsString = false);
	CompilerTokenizer(const CompilerTokenizer& xOther);
	virtual ~CompilerTokenizer() {};

	static KEYWORD_TYPE GetKeywordType(const std::string& xKeyword);
	static const char*  GetKeywordString(KEYWORD_TYPE eKeywordType);

private:
	virtual TOKEN_TYPE FindTokenType(const std::string& xToken) const;
};

#endif // _COMPILER_TOKENIZER_