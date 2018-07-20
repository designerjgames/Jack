#ifndef _PARSER_
#define _PARSER_

#include <string>
#include <vector>
#include "Assignment.h"
#include "Function.h"
#include "FunctionCall.h"
#include "Tokenizer.h"
#include "SymbolTable.h"
#include "Variable.h"
#include "VMWriter.h"

class Parser
{
public:
	Parser(const string& xFilename, CompilerTokenizer& xTokenizer);
	Parser(const Parser& xParser); // copy constructor
	~Parser();

	bool Compile();

	// Returns the variable from the symbol table, working from current scope to the wider one. (i.e., from method scope then class scope).
	const Variable* GetVariable( const string& xName ) const;

	const string& GetClassName()    const { return m_xClassName;		    }
	const string& GetFunctionName() const { return m_xFunctionName;			}
	const string& GetFileName()     const { return m_xFileName;			    }

	// Error
	const string& GetError()        const { return m_xError;				}

	unsigned int  GetClassSize()    const { return m_uClassSize;            }

	const stringstream&  GetVMCode()const { return m_pxVMWriter->GetVMCode(); }

	const SymbolTable<Variable>&  GetVariableClassSymbolTable() const { return m_xVariableClassSymbolTable; }

	static const SymbolTable<Parser>& GetParsersSymbolTable() { return s_xParsersSymbolTable;  }

	static char s_cTypeOfArrayPrefix;

private:
	// Inheritance and polymorphic code support functions
	bool   DetectPolymorphism();
	void   InheritBaseClassDetails(const Parser& xParser);
	string GetPolymorphicVTableVarDecCode() const;
	string GetPolymorphicVPointerVarDecCode() const;
	string GetPolymorphicVTableAndVPointerInitCode() const;

	bool   CompileArray(Expression& xExpression, Object& xObject, string& xIdentifierName, bool bAssignment);
	bool   CompileClass();
	bool   CompileClassBody();
	bool   CompileClassBodyInternal(bool bVars);
	bool   CompileClassVarDec();
	bool   CompileSubroutine();
	bool   CompileParameterList();
	bool   CompileVarDec(bool bVars = true);
	bool   CompileStatements();
	bool   CompileDo();
	bool   CompileLet();
	bool   CompileWhile();
	bool   CompileReturn();
	bool   CompileIf();
	bool   CompileASM();
	bool   CompileExpression(Expression& xExpression);
	bool   CompileTerm(Expression& xExpression, Object& xObject, bool bAssignment);
	bool   CompileExpressionList(unsigned int& uNumberOfArgs, FunctionCall& xFunctionCall);
	bool   CompileType(string& xVarType);
	bool   CompileSubroutineBody(CompilerTokenizer::KEYWORD_TYPE eFunctionType, const string& xNumLocalsMarker);
	bool   CompileSubroutineCall(const string& xIndentifier, Object xObject, bool bBaseConstructor = false);
	bool   CompileSubArgs(unsigned int& uNumberOfArgs, FunctionCall& xFunctionCall);
	bool   CompileAssignment(Assignment& xAssignment);

	bool   GetNextToken();

	CompilerTokenizer*			m_pxTokenizer;
	VMWriter					m_xVMWriter;
	VMWriter*					m_pxVMWriter;

	bool						m_NeedToGetNextToken;

	// Symbol tables
	SymbolTable<Variable>		m_xVariableClassSymbolTable;
	SymbolTable<Variable>		m_xVariableMethodSymbolTable;


	// Segment vars indices
	unsigned int				m_uStaticVarIndex;
	unsigned int				m_uFieldVarIndex;
	unsigned int				m_uLocalVarIndex;
	unsigned int				m_uArgVarIndex;

	unsigned int                m_uVirtualFunctionIndex;
	bool                        m_bIsBaseClass;

	string						m_xClassName;          // Class name
	string						m_xBaseClassName;      // Class name
	unsigned int				m_uClassSize;          // Number of class fields in words
	string						m_xFunctionName;
	unsigned int				m_uNumberOfIFConditionals;
	unsigned int				m_uNumberOfWhileConditionals;
	string						m_xFileName;
	bool                        m_bAddressExpression;
	SymbolTable<string>         m_xAncestorList;
	string						m_xError;

	bool                        m_bClassPolymorphic;
	SymbolTable<Function>       m_xVirtualFunctionList;

	unsigned int				m_uFieldAccessCount;
	unsigned int				m_uFunctionsCount;

	static string               s_xFieldMarker;
	static string               s_xLocalsMarker;
	static SymbolTable<Parser>	s_xParsersSymbolTable;
};

#endif //_PARSER_