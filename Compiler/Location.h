#ifndef _LOCATION_
#define _LOCATION_

#include <string>

using namespace std;

class Location
{
public:
	string			   m_xFile;
	string			   m_xClass;
	string             m_xFunction;
	unsigned int	   m_uLine;
};

#endif //_LOCATION_