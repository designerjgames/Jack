#ifndef _FUNCTION_CALL_
#define _FUNCTION_CALL_

#include <string>
#include <vector>
#include "Expression.h"
#include "Location.h"
#include "Variable.h"

using namespace std;

class FunctionCall : public Location
{
public:
	bool                 m_bStaticCall; // True if static call, false if from an object (static function should be called statically)
	Variable		     m_xVar;
	string               m_xCalledFunction;
	vector< Expression > m_xArgumentList;
	string               m_xReturnType;  //Resolved by the resolver
	string               m_xVMCode;
	bool                 m_bFieldCall;
};

#endif //_VARIABLE_