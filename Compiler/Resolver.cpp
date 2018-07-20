#include "Resolver.h"

#include "Error.h"
#include <iostream> 

vector<Variable>	  Resolver::s_xVariableList;
vector<FunctionCall>  Resolver::s_xCallList;
vector<Assignment>	  Resolver::s_xAssignmentList;
vector<Expression>	  Resolver::s_xReturnList;
vector<Expression>    Resolver::s_xConditionList;
vector<Expression>    Resolver::s_xArrayIndexList;
SymbolTable<Function> Resolver::s_xFunctionSymbolTable;
SymbolTable<Class>	  Resolver::s_xClassSymbolTable;
SymbolTable<VMCode>	  Resolver::s_xVMCodeTable;
stack<string>         Resolver::s_xExpressionStack;
bool				  Resolver::s_bNeedToGetNextToken;
bool				  Resolver::s_bAddressExpression;
vector<string>   	  Resolver::s_xErrorList;
const string  		  Resolver::s_xArrayType  = "Array@";
const string  		  Resolver::s_xArrayClass = "Array";

bool Resolver::Resolve()
{
	bool bSuccess = true;

	s_xErrorList.clear();

	bSuccess &= ResolveAllVariableDelarations();

	bSuccess &= ResolveAllFunctionArguments();

	bSuccess &= ResolveClassesDoingStaticFunctionCalls();

	// If we have errors here, no need to continue
	if (!bSuccess)
		return false;

	if( !ResolveAllFunctionCallArgumentsAndReturnTypes() )
		return false;

	// By now, we should know the types of every object and field

	// Check the expressions types
	bSuccess &= ResolveAllAssigments();

	bSuccess &= ResolveAllReturns();

	bSuccess &= ResolveAllConditions();

	bSuccess &= ResolveAllArrayIndices();

	// Write VM code for function locals count
	bSuccess &= WriteAllFunctionLocalsVMCode();
	// If we have errors here, no need to continue
	if (!bSuccess)
		return false;

	// Resolve virtual function calls
	bSuccess &= WriteVirtualFunctionCallsVMCode();
	// If we have errors here, no need to continue
	if (!bSuccess)
		return false;

	// Write VM code for field access
	bSuccess &= WriteAllMemberAccessVMCode();
	// If we have errors here, no need to continue
	if (!bSuccess)
		return false;

	return bSuccess;
}

bool Resolver::ResolveMemberAccess(Object& xObject, const Location& xLocation)
{
	bool bSuccess = true;
	string xParentType = xObject.m_xType;

	// Check type is static access
	if (xObject.m_bStatic)
	{
		const Class* pxClass = s_xClassSymbolTable.GetSymbol(xParentType);
		if (!pxClass)
		{
			string xError;
			Error::BuildErrorString(xError, xLocation, Error::ERROR_UNRESOLVED_CLASS, xParentType.c_str());
			s_xErrorList.push_back(xError);
			return false;
		}
	}

	for (unsigned int w = 0; w < xObject.m_xMemberList.size(); ++w)
	{
		Object& xMember = xObject.m_xMemberList[w];

		// The member type and info
		// Get parser where this member belong
		const Parser* pxParser = Parser::GetParsersSymbolTable().GetSymbol(xParentType);

		// An object or function?
		if (xMember.m_bFunctionReturnType && !xMember.m_bFunctionReturnResolved)
		{
			// Find the function call
			FunctionCall& xFunctionCall  = s_xCallList[xMember.m_uFunctionCallIndex];

			// Set what we know about the type of the caller
			xFunctionCall.m_xVar.m_xType = xParentType;

			if (xFunctionCall.m_xReturnType != "")
			{
				xMember.m_bFunctionReturnResolved = true;
				xMember.m_xType                   = xFunctionCall.m_xReturnType;
			}
			else
			{
				ResolveFunctionCallArgumentsAndReturnType(xFunctionCall);
			}
		}
		else
		{
			// Find the variable
			const Variable* pxVar = pxParser->GetVariableClassSymbolTable().GetSymbol(xMember.m_xName);

			//Should be taking dimension into account, and set ut as the m_xtype and parent type
			if (pxVar)
			{
				xMember.m_xResolvedVar = *pxVar;
				xMember.m_xType = pxVar->m_xType;
			}
			else
			{
				bSuccess = false;
				string xError;
				Error::BuildErrorString(xError, xLocation, Error::ERROR_UNRESOLVED_CLASS, xMember.m_xName.c_str());
				s_xErrorList.push_back(xError);
			}
		}

		// By now we should know the member type
		// Check for array dimensions
		for (unsigned int x = 0; x < xMember.m_uDimension; ++x)
		{
			// Advance on type after the .
			size_t iDotfound = xMember.m_xType.find(Parser::s_cTypeOfArrayPrefix);
			if (iDotfound != std::string::npos)
			{
				xMember.m_xType = xMember.m_xType.substr(iDotfound + 1, string::npos);
			}
			else
			{
				//We shouldn't dereference this array more
				bSuccess = false;
				string xError;
				Error::BuildErrorString(xError, xLocation, Error::ERROR_UNRESOLVED_CLASS, xMember.m_xName.c_str());
				s_xErrorList.push_back(xError);
			}
		}

		// If taking an address, the type becomes int
		if (xMember.m_bTakingAddress)
			xMember.m_xType = "int";

		xParentType = xMember.m_xType;
	}

	//On success, we have the end type for this object (for type checking), we also have all of it's members resolved, which we will use when writing VM access code

	return bSuccess;
}

// Need to be called first
bool Resolver::ResolveClassesDoingStaticFunctionCalls()
{
	bool bSuccess = true;
	for( unsigned int u = 0; u < s_xCallList.size(); ++u )
	{
		FunctionCall& xFunctionCall = s_xCallList[u];

		if (xFunctionCall.m_bStaticCall)
		{
			const Class* pxClass = s_xClassSymbolTable.GetSymbol(xFunctionCall.m_xVar.m_xType);
			if (!pxClass)
			{
				bSuccess = false;
				string xError;
				Error::BuildErrorString(xError, xFunctionCall, Error::ERROR_UNRESOLVED_CLASS, xFunctionCall.m_xVar.m_xType.c_str());
				s_xErrorList.push_back( xError );
			}
		}
	}

	return bSuccess;
}

// 2nd
bool Resolver::ResolveAllVariableDelarations()
{
	bool bSuccess = true;
	for (unsigned int u = 0; u < s_xVariableList.size(); ++u)
	{
		Variable& xVariable = s_xVariableList[u];
		if (!ResolveVariableType(xVariable.m_xType, xVariable))
			bSuccess = false;
	}

	return bSuccess;
}

// 3rd
bool Resolver::ResolveAllFunctionArguments()
{
	bool bSuccess = true;

	for (unordered_map<string, const Function>::const_iterator it = s_xFunctionSymbolTable.GetTable().begin(); it != s_xFunctionSymbolTable.GetTable().end(); ++it)
	{
		const Function& xFunction = it->second;
		for (unsigned int u = 0; u < xFunction.m_xArgumentList.size(); ++u)
		{
			const Variable& xArgument = xFunction.m_xArgumentList[u];
			if (!ResolveVariableType(xArgument.m_xType, xArgument))
				bSuccess = false;
		}
	}

	return bSuccess;
}

bool Resolver::ResolveVariableType(const string& xVarType, const Location& xLocation)
{
	if (!IsUserType(xVarType))
		return true;

	string        xBaseArrayType;
	const Class* pxClass = NULL;

	// Check if it is an array type "Array@x..."
	if (xVarType.length() >= s_xArrayType.length() && xVarType.substr(0, s_xArrayType.length()) == s_xArrayType)
	{
		// Check for Array class
		pxClass = s_xClassSymbolTable.GetSymbol(s_xArrayClass);

		if (!pxClass)
		{
			string xError;
			Error::BuildErrorString(xError, xLocation, Error::ERROR_UNRESOLVED_CLASS, s_xArrayClass.c_str());
			s_xErrorList.push_back(xError);
			return false;
		}

		// Check for base object type
		size_t iDotfound = xVarType.find_last_of('@');
		if (iDotfound != std::string::npos)
		{
			xBaseArrayType = xVarType.substr(iDotfound + 1, string::npos);
			if (!IsUserType(xBaseArrayType))
				return true;

			pxClass = s_xClassSymbolTable.GetSymbol(xBaseArrayType);
		}
		else
		{
			string xError;
			Error::BuildErrorString(xError, xLocation, Error::ERROR_INTERNAL);
			s_xErrorList.push_back(xError);
			return false;
		}
	}
	else
	{
		pxClass = s_xClassSymbolTable.GetSymbol(xVarType);
	}

	if (!pxClass)
	{
		string xError;
		Error::BuildErrorString(xError, xLocation, Error::ERROR_UNRESOLVED_CLASS, xBaseArrayType.empty() ? xVarType.c_str() : xBaseArrayType.c_str());
		s_xErrorList.push_back(xError);
		return false;
	}

	return true;
}

bool Resolver::ResolveAllAssigments()
{
	bool bSuccess = true;
	for( unsigned int u = 0; u < s_xAssignmentList.size(); ++u )
	{
		Assignment& xAssignment = s_xAssignmentList[u];
		if (ResolveExpression(xAssignment.m_xRightExpression))
		{
			if (ResolveExpression(xAssignment.m_xLeftExpression))
			{
				if (!IsTypeConform(xAssignment.m_xLeftExpression.m_xType, xAssignment.m_xRightExpression.m_xType))
				{
					string xError;
					Error::BuildErrorString(xError, xAssignment, Error::ERROR_INVALID_TYPE, xAssignment.m_xRightExpression.m_xType.c_str(), xAssignment.m_xLeftExpression.m_xType.c_str());
					s_xErrorList.push_back(xError);
					bSuccess = false;
				}
			}
			else
			{
				bSuccess = false;
			}
		}
		else
		{
			bSuccess = false;
		}
	}
	return bSuccess;
}

bool Resolver::ResolveAllReturns()
{
	bool bSuccess = true;
	for( unsigned int u = 0; u < s_xReturnList.size(); ++u )
	{
		Expression& xReturn = s_xReturnList[u];
		if (ResolveExpression(xReturn))
		{
			// All good, we should have now all parameters as proper types;
			// Find the function
			string xFunctionName = xReturn.m_xClass;
			xFunctionName       += ".";
			xFunctionName		+= xReturn.m_xFunction;

			const Function* xFunction = s_xFunctionSymbolTable.GetSymbol( xFunctionName );
			if( xFunction )
			{
				if (!IsTypeConform(xFunction->m_xReturnType, xReturn.m_xType))
				{
					string xError;
					Error::BuildErrorString(xError, xReturn, Error::ERROR_INVALID_TYPE, xReturn.m_xType.c_str(), xFunction->m_xReturnType.c_str());
					s_xErrorList.push_back(xError);
					bSuccess = false;
				}
			}
			else
			{
				string xError;
				Error::BuildErrorString(xError, xReturn, Error::ERROR_INTERNAL); // We should have this function is the symbol table
				s_xErrorList.push_back(xError);
				bSuccess = false;
			}
		}
		else
		{
			bSuccess = false;
		}
	}
	return bSuccess;
}

bool Resolver::ResolveAllConditions()
{
	bool bSuccess = true;
	for( unsigned int u = 0; u < s_xConditionList.size(); ++u )
	{
		Expression& xCondition = s_xConditionList[u];
		if (ResolveExpression(xCondition))
		{
			// Make sure the type is int or int constant
			// If function type is int, and the param is int constant accept it
			if ((xCondition.m_xType != "boolean") && (xCondition.m_xType != "true") && (xCondition.m_xType != "false"))
			{
				string xError;
				Error::BuildErrorString(xError, xCondition, Error::ERROR_INVALID_TYPE, xCondition.m_xType.c_str(), "boolean");
				s_xErrorList.push_back(xError);
				bSuccess = false;
			}
		}
		else
		{
			bSuccess = false;
		}
	}
	return bSuccess;
}

bool Resolver::ResolveAllArrayIndices()
{
	bool bSuccess = true;
	for( unsigned int u = 0; u < s_xArrayIndexList.size(); ++u )
	{
		Expression& xArrayIndex = s_xArrayIndexList[u];
		if (ResolveExpression(xArrayIndex))
		{
			// Make sure the type is int or int constant
			// If function type is int, and the param is int constant accept it
			if ((xArrayIndex.m_xType != "int") && (xArrayIndex.m_xType != "int_constant"))
			{
				string xError;
				Error::BuildErrorString(xError, xArrayIndex, Error::ERROR_INVALID_TYPE, xArrayIndex.m_xType.c_str(), "int");
				s_xErrorList.push_back(xError);
				bSuccess = false;
			}
		}
		else
		{
			bSuccess = false;
		}
	}
	return bSuccess;
}

bool Resolver::ResolveExpression( Expression& xExpression )
{
	for( unsigned int u = 0; u < xExpression.m_xObjectList.size(); ++u )
	{
		Object& xObject = xExpression.m_xObjectList[u];
		if (xObject.m_bFieldAccess)
		{
			ResolveMemberAccess(xObject, xExpression);
		}
		else if(xObject.m_bFunctionReturnType && !xObject.m_bFunctionReturnResolved)
		{
			// Get the function call
			FunctionCall& xFunctionCall = s_xCallList[xObject.m_uFunctionCallIndex];
			if( xFunctionCall.m_xReturnType != "")
			{
				xObject.m_bFunctionReturnResolved = true;
				xObject.m_xType                   = xFunctionCall.m_xReturnType;
			}
			else
			{
				ResolveFunctionCallArgumentsAndReturnType( xFunctionCall );
			}
		}
	}

	// By now all the object should have a known type, put the types in the expression template
	for( unsigned int u = 0; u < xExpression.m_xObjectList.size(); ++u )
	{
		Object& xObject = xExpression.m_xObjectList[u];
		if (xObject.m_bFunctionReturnType && !xObject.m_bFunctionReturnResolved)
		{
			string xError;
			Error::BuildErrorString(xError, xExpression, Error::ERROR_INTERNAL);
			s_xErrorList.push_back(xError);
			return false;
		}

		// If object is casted, check that the cast type exists
		const string& xCastType = xObject.m_xCastType;
		if (xCastType != "") //skip default types
		{
			if (!ResolveVariableType(xCastType, xExpression))
				return false;
		}

		size_t iFoundPosition = xExpression.m_xTemplate.find(xExpression.s_xTemplateVar);
		xExpression.m_xTemplate.replace(iFoundPosition, xExpression.s_xTemplateVar.length(), ((xCastType != "") ? xCastType : xObject.GetEndMemberType()));
	}

	// Template should have correct types now, get expression type
	s_bNeedToGetNextToken = true;
	CompilerTokenizer xTokenizer( xExpression.m_xTemplate, true );

	// All objects should have correct type now
	// Verify this expression type
	if (!VerifyExpression(xTokenizer, xExpression))
	{
		// Pop what might have gon einto the stack
		while (!s_xExpressionStack.empty())
		{
			s_xExpressionStack.pop();
		}
		return false;
	}
		
	// Expression type is in the stack now
	xExpression.m_xType = s_xExpressionStack.top();
	s_xExpressionStack.pop();

	return true;
}

// This should be called second
bool Resolver::ResolveAllFunctionCallArgumentsAndReturnTypes()
{
	for( unsigned int u = 0; u < s_xCallList.size(); ++u )
	{
		FunctionCall& xFunctionCall = s_xCallList[u];

		if( xFunctionCall.m_xReturnType != "" )
			continue; // Type is known

		if( !ResolveFunctionCallArgumentsAndReturnType( xFunctionCall ) )
			return false;
	}
	
	return true;
}

bool Resolver::ResolveFunctionCallArgumentsAndReturnType( FunctionCall& xFunctionCall )
{
	// Get the expression type
	for( unsigned int u = 0; u < xFunctionCall.m_xArgumentList.size(); ++u )
	{
		Expression& xArgument = xFunctionCall.m_xArgumentList[u];

		if( xArgument.m_xType != "" )
			continue; // This argument's expression has a known type

		// Resolve this expression
		if( !ResolveExpression( xArgument ) )
			return false;

		//continue to the other arguments now
	}

	// All good, we should have now all parameters as proper types;
	// Find the function
	string xFunctionName  = xFunctionCall.m_xVar.m_xType;
	xFunctionName        += ".";
	xFunctionName        += xFunctionCall.m_xCalledFunction;

	const Function* xFunction = s_xFunctionSymbolTable.GetSymbol( xFunctionName );
	if( xFunction )
	{
		// Check they have the same number of arguments
		if (xFunction->m_xArgumentList.size() != xFunctionCall.m_xArgumentList.size())
		{
			string xError;
			Error::BuildErrorString(xError, xFunctionCall, Error::ERROR_ARGUMENT_NUMBER, std::to_string(xFunctionCall.m_xArgumentList.size()).c_str(), std::to_string(xFunction->m_xArgumentList.size()).c_str());
			s_xErrorList.push_back(xError);
			return false;
		}

		// Check types now
		for( unsigned int u = 0; u < xFunction->m_xArgumentList.size(); ++u )
		{
			if (!IsTypeConform(xFunction->m_xArgumentList[u].m_xType, xFunctionCall.m_xArgumentList[u].m_xType))
			{
				string xError;
				Error::BuildErrorString(xError, xFunctionCall.m_xArgumentList[u], Error::ERROR_INVALID_TYPE, xFunctionCall.m_xArgumentList[u].m_xType.c_str(), xFunction->m_xArgumentList[u].m_xType.c_str());
				s_xErrorList.push_back(xError);
				return false;
			}	
		}

		// All is good, set this function type now
		xFunctionCall.m_xReturnType = xFunction->m_xReturnType;

		return true;
	}
	else
	{
		// Function not found
		string xError;
		Error::BuildErrorString(xError, xFunctionCall, Error::ERROR_UNRESOLVED_FUNCTION, xFunctionName.c_str());
		s_xErrorList.push_back(xError);
		return false;
	}
}

bool Resolver::ApplyOperator(const string& xOperator, string& xLeftObjectType, string& xRightObjectType)
{
	if( xOperator == "+" || xOperator == "-" )
	{
		xRightObjectType = s_xExpressionStack.top();
		s_xExpressionStack.pop();
		xLeftObjectType  = s_xExpressionStack.top();
		s_xExpressionStack.pop();

		if( xLeftObjectType == "int" || xLeftObjectType == "int_constant" || xLeftObjectType == "char" )
		{
			if( xRightObjectType == "int" || xRightObjectType == "int_constant" || xRightObjectType == "char" )// All good
			{		
				//Push resulting type
				if( xLeftObjectType == "char" || xRightObjectType == "char" )
					s_xExpressionStack.push("char");
				else
					s_xExpressionStack.push("int");

				return true;
			}
		}

		// allow pointer increase/decrease
		if (xRightObjectType == "int" || xRightObjectType == "int_constant")
		{
			if(xLeftObjectType != "null" && xLeftObjectType != "void")
				s_xExpressionStack.push(xLeftObjectType);
			else
				s_xExpressionStack.push("int");

			return true;
		}
		else if (xLeftObjectType == "int" || xLeftObjectType == "int_constant")
		{
			if (xOperator == "+")
			{
				if (xRightObjectType != "null" && xRightObjectType != "void")
					s_xExpressionStack.push(xRightObjectType);
				else
					s_xExpressionStack.push("int");

				return true;
			}
		}

		return false;
	}
	else if( xOperator == "neg" ) // unary
	{
		xLeftObjectType = s_xExpressionStack.top();
		s_xExpressionStack.pop();
		// applies only to it
		if (xLeftObjectType == "int" || xLeftObjectType == "int_constant")
		{
			s_xExpressionStack.push("int");
			return true;
		}

		// anything else is false. Object need to be casted to int before
		return false;
	}
	else if( xOperator == "*" || xOperator == "/" )
	{
		xRightObjectType = s_xExpressionStack.top();
		s_xExpressionStack.pop();
		xLeftObjectType  = s_xExpressionStack.top();
		s_xExpressionStack.pop();

		if( xLeftObjectType == "int" || xLeftObjectType == "int_constant" )
		{
			if( xRightObjectType == "int" || xRightObjectType == "int_constant"  )
			{
				s_xExpressionStack.push("int");
				return true;
			}
		}

		// anything else is false. Object need to be casted to int before
		return false;
	}
	else if( xOperator == "&" || xOperator == "|" )
	{
		xRightObjectType = s_xExpressionStack.top();
		s_xExpressionStack.pop();
		xLeftObjectType  = s_xExpressionStack.top();
		s_xExpressionStack.pop();

		if( xLeftObjectType == "int" || xRightObjectType == "int_constant"  )
		{
			if( xRightObjectType == "int" || xRightObjectType == "int_constant"  )
			{
				s_xExpressionStack.push("int");
				return true;
			}
		}

		if( xLeftObjectType == "boolean" || xLeftObjectType == "true" || xLeftObjectType == "false"  )
		{
			if( xRightObjectType == "boolean" || xRightObjectType == "true" || xRightObjectType == "false"  )
			{
				s_xExpressionStack.push("boolean");
				return true;
			}
		}

		if( IsUserType( xLeftObjectType ) && IsUserType( xRightObjectType ) )
		{
			return true;
		}

		// anything else is false. Object need to be casted to int before
		return false;
	}
	else if( xOperator == "~" )
	{
		xLeftObjectType = s_xExpressionStack.top();
		s_xExpressionStack.pop();

		// applies only to it
		if (xLeftObjectType == "boolean")
		{
			s_xExpressionStack.push("boolean");
			return true;
		}

		if (xLeftObjectType == "int" || xLeftObjectType == "int_constant")
		{
			s_xExpressionStack.push("int");
			return true;
		}

		// anything else is false. Object need to be casted to int before
		return false;
	}
	else if( xOperator == "<" || xOperator == ">"  || xOperator == "<=" || xOperator == ">=" )
	{
		xRightObjectType = s_xExpressionStack.top();
		s_xExpressionStack.pop();
		xLeftObjectType = s_xExpressionStack.top();
		s_xExpressionStack.pop();

		if (xLeftObjectType == "int" || xLeftObjectType == "int_constant" || xLeftObjectType == "char")
		{
			if (xRightObjectType == "int" || xRightObjectType == "int_constant" || xRightObjectType == "char")
			{
				s_xExpressionStack.push("boolean");
				return true;
			}
		}
		return false; // invalid, require cast
	}
	else if( xOperator == "=" )
	{
		xRightObjectType = s_xExpressionStack.top();
		s_xExpressionStack.pop();
		xLeftObjectType = s_xExpressionStack.top();
		s_xExpressionStack.pop();

		if( xLeftObjectType == "int" || xLeftObjectType == "int_constant" || xLeftObjectType == "char" )
		{
			if( xRightObjectType == "int" || xRightObjectType == "int_constant" || xRightObjectType == "char" )
			{
				s_xExpressionStack.push("boolean");
				return true;
			}

			return false; // invalid, require cast
		}
		else if( xLeftObjectType == "boolean" || xRightObjectType == "true" || xRightObjectType == "false" )
		{
			s_xExpressionStack.push("boolean");
			return true;
		}
		else if( IsUserType( xLeftObjectType ) )
		{
			if( xLeftObjectType == xRightObjectType || ( xRightObjectType == "null" ) ) // All good				
			{
				s_xExpressionStack.push("boolean");
				return true;
			}

			return false;
		}
	}

	return false;
}

bool Resolver::VerifyExpression( CompilerTokenizer& xTokenizer, const Location& xLocation )
{
	if (PushTermType( xTokenizer, xLocation))
	{
		// After the term, check 
		__pragma(warning(suppress:4127))
		while( true )
		{
			GetNextToken( xTokenizer );

			//is it an operator
			if (xTokenizer.GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL &&
				(xTokenizer.GetTokenValue() == "+" || xTokenizer.GetTokenValue() == "-" || xTokenizer.GetTokenValue() == "*" || xTokenizer.GetTokenValue() == "/" || xTokenizer.GetTokenValue() == "&" || xTokenizer.GetTokenValue() == "|" || xTokenizer.GetTokenValue() == "<" || xTokenizer.GetTokenValue() == ">" || xTokenizer.GetTokenValue() == "="))
			{

				string xOperator = xTokenizer.GetTokenValue(); //save operator as the value will change

				if (xTokenizer.GetTokenValue() == "<" || xTokenizer.GetTokenValue() == ">")
				{
					//Get next token and check if it is the equal sign
					GetNextToken(xTokenizer);

					if (xTokenizer.GetTokenType() == CompilerTokenizer::TOKEN_SYMBOL && xTokenizer.GetTokenValue() == "=")
						xOperator += xTokenizer.GetTokenValue();
					else
						s_bNeedToGetNextToken = false;
				}

				if (PushTermType(xTokenizer, xLocation))
				{
					string xLeftObjectType;
					string xRightObjectType;
					if (!ApplyOperator(xOperator, xLeftObjectType, xRightObjectType))
					{
						string xError;
						Error::BuildErrorString(xError, xLocation, Error::ERROR_OPERATOR_TYPE );
						s_xErrorList.push_back(xError);
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
		s_bAddressExpression = false;
		return false;
	}
}

bool Resolver::PushTermType( CompilerTokenizer& xTokenizer, const Location& xLocation )
{
	GetNextToken( xTokenizer );
	switch (xTokenizer.GetTokenType())
	{
		case CompilerTokenizer::TOKEN_KEYWORD:
		{
			CompilerTokenizer::KEYWORD_TYPE eType = xTokenizer.GetKeywordType(xTokenizer.GetTokenValue());
			switch (eType)
			{
			case CompilerTokenizer::KEYWORD_INT:
			case CompilerTokenizer::KEYWORD_BOOLEAN:
			case CompilerTokenizer::KEYWORD_CHAR:
			case CompilerTokenizer::KEYWORD_VOID:
			case CompilerTokenizer::KEYWORD_NULL:
			case CompilerTokenizer::KEYWORD_TRUE:
			case CompilerTokenizer::KEYWORD_FALSE:
			{
				if( s_bAddressExpression )
					s_xExpressionStack.push(CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_INT));
				else
					s_xExpressionStack.push(CompilerTokenizer::GetKeywordString(eType));
				return true;
			}
			case CompilerTokenizer::KEYWORD_ADDRESS:
			{
				s_bAddressExpression = true;
				if (PushTermType(xTokenizer, xLocation))
				{
					s_bAddressExpression = false;
					return true;
				}

				return false;
			}
			default:
				string xError;
				Error::BuildErrorString(xError, xLocation, Error::ERROR_INTERNAL);
				s_xErrorList.push_back(xError);
				return false;
			}
		}
		case CompilerTokenizer::TOKEN_INDENTIFIER:
		{
			if (s_bAddressExpression)
				s_xExpressionStack.push(CompilerTokenizer::GetKeywordString(CompilerTokenizer::KEYWORD_INT));
			else
				s_xExpressionStack.push(xTokenizer.GetTokenValue()); // class names, int_constant, string_constant

			return true;
		}
		case CompilerTokenizer::TOKEN_SYMBOL:
		{
			if (xTokenizer.GetTokenValue() == "(")
			{
				return VerifyExpression(xTokenizer, xLocation);
			}
			else if (xTokenizer.GetTokenValue() == "-" || xTokenizer.GetTokenValue() == "~")
			{
				string xUnaryOPValue = xTokenizer.GetTokenValue();
				if (PushTermType(xTokenizer, xLocation))
				{
					string xUnaryOpreator = (xUnaryOPValue == "-") ? "neg" : xUnaryOPValue;
					// Apply the operator here

					string xLeftObjectType;
					string xRightObjectType;
					if (!ApplyOperator(xUnaryOpreator, xLeftObjectType, xRightObjectType))
					{
						string xError;
						Error::BuildErrorString(xError, xLocation, Error::ERROR_OPERATOR_TYPE);
						s_xErrorList.push_back(xError);
						return false;
					}
					return true;
				}
			}
			else
			{
				string xError;
				Error::BuildErrorString(xError, xLocation, Error::ERROR_INTERNAL);
				s_xErrorList.push_back(xError);
				return false;
			}
		}
		default:
			string xError;
			Error::BuildErrorString(xError, xLocation, Error::ERROR_INTERNAL);
			s_xErrorList.push_back(xError);
			return false;
	}
}

bool Resolver::IsUserType( const string& xType )
{
	if(    xType == "int"  || xType == "boolean" || xType == "char" || xType == "void" ||  xType == "null"
		|| xType == "true" || xType == "false" )
		return false;

	return true;
}

bool Resolver::IsTypeConform( const string& xExpectedType, const string& xUsedType )
{
	if( xExpectedType != xUsedType )
	{
		// If function type is string, and the param is string constant accept it
		if ((xExpectedType == "String") && (xUsedType == "string_constant"))
			return true;

		// If it is an identifier, null as a type is permitted, check it
		if( IsUserType( xExpectedType ) && xUsedType == "null" )
			return true;

		// If function type is boolean, and the param is true or false, this is permitted
		if( ( xExpectedType == "boolean") && ( xUsedType == "true" || xUsedType == "false" ) )
			return true;

		// If function type is int, and the param is int constant accept it
		if ((xExpectedType == "int") && (xUsedType == "int_constant" || xExpectedType == "char"))
			return true;

		// Temporary accept assignment between char and int
		if ((xExpectedType == "char") && (xUsedType == "int" || xUsedType == "int_constant"))
			return true;

		return false;
	}
	else
	{
		return true;
	}
}

bool Resolver::WriteVirtualFunctionCallsVMCode()
{
	for (unsigned int u = 0; u < s_xCallList.size(); ++u)
	{
		FunctionCall& xFunctionCall = s_xCallList[u];

		if (xFunctionCall.m_bFieldCall || xFunctionCall.m_bStaticCall) // Filed calls are handled in the resolve member access part
			continue;

		string xFunctionName = xFunctionCall.m_xVar.m_xType;
		xFunctionName += ".";
		xFunctionName += xFunctionCall.m_xCalledFunction;

		const Function* pxFunction = s_xFunctionSymbolTable.GetSymbol(xFunctionName);
		if (pxFunction)
		{
			// Check if virtual
			if (pxFunction->m_bVirtual)
			{
				// Write the virtual function code;
				string xVFCode = "push ";

				// Check the variable segment
				switch (xFunctionCall.m_xVar.m_eSegment)
				{
				case Variable::SEGMENT_STATIC:
					xVFCode += "static ";
					break;
				case Variable::SEGMENT_FIELD:
					xVFCode += "this ";
					break;
				case Variable::SEGMENT_ARGUMENT:
					xVFCode += "argument ";
					break;
				case Variable::SEGMENT_LOCAL:
					xVFCode += "local ";
					break;
				default:
					string xError;
					Error::BuildErrorString(xError, xFunctionCall, Error::ERROR_INTERNAL);
					s_xErrorList.push_back(xError);
					return false;
				}
					
				// add index
				xVFCode += to_string(xFunctionCall.m_xVar.m_uIndex);
				xVFCode += '\n';

				xVFCode += "callv ";
				xVFCode += to_string(xFunctionCall.m_xArgumentList.size() + 1); //1 this pointer
				xVFCode += " ";
				xVFCode += to_string(pxFunction->m_uIndex);
				xVFCode += '\n';

				// Replace the code line now
				return ReplaceVMMarkerWithVMCode(xFunctionCall, xFunctionCall.m_xVMCode, xVFCode);
			}
		}
		else
		{
			string xError;
			Error::BuildErrorString(xError, xFunctionCall, Error::ERROR_INTERNAL);
			s_xErrorList.push_back(xError);
			return false;
		}
	}

	return true;
}

bool Resolver::WriteAllMemberAccessVMCode()
{	
	for (unsigned int u = 0; u < s_xReturnList.size(); ++u)
	{
		if (!WriteAllExpressionMemberAccessVMCode(s_xReturnList[u], false))
			return false;
	}

	for (unsigned int u = 0; u < s_xConditionList.size(); ++u)
	{
		if (!WriteAllExpressionMemberAccessVMCode(s_xConditionList[u], false))
			return false;
	}

	for (unsigned int u = 0; u < s_xArrayIndexList.size(); ++u)
	{
		if (!WriteAllExpressionMemberAccessVMCode(s_xArrayIndexList[u], false))
			return false;
	}

	for (unsigned int u = 0; u < s_xAssignmentList.size(); ++u)
	{
		if (!WriteAllExpressionMemberAccessVMCode(s_xAssignmentList[u].m_xRightExpression, false))
			return false;
	}

	for (unsigned int u = 0; u < s_xAssignmentList.size(); ++u)
	{
		if (!WriteAllExpressionMemberAccessVMCode(s_xAssignmentList[u].m_xLeftExpression, true))
			return false;
	}

	return true;
}

bool Resolver::WriteAllExpressionMemberAccessVMCode(Expression& xExpression, bool bAssignment)
{
	for (unsigned int v = 0; v < xExpression.m_xObjectList.size(); ++v)
	{
		Object& xObject = xExpression.m_xObjectList[v];
		if (xObject.m_bFieldAccess)
			if (!WriteMemberAccessVMCode(xObject, xExpression, bAssignment))
				return false;
	}

	return true;
}

bool Resolver::WriteMemberAccessVMCode(const Object& xObject, const Location& xLocation, bool bAssignment)
{
	if (xObject.m_bStatic)
	{
		// Get the last member
		const Object& xMember = xObject.m_xMemberList[xObject.m_xMemberList.size() - 1];
		if (xMember.m_bFunctionReturnType)
			return true; // Nothing to resolve here

		string xVMCode;
		if (bAssignment)
			xVMCode = "pop static ";
		else
			xVMCode = "push static ";

		unsigned int uIndex = xMember.m_xResolvedVar.m_uIndex;

		xVMCode += to_string(uIndex);

		xVMCode += " ";

		xVMCode += xObject.m_xName; // or type, they should be the same

		xVMCode += '\n';

		return ReplaceVMMarkerWithVMCode(xLocation, xMember.m_xVMMarker, xVMCode);
	}
	else
	{
		// if a function call of the first object is not referenced, return as this call is already resolved
		if ((xObject.m_xMemberList.size() == 1) && (xObject.m_xMemberList[0].m_bFunctionReturnType))
			return true;

		// Fields now excluding the last one
		for (unsigned int u = 0; u < xObject.m_xMemberList.size() - 1; ++u)
		{
			string xVMCode;

			// Save this of the context object in the temp memory segment
			if (u == 0)
			{
				xVMCode = "push pointer 0\npop temp 0\n";

				// If it is an assigment, top of stack has result value, put it in a temporary segment
				if (bAssignment)
					xVMCode += "pop temp 1\n";
			}

			// The current object should have been pushed already
			// Set it as the this
			xVMCode += "pop pointer 0\n";

			const Object& xMember = xObject.m_xMemberList[u];

			// Push the parent object
			// Is it a member object or member function?
			if (!xMember.m_bFunctionReturnType)
			{	// Object
				xVMCode += "push this ";
				xVMCode += to_string(xMember.m_xResolvedVar.m_uIndex);
				xVMCode += "\n";
			}
			else
			{
				//Function, call this function here, it will leave the return value in the stack
				FunctionCall& xFunctionCall = s_xCallList[xMember.m_uFunctionCallIndex];
				string xFunctionName = xFunctionCall.m_xVar.m_xType;
				xFunctionName += ".";
				xFunctionName += xFunctionCall.m_xCalledFunction;

				const Function* pxFunction = s_xFunctionSymbolTable.GetSymbol(xFunctionName);
				if (pxFunction)
				{
					// Check if virtual
					if (!pxFunction->m_bVirtual)
					{

						xVMCode += "call ";
						xVMCode += xFunctionName;
						xVMCode += " ";
						xVMCode += to_string(xFunctionCall.m_xArgumentList.size() + 1);
						xVMCode + "\n";
					}
					else
					{
						// Virtual call, push this pointer
						xVMCode += "push pointer 0\n";

						xVMCode += "callv ";
						xVMCode += to_string(xFunctionCall.m_xArgumentList.size() + 1); //1 this pointer
						xVMCode += " ";
						xVMCode += to_string(pxFunction->m_uIndex);
						xVMCode += '\n';
					}
				}
				else
				{
					string xError;
					Error::BuildErrorString(xError, xLocation, Error::ERROR_INTERNAL);
					s_xErrorList.push_back(xError);
					return false;
				}
			}

			// Replace the field marker now with the actual code
			// Get the VM file
			const VMCode* pxVMCode = s_xVMCodeTable.GetSymbol(xLocation.m_xFile);
			if (pxVMCode)
			{
				size_t iFoundPosition = pxVMCode->m_xCode.find(xMember.m_xVMMarker);
				if (iFoundPosition != std::string::npos)
					const_cast<VMCode*>(pxVMCode)->m_xCode.replace(iFoundPosition, xMember.m_xVMMarker.length(), xVMCode);
				else
				{
					string xError;
					Error::BuildErrorString(xError, xLocation, Error::ERROR_INTERNAL);
					s_xErrorList.push_back(xError);
					return false;
				}
			}
			else
			{
				string xError;
				Error::BuildErrorString(xError, xLocation, Error::ERROR_INTERNAL);
				s_xErrorList.push_back(xError);
				return false;
			}
		}

		string xVMCode;
		if (xObject.m_xMemberList.size() == 1) // If only one member, save the context and result here
		{
			xVMCode += "push pointer 0\npop temp 0\n";
			// If it is an assigment, top of stack has result value, put it in a temporary segment
			if (bAssignment)
				xVMCode += "pop temp 1\n";
		}

		// Last member now, pop parent into place to set the this
		xVMCode += "pop pointer 0\n";

		// Get the last member
		const Object& xMember = xObject.m_xMemberList[xObject.m_xMemberList.size() - 1];

		// If it is an assigment, pop the value into it, else push it into the stack
		if (bAssignment)
		{
			xVMCode += "push temp 1\n";
			xVMCode += "pop this ";
		}
		else
			xVMCode += "push this ";

		xVMCode += to_string(xMember.m_xResolvedVar.m_uIndex);
		xVMCode += "\n";

		// Restore current context now
		xVMCode += "push temp 0\npop pointer 0";

		return ReplaceVMMarkerWithVMCode(xLocation, xMember.m_xVMMarker, xVMCode);
	}
}

bool Resolver::ReplaceVMMarkerWithVMCode(const Location& xLocation, const string& xMarker, const string& xVMCode)
{
	// Replace the field marker now with the actual code
	// Get the VM file
	const VMCode* pxVMCode = s_xVMCodeTable.GetSymbol(xLocation.m_xFile);
	if (pxVMCode)
	{
		size_t iFoundPosition = pxVMCode->m_xCode.find(xMarker);
		if (iFoundPosition != std::string::npos)
			const_cast<VMCode*>(pxVMCode)->m_xCode.replace(iFoundPosition, xMarker.length(), xVMCode);
		else
		{
			string xError;
			Error::BuildErrorString(xError, xLocation, Error::ERROR_INTERNAL);
			s_xErrorList.push_back(xError);
			return false;
		}
	}
	else
	{
		string xError;
		Error::BuildErrorString(xError, xLocation, Error::ERROR_INTERNAL);
		s_xErrorList.push_back(xError);
		return false;
	}

	return true;
}

bool Resolver::WriteAllFunctionLocalsVMCode()
{
	for (unordered_map<string, const Function>::const_iterator it = s_xFunctionSymbolTable.GetTable().begin(); it != s_xFunctionSymbolTable.GetTable().end(); ++it)
	{
		// Replace the field marker now with the actual code
		// Get the VM file
		const Function& xFunction = it->second;
		const VMCode* pxVMCode = s_xVMCodeTable.GetSymbol(xFunction.m_xFile);
		if (pxVMCode)
		{
			size_t iFoundPosition = pxVMCode->m_xCode.find(xFunction.m_xVMMarker);
			if (iFoundPosition != std::string::npos)
				const_cast<VMCode*>(pxVMCode)->m_xCode.replace(iFoundPosition, xFunction.m_xVMMarker.length(), to_string(xFunction.m_uNumberOfLocals));
			else
			{
				string xError;
				Error::BuildErrorString(xError, xFunction, Error::ERROR_INTERNAL);
				s_xErrorList.push_back(xError);
				return false;
			}
		}
		else
		{
			string xError;
			Error::BuildErrorString(xError, xFunction, Error::ERROR_INTERNAL);
			s_xErrorList.push_back(xError);
			return false;
		}
	}

	return true;
}

bool Resolver::WriteVMFiles(bool bUnresolved /*=false*/)
{
	for (unordered_map<string, const VMCode>::const_iterator it = s_xVMCodeTable.GetTable().begin(); it != s_xVMCodeTable.GetTable().end(); ++it)
	{
		const string& xFileName = it->second.m_xName;
		const string& xVMCode   = it->second.m_xCode;
		
		string xFileNameWithoutExtension = xFileName.substr(0, xFileName.length() - 5);

		ofstream xVMFile;
		xVMFile.open( xFileNameWithoutExtension + (bUnresolved ? ".vo" : ".vm") );
		if (xVMFile.fail())
		{
			cout << "file opening failed" << endl;
			return false;
		}

		xVMFile << xVMCode;
		xVMFile.close();
	}

	return false;
}

bool Resolver::GetNextToken( CompilerTokenizer& xTokenizer )
{
	if ( s_bNeedToGetNextToken )
	{
		return xTokenizer.GetNextToken();
	}
	else
	{
		s_bNeedToGetNextToken = true;
		return true;
	}
}

void Resolver::PrintErrors()
{
	for (unsigned int u = 0; u < s_xErrorList.size(); ++u)
	{
		// print out error
		cout << s_xErrorList[u];
	}
}