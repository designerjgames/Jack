#ifndef _PARSER_
#define _PARSER_

#include <fstream>
#include <string>
#include "Code.h"

using namespace std;

class Parser
{
public:
	enum LINE_TYPE
	{
		LINE_EMPTY,
		LINE_COMMAND,
		LINE_COMMENT
	};

	enum COMMAND_TYPE
	{
		COMMAND_A,
		COMMAND_C,
		COMMAND_L,
		COMMAND_ERROR
	};

	Parser( const string& xFilename );
	~Parser();

	bool AdvanceToNextCommand();
	void ResetStreamReadingPosition();

	COMMAND_TYPE		   GetCommandType() const;
	string				   GetSymbol()      const;
	Code::DESTINATION_TYPE GetDestination() const;
	Code::COMPUTATION_TYPE GetComputation() const;
	Code::JUMP_TYPE        GetJump()        const;
	const string&          GetCommand()     const { return m_xCommand; }

private:
	bool IsNumber(const string& xSymbol)        const;
	bool IsSymbolConform(const string& xSymbol) const;

	LINE_TYPE CheckLine();

	string   m_xCommand;
	string   m_xError;
	ifstream m_xFile;
};

#endif //_PARSER_