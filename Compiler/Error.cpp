#include "Error.h"

void Error::BuildErrorString( string& xError,
							  const Parser& xParser,
							  const CompilerTokenizer& xTokenizer,
							  ERROR eError,
							  const char* pxInvalid /*= NULL*/,
							  const char* pxExpected /*= NULL*/,
							  int iKeyword /*= 0*/ )
{
	xError += "Compiler error. In file: ";
	xError += xParser.GetFileName();
	if (xParser.GetClassName() != "")
	{
		xError += ", class: ";
		xError += xParser.GetClassName();
	}
	if (xParser.GetFunctionName() != "")
	{
		xError += ", function: ";
		xError += xParser.GetFunctionName();
	}
	xError += ", line: ";
	xError += std::to_string(xTokenizer.GetLineNumber());
	xError += " : ";

	BuildErrorStringInternal(xError, eError, pxInvalid, pxExpected, iKeyword);
}

void Error::BuildErrorString( string& xError,
							  const Location& xLocation,
							  ERROR eError,
							  const char* pxInvalid /*= NULL*/,
							  const char* pxExpected /*= NULL*/)
{
	xError += "Resolver error. In file: ";
	xError += xLocation.m_xFile;
	if (xLocation.m_xClass != "")
	{
		xError += ", class: ";
		xError += xLocation.m_xClass;
	}
	if (xLocation.m_xFunction != "")
	{
		xError += ", function: ";
		xError += xLocation.m_xFunction;
	}
	xError += ", line: ";
	xError += std::to_string(xLocation.m_uLine);
	xError += " : ";

	BuildErrorStringInternal(xError, eError, pxInvalid, pxExpected);
}

void Error::FillErrorWithKeywordList(string& xError, int iKeyword)
{
	bool bAddOr = false;
	if (iKeyword & CompilerTokenizer::KEYWORD_CLASS)
	{
		bAddOr = true;
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_CLASS);
	}

	if (iKeyword & CompilerTokenizer::KEYWORD_METHOD)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_METHOD);
		bAddOr = true;
	}

	if (iKeyword & CompilerTokenizer::KEYWORD_FUNCTION)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_METHOD);
		bAddOr = true;
	}

	if (iKeyword & CompilerTokenizer::KEYWORD_CONSTRUCTOR)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_CONSTRUCTOR);
		bAddOr = true;
	}

	if (iKeyword & CompilerTokenizer::KEYWORD_INT)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_INT);
		bAddOr = true;
	}

	if (iKeyword & CompilerTokenizer::KEYWORD_BOOLEAN)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_BOOLEAN);
		bAddOr = true;
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_CHAR)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_CHAR);
		bAddOr = true;
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_VOID)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_VOID);
		bAddOr = true;
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_VAR)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_VAR);
		bAddOr = true;
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_STATIC)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_STATIC);
		bAddOr = true;
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_FIELD)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_FIELD);
		bAddOr = true;
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_LET)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_LET);
		bAddOr = true;
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_DO)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_DO);
		bAddOr = true;
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_IF)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_IF);
		bAddOr = true;
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_ELSE)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_ELSE);
		bAddOr = true;
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_WHILE)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_WHILE);
		bAddOr = true;
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_RETURN)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_RETURN);
		bAddOr = true;
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_TRUE)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_TRUE);
		bAddOr = true;
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_FALSE)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_FALSE);
		bAddOr = true;
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_NULL)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_NULL);
		bAddOr = true;
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_THIS)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_THIS);
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_DESTRUCTOR)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_DESTRUCTOR);
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_VIRTUAL)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_VIRTUAL);
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_CAST)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_CAST);
	}
	if (iKeyword & CompilerTokenizer::KEYWORD_ADDRESS)
	{
		if (bAddOr) xError += " or ";
		xError += CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_ADDRESS);
	}

	xError += ".";
}

void Error::BuildErrorStringInternal( string& xError,
									  int eError,
									  const char* pxInvalid /*= NULL*/,
									  const char* pxExpected /*= NULL*/,
									  int iKeyword /*= 0*/ )
{
	xError += GetErrorString(eError);

	if (pxInvalid)
	{
		xError += ": '";
		xError += pxInvalid;
		xError += "'";
	}

	if (pxExpected)
	{
		xError += " Expected: '";
		xError += pxExpected;
		xError += "'";
	}

	if(iKeyword!=0)
		FillErrorWithKeywordList(xError, iKeyword);

	if (!pxInvalid && !pxExpected)
	{
		xError += ".";
	}

	xError += '\n';
}

const char* Error::GetErrorString(int eError)
{
	if (eError == ERROR_INTERNAL             ) return "ERROR_INTERNAL";
	if (eError == ERROR_UNRESOLVED_IDENTIFIER) return "ERROR_UNRESOLVED_IDENTIFIER";
	if (eError == ERROR_EXPECTED_KEYWORD     ) return "ERROR_EXPECTED_KEYWORD";
	if (eError == ERROR_EXPECTED_SYMBOL      ) return "ERROR_EXPECTED_SYMBOL";
	if (eError == ERROR_EXPECTED_IDENTIFIER  ) return "ERROR_EXPECTED_IDENTIFIER";
	if (eError == ERROR_EXPECTED_TYPE        ) return "ERROR_EXPECTED_TYPE";
	if (eError == ERROR_FUNCTION_REDEFINITION) return "ERROR_FUNCTION_REDEFINITION";
	if (eError == ERROR_VARIABLE_REDEFINITION) return "ERROR_VARIABLE_REDEFINITION";
	if (eError == ERROR_CLASS_REDEFINITION   ) return "ERROR_CLASS_REDEFINITION";
	if (eError == ERROR_ARRAY_DIMENSION      ) return "ERROR_ARRAY_DIMENSION";
	if (eError == ERROR_UNRESOLVED_CLASS     ) return "ERROR_UNRESOLVED_CLASS";
	if (eError == ERROR_UNRESOLVED_FUNCTION  ) return "ERROR_UNRESOLVED_FUNCTION";
	if (eError == ERROR_ARGUMENT_NUMBER      ) return "ERROR_ARGUMENT_NUMBER";
	if (eError == ERROR_INVALID_TYPE         ) return "ERROR_INVALID_TYPE";
	if (eError == ERROR_OPERATOR_TYPE        ) return "WRONG OPERATOR OR WRONG LEFT OR RIGHT OPERANDS TYPE";
	if (eError == ERROR_ADDRESS              ) return "ERROR_ADDRESS";
	if (eError == ERROR_CONSTRUCTOR          ) return "ERROR_CONSTRUCTOR";
	if (eError == ERROR_DESTRUCTOR           ) return "ERROR_DESTRUCTOR";
	if (eError == ERROR_LEFT_VALUE			 ) return "ERROR_LEFT_VALUE";

	return "invalid error!";
}