#ifndef _VARIABLE_
#define _VARIABLE_

#include <string>
#include "Location.h"

using namespace std;

class Variable : public Location
{
public:
	enum SEGMENT
	{
		SEGMENT_INVALID,
		SEGMENT_STATIC,
		SEGMENT_FIELD,
		SEGMENT_ARGUMENT,
		SEGMENT_LOCAL
	};

	string		 m_xName;
	string		 m_xType;
	SEGMENT      m_eSegment;
	unsigned int m_uIndex;

	const char* GetSegmentString() const
	{
		switch (m_eSegment)
		{
			case SEGMENT_STATIC:
				return "static";
			case SEGMENT_FIELD:
				return "field";
			case SEGMENT_ARGUMENT:
				return "argument";
			case SEGMENT_LOCAL:
				return "local";

			default:
				return "invalid segment";
		}
	}
};

#endif //_VARIABLE_