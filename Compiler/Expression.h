#ifndef _EXPRESSION_
#define _EXPRESSION_

#include <string>
#include <vector>
#include "Object.h"
#include "Location.h"

using namespace std;

class Expression : public Location
{
public:
	vector< Object > m_xObjectList;
	string           m_xType;
	string           m_xTemplate;     // Used in the resolver to check the types
	
	static string s_xTemplateVar;     // Compiler used only identifier
	static string s_xTemplatePointer; // Compiler used only identifier
};

#endif //_VARIABLE_