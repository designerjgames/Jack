#ifndef _ASSIGNMENT_
#define _ASSIGNMENT_

#include "Object.h"
#include "Expression.h"
#include "Location.h"

using namespace std;

class Assignment : public Location
{
public:
	Expression m_xLeftExpression;
	Expression m_xRightExpression;
};

#endif //_ASSIGNMENT_