#include "Parser.h"

#include <bitset>
#include <iostream>
#include <cctype>
#include "Error.h"
#include "Resolver.h"

SymbolTable<Parser>	Parser::s_xParsersSymbolTable;
char   Parser::s_cTypeOfArrayPrefix = '@';
string Parser::s_xFieldMarker  = "#F";
string Parser::s_xLocalsMarker = "#L";

Parser::Parser( const string& xFilename, CompilerTokenizer& xTokenizer )
: m_pxTokenizer( &xTokenizer )
, m_xVMWriter( *this )
, m_NeedToGetNextToken(true)
, m_xVariableClassSymbolTable()
, m_xVariableMethodSymbolTable()
, m_uStaticVarIndex(0)
, m_uFieldVarIndex(0)
, m_uLocalVarIndex(0)
, m_uArgVarIndex(0)
, m_uVirtualFunctionIndex(0)
, m_bIsBaseClass(false)
, m_xClassName()
, m_xBaseClassName()
, m_uClassSize(0)
, m_xFunctionName()
, m_uNumberOfIFConditionals(0)
, m_uNumberOfWhileConditionals(0)
, m_xFileName(xFilename)
, m_bAddressExpression(false)
, m_xAncestorList()
, m_xError()
, m_bClassPolymorphic(false)
, m_xVirtualFunctionList()
, m_uFieldAccessCount(0)
, m_uFunctionsCount(0)
{
	m_pxVMWriter = &m_xVMWriter;
}

Parser::Parser(const Parser& xOther)
: m_pxTokenizer(xOther.m_pxTokenizer)
, m_xVMWriter(*this)
, m_NeedToGetNextToken(xOther.m_NeedToGetNextToken)
, m_xVariableClassSymbolTable(xOther.m_xVariableClassSymbolTable)
, m_xVariableMethodSymbolTable(xOther.m_xVariableMethodSymbolTable)
, m_uStaticVarIndex(xOther.m_uStaticVarIndex)
, m_uFieldVarIndex(xOther.m_uFieldVarIndex)
, m_uLocalVarIndex(xOther.m_uLocalVarIndex)
, m_uArgVarIndex(xOther.m_uArgVarIndex)
, m_uVirtualFunctionIndex(xOther.m_uVirtualFunctionIndex)
, m_bIsBaseClass(xOther.m_bIsBaseClass)
, m_xClassName(xOther.m_xClassName)
, m_xBaseClassName(xOther.m_xClassName)
, m_uClassSize(xOther.m_uClassSize)
, m_xFunctionName(xOther.m_xFunctionName)
, m_uNumberOfIFConditionals(xOther.m_uNumberOfIFConditionals)
, m_uNumberOfWhileConditionals(xOther.m_uNumberOfWhileConditionals)
, m_xFileName(xOther.m_xFileName)
, m_bAddressExpression(xOther.m_bAddressExpression)
, m_xAncestorList(xOther.m_xAncestorList)
, m_xError(xOther.m_xError)
, m_bClassPolymorphic(xOther.m_bClassPolymorphic)
, m_xVirtualFunctionList(xOther.m_xVirtualFunctionList)
, m_pxVMWriter(xOther.m_pxVMWriter)
{

}

Parser::~Parser()
{

}

bool Parser::DetectPolymorphism()
{
	while (GetNextToken())
	{
		if ((m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_KEYWORD) && (m_pxTokenizer->GetKeywordType(m_pxTokenizer->GetTokenValue()) == CompilerTokenizer::KEYWORD_VIRTUAL))
		{
			// Get function name
			// Assume polymorphic
			m_bClassPolymorphic = true;

			string xReturnType;
			if (!CompileType(xReturnType))
			{
				return false;
			}

			//Expected an identifier now
			GetNextToken();
			if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_INDENTIFIER)
			{
				Function xFunction{};
				xFunction.m_xName += m_pxTokenizer->GetTokenValue();

				if (!m_xVirtualFunctionList.Add(xFunction.m_xName, xFunction))
				{
					//Expected identifier subroutine name
					Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_FUNCTION_REDEFINITION, m_pxTokenizer->GetTokenValue().c_str());
					return false;
				}
			}
			else
			{
				//Expected identifier subroutine name
				Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_IDENTIFIER, m_pxTokenizer->GetTokenValue().c_str());
				return false;
			}
		}
		else if ((m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_KEYWORD) && (m_pxTokenizer->GetKeywordType(m_pxTokenizer->GetTokenValue()) == CompilerTokenizer::KEYWORD_ASM))
		{
			while (GetNextToken())
			{
				if ((m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL) && (m_pxTokenizer->GetTokenValue() == "}"))
					break;
			}
		}
	}

	m_pxTokenizer->Reset();
	return true;
}

bool Parser::Compile()
{
	if (!DetectPolymorphism())
		return false;

	// Start of Jack file, the first token should be "class"
	if ( GetNextToken() )
	{
		if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_KEYWORD)
		{
			if (m_pxTokenizer->GetKeywordType(m_pxTokenizer->GetTokenValue()) == CompilerTokenizer::KEYWORD_CLASS)
			{
				if( !CompileClass() )
					return false;

				// Add class name and size to class symbol table ( To detect non valid static function calls )
				Class xClass{};
				xClass.m_xName = m_xClassName;
				xClass.m_uSize = m_uClassSize;

				if (!Resolver::AddClass(xClass))
				{
					Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_CLASS_REDEFINITION, m_xClassName.c_str());
					return false;
				}
				return true;
			}
		}
	}

	Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_KEYWORD, m_pxTokenizer->GetTokenValue().c_str(), NULL, CompilerTokenizer::KEYWORD_CLASS );
	return false;
}

bool Parser::CompileClass()
{
	//Next should be the class name
	GetNextToken();
	if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_INDENTIFIER)
	{
		// Save class name
		m_xClassName = m_pxTokenizer->GetTokenValue();

		// Check that the class name and file name match
		if (m_xClassName != m_xFileName.substr(0, m_xFileName.size() - 5))
		{
			//identifier expected
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_IDENTIFIER, m_xClassName.c_str(), m_xFileName.c_str());
			return false;
		}

		//next should be "{" symbol or ":" for inheritance
		GetNextToken();
		if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "{")
		{
			// This is a base class
			m_bIsBaseClass = true;

			// If we have a polymorphic class add the static vtable, and if the base class add the vtable pointer
			if (m_bClassPolymorphic)
			{
				// Write the poly code
				string xVtableAndVPointer  = "{";
				xVtableAndVPointer		  += GetPolymorphicVTableVarDecCode();
				xVtableAndVPointer        += GetPolymorphicVPointerVarDecCode();
				xVtableAndVPointer        += "}";

				CompilerTokenizer* pxFileTokenizer = m_pxTokenizer;
				CompilerTokenizer xTokenizer(xVtableAndVPointer, true);
				m_pxTokenizer = &xTokenizer;

				if (!CompileClassBody())
					return false;

				m_pxTokenizer = pxFileTokenizer;
			}

			m_NeedToGetNextToken = false;
			if (CompileClassBody())
			{
				// Add it the parsers symbol table
				s_xParsersSymbolTable.Add(m_xClassName, *this);
				return true;
			}
			return false;
		}
		else if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ":")
		{
			// derived class
			m_bIsBaseClass = false;

			//Inheritance, get the base class name
			GetNextToken();
			if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_INDENTIFIER)
			{
				//Check if we didn't compile this one already
				const Parser* pxBaseParser = s_xParsersSymbolTable.GetSymbol(m_pxTokenizer->GetTokenValue());
				if (!pxBaseParser)
				{
					// Compile base first
					// build file name
					string xBaseFileName = m_pxTokenizer->GetTokenValue();
					xBaseFileName       += ".jack";
					CompilerTokenizer xTokenizer(xBaseFileName);
					Parser            xParser(xBaseFileName, xTokenizer);

					if (!xParser.Compile())
					{
						cout << xParser.GetError();
						return false;
					}

					InheritBaseClassDetails(xParser);

					// Add it the parsers symbol table
					s_xParsersSymbolTable.Add(xParser.m_xClassName, xParser);
				}
				else
				{
					InheritBaseClassDetails(*pxBaseParser);
				}

				// If we have a polymorphic class add the static vtable
				if (m_bClassPolymorphic)
				{
					// Write the poly code
					string xVtable = "{";
					xVtable       += GetPolymorphicVTableVarDecCode();
					xVtable       += "}";

					CompilerTokenizer* pxFileTokenizer = m_pxTokenizer;
					CompilerTokenizer  xTokenizer(xVtable, true);
					m_pxTokenizer = &xTokenizer;

					if (!CompileClassBody())
						return false;

					m_pxTokenizer = pxFileTokenizer;
				}

				// Compile myself now
				if (CompileClassBody())
				{
					// Add it the parsers symbol table
					s_xParsersSymbolTable.Add(m_xClassName, *this);
					return true;
				}
				return false;
			}
			else
			{
				//Expected identifier
				Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_IDENTIFIER, m_pxTokenizer->GetTokenValue().c_str());
				return false;
			}
		}
		else
		{
			//expected "{" symbol
			Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "{ or :");
			return false;
		}
	}
	else
	{
		//identifier expected
		Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_IDENTIFIER, m_pxTokenizer->GetTokenValue().c_str());
		return false;
	}
}

string Parser::GetPolymorphicVPointerVarDecCode() const
{
	return " field int pointer@Vtable; ";
}

string Parser::GetPolymorphicVTableVarDecCode() const
{
	// write jack code to declare vtable
	string xCode = " static Array int ";
	xCode       += m_xClassName;
	xCode       += "@Vtable; ";

	return xCode;
}

string Parser::GetPolymorphicVTableAndVPointerInitCode() const
{
	// write jack code to perform vtable and vpoitner initialisation
	string xVtable = m_xClassName;
	xVtable       += "@Vtable";

	string xVtableInit = "if(";
	xVtableInit		  += xVtable;
	xVtableInit		  += "= null) { let ";
	xVtableInit		  += xVtable;
	xVtableInit		  += "= cast<Array int>(Array.new(";
	xVtableInit		  += to_string(m_xVirtualFunctionList.GetTable().size());
	xVtableInit		  += "));";

	unsigned u = 0;
	for (unordered_map<string, const Function>::const_iterator it = m_xVirtualFunctionList.GetTable().begin(); it != m_xVirtualFunctionList.GetTable().end(); ++it)
	{
		xVtableInit += "let ";
		xVtableInit += xVtable;
		xVtableInit += "[";
		xVtableInit += to_string(u++);
		xVtableInit += "] = #";
		xVtableInit += m_xClassName;
		xVtableInit += ".";
		xVtableInit += it->second.m_xName;
		xVtableInit += "();";
	}

	xVtableInit += "}";
	xVtableInit += "let pointer@Vtable = cast<int>(";
	xVtableInit += xVtable;
	xVtableInit += ");";

	return xVtableInit;
}

void Parser::InheritBaseClassDetails(const Parser& xParser)
{
	// Get the base data to expend on
	m_xVariableClassSymbolTable = xParser.m_xVariableClassSymbolTable;
	m_uStaticVarIndex			= xParser.m_uStaticVarIndex;
	m_uFieldVarIndex			= xParser.m_uFieldVarIndex;
	m_uClassSize			   += xParser.m_uClassSize;
	m_bClassPolymorphic		   |= xParser.m_bClassPolymorphic;
	m_xBaseClassName            = xParser.m_xClassName;

	// make the base ancestors mine too
	m_xAncestorList = xParser.m_xAncestorList;

	// Get the base class name
	m_xAncestorList.Add(m_pxTokenizer->GetTokenValue(), m_pxTokenizer->GetTokenValue());
}

bool Parser::CompileClassBody()
{
	// Start with Vars
	CompilerTokenizer* pxSavedTokenizer =  m_pxTokenizer;
	CompilerTokenizer  xTokenizer(*m_pxTokenizer);
	
	bool bSavedNeedToGetNextToken = m_NeedToGetNextToken;

	// Compile vars
	m_pxTokenizer = &xTokenizer;
	if (!CompileClassBodyInternal(true))
		return false;

	// Resume compile with subroutines and proper checks
	m_pxTokenizer        = pxSavedTokenizer;
	m_NeedToGetNextToken = bSavedNeedToGetNextToken;
	return CompileClassBodyInternal(false);
}

bool Parser::CompileClassBodyInternal(bool bVars)
{
	GetNextToken();
	if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "{")
	{
		//Now we need to read class var decs or subroutine decs
		while (GetNextToken())
		{
			if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_KEYWORD)
			{
				//Subroutine or var?
				switch (m_pxTokenizer->GetKeywordType(m_pxTokenizer->GetTokenValue()))
				{
					case CompilerTokenizer::KEYWORD_STATIC:
					case CompilerTokenizer::KEYWORD_FIELD:
					{
						if (!CompileVarDec(bVars))
							return false;

						break;
					}
					case CompilerTokenizer::KEYWORD_CONSTRUCTOR:
					case CompilerTokenizer::KEYWORD_FUNCTION:
					case CompilerTokenizer::KEYWORD_METHOD:
					case CompilerTokenizer::KEYWORD_VIRTUAL:
					case CompilerTokenizer::KEYWORD_DESTRUCTOR:
					{
						if (bVars)
							continue;

						// Clear method symbol table
						m_xVariableMethodSymbolTable.Clear();

						// Reset local and arg indexes
						m_uLocalVarIndex             = 0;
						m_uNumberOfIFConditionals    = 0;
						m_uNumberOfWhileConditionals = 0;

						if (!CompileSubroutine())
							return false;

						//After all ClassVarDecs and ClassSubroutines
						if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "}")
						{
							break;
						}
						else
						{
							//Expected "}" symbol
							Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "}");
							return false;
						}
					}
					default:
					{
						if (bVars)
							continue;

						//wrong keyword
						Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_KEYWORD, m_pxTokenizer->GetTokenValue().c_str(), NULL,
							CompilerTokenizer::KEYWORD_STATIC | CompilerTokenizer::KEYWORD_FIELD | CompilerTokenizer::KEYWORD_CONSTRUCTOR | CompilerTokenizer::KEYWORD_FUNCTION | CompilerTokenizer::KEYWORD_METHOD |
							CompilerTokenizer::KEYWORD_DESTRUCTOR | CompilerTokenizer::KEYWORD_VIRTUAL);
						return false;
					}
				}
			}
			else
			{
				if (bVars)
					continue;

				// Might be end of class
				return true;
			}
		}

		if (bVars)
		{
			return true; // Assume all fine, next pass for functions will detect any errors
		}
		return false;
	}
	else
	{
		//Expected "}" symbol
		Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "{");
		return false;
	}
}

bool Parser::CompileSubroutine()
{
	// Prepare to define a new function
	Function xFunction {};

	// Fill in the file name
	xFunction.m_xFile  = m_xFileName;

	xFunction.m_uLine  = m_pxTokenizer->GetLineNumber();

	// Fill in the class name
	xFunction.m_xClass = m_xClassName;

	xFunction.m_xName  = m_xClassName;

	if (m_pxTokenizer->GetKeywordType(m_pxTokenizer->GetTokenValue()) == CompilerTokenizer::KEYWORD_VIRTUAL)
	{
		xFunction.m_bVirtual = true;
		xFunction.m_uIndex   = m_uVirtualFunctionIndex++;
	}

	//save if this a constructor, method or function
     CompilerTokenizer::KEYWORD_TYPE eKeywordType = m_pxTokenizer->GetKeywordType(m_pxTokenizer->GetTokenValue());

	 xFunction.m_eType = eKeywordType;

	 //Number of subroutine args;
	 unsigned int uArgVarIndex = 0;

	 if( eKeywordType != CompilerTokenizer::KEYWORD_FUNCTION )
		 ++uArgVarIndex; // This pointer argument

	//This reads the type
	string xReturnType;
	if ( !CompileType(xReturnType))
	{
		return false;
	}

	// If we are a contructor, return type should be class type
	if (eKeywordType == CompilerTokenizer::KEYWORD_CONSTRUCTOR)
	{
		if (xReturnType != m_xClassName)
		{
			// SPECIAL CASE for Array class (accept Array int as return...)
			if (m_xClassName != "Array" || xReturnType != "Array@int")
			{
				Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_CONSTRUCTOR);
				return false;
			}
		}
	}

	if (eKeywordType == CompilerTokenizer::KEYWORD_DESTRUCTOR)
	{
		if (xReturnType != "void")
		{
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_DESTRUCTOR);
			return false;
		}
	}

	// set the return type
	xFunction.m_xReturnType = xReturnType;

	//Expected an identifier now
	GetNextToken();
	if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_INDENTIFIER)
	{
		m_xFunctionName = m_pxTokenizer->GetTokenValue();

		// Fill in the function name
		xFunction.m_xName += ".";
		xFunction.m_xName += m_xFunctionName;

		// Write the function declaration
		string xLocals = s_xLocalsMarker;
		xLocals       += to_string(m_uFunctionsCount++);

		xFunction.m_xVMMarker = xLocals;
	}
	else
	{
		//Expected identifier subroutine name
		Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_IDENTIFIER, m_pxTokenizer->GetTokenValue().c_str());
		return false;
	}

	// If we are a contructor, function name should be "new"
	if (eKeywordType == CompilerTokenizer::KEYWORD_CONSTRUCTOR)
	{
		if (m_xFunctionName != "new")
		{
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_CONSTRUCTOR);
			return false;
		}
	}
	else
	{	// not constructor, should not be new
		if (m_xFunctionName == "new")
		{
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_CONSTRUCTOR);
			return false;
		}
	}

	// destructor, should be dispose
	if (eKeywordType == CompilerTokenizer::KEYWORD_DESTRUCTOR)
	{
		if (m_xFunctionName != "dispose")
		{
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_DESTRUCTOR);
			return false;
		}
	}
	else
	{
		// not destructor, should not be dispose
		if (m_xFunctionName == "dispose")
		{
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_DESTRUCTOR);
			return false;
		}
	}

	//Expect a "(" now
	GetNextToken();
	if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "(")
	{
		Variable xVar = {};

		// we know this is now arguments
		xVar.m_eSegment = Variable::SEGMENT_ARGUMENT;

		// varnames
		bool bExpectedType       = true;
		bool bExpectedIdentifier = false;
		bool bExpectedSymbol     = true;

		while (GetNextToken())
		{
			switch (m_pxTokenizer->GetTokenType())
			{
				case CompilerTokenizer::TOKEN_SYMBOL:
				{
					if (bExpectedSymbol)
					{
						if (m_pxTokenizer->GetTokenValue() == ")")
						{
							// Add the function signature to the functions symbol table
							// Check if it is not a redefinition
							if( Resolver::GetFunctionSymbolTable().GetSymbol(xFunction.m_xName))
							{
								// Function redefinition error
								Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_FUNCTION_REDEFINITION, xFunction.m_xName.c_str());
								return false;
							}
							else
							{
								if (CompileSubroutineBody(eKeywordType, xFunction.m_xVMMarker))
								{
									xFunction.m_uNumberOfLocals = m_xVariableMethodSymbolTable.GetTable().size() - xFunction.m_xArgumentList.size();
									if( !Resolver::AddFunction(xFunction) )
										Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);

									return true;
								}
								return false;
							}
						}
						else if (m_pxTokenizer->GetTokenValue() == "," && !bExpectedType) // !bExpectedType means that we already had an identifier set
						{
							bExpectedIdentifier = false;
							bExpectedSymbol     = false;
							bExpectedType       = true;
						}
						else
						{
							// ) or, expected
							Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ", or )" );
							return false;
						}
					}
					else
					{
						Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ", or )");
						return false;
					}
					break;
				}
				default:
				{
					if (bExpectedIdentifier)
					{
						//All good
						bExpectedIdentifier = false;
						bExpectedSymbol		= true;
						bExpectedType		= false;

						// Add var name
						xVar.m_xName = m_pxTokenizer->GetTokenValue();

						// increase arg index
						xVar.m_uIndex = uArgVarIndex++;

						// Add to method symbol table
						m_xVariableMethodSymbolTable.Add(xVar.m_xName, xVar);

						break;
					}
					//is it a type
					else if (bExpectedType)
					{
						m_NeedToGetNextToken = false;

						string xVarType;
						if (CompileType(xVarType))
						{
							xVar.m_xType				 = xVarType;
							static_cast<Location&>(xVar) = static_cast<Location&>(xFunction);
							xVar.m_uLine                 = m_pxTokenizer->GetLineNumber();

							// Add object to argument list
							xFunction.m_xArgumentList.push_back(xVar);

							bExpectedIdentifier = true;
							bExpectedSymbol     = false;
							bExpectedType       = false;
						}
						else
						{
							return false;
						}
					}
					else
					{
						Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_TYPE);
						Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_IDENTIFIER);
						return false;
					}
				}
			}
		}

		if (bExpectedSymbol)
		{
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ", or )");
			return false;
		}

		if (bExpectedIdentifier)
		{
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_IDENTIFIER);
			return false;
		}

		if (bExpectedType)
		{
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_TYPE);
			return false;
		}

		Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
		return false;
	}
	else
	{
		//Expected identifier subroutine name
		Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "(" );
		return false;
	}
}

bool Parser::CompileVarDec(bool bVars)
{
	Variable::SEGMENT eSegment = Variable::SEGMENT_INVALID;
	unsigned int*     puIndex  = NULL;
	//Is it a Static, field or Var (local)
	switch (m_pxTokenizer->GetKeywordType(m_pxTokenizer->GetTokenValue()))
	{
		case CompilerTokenizer::KEYWORD_STATIC:
		{
			eSegment = Variable::SEGMENT_STATIC;
			puIndex  = &m_uStaticVarIndex;
			break;
		}

		case CompilerTokenizer::KEYWORD_FIELD:
		{
			eSegment = Variable::SEGMENT_FIELD;
			puIndex  = &m_uFieldVarIndex;
			break;
		}

		case CompilerTokenizer::KEYWORD_VAR:
		{
			eSegment = Variable::SEGMENT_LOCAL;
			puIndex  = &m_uLocalVarIndex;
			break;
		}

		default:
		{
			//Should never happen, anyway...Internal error
			Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
			return false;
		}
	}
	
	//Now we need to read class var decs or subroutine decs
	//This reads the type
	string xVarType;
	if (!CompileType( xVarType )) 
		return false;

	// varnames
	bool bExpectedIdentifier = true;
	while (GetNextToken())
	{
		switch (m_pxTokenizer->GetTokenType())
		{
			case CompilerTokenizer::TOKEN_SYMBOL:
			{
				if (!bExpectedIdentifier)
				{
					if (m_pxTokenizer->GetTokenValue() == ",")
					{
						bExpectedIdentifier = true;
					}
					else if (m_pxTokenizer->GetTokenValue() == ";")
					{
						//We finished with these vardecs return true
						return true;
					}
					else
					{
						//unexpected symbol
						Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ", or ;" );
						return false;
					}
				}
				else
				{
					//symbol expected
					Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ", or ;");
					return false;
				}
				break;
			}
			case CompilerTokenizer::TOKEN_INDENTIFIER:
			{
				if (bExpectedIdentifier)
				{
					//All good
					bExpectedIdentifier = false;

					if (!bVars)
						break;

					//Create the variable symbol entry here
					Variable xVar{};

					// Set segment
					xVar.m_eSegment = eSegment;

					// Set type
					xVar.m_xType    = xVarType;

					// index
					xVar.m_uIndex   = (*puIndex)++;

					// Add var name
					xVar.m_xName = m_pxTokenizer->GetTokenValue();

					// Add it to symbol table
					if ( xVar.m_eSegment == Variable::SEGMENT_FIELD || xVar.m_eSegment == Variable::SEGMENT_STATIC )
					{
						if (!m_xVariableClassSymbolTable.Add(xVar.m_xName, xVar))
						{
							Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_VARIABLE_REDEFINITION, m_pxTokenizer->GetTokenValue().c_str());
							return false;
						}

						// If it is a non static field, add it to the object size
						if( xVar.m_eSegment == Variable::SEGMENT_FIELD )
							++m_uClassSize;
					}
					else
					{
						m_xVariableMethodSymbolTable.Add(xVar.m_xName, xVar);
					}

					// Add it to the resolver variable list
					xVar.m_xFile     = m_xFileName;
					xVar.m_xClass    = m_xClassName;
					xVar.m_xFunction = m_xFunctionName;
					xVar.m_uLine     = m_pxTokenizer->GetLineNumber();
					
					Resolver::AddVariable(xVar);
				}
				else
				{
					Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ", or ;");
					return false;
				}
				break;
			}
			default:
			{
				//unexpected something
			    Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_IDENTIFIER, m_pxTokenizer->GetTokenValue().c_str());
				Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, ", or ;", m_pxTokenizer->GetTokenValue().c_str());
				return false;
			}
		}
	}
	return false;
}

bool Parser::CompileType( string& xVarType )
{
	GetNextToken();

	if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_KEYWORD)
	{
		//Subroutine or var?
		switch (m_pxTokenizer->GetKeywordType(m_pxTokenizer->GetTokenValue()))
		{
			case CompilerTokenizer::KEYWORD_INT:
			case CompilerTokenizer::KEYWORD_CHAR:
			case CompilerTokenizer::KEYWORD_BOOLEAN:
			case CompilerTokenizer::KEYWORD_VOID:
			{
				xVarType += m_pxTokenizer->GetTokenValue();
				return true;
			}
			default:
			{
				//wrong keyword
				Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_KEYWORD, m_pxTokenizer->GetTokenValue().c_str(), NULL, CompilerTokenizer::KEYWORD_INT | CompilerTokenizer::KEYWORD_CHAR | CompilerTokenizer::KEYWORD_BOOLEAN | CompilerTokenizer::KEYWORD_VOID );
				return false;
			}
		}
	}
	else if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_INDENTIFIER)
	{
		// Special case for arrays, elements type should follow
		if( m_pxTokenizer->GetTokenValue() == "Array" )
		{
			xVarType += m_pxTokenizer->GetTokenValue();
			xVarType += s_cTypeOfArrayPrefix;

			return CompileType( xVarType ); // in the end, we might have the type as array of array of array of array of int :)
		}

		xVarType += m_pxTokenizer->GetTokenValue();
		return true;//CLASSNAME
	}
	else
	{
		Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_KEYWORD, m_pxTokenizer->GetTokenValue().c_str(), NULL, CompilerTokenizer::KEYWORD_INT | CompilerTokenizer::KEYWORD_CHAR | CompilerTokenizer::KEYWORD_BOOLEAN );
		Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_IDENTIFIER, m_pxTokenizer->GetTokenValue().c_str());
		return false;
	}
}

bool Parser::CompileSubroutineBody( CompilerTokenizer::KEYWORD_TYPE eFunctionType, const string& xNumLocalsMarker )
{
	string xBaseConstructorCallCode;
	//We must find { or :
	if ( !m_bIsBaseClass && eFunctionType == CompilerTokenizer::KEYWORD_CONSTRUCTOR )
	{
		GetNextToken();
		if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ":")
		{
			GetNextToken();
			if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_INDENTIFIER)
			{
				string xIdentifier = m_pxTokenizer->GetTokenValue();

				// Check that the identifier is the same as the base class name
				if (xIdentifier == m_xBaseClassName)
				{
					GetNextToken();
					if ((m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL) && (m_pxTokenizer->GetTokenValue() == "."))
					{
						// Use a new VMWriter here
						VMWriter* pxOriginalWriter = m_pxVMWriter;
						VMWriter xWriter(*this);
						m_pxVMWriter = &xWriter;

						Object xObject{};
						if (!CompileSubroutineCall(xIdentifier, xObject, true))
							return false;

						xBaseConstructorCallCode = m_pxVMWriter->GetVMCode().str();

						m_pxVMWriter = pxOriginalWriter;
						m_pxVMWriter->SetSkippedAllocs(m_pxVMWriter->GetSkippedAllocs() + xWriter.GetSkippedAllocs());
					}
					else
					{
						Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ".");
						return false;
					}
				}
				else
				{
					Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_IDENTIFIER, m_pxTokenizer->GetTokenValue().c_str(), m_xBaseClassName.c_str());
					return false;
				}
			}
			else
			{
				Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_IDENTIFIER, m_pxTokenizer->GetTokenValue().c_str());
				return false;
			}
		}
		else
		{
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ":");
			return false;
		}
	}

	GetNextToken();
	if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "{")
	{
		m_pxVMWriter->WriteFunctionDeclaration(m_xClassName + "." + m_xFunctionName, xNumLocalsMarker);

		if (!m_pxVMWriter->WriteFunctionType(eFunctionType, m_uClassSize)) // Class size ignored if this is not a constructor
		{
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
			return false;
		}

		// Base call should be here
		if (!xBaseConstructorCallCode.empty())
		{
			m_pxVMWriter->GetVMCode() << xBaseConstructorCallCode;
		}

		if (eFunctionType == CompilerTokenizer::KEYWORD_CONSTRUCTOR && m_bClassPolymorphic)
		{
			CompilerTokenizer xTokenizer(GetPolymorphicVTableAndVPointerInitCode(), true);
			CompilerTokenizer* pxFileTokenizer = m_pxTokenizer;
			m_pxTokenizer = &xTokenizer;

			GetNextToken();
			if (!CompileStatements())
				return false;

			m_pxTokenizer = pxFileTokenizer;
		}

		//Now we need to read vars or statements
		while (GetNextToken())
		{
			//var?
			if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_KEYWORD)
			{
				if (m_pxTokenizer->GetKeywordType(m_pxTokenizer->GetTokenValue()) == CompilerTokenizer::KEYWORD_VAR)
				{
					if (!CompileVarDec())
						return false;
				}
				else
				{
					if (!CompileStatements())
						return false;
					
					m_NeedToGetNextToken = false;
				}
			}
			else if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "}")
			{
				if (eFunctionType == CompilerTokenizer::KEYWORD_DESTRUCTOR)
				{
					// Add code to call base class destructor
					// Get base class destructor (should be called "dispose")
					// Base class call, check the subroutine type (constructor, method  or function )
					if (!m_bIsBaseClass)
					{
						string xFunctionName = m_xBaseClassName;
						xFunctionName += ".";
						xFunctionName += "dispose";

						const Function* xFunction = Resolver::GetFunctionSymbolTable().GetSymbol(xFunctionName);
						if (xFunction)
						{
							// This must be the base destructor
							// write jack code to call the destructor
							string xDestructorCall = "do ";
							xDestructorCall += xFunctionName;
							xDestructorCall += "();";

							CompilerTokenizer xTokenizer(xDestructorCall, true);

							CompilerTokenizer* pxFileTokenizer = m_pxTokenizer;
							m_pxTokenizer = &xTokenizer;

							if (!CompileStatements())
								return false;

							m_pxTokenizer = pxFileTokenizer;
						}
						else
						{
							Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ".");
							return false;
						}
					}
				}

				return true;
			}
			else
			{
				Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "}");
				return false;
			}
		}
		return false;
	}
	else
	{
		Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "{");
		return false;
	}
}

bool Parser::CompileStatements()
{
	while (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_KEYWORD)
	{
		switch (m_pxTokenizer->GetKeywordType(m_pxTokenizer->GetTokenValue()))
		{
			case CompilerTokenizer::KEYWORD_LET:
			{
				if (!CompileLet()) return false;
				break;
			}
			case CompilerTokenizer::KEYWORD_IF:
			{
				if (!CompileIf()) return false;
				break;
			}
			case CompilerTokenizer::KEYWORD_WHILE:
			{
				if (!CompileWhile()) return false;
				break;
			}
			case CompilerTokenizer::KEYWORD_RETURN:
			{
				if (!CompileReturn()) return false;
				break;
			}
			case CompilerTokenizer::KEYWORD_DO:
			{
				if (!CompileDo()) return false;
				break;
			}
			case CompilerTokenizer::KEYWORD_ASM:
			{
				if (!CompileASM()) return false;
				break;
			}
		}

		GetNextToken();
	}
	return true;
}

bool Parser::CompileDo()
{
	Expression xExpression{};
	xExpression.m_xClass    = m_xClassName;
	xExpression.m_xFile     = m_xFileName;
	xExpression.m_xFunction = m_xFunctionName;
	xExpression.m_uLine     = m_pxTokenizer->GetLineNumber();

	Object xObject{};

	if (CompileTerm(xExpression, xObject, false))
	{
		if (    ( !xObject.m_bFieldAccess && xObject.m_bFunctionReturnType && ( xObject.m_uDimension == 0 ))
			 || ( xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_bFunctionReturnType && xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_uDimension == 0 )    )
		{
			GetNextToken();
			if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ";")
			{
				//Pop the unused return value
				m_pxVMWriter->WriteRemoveZeroReturn();
				return true;
			}
			else
			{
				Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ";");
				return false;
			}
		}
		else
		{
			// Expected a function call
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_FUNCTION_CALL);
			return false;
		}
	}

	return false;
}

bool Parser::CompileReturn()
{
	Expression xReturn {};

	xReturn.m_xClass    = m_xClassName;
	xReturn.m_xFile     = m_xFileName;
	xReturn.m_xFunction = m_xFunctionName;
	xReturn.m_uLine     = m_pxTokenizer->GetLineNumber();

	GetNextToken();
	if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ";")
	{
		// push a zero as return.
		m_pxVMWriter->WriteZeroReturn();
		m_pxVMWriter->WriteReturn();

		// Void return type
		Object xObject {};
		xObject.m_xType = CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_VOID);
		xReturn.m_xObjectList.push_back( xObject );

		xReturn.m_xTemplate = Expression::s_xTemplateVar;

		Resolver::AddReturn(xReturn);

		return true;
	}
	else 
	{
		m_NeedToGetNextToken = false;

		if (CompileExpression( xReturn ))
		{
			if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ";")
			{
				m_pxVMWriter->WriteReturn();

				Resolver::AddReturn(xReturn);

				return true;
			}
			else
			{
				Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ";");
				return false;
			}
		}
		else
		{
			return false;
		}
	}
}

bool Parser::CompileWhile()
{
	//This reads the type
	GetNextToken();
	if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "(")
	{
		unsigned int uNumberOfWhileConditionals = m_uNumberOfWhileConditionals++;

		m_pxVMWriter->WriteWhileLabel(uNumberOfWhileConditionals);

		Expression xExpression {};
		xExpression.m_xClass        = m_xClassName;
		xExpression.m_xFile         = m_xFileName;
		xExpression.m_xFunction     = m_xFunctionName;
		xExpression.m_uLine         = m_pxTokenizer->GetLineNumber();
		if (CompileExpression(xExpression))
		{
			Resolver::AddCondition( xExpression );

			if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ")")
			{
				m_pxVMWriter->WriteWhileGotoTrue(uNumberOfWhileConditionals);

				m_pxVMWriter->WriteWhileGotoFalse(uNumberOfWhileConditionals);

				GetNextToken();
				if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "{")
				{
					m_pxVMWriter->WriteWhileTrueLabel(uNumberOfWhileConditionals);
					GetNextToken();
					if (CompileStatements())
					{
						if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "}")
						{
							m_pxVMWriter->WriteWhile(uNumberOfWhileConditionals);
							m_pxVMWriter->WriteWhileFalseLabel(uNumberOfWhileConditionals);
							return true;
						}
						else
						{
							Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "}" );
							return false;
						}
					}
					else
					{
						return false;
					}
				}
				else
				{
					Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "{");
					return false;
				}
			}
			else
			{
				Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ")" );
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "(" );
		return false;
	}
}

bool Parser::CompileLet()
{
	// Assignment
	Assignment xAssignment{};

	xAssignment.m_xFile		= m_xFileName;
	xAssignment.m_xClass	= m_xClassName;
	xAssignment.m_xFunction = m_xFunctionName;
	xAssignment.m_uLine		= m_pxTokenizer->GetLineNumber();

	Object xObject{};

	// Get debug info from the assignment object
	static_cast<Location&>(xAssignment.m_xLeftExpression) = static_cast<Location&>(xAssignment);

	if (CompileTerm(xAssignment.m_xLeftExpression, xObject, true))
	{
		GetNextToken();
		if (m_pxTokenizer->GetTokenValue() == "=")
		{
			if (CompileAssignment(xAssignment))
			{
				Resolver::AddAssignment(xAssignment);
				return true;
			}

			return false;
		}
		else
		{
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "=");
			return false;
		}
	}

	return false;
}

bool Parser::CompileAssignment( Assignment& xAssignment )
{
	// Get debug info from the assignment object
	static_cast<Location&>(xAssignment.m_xRightExpression) = static_cast<Location&>(xAssignment);

	if (CompileExpression(xAssignment.m_xRightExpression))
	{
		if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ";")
		{
			if (xAssignment.m_xLeftExpression.m_xObjectList.size() != 1)
			{
				// Wrong left expression
				Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_LEFT_VALUE);
				return false;
			}

			Object& xLeftObject = xAssignment.m_xLeftExpression.m_xObjectList[0];
			if (xLeftObject.m_bFieldAccess )
			{
				string xField = s_xFieldMarker;
				xField       += to_string(m_uFieldAccessCount++);
				xLeftObject.m_xMemberList[xLeftObject.m_xMemberList.size() - 1].m_xVMMarker = xField;

				m_xVMWriter.WriteString(xField);

				return true;
			}
			else
			{
				if (!m_pxVMWriter->WriteAssignment(xLeftObject.m_xName, (xLeftObject.m_uDimension > 0)))
				{
					Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
					return false;
				}
			}
				
			return true;
		}
		else
		{
			//expected;
			Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ";" );
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool Parser::CompileIf()
{
	//This reads the type
	GetNextToken();

	if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "(")
	{
		Expression xExpression {};
		xExpression.m_xClass        = m_xClassName;
		xExpression.m_xFile         = m_xFileName;
		xExpression.m_xFunction     = m_xFunctionName;
		xExpression.m_uLine         = m_pxTokenizer->GetLineNumber();
		if (CompileExpression(xExpression))
		{
			Resolver::AddCondition(xExpression);

			if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ")")
			{
				unsigned int uNumberOfIFConditionals = m_uNumberOfIFConditionals++;

				m_pxVMWriter->WriteIfGotoTrue(uNumberOfIFConditionals);

				m_pxVMWriter->WriteGotoFalse(uNumberOfIFConditionals);

				GetNextToken();
				if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "{")
				{
					m_pxVMWriter->WriteTrueLabel(uNumberOfIFConditionals);

					GetNextToken();
					if (CompileStatements())
					{
						if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "}")
						{
							//else might be expected
							GetNextToken();
							if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_KEYWORD && m_pxTokenizer->GetKeywordType(m_pxTokenizer->GetTokenValue()) == CompilerTokenizer::KEYWORD_ELSE)
							{
								m_pxVMWriter->WriteGotoEnd(uNumberOfIFConditionals);
								m_pxVMWriter->WriteFalseLabel(uNumberOfIFConditionals);

								GetNextToken();
								if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "{")
								{
									GetNextToken();
									if (CompileStatements())
									{
										if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "}")
										{
											m_pxVMWriter->WriteEndLabel(uNumberOfIFConditionals);
											return true;
										}
										else
										{
											Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "}" );
											return false;
										}
									}
									else
									{
										return false;
									}
								}
								else
								{
									Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "{" );
									return false;
								}
							}
							else
							{
								m_pxVMWriter->WriteFalseLabel(uNumberOfIFConditionals);

								m_NeedToGetNextToken = false;
								return true;
							}
						}
						else
						{
							Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "}" );
							return false;
						}
					}
					else
					{
						return false;
					}
				}
				else
				{
					Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "{" );
					return false;
				}
			}
			else
			{
				//expected;
				Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ")" );
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "(" );
		return false;
	}
}

bool Parser::CompileASM()
{
	m_xVMWriter.WriteString(m_pxTokenizer->GetTokenValue());

	//write everithing until hittin }
	m_pxTokenizer->GetNextToken();
	if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "{")
	{
		m_xVMWriter.WriteString(m_pxTokenizer->GetTokenValue());
		string xAsm;
		while (GetNextToken())
		{
			bool bPreviousTokenIdentifier = m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_INDENTIFIER;

			if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ";")
			{
				xAsm += m_pxTokenizer->GetTokenValue();
				m_xVMWriter.WriteString(xAsm);
				xAsm = "";
				continue;
			}
			else if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "}")
			{
				m_xVMWriter.WriteString(m_pxTokenizer->GetTokenValue());
				return true;
			}
			xAsm += m_pxTokenizer->GetTokenValue();

			if(bPreviousTokenIdentifier)
				xAsm += " ";
		}
	}
	else
	{
		Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "{");
		return false;
	}

	Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "}");
	return false;
}

bool Parser::CompileExpression( Expression& xExpression )
{
	Object xLeftObject{};
	if (CompileTerm( xExpression, xLeftObject, false))
	{
		// After the term, check 
		__pragma(warning(suppress:4127))
		while( true )
		{
			GetNextToken();

			//is it an operator
			if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL &&
				(m_pxTokenizer->GetTokenValue() == "+" || m_pxTokenizer->GetTokenValue() == "-" || m_pxTokenizer->GetTokenValue() == "*" || m_pxTokenizer->GetTokenValue() == "/" || m_pxTokenizer->GetTokenValue() == "&" || m_pxTokenizer->GetTokenValue() == "|" || m_pxTokenizer->GetTokenValue() == "<" || m_pxTokenizer->GetTokenValue() == ">" || m_pxTokenizer->GetTokenValue() == "="))
			{
				xExpression.m_xTemplate += m_pxTokenizer->GetTokenValue();

				string xOperator = m_pxTokenizer->GetTokenValue(); //save operator as the value will change

				if (m_pxTokenizer->GetTokenValue() == "<" || m_pxTokenizer->GetTokenValue() == ">")
				{
					//Get next token and check if it is the equal sign
					GetNextToken();

					if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "=")
						xOperator += m_pxTokenizer->GetTokenValue();
					else
						m_NeedToGetNextToken = false;
				}
				
				Object xRightObject{};
				if ( CompileTerm( xExpression, xRightObject, false) )
				{
					// Apply the operator here
					if (!m_pxVMWriter->WriteOperator(xOperator))
					{
						Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				return true;
			}
		}
	}
	else
	{
		return false;
	}
}

bool Parser::CompileTerm( Expression& xExpression, Object& xObject, bool bAssignment )
{
	//This reads the type
	GetNextToken();

	switch (m_pxTokenizer->GetTokenType())
	{
		case CompilerTokenizer::TOKEN_INT_CONSTANT:
		{
			if (m_bAddressExpression)
			{
				Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_ADDRESS, m_pxTokenizer->GetTokenValue().c_str());
				return false;
			}
				
			if (!m_pxVMWriter->WriteTerm(VMWriter::TERM_INT_CONSTANT, &m_pxTokenizer->GetTokenValue()))
			{
				Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
				return false;
			}
				
			xObject.m_xType = "int_constant";

			xExpression.m_xObjectList.push_back( xObject );

			xExpression.m_xTemplate += Expression::s_xTemplateVar;

			return true;
		}
		case CompilerTokenizer::TOKEN_STRING_CONSTANT:
		{
			if (m_bAddressExpression)
			{
				Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_ADDRESS, m_pxTokenizer->GetTokenValue().c_str());
				return false;
			}

			if (!m_pxVMWriter->WriteTerm(VMWriter::TERM_STRING_CONSTANT, &m_pxTokenizer->GetTokenValue()))
			{
				Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
				return false;
			}

			xObject.m_xType = "string_constant";

			xExpression.m_xObjectList.push_back( xObject );

			xExpression.m_xTemplate += Expression::s_xTemplateVar;

			return true;
		}
		case CompilerTokenizer::TOKEN_KEYWORD:
		{
			switch (m_pxTokenizer->GetKeywordType(m_pxTokenizer->GetTokenValue()))
			{
				case CompilerTokenizer::KEYWORD_TRUE:
				{
					if (m_bAddressExpression)
					{
						Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_ADDRESS, m_pxTokenizer->GetTokenValue().c_str());
						return false;
					}

					xExpression.m_xTemplate += Expression::s_xTemplateVar;

					if (!m_pxVMWriter->WriteTerm(VMWriter::TERM_TRUE))
					{
						Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
						return false;
					}

					xObject.m_xType = "true";

					xExpression.m_xObjectList.push_back( xObject );
					return true;
				}
				case CompilerTokenizer::KEYWORD_FALSE:
				{
					if (m_bAddressExpression)
					{
						Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_ADDRESS, m_pxTokenizer->GetTokenValue().c_str());
						return false;
					}

					xExpression.m_xTemplate += Expression::s_xTemplateVar;

					if (!m_pxVMWriter->WriteTerm(VMWriter::TERM_FALSE))
					{
						Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
						return false;
					}

					xObject.m_xType = "false";

					xExpression.m_xObjectList.push_back( xObject );

					return true;
				}
				case CompilerTokenizer::KEYWORD_NULL:
				{
					if (m_bAddressExpression)
					{
						Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_ADDRESS, m_pxTokenizer->GetTokenValue().c_str());
						return false;
					}

					xExpression.m_xTemplate += Expression::s_xTemplateVar;

					if (!m_pxVMWriter->WriteTerm(VMWriter::TERM_NULL))
					{
						Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
						return false;
					}

					xObject.m_xType = "null"; // null can be any object type

					xExpression.m_xObjectList.push_back( xObject );

					return true;
				}
				case CompilerTokenizer::KEYWORD_THIS:
				{
					xExpression.m_xTemplate += Expression::s_xTemplateVar;

					if (!m_pxVMWriter->WriteTerm(VMWriter::TERM_THIS, NULL, m_bAddressExpression))
					{
						Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
						return false;
					} 

					xObject.m_xType = m_xClassName;

					xExpression.m_xObjectList.push_back( xObject );

					return true;
				}
				case CompilerTokenizer::KEYWORD_ADDRESS:
				{
					xExpression.m_xTemplate += Expression::s_xTemplatePointer;
					m_bAddressExpression     = true;
					if (CompileTerm(xExpression, xObject, bAssignment))
					{
						m_bAddressExpression = false;

						xObject.m_xType = "int";

						xExpression.m_xObjectList.push_back(xObject);
						return true;
					}

					return false;
				}
				case CompilerTokenizer::KEYWORD_CAST:
				{
					GetNextToken();
					if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "<")
					{
						// Get cast to type
						string xCastedType;
						if (!CompileType(xCastedType))
							return false;

						GetNextToken();
						if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ">")
						{
							GetNextToken();
							if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "(")
							{
								size_t uOriginalObjectListSize = xExpression.m_xObjectList.size();
								if (CompileExpression(xExpression))
								{
									if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ")")
									{
										// Set the casted to type
										for (unsigned int u = uOriginalObjectListSize; u < xExpression.m_xObjectList.size(); ++u )
										{
											xExpression.m_xObjectList[u].m_xCastType = xCastedType;
										}

										return true;
									}
									else
									{
										Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ")" );
										return false;
									}
								}
								else
								{
									return false;
								}
							}
							else
							{
								Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "(" );
								return false;
							}
						}
						else
						{
							Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ">" );
							return false;
						}
					}
					else
					{
						Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "<" );
						return false;
					}
				}
				default:
				{
					Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_UNRESOLVED_IDENTIFIER, m_pxTokenizer->GetTokenValue().c_str(), NULL,
						CompilerTokenizer::KEYWORD_TRUE|CompilerTokenizer::KEYWORD_FALSE|CompilerTokenizer::KEYWORD_NULL|CompilerTokenizer::KEYWORD_THIS);
					return false;
				}
			}
		}
		case CompilerTokenizer::TOKEN_INDENTIFIER:
		{
			if(!xObject.m_bFieldAccess) // If accessing a field, we already have it in the template
				xExpression.m_xTemplate += Expression::s_xTemplateVar;

			string xIdentifierName   = m_pxTokenizer->GetTokenValue();

			if (!xObject.m_bFieldAccess)
				xObject.m_xName = xIdentifierName;
			else
				xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_xName = xIdentifierName;

			// Array
			GetNextToken();
			if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "[")
			{
				// Compile the array
				return CompileArray(xExpression, xObject, xIdentifierName, bAssignment);
			}
			else if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && (m_pxTokenizer->GetTokenValue() == "("))
			{
				// Function call
				if (!m_bAddressExpression)
				{
					m_NeedToGetNextToken = false;
					if (CompileSubroutineCall(xIdentifierName, xObject))
					{
						if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ")")
						{
							GetNextToken();
							m_NeedToGetNextToken = false;

							if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL &&  m_pxTokenizer->GetTokenValue() == "[")
							{
								// Compile the array
								return CompileArray(xExpression, xObject, xIdentifierName, bAssignment);
							}
							else if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL &&  m_pxTokenizer->GetTokenValue() == ".")
							{
								// Field access
								// Accessing an object field or static
								xObject.m_bFieldAccess = true;

								// Add a new member object
								Object xMember{};
								xObject.m_xMemberList.push_back(xMember);

								// Call compile term again
								return CompileTerm(xExpression, xObject, bAssignment);
							}
							else
							{
								// All good
								// return type will be deduced from the function call signature
								if (!xObject.m_bFieldAccess)
								{
									xObject.m_bFunctionReturnType = true;
									xObject.m_uFunctionCallIndex = Resolver::GetNumberOfFunctionCalls() - 1;
								}
								else
								{
									xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_xName       = xIdentifierName;
									xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_bFunctionReturnType = true;
									xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_uFunctionCallIndex  = Resolver::GetNumberOfFunctionCalls() - 1;
								}

								xExpression.m_xObjectList.push_back(xObject);

								return true;
							}
						}
						else
						{
							Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ")");
							return false;
						}
					}
					else
					{
						return false;
					}
				}
				else
				{
					// Assumed Function pointer (assumed static access)
					GetNextToken();
					if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ")")
					{
						string xClassName = xObject.m_bFieldAccess ? xObject.m_xType : m_xClassName;

						// All good
						string xFunction = m_xClassName;
						xFunction       += ".";
						xFunction       += xIdentifierName;
						m_pxVMWriter->WriteTerm(VMWriter::TERM_FUNCTION_ADDRESS, &xFunction, true);

						return true;
					}
					else
					{
						Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_ADDRESS, m_pxTokenizer->GetTokenValue().c_str());
						return false;
					}
				}
			}
			else if ( m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && (m_pxTokenizer->GetTokenValue() == ".") )
			{
				if (xObject.m_bFieldAccess && !xObject.m_bFunctionReturnType)
				{
					// Write string
					// Mark that is here where we need to replace this object in this expression with the correct value
					string xField = s_xFieldMarker;
					xField += to_string(m_uFieldAccessCount++);

					m_pxVMWriter->WriteString(xField);

					// Field access, Array object type not known
					// member object was pushed back in the calling recusrsive function 
					xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_xName = xIdentifierName;
					xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_xVMMarker = xField;
					xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_bTakingAddress = m_bAddressExpression;
				}
				else if(!xObject.m_bFieldAccess)
				{
					const Variable* pxVar = GetVariable(xIdentifierName);
					if (pxVar)
					{
						if (!m_pxVMWriter->WriteTerm(VMWriter::TERM_INDENTIFIER, &xIdentifierName, m_bAddressExpression))
						{
							Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
							return false;
						}

						xObject.m_xType = pxVar->m_xType;
					}
					else
					{
						// Might be static access or inheritance access or error
						if (!m_xAncestorList.GetSymbol(xObject.m_xName))
						{
							//assume static
							xObject.m_xType = xIdentifierName;
							xObject.m_bStatic = true;
						}
					}
				}

				// Accessing an object field or static
				xObject.m_bFieldAccess = true;

				// Add a new member object
				Object xMember{};
				xObject.m_xMemberList.push_back(xMember);

				// Call compile term again
				return CompileTerm(xExpression, xObject, bAssignment);
			}
			else
			{
				if (!xObject.m_bFieldAccess)
				{
					const Variable* pxVar = GetVariable(xIdentifierName);
					if (pxVar)
					{
						if (!bAssignment)
						{
							if (!m_pxVMWriter->WriteTerm(VMWriter::TERM_INDENTIFIER, &xIdentifierName, m_bAddressExpression))
							{
								Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
								return false;
							}
						}

						xObject.m_bFunctionReturnType = false;
						xObject.m_xType = pxVar->m_xType;

						xExpression.m_xObjectList.push_back(xObject);

						m_NeedToGetNextToken = false;

						return true;
					}
					else
					{
						Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_UNRESOLVED_IDENTIFIER, xIdentifierName.c_str());
						return false;
					}
				}
				else
				{
					if (!bAssignment)
					{
						// Write string
						// Mark that is here where we need to replace this object in this expression with the correct value
						string xField = s_xFieldMarker;
						xField += to_string(m_uFieldAccessCount++);

						m_pxVMWriter->WriteString(xField);

						// Field access, Array object type not known
						// member object was pushed back in the calling recusrsive function 
						xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_xName          = xIdentifierName;
						xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_xVMMarker      = xField;
						xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_bTakingAddress = m_bAddressExpression;
					}

					xExpression.m_xObjectList.push_back(xObject);

					m_NeedToGetNextToken = false;

					return true;
				}
			}
		}
		case CompilerTokenizer::TOKEN_SYMBOL:
		{
			if (m_bAddressExpression)
			{
				Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_ADDRESS, m_pxTokenizer->GetTokenValue().c_str());
				return false;
			}

			xExpression.m_xTemplate += m_pxTokenizer->GetTokenValue();

			if (m_pxTokenizer->GetTokenValue() == "(")
			{
				if (CompileExpression(xExpression))
				{
					if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ")")
					{
						xExpression.m_xTemplate += m_pxTokenizer->GetTokenValue();

						return true;
					}
					else
					{
						Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ")");
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			else if (m_pxTokenizer->GetTokenValue() == "-" || m_pxTokenizer->GetTokenValue() == "~")
			{
				string xUnaryOPValue = m_pxTokenizer->GetTokenValue();
				if (CompileTerm(xExpression, xObject, bAssignment))
				{
					// Apply the operator here
					if (xUnaryOPValue == "-") //Unary minus
					{
						if (!m_pxVMWriter->WriteOperator("U-"))
						{
							Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
							return false;
						}
					}
					else
					{
						if (!m_pxVMWriter->WriteOperator(xUnaryOPValue))
						{
							Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
							return false;
						}
					}

					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "( or - or ~");
				return false;
			}
		}
		default: //shouldn't happen
		{
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
			return false;
		}
	}
}

bool Parser::CompileArray(Expression& xExpression, Object& xObject, string& xIdentifierName, bool bAssignment)
{
	if (!xObject.m_bFieldAccess)
	{
		const Variable* pxVar = GetVariable(xIdentifierName);
		if (pxVar)
		{
			xObject.m_xType = pxVar->m_xType;
			++xObject.m_uDimension;
		}
		else
		{
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_UNRESOLVED_IDENTIFIER, xIdentifierName.c_str());
			return false;
		}

		if (!m_pxVMWriter->WriteTerm(VMWriter::TERM_INDENTIFIER, &xIdentifierName, m_bAddressExpression))
		{
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
			return false;
		}
	}
	else
	{
		// Write string
		// Mark that is here where we need to replace this object in this expression with the correct value
		string xField = s_xFieldMarker;
		xField       += to_string(m_uFieldAccessCount++);

		m_pxVMWriter->WriteString(xField);

		// Field access, Array object type not known
		// member object was pushed back in the calling recusrsive function 
		xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_xName = xIdentifierName;
		xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_xVMMarker  = xField;
		xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_xVMMarker  = m_bAddressExpression;
		++xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_uDimension;
	}

	m_NeedToGetNextToken = false;

	// Might be a multidimensional array
	__pragma(warning(suppress:4127))
	while (true)
	{
		GetNextToken();
		if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "[")
		{
			Expression xArrayIndexExpression{};
			if (CompileExpression(xArrayIndexExpression))
			{
				m_xVMWriter.WriteString("memory");
				xArrayIndexExpression.m_xClass = m_xClassName;
				xArrayIndexExpression.m_xFile = m_xFileName;
				xArrayIndexExpression.m_xFunction = m_xFunctionName;
				xArrayIndexExpression.m_uLine = m_pxTokenizer->GetLineNumber();

				Resolver::AddArrayIndex(xArrayIndexExpression);

				if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "]")
				{
					// Look ahead, if we have a field access or another array write the array, else judge by bAssignement
					GetNextToken();
					m_NeedToGetNextToken = false;

					if(		m_pxTokenizer->GetTokenType()  == CompilerTokenizer::TOKEN_SYMBOL 
						 && ( m_pxTokenizer->GetTokenValue() == "." || m_pxTokenizer->GetTokenValue() == "[" )    )
						m_pxVMWriter->WriteArray(true);
					else
						m_pxVMWriter->WriteArray(!bAssignment);

					if (!xObject.m_bFieldAccess)
					{
						// Advance on type after the .
						size_t iDotfound = xObject.m_xType.find(s_cTypeOfArrayPrefix);
						if (iDotfound != std::string::npos)
						{
							++xObject.m_uDimension;
							xObject.m_xType = xObject.m_xType.substr(iDotfound + 1, string::npos);
						}
						else
						{
							//We shouldn't dereference this array more
							Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_ARRAY_DIMENSION, xIdentifierName.c_str());
							return false;
						}
					}
					else
					{
						++xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_uDimension;
					}
				}
				else
				{
					Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "]");
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else if (m_pxTokenizer->GetTokenValue() == ".")
		{
			// Accessing an object field or static
			xObject.m_bFieldAccess = true;

			// Add a new member object
			Object xMember{};
			xObject.m_xMemberList.push_back(xMember);

			// Call compile term again
			return CompileTerm(xExpression, xObject, bAssignment);
		}
		else
		{
			m_NeedToGetNextToken = false;

			xExpression.m_xObjectList.push_back(xObject);

			return true;
		}
	}
}

bool Parser::CompileExpressionList( unsigned int& uNumberOfArgs, FunctionCall& xFunctionCall)
{
	GetNextToken();
	if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ")") // subroutine
	{
		return true;
	}
	else
	{
		m_NeedToGetNextToken = false;

		__pragma(warning(suppress:4127))
		while (true)
		{
			if ((uNumberOfArgs == 0 ) 
				|| ( xFunctionCall.m_bStaticCall == false && uNumberOfArgs == 1 )
				|| (xFunctionCall.m_bStaticCall == true && uNumberOfArgs == 1 && xFunctionCall.m_xCalledFunction == "new" )
				|| m_pxTokenizer->GetTokenValue() == ",")
			{
				Expression xExpression{};
				xExpression.m_xClass = m_xClassName;
				xExpression.m_xFile = m_xFileName;
				xExpression.m_xFunction = m_xFunctionName;
				xExpression.m_uLine = m_pxTokenizer->GetLineNumber();
				if (CompileExpression(xExpression))
				{
					++uNumberOfArgs;
					xFunctionCall.m_xArgumentList.push_back(xExpression);
				}
				else
				{
					return false;
				}
			}
			else
			{
				return true;
			}
		}
	}
}

bool Parser::CompileSubArgs( unsigned int& uNumberOfArgs, FunctionCall& xFunctionCall)
{
	GetNextToken();
	if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == "(") // subroutine
	{
		if (CompileExpressionList(uNumberOfArgs, xFunctionCall))
		{
			if (m_pxTokenizer->GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && m_pxTokenizer->GetTokenValue() == ")") // subroutine
			{
				return true;
			}
			else
			{
				Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), ")" );
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		Error::BuildErrorString( m_xError, *this, *m_pxTokenizer, Error::ERROR_EXPECTED_SYMBOL, m_pxTokenizer->GetTokenValue().c_str(), "(" );
		return false;
	}
}

bool Parser::CompileSubroutineCall( const string& xIndentifier, Object xObject, bool bBaseConstructor /*=false*/ )
{
	FunctionCall xFunctionCall {};

	xFunctionCall.m_xFile     = m_xFileName;
	xFunctionCall.m_xClass    = m_xClassName;
	xFunctionCall.m_xFunction = m_xFunctionName;
	xFunctionCall.m_uLine     = m_pxTokenizer->GetLineNumber();

	if (!xObject.m_bFieldAccess) // subroutine
	{
		unsigned int uNumberOfArgs = 0;

		// Push this pointer argument
		if (!m_pxVMWriter->WriteTerm(VMWriter::TERM_THIS))
		{
			Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
			return false;
		}

		++uNumberOfArgs;

		// Non static call
		xFunctionCall.m_bStaticCall     = false;

		xFunctionCall.m_xVar.m_xType    = m_xClassName;

		xFunctionCall.m_xCalledFunction = xIndentifier;

		if (CompileSubArgs(uNumberOfArgs, xFunctionCall))
		{
			// Check if this is a virtual function
			const Function* pxFunction = m_xVirtualFunctionList.GetSymbol(xIndentifier);
			if(pxFunction)
			   m_pxVMWriter->WriteSubroutineCall( m_xClassName + "." + xIndentifier, uNumberOfArgs, true, pxFunction->m_uIndex );
			else
			   m_pxVMWriter->WriteSubroutineCall( m_xClassName + "." + xIndentifier, uNumberOfArgs );

			Resolver::AddFunctionCall( xFunctionCall );

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		string xSubroutineName = xIndentifier;
		xFunctionCall.m_xCalledFunction = xSubroutineName;

		if (xObject.m_xMemberList.size() > 1)
		{
			xFunctionCall.m_bStaticCall = false;  // IMPORTANT Not allowed static access from field or from static field (Need to be checked in the resolver)

			xFunctionCall.m_bFieldCall  = true;   

			unsigned int uNumberOfArgs = 1;
			if (CompileSubArgs(uNumberOfArgs, xFunctionCall))
			{
				// Mark here that we have to make a function call
				// Write string
				// Mark that is here where we need to replace this object in this expression with the correct value
				string xField = s_xFieldMarker;
				xField       += to_string(m_uFieldAccessCount++);

				m_pxVMWriter->WriteString(xField);

				// Field access, Array object type not known
				// member object was pushed back in the calling recursive function 
				xObject.m_xMemberList[xObject.m_xMemberList.size() - 1].m_xVMMarker = xField;

				Resolver::AddFunctionCall(xFunctionCall);

				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			unsigned int uNumberOfArgs = 0;

			//Push the object base address if we are not accessing a static function
			const Variable* pxVar = GetVariable(xObject.m_xName);
			if (pxVar)
			{
				// Object was pushed already by the CompileTerm function
				uNumberOfArgs++;
				xFunctionCall.m_bStaticCall  = false;
				xFunctionCall.m_xVar.m_xType = xObject.m_xType;
			}
			else if(m_xAncestorList.GetSymbol(xObject.m_xName))
			{
				// parent class call, check the subroutine type (constructor, method  or function )
				string xFunctionName = xObject.m_xName;
				xFunctionName       += ".";
				xFunctionName       += xSubroutineName;

				const Function* xFunction = Resolver::GetFunctionSymbolTable().GetSymbol(xFunctionName);
				if (xFunction)
				{
					if (   (xFunction->m_eType == CompilerTokenizer::KEYWORD_METHOD) || (xFunction->m_eType == CompilerTokenizer::KEYWORD_DESTRUCTOR)
						|| (xFunction->m_eType == CompilerTokenizer::KEYWORD_VIRTUAL)	)
					{
						// Push this pointer argument
						if (!m_pxVMWriter->WriteTerm(VMWriter::TERM_THIS))
						{
							Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
							return false;
						}

						++uNumberOfArgs;

						// Non static call
						xFunctionCall.m_bStaticCall  = false;
						xFunctionCall.m_xVar.m_xType = xObject.m_xType;
					}
					else
					{
						// Static call
						xFunctionCall.m_bStaticCall  = true;
						xFunctionCall.m_xVar.m_xType = xObject.m_xType;

						// if a constructor call
						if(xFunction->m_eType == CompilerTokenizer::KEYWORD_CONSTRUCTOR)
						{
							++uNumberOfArgs; // Allocate or not argument

							if (!bBaseConstructor)
							{
								// derived initial constructor, false means allocate
								if (!m_pxVMWriter->WriteTerm(VMWriter::TERM_FALSE))
								{
									Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
									return false;
								}
							}
							else
							{
								// don't allocate
								if (!m_pxVMWriter->WriteTerm(VMWriter::TERM_TRUE))
								{
									Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
									return false;
								}
							}
						}
					}
				}
				else
				{
					//internal error
					Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_UNRESOLVED_FUNCTION, xFunctionName.c_str());
				}
			}
			else
			{
				if ( xSubroutineName == "new")
				{
					++uNumberOfArgs; // Allocate or not argument

					// initial constructor, false means allocate
					if (!m_pxVMWriter->WriteTerm(VMWriter::TERM_FALSE))
					{
						Error::BuildErrorString(m_xError, *this, *m_pxTokenizer, Error::ERROR_INTERNAL);
						return false;
					}
				}

				// Might be a static call or an error which will be detected when checking symbol resolving
				// Assume static
				xFunctionCall.m_bStaticCall  = true;
				xFunctionCall.m_xVar.m_xType = xObject.m_xName;
			}

			if (CompileSubArgs(uNumberOfArgs, xFunctionCall))
			{
				if(xFunctionCall.m_bStaticCall)
					m_pxVMWriter->WriteSubroutineCall(xFunctionCall.m_xVar.m_xType + "." + xSubroutineName, uNumberOfArgs);
				else
					m_pxVMWriter->WriteSubroutineCall(xFunctionCall.m_xVar.m_xType + "." + xSubroutineName, uNumberOfArgs, false, 0, &xFunctionCall.m_xVMCode); // might be virtual, will get resolved by the resolver

				Resolver::AddFunctionCall( xFunctionCall );

				return true;
			}
			else
			{
				return false;
			}
		}
	}
}

const Variable* Parser::GetVariable( const string& xName ) const
{
	//Check method first
	const Variable* pxVar = m_xVariableMethodSymbolTable.GetSymbol(xName);
	if( !pxVar )
		pxVar = m_xVariableClassSymbolTable.GetSymbol(xName);

	return pxVar;	
}

bool Parser::GetNextToken()
{
	if ( m_NeedToGetNextToken )
	{
		 return m_pxTokenizer->GetNextToken();
	}
	else
	{
		m_NeedToGetNextToken = true;
		return true;
	}
}