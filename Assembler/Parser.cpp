#include "Parser.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <regex>

Parser::Parser( const string& xFilename )
{
	m_xFile.open(xFilename);
	if (m_xFile.fail())
	{
		cout << "file opening failed" << endl;
	} 
}

Parser::~Parser()
{
	m_xFile.close();
}

Parser::LINE_TYPE Parser::CheckLine()
{
	//Remove all space from the command
	m_xCommand.erase(remove_if(m_xCommand.begin(), m_xCommand.end(), isspace), m_xCommand.end());

	// Check if there is a comment AFTER the command and remove it
	size_t iCommentfound = m_xCommand.find("//");
	if( iCommentfound != std::string::npos && iCommentfound != 0 )
	{
		// we have a comment, dismiss it
		m_xCommand = m_xCommand.substr(0, iCommentfound);
	}

	//Check if empty
	if (m_xCommand.length() == 0)
	{
		return LINE_EMPTY;
	}
	
	//Get the first char
	if (m_xCommand[0] == '/')
	{
		//check second char, if it's "/" then comment, else it's a command
		if (m_xCommand.length() >= 2)
		{
			if (m_xCommand[1] == '/')
			{
				return LINE_COMMENT;
			}
		}
		return LINE_COMMAND;
	}

	//Looks to be a command
	return LINE_COMMAND;
}

bool Parser::IsNumber(const string& xSymbol) const
{
    std::string::const_iterator it = xSymbol.begin();
    while (it != xSymbol.end() && std::isdigit(*it)) ++it;
    return !xSymbol.empty() && it == xSymbol.end();
}

bool Parser::IsSymbolConform(const string& xSymbol) const
{
	if(IsNumber(m_xCommand.substr(1, m_xCommand.length() - 1)))
	{
		return true;
	}

	// It shouldn't start with a number
	if(isdigit(xSymbol[0]))
	{
		return false;
	}

	std::regex xRgx("[a-zA-Z0-9_.$:]+");
    if (!std::regex_match (xSymbol,xRgx))
    {
		return false;
    }

    return true;
}

Parser::COMMAND_TYPE Parser::GetCommandType() const
{
	//Get the first char
	if (m_xCommand[0] == '@')
	{
		//A command
		// Make sure it is conform either number or doesn't start with digit
		if( IsSymbolConform(m_xCommand.substr(1, m_xCommand.length() - 1)) )
		{
			return COMMAND_A;
		}
		// Error
		else
		{
			return COMMAND_ERROR;
		}
	}
	else if (m_xCommand[0] == '(' && m_xCommand[m_xCommand.length() - 1] == ')')
	{
		//L command
		// Make sure it is conform either number or doesn't start with digit
		if( IsSymbolConform(m_xCommand.substr(1, m_xCommand.length() - 2)) )
		{
			return COMMAND_L;
		}
		// Error
		else
		{
			return COMMAND_ERROR;
		}
	}
	else
	{
		//C command
		return COMMAND_C;
	}
}

string Parser::GetSymbol() const
{
	if (GetCommandType() == COMMAND_A)
	{
		return m_xCommand.substr(1, m_xCommand.length() - 1);
	}
	else // (GetCommandType() == COMMAND_L)
	{
		return m_xCommand.substr(1, m_xCommand.length() - 2);
	}
}

Code::DESTINATION_TYPE Parser::GetDestination() const
{
	size_t ifound = m_xCommand.find("=");
	if (ifound == std::string::npos)
	{
		return Code::DEST_NULL;
	}

	// = found, get destination
	bool bAfound = false;
	bool bDfound = false;
	bool bMfound = false;
	string::const_iterator xIt = m_xCommand.begin();
	while (*xIt != '=')
	{
		if (*xIt == 'A')
		{
			if (bAfound)
			{
				// Another A, error
				return Code::DEST_ERROR;
			}
			bAfound = true;
		}
		else if (*xIt == 'D')
		{
			if (bDfound)
			{
				// Another D, error
				return Code::DEST_ERROR;
			}
			bDfound = true;
		}
		else if (*xIt == 'M')
		{
			if (bMfound)
			{
				// Another M, error
				return Code::DEST_ERROR;
			}
			bMfound = true;
		}
		else
		{
			// non supported destination
			return Code::DEST_ERROR;
		}
		++xIt;
	}

	int iDest = 0;
	if (bAfound)
	{
		iDest |= Code::DEST_A;
	}
	if (bDfound)
	{
		iDest |= Code::DEST_D;
	}
	if (bMfound)
	{
		iDest |= Code::DEST_M;
	}

	return static_cast<Code::DESTINATION_TYPE>(iDest);
}

Code::COMPUTATION_TYPE Parser::GetComputation() const
{
	size_t iEQfound    = m_xCommand.find("=");
	size_t iCOMMAfound = m_xCommand.find(";");

	if ( (iEQfound == std::string::npos) && (iCOMMAfound == std::string::npos))
	{
		// We should have a computation
		return Code::COMP_ERROR;
	}

	int iCompStartIndex = 0;
	int iCompLength     = m_xCommand.length();

	if (iEQfound != std::string::npos)
	{
		iCompStartIndex = iEQfound + 1;
	}

	if (iCOMMAfound != std::string::npos)
	{
		iCompLength = iCOMMAfound - iCompStartIndex;
	}

	if ( (iCompLength <= 0) )
	{
		return Code::COMP_ERROR;
	}

	string xComputation = m_xCommand.substr(iCompStartIndex, iCompLength);

	//Check what computation, 28 possibilities

	if (xComputation == "0")
	{
		return Code::COMP_0;
	}
	else if (xComputation == "1")
	{
		return Code::COMP_1;
	}
	else if (xComputation == "-1")
	{
		return Code::COMP_NEG_1;
	}
	else if (xComputation == "D")
	{
		return Code::COMP_D;
	}
	else if (xComputation == "A")
	{
		return Code::COMP_A;
	}
	else if (xComputation == "!D")
	{
		return Code::COMP_NOT_D;
	}
	else if (xComputation == "0")
	{
		return Code::COMP_NOT_A;
	}
	else if (xComputation == "0")
	{
		return Code::COMP_NEG_D;
	}
	else if (xComputation == "!A")
	{
		return Code::COMP_NEG_A;
	}
	else if ((xComputation == "D+1") || (xComputation == "1+D"))
	{
		return Code::COMP_D_ADD_1;
	}
	else if ((xComputation == "A+1") || (xComputation == "1+A"))
	{
		return Code::COMP_A_ADD_1;
	}
	else if (xComputation == "D-1")
	{
		return Code::COMP_D_SUB_1;
	}
	else if (xComputation == "A-1")
	{
		return Code::COMP_A_SUB_1;
	}
	else if ((xComputation == "D+A") || (xComputation == "A+D"))
	{
		return Code::COMP_D_ADD_A;
	}
	else if (xComputation == "D-A")
	{
		return Code::COMP_D_SUB_A;
	}
	else if (xComputation == "A-D")
	{
		return Code::COMP_A_SUB_D;
	}
	else if ((xComputation == "D&A") || (xComputation == "A&D"))
	{
		return Code::COMP_D_AND_A;
	}
	else if ((xComputation == "D|A") || (xComputation == "A|D"))
	{
		return Code::COMP_D_OR_A;
	}
	else if (xComputation == "M")
	{
		return Code::COMP_M;
	}
	else if (xComputation == "!M")
	{
		return Code::COMP_NOT_M;
	}
	else if (xComputation == "-M")
	{
		return Code::COMP_NEG_M;
	}
	else if ((xComputation == "M+1") || (xComputation == "1+M"))
	{
		return Code::COMP_M_ADD_1;
	}
	else if (xComputation == "M-1")
	{
		return Code::COMP_M_SUB_1;
	}
	else if ((xComputation == "D+M") || (xComputation == "M+D"))
	{
		return Code::COMP_D_ADD_M;
	}
	else if (xComputation == "D-M")
	{
		return Code::COMP_D_SUB_M;
	}
	else if (xComputation == "M-D")
	{
		return Code::COMP_M_SUB_D;
	}
	else if ((xComputation == "D&M") || (xComputation == "M&D"))
	{
		return Code::COMP_D_AND_M;
	}
	else if ((xComputation == "D|M") || (xComputation == "M|D"))
	{
		return Code::COMP_D_OR_M;
	}
	else
	{
		return Code::COMP_ERROR;
	}
}

Code::JUMP_TYPE Parser::GetJump() const
{
	size_t iCOMMAfound = m_xCommand.find(";");

	if ((iCOMMAfound == std::string::npos))
	{
		return Code::JUMP_NULL;
	}

	int iCompStartIndex = iCOMMAfound + 1;
	int iCompEndIndex   = m_xCommand.length();

	if ((iCompStartIndex < 0) || (iCompStartIndex > iCompEndIndex))
	{
		return Code::JUMP_ERROR;
	}

	string xJump = m_xCommand.substr(iCompStartIndex, iCompEndIndex);

	//Check what jump, 6 possibilities
	if (xJump == "JGT")
	{
		return Code::JUMP_JGT;
	}
	else if (xJump == "JEQ")
	{
		return Code::JUMP_JEQ;
	}
	else if (xJump == "JGE")
	{
		return Code::JUMP_JGE;
	}
	else if (xJump == "JLT")
	{
		return Code::JUMP_JLT;
	}
	else if (xJump == "JNE")
	{
		return Code::JUMP_JNE;
	}
	else if (xJump == "JLE")
	{
		return Code::JUMP_JLE;
	}
	else if (xJump == "JMP")
	{
		return Code::JUMP_JMP;
	}
	else
	{
		return Code::JUMP_ERROR;
	}
}

void Parser::ResetStreamReadingPosition()
{
	m_xFile.clear(); // clear fail and eof bits
	m_xFile.seekg(0, ios::beg);
}

bool Parser::AdvanceToNextCommand()
{
	LINE_TYPE eLineType;
	while (!m_xFile.eof())
	{
		m_xCommand = "";
		getline(m_xFile, m_xCommand);

		eLineType = CheckLine();

		switch (eLineType)
		{
		case LINE_EMPTY:
		case LINE_COMMENT:
			continue;

		case LINE_COMMAND:
			return true;
		}
	}

	// EOF
	return false;
}