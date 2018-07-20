#ifndef _Resolver_
#define _Resolver_

#include <string>
#include <stack>

#include "Assignment.h"
#include "Class.h"
#include "Function.h"
#include "FunctionCall.h"
#include "SymbolTable.h"
#include "Variable.h"
#include "VMCode.h"

using namespace std;

class Resolver
{
public:
	enum ERROR
	{
		ERROR_INTERNAL,
		ERROR_UNRESOLVED_CLASS,
		ERROR_UNRESOLVED_FUNCTION,
		ERROR_ARGUMENT_NUMBER,
		ERROR_INVALID_TYPE,
		ERROR_OPERATOR_TYPE
	};

	static void   AddFunctionCall( const FunctionCall& xFunctionCall ) { s_xCallList.push_back(xFunctionCall);           }
	static void   AddAssignment  ( const Assignment& xAssignment     ) { s_xAssignmentList.push_back(xAssignment);       }
	static void   AddReturn      ( const Expression& xReturn         ) { s_xReturnList.push_back(xReturn);               }
	static void   AddCondition   ( const Expression& xCondition      ) { s_xConditionList.push_back(xCondition);         }
	static void   AddArrayIndex  ( const Expression& xArrayIndex     ) { s_xArrayIndexList.push_back(xArrayIndex);       }
	static void   AddVariable    ( const Variable& xVariable         ) { s_xVariableList.push_back(xVariable);           }

	static bool   AddVMCode      ( const VMCode& xVMCode             ) { return s_xVMCodeTable.Add(xVMCode.m_xName, xVMCode);              }
	static bool   AddFunction    ( const Function& xFunction         ) { return s_xFunctionSymbolTable.Add(xFunction.m_xName, xFunction ); }
	static bool   AddClass       ( const Class& xClass               ) { return s_xClassSymbolTable.Add(xClass.m_xName, xClass );          }

	static size_t GetNumberOfFunctionCalls() { return s_xCallList.size(); }

	static SymbolTable<Function>& GetFunctionSymbolTable()             { return s_xFunctionSymbolTable; }

	// Resolve all symbols, and check types
	static bool Resolve();

	static bool WriteVMFiles(bool bUnresolved = false );

	static void PrintErrors();

private:
	static bool ResolveVariableType(const string& xVarType, const Location& xLocation);
	static bool ResolveAllVariableDelarations();
	static bool ResolveAllFunctionArguments();
	static bool ResolveClassesDoingStaticFunctionCalls();
	static bool ResolveAllFunctionCallArgumentsAndReturnTypes();
	static bool ResolveFunctionCallArgumentsAndReturnType( FunctionCall& xFunctionCall );
	static bool ResolveAllAssigments();
	static bool ResolveAllReturns();
	static bool ResolveAllConditions();
	static bool ResolveAllArrayIndices();
	static bool ResolveMemberAccess(Object& xObject, const Location& xLocation);

	static bool WriteAllFunctionLocalsVMCode();
	static bool WriteVirtualFunctionCallsVMCode();
	static bool WriteAllMemberAccessVMCode();
	static bool WriteAllExpressionMemberAccessVMCode(Expression& xExpression, bool bAssignment);
	static bool WriteMemberAccessVMCode(const Object& xObject, const Location& xLocation, bool bAssignment);
	
	static bool ReplaceVMMarkerWithVMCode(const Location& xLocation, const string& xMarker, const string& xVMCode);

	static bool ResolveExpression( Expression& xExpression );
	static bool VerifyExpression ( CompilerTokenizer& xTokenizer, const Location& xLocation );
	static bool PushTermType     ( CompilerTokenizer& xTokenizer, const Location& xLocation);
	static bool ApplyOperator	 ( const string& xOperator, string& xRightObjectType, string& xLeftObjectType );

	static bool IsUserType   ( const string& xType );
	static bool IsTypeConform( const string& xExpectedType, const string& xUsedType );

	// Expression Tokenizer
	static bool GetNextToken( CompilerTokenizer& xTokenizer );

	// Need to be synchronized for threading
	static vector<FunctionCall>	   s_xCallList;	       // type of call, file, class, object, line number 
	static vector<Assignment>	   s_xAssignmentList;  // assignments list, hold left and right objects compilation details
	static vector<Expression>	   s_xReturnList;      // Return list, hold return object type
	static vector<Expression>	   s_xConditionList;   // used in conditionals (if, while)
	static vector<Expression>	   s_xArrayIndexList;  // used in array indices
	static vector<Variable>	       s_xVariableList;    // used in array indices

	static SymbolTable<Function>   s_xFunctionSymbolTable;
	static SymbolTable<Class>	   s_xClassSymbolTable;
	static SymbolTable<VMCode>	   s_xVMCodeTable;

	static stack<string>           s_xExpressionStack;
	static bool					   s_bNeedToGetNextToken;

	static bool					   s_bAddressExpression;

	static vector<string>		   s_xErrorList;

	const static string            s_xArrayType;
	const static string            s_xArrayClass;
};

#endif //_Resolver_