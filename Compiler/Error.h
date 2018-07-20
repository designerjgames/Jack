#ifndef _ERROR_
#define _ERROR_

#include <string>
#include "Location.h"
#include "Tokenizer.h"
#include "Parser.h"

using namespace std;

class Error
{
public:
	enum ERROR
	{
		// Parser
		ERROR_ADDRESS,
		ERROR_INTERNAL,
		ERROR_UNRESOLVED_IDENTIFIER,
		ERROR_EXPECTED_KEYWORD,
		ERROR_EXPECTED_SYMBOL,
		ERROR_EXPECTED_FUNCTION_CALL,
		ERROR_EXPECTED_IDENTIFIER,
		ERROR_EXPECTED_TYPE,
		ERROR_FUNCTION_REDEFINITION,
		ERROR_VARIABLE_REDEFINITION,
		ERROR_CLASS_REDEFINITION,
		ERROR_ARRAY_DIMENSION,
		ERROR_CONSTRUCTOR,
		ERROR_DESTRUCTOR,
		ERROR_LEFT_VALUE,

		// Resolver
		ERROR_UNRESOLVED_CLASS,
		ERROR_UNRESOLVED_FUNCTION,
		ERROR_ARGUMENT_NUMBER,
		ERROR_INVALID_TYPE,
		ERROR_OPERATOR_TYPE
	};
	
	static void BuildErrorString( string&          xError,
							      const Parser&    xParser,
							      const CompilerTokenizer& xTokenizer,
							      ERROR     eError,
							      const char*	   pxInvalid  = NULL,
							      const char*    pxExpected = NULL,
							      int              iKeyword   = 0);

	static void BuildErrorString( string&         xError,
							      const Location& xLocation,
							      ERROR  eError,
							      const char*   pxInvalid  = NULL,
							      const char*   pxExpected = NULL);

	static void BuildErrorStringInternal( string&		 xError,
								   int           eError,
								   const char* pxInvalid  = NULL,
								   const char* pxExpected = NULL,
								   int iKeyword = 0);

	static void FillErrorWithKeywordList( string& xError, int iKeyword );

	static const char* GetErrorString(int eError);
};

#endif //Error