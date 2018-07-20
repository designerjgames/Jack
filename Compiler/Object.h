#ifndef _ARGUMENT_
#define _ARGUMENT_

#include <string>
#include <vector>
#include "Variable.h"

using namespace std;

class Object
{
public:
	bool           m_bFunctionReturnResolved;
	bool           m_bFunctionReturnType; // With function return type, we might access a field if function returns object
	string         m_xCastType;
	string         m_xType;               // Type as string
	unsigned int   m_uFunctionCallIndex;
	bool           m_bFieldAccess;        // If accessing a field, might a function return with field access too...
	string         m_xVMMarker;
	string         m_xName;
	unsigned int   m_uDimension;          // Array dimension, 0 if not an array
	vector<Object> m_xMemberList;
	bool           m_bTakingAddress;      // Taking address of object or function
	Variable       m_xResolvedVar;         // For fields
	bool           m_bStatic;

	string         GetEndMemberType() 
	{ 
		return m_xMemberList.size() > 0 ? m_xMemberList[m_xMemberList.size() - 1].m_xType : m_xType; 
	}

	// When accessing a field, in case of function, the return type address is on the stack, if not function we need to push the type address to the stack
	// then, add indexes, pop this, handle arrays etc...
};

#endif //_VARIABLE_