#include "VMWriter.h"

#include <iostream>
#include <assert.h>
#include "Parser.h"
#include "Variable.h"

unsigned int VMWriter::s_uFunctionCalls = 0;

VMWriter::VMWriter( const Parser& xParser )
: m_xParser( xParser )
, m_xVMCode()
, m_uSkippedAllocs(0)
{

}

VMWriter::~VMWriter()
{

}

void VMWriter::WriteIfGotoTrue( unsigned int uNumberOfFunctionConditionals )
{
	m_xVMCode << "if-goto IF_TRUE" << uNumberOfFunctionConditionals << '\n';
}

void VMWriter::WriteWhileGotoTrue(unsigned int uNumberOfFunctionConditionals)
{
	m_xVMCode << "if-goto WHILE_TRUE" << uNumberOfFunctionConditionals << '\n';
}

void VMWriter::WriteGotoFalse( unsigned int uNumberOfFunctionConditionals )
{
	m_xVMCode << "goto IF_FALSE"   << uNumberOfFunctionConditionals << '\n';
}

void VMWriter::WriteWhileGotoFalse(unsigned int uNumberOfFunctionConditionals)
{
	m_xVMCode << "goto WHILE_FALSE" << uNumberOfFunctionConditionals << '\n';
}

void VMWriter::WriteGotoEnd( unsigned int uNumberOfFunctionConditionals )
{
	m_xVMCode << "goto IF_END"   << uNumberOfFunctionConditionals << '\n';
}

void VMWriter::WriteTrueLabel( unsigned int uNumberOfFunctionConditionals )
{
	m_xVMCode << "label IF_TRUE"   << uNumberOfFunctionConditionals << '\n';
}

void VMWriter::WriteWhileTrueLabel(unsigned int uNumberOfFunctionConditionals)
{
	m_xVMCode << "label WHILE_TRUE" << uNumberOfFunctionConditionals << '\n';
}

void VMWriter::WriteFalseLabel( unsigned int uNumberOfFunctionConditionals )
{
	m_xVMCode << "label IF_FALSE"   << uNumberOfFunctionConditionals << '\n';
}

void VMWriter::WriteWhileFalseLabel(unsigned int uNumberOfFunctionConditionals)
{
	m_xVMCode << "label WHILE_FALSE" << uNumberOfFunctionConditionals << '\n';
}

void VMWriter::WriteEndLabel( unsigned int uNumberOfFunctionConditionals )
{
	m_xVMCode << "label IF_END"   << uNumberOfFunctionConditionals << '\n';
}

void VMWriter::WriteWhileLabel( unsigned int uNumberOfFunctionConditionals )
{
	m_xVMCode << "label WHILE"   << uNumberOfFunctionConditionals << '\n';
}

void VMWriter::WriteWhile( unsigned int uNumberOfFunctionConditionals )
{
	m_xVMCode << "goto WHILE"   << uNumberOfFunctionConditionals << '\n';
}

void VMWriter::WriteRemoveZeroReturn()
{
	// pop unused return value
	m_xVMCode << "pop temp 0" << '\n';
}

void VMWriter::WriteZeroReturn()
{
	// push a zero on the stack for void function return
	m_xVMCode << "push constant 0" << '\n';
}

void VMWriter::WriteReturn()
{
	// push a zero on the stack for void function return
	m_xVMCode << "return" << '\n';
}

bool VMWriter::WriteFunctionType( CompilerTokenizer::KEYWORD_TYPE eKeywordType, unsigned int uObjectSize /*= 0*/ )
{
	switch (eKeywordType)
	{
	case CompilerTokenizer::KEYWORD_CONSTRUCTOR:
		{
			// Argument 0 is allocate or not (true or false)
			// Push 0 (false) to compare
			m_xVMCode << "push argument 0" << '\n';
			m_xVMCode << "if-goto SKIP_NEW_ALLOC" << m_uSkippedAllocs << '\n';
			m_xVMCode << "push constant " << uObjectSize << '\n'; // Push object size
			m_xVMCode << "call Memory.alloc 1" << '\n';           // Allocate the object memory and return object address
			m_xVMCode << "pop pointer 0" << '\n';                 // "this" now points to the object address
			m_xVMCode << "label SKIP_NEW_ALLOC" << m_uSkippedAllocs << '\n';
			++m_uSkippedAllocs;
			break;
		}

	case CompilerTokenizer::KEYWORD_DESTRUCTOR:
	case CompilerTokenizer::KEYWORD_METHOD:
	case CompilerTokenizer::KEYWORD_VIRTUAL:
		{
			m_xVMCode << "push argument 0" << '\n';  // Argument has object address
			m_xVMCode << "pop pointer 0" << '\n';    // "this" now points to the object address
			break;
		}

	case CompilerTokenizer::KEYWORD_FUNCTION: // Static function, do nothing
		{
			break;
		}

	default:
		{
			assert(false && "function type error!");
			return false;
		}
	}

	return true;
}

void VMWriter::WriteFunctionDeclaration( const string& xIdentifierName, const string& xLocalsMarker )
{
	m_xVMCode << "function " <<  xIdentifierName << " " << xLocalsMarker << '\n';
}

void VMWriter::WriteSubroutineCall( const string& xIdentifierName, unsigned int uNumberOfArgs, bool bVirtual, unsigned int uVFIndex, string* pxVMCodeLine)
{
	if (bVirtual)
	{
		//m_xVMCode << "push this " << m_xParser.GetClassSize() - 1 << '\n'; // Vpointer
		m_xVMCode << "callv " << xIdentifierName << " " << uNumberOfArgs << " " << uVFIndex << '\n';
	}
	else
	{ 
		string xVMCode;
		xVMCode  = "call ";
		xVMCode += xIdentifierName;
		xVMCode += " ";
		xVMCode += to_string(uNumberOfArgs);
		xVMCode += " //";
		xVMCode += to_string(s_uFunctionCalls++);
		
		xVMCode += '\n';

		if (pxVMCodeLine)
			*pxVMCodeLine = xVMCode;

		m_xVMCode << xVMCode;
	}
}

bool VMWriter::WriteAssignment( const string& xIdentifierName, bool bArray /*= false*/)
{
	// Get it from the symbol table
	const Variable* pxVar = m_xParser.GetVariable( xIdentifierName );
	if( pxVar )
	{
		if( bArray )
		{
			// object in array has now result value
			m_xVMCode << "pop that 0" << '\n';
		}
		else
		{
			switch (pxVar->m_eSegment)
			{
			case Variable::SEGMENT_STATIC:
				{
					m_xVMCode << "pop static " << pxVar->m_uIndex << " $" << m_xParser.GetClassName() << '\n';
					break;
				}

			case Variable::SEGMENT_LOCAL:
				{
					m_xVMCode << "pop local " << pxVar->m_uIndex << '\n';
					break;
				}

			case Variable::SEGMENT_ARGUMENT:
				{
					m_xVMCode << "pop argument " << pxVar->m_uIndex << '\n';
					break;
				}

			case Variable::SEGMENT_FIELD:
				{
					m_xVMCode << "pop this " << pxVar->m_uIndex << '\n';
					break;
				}

			default:
				{
					//Should never happen
					assert(false && "identifier segment error!");
					return false;
				}
			}
		}
	}
	else
	{
		//Should never happen
		assert(false && "Unresolved symbol!");
		return false;
	}

	return true;
}

bool VMWriter::WriteOperator( const string& xOperator )
{
	if( xOperator == "+" )
	{
		m_xVMCode << "add" << '\n';
	}
	else if( xOperator == "-" )
	{
		m_xVMCode << "sub" << '\n';
	}
	else if( xOperator == "U-" )
	{
		m_xVMCode << "neg" << '\n';
	}
	else if( xOperator == "*" )
	{
		m_xVMCode << "call Math.multiply 2" << '\n';
	}
	else if( xOperator == "/" )
	{
		m_xVMCode << "call Math.divide 2" << '\n';
	}
	else if( xOperator == "&" )
	{
		m_xVMCode << "and" << '\n';
	}
	else if( xOperator == "|" )
	{
		m_xVMCode << "or" << '\n';
	}
	else if( xOperator == "~" )
	{
		m_xVMCode << "not" << '\n';
	}
	else if( xOperator == "<" )
	{
		m_xVMCode << "lt" << '\n';
	}
	else if( xOperator == ">" )
	{
		m_xVMCode << "gt" << '\n';
	}
	else if (xOperator == "<=")
	{
		m_xVMCode << "le" << '\n';
	}
	else if (xOperator == ">=")
	{
		m_xVMCode << "ge" << '\n';
	}
	else if( xOperator == "=" )
	{
		m_xVMCode << "eq" << '\n';
	}
	else
	{
		assert(false && "Wrong operator!");
		return false;
	}

	return true;
}

bool VMWriter::WriteTerm( TERM eTerm, const string* pxName /* = NULL */, bool bAddress /*=false*/)
{
	string xPush = bAddress ? "pusha" : "push";
	switch (eTerm)
	{
		case TERM_INT_CONSTANT:
		{
			if( pxName )
				m_xVMCode << "push constant " << *pxName << '\n';
			else
			{
				assert(false && "INT_CONSTANT name expected!");
				return false;
			}

			break;
		}

		case TERM_STRING_CONSTANT:
		{
			if( pxName )
			{
				m_xVMCode << "push constant 0" << '\n';

				//Push the length of string for the string new constructor
				m_xVMCode << "push constant " << pxName->length() << '\n';

				//Create the string
				m_xVMCode << "call String.new 2" << '\n'; //1 argument, string size

				for (unsigned int u = 0; u < pxName->length(); ++u)
				{
					m_xVMCode << "push constant " << static_cast<int>((*pxName)[u]) << '\n';

					//We have now This, and the constant on the current stack frame
					m_xVMCode << "call String.appendChar 2" << '\n'; // 2 for this and the constant
						                                             // When appendChar returns, it puts this as a return value on the current stack top

				}
			}
			else
			{
				assert(false && "STRING_CONSTANT name expected!");
				return false;
			}
			break;
		}

		case TERM_TRUE:
		{
			m_xVMCode << "push constant 0" << '\n';
			m_xVMCode << "not" << '\n';
			break;
		}

		case TERM_FALSE:
		{
			m_xVMCode << "push constant 0" << '\n';
			break;
		}

		case TERM_NULL:
		{
			m_xVMCode << "push constant 0" << '\n';
			break;
		}

		case TERM_THIS:
		{
			m_xVMCode << xPush << " pointer 0" << '\n';
			break;
		}

		case TERM_FUNCTION_ADDRESS:
		{
			m_xVMCode << "pushf " << *pxName << '\n';
			break;
		}

		case TERM_INDENTIFIER:
		{
			if( pxName )
			{
				// Get it from the symbol table
				const Variable* pxVar = m_xParser.GetVariable( *pxName );
				if( pxVar )
				{
					switch (pxVar->m_eSegment)
					{
						case Variable::SEGMENT_STATIC:
						{
							m_xVMCode << xPush << " static " << pxVar->m_uIndex << " $" << m_xParser.GetClassName() <<  '\n';
							break;
						}

						case Variable::SEGMENT_LOCAL:
						{
							m_xVMCode << xPush << " local " << pxVar->m_uIndex << '\n';
							break;
						}

						case Variable::SEGMENT_ARGUMENT:
						{
							m_xVMCode << xPush << " argument " << pxVar->m_uIndex << '\n';
							break;
						}

						case Variable::SEGMENT_FIELD:
						{
							m_xVMCode << xPush << " this " << pxVar->m_uIndex << '\n';
							break;
						}

						default:
						{
							assert(false && "identifier segment error!");
							break;
						}
					}
				}
				else
				{
					assert(false && "Unresolved symbol!");
				}
			}
			break;
		}

		default:
		{
			assert(false && "Invalid term!");
			return false;
		}
	}

	return true;
}

void VMWriter::WriteArray(bool bPushArray)
{
	m_xVMCode << "add"           << '\n';
	m_xVMCode << "pop pointer 1" << '\n';

	if(bPushArray)
		m_xVMCode << "push that 0"   << '\n';
}

void VMWriter::WriteString(const string& xString)
{
	m_xVMCode << xString << '\n';
}