#ifndef _PARSER_
#define _PARSER_

#include <fstream>
#include <string>
#include "Code.h"
#include "VMTokenizer.h"

class Parser
{
public:
	Parser(const std::string& xFilename, VMTokenizer& xTokenizer, Code& xCode);
	~Parser();

	bool Compile();
private:
	bool				IsKeywordCommand(const std::string& xKeyword) const;
	VMTokenizer& m_xTokenizer;
	Code&        m_xCode;
	std::string		 m_xError;
	std::ifstream	 m_xFile;
	
	std::string m_xSourceFileNameWithoutExtention;
	std::string m_xCurrentFunctionName;
};

#endif //_PARSER_