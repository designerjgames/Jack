#ifndef _FUNCTION_
#define _FUNCTION_

#include <string>
#include <vector>

#include "Location.h"
#include "CompilerTokenizer.h"
#include "Variable.h"

using namespace std;

class Function : public Location
{
public:
	CompilerTokenizer::KEYWORD_TYPE m_eType;		 // function, method, constructor
	string                  m_xReturnType;   // return type
	string                  m_xName;         // Class + "." + function
	vector< Variable >      m_xArgumentList;
	bool                    m_bVirtual;
	unsigned int            m_uIndex;    
	unsigned int            m_uNumberOfLocals;
	string                  m_xVMMarker;
};

#endif //_VARIABLE_