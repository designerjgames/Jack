#include "Code.h"

#include <bitset>
#include <iostream>
#include <cctype>
#include "Parser.h"

Code::Code( const string& xFilename )
: m_uRamAddress( USER_RAM_START )
{
	m_xComputation[COMP_0]       = "1110101010";
	m_xComputation[COMP_1]       = "1110111111";
	m_xComputation[COMP_NEG_1]   = "1110111010";
	m_xComputation[COMP_D]       = "1110001100";
	m_xComputation[COMP_A]       = "1110110000";
	m_xComputation[COMP_NOT_D]   = "1110001101";
	m_xComputation[COMP_NOT_A]   = "1110110001";
	m_xComputation[COMP_NEG_D]   = "1110001111";
	m_xComputation[COMP_NEG_A]   = "1110110011";
	m_xComputation[COMP_D_ADD_1] = "1110011111";
	m_xComputation[COMP_A_ADD_1] = "1110110111";
	m_xComputation[COMP_D_SUB_1] = "1110001110";
	m_xComputation[COMP_A_SUB_1] = "1110110010";
	m_xComputation[COMP_D_ADD_A] = "1110000010";
	m_xComputation[COMP_D_SUB_A] = "1110010011";
	m_xComputation[COMP_A_SUB_D] = "1110000111";
	m_xComputation[COMP_D_AND_A] = "1110000000";
	m_xComputation[COMP_D_OR_A]  = "1110010101";
	m_xComputation[COMP_M]       = "1111110000";
	m_xComputation[COMP_NOT_M]   = "1111110001";
	m_xComputation[COMP_NEG_M]   = "1111110011";
	m_xComputation[COMP_M_ADD_1] = "1111110111";
	m_xComputation[COMP_M_SUB_1] = "1111110010";
	m_xComputation[COMP_D_ADD_M] = "1111000010";
	m_xComputation[COMP_D_SUB_M] = "1111010011";
	m_xComputation[COMP_M_SUB_D] = "1111000111";
	m_xComputation[COMP_D_AND_M] = "1111000000";
	m_xComputation[COMP_D_OR_M]  = "1111010101";

	m_xDestination[DEST_NULL] = "000";
	m_xDestination[DEST_M]    = "001";
	m_xDestination[DEST_D]    = "010";
	m_xDestination[DEST_MD]   = "011";
	m_xDestination[DEST_A]    = "100";
	m_xDestination[DEST_AM]   = "101";
	m_xDestination[DEST_AD]   = "110";
	m_xDestination[DEST_AMD]  = "111";

	m_xJump[JUMP_NULL] = "000";
	m_xJump[JUMP_JGT]  = "001";
	m_xJump[JUMP_JEQ]  = "010";
	m_xJump[JUMP_JGE]  = "011";
	m_xJump[JUMP_JLT]  = "100";
	m_xJump[JUMP_JNE]  = "101";
	m_xJump[JUMP_JLE]  = "110";
	m_xJump[JUMP_JMP]  = "111";

	m_xBinaryFile.open( xFilename.substr(0, xFilename.length() - 4) + ".hack" );
	if (m_xBinaryFile.fail())
	{
		cout << "file opening failed" << endl;
	}
}

Code::~Code()
{
	m_xBinaryFile.close();
}

string Code::GetSymbol( const string& xSymbol )
{
	if( IsNumber(xSymbol) )
	{
		return std::bitset< 16 >( std::stoi( xSymbol ) ).to_string();
	}
	else
	{
		if(m_xSymbolTable.Has(xSymbol))
		{
			// get the address
			return std::bitset< 16 >( m_xSymbolTable.GetAddress( xSymbol ) ).to_string();
		}
		else
		{
			// New variable, add it to symbol table
			m_xSymbolTable.Add( xSymbol, m_uRamAddress );
		
			return std::bitset< 16 >( m_uRamAddress++ ).to_string();
		}
	}
}

bool Code::IsNumber(const string& xSymbol) const
{
    std::string::const_iterator it = xSymbol.begin();
    while (it != xSymbol.end() && std::isdigit(*it)) ++it;
    return !xSymbol.empty() && it == xSymbol.end();
}

void Code::AddLabelsToSymbolTable( Parser& xParser )
{
	xParser.ResetStreamReadingPosition();

	unsigned int uRomAddress = 0;
	while( xParser.AdvanceToNextCommand() )
	{
		Parser::COMMAND_TYPE eCmdType = xParser.GetCommandType();
		if( eCmdType == Parser::COMMAND_L )
		{
			if( !m_xSymbolTable.Add(xParser.GetSymbol(), uRomAddress) )
			{
				// output error
				cout << "Jump symbol duplicate error: " << xParser.GetSymbol() << '\n';
			}
		}
		else
		{
			++uRomAddress;
		}
	}
}

void Code::Generate( Parser& xParser )
{
	xParser.ResetStreamReadingPosition();

	while( xParser.AdvanceToNextCommand() )
	{
		Parser::COMMAND_TYPE eCmdType = xParser.GetCommandType();
		switch ( eCmdType )
		{
			case Parser::COMMAND_A:
			{
				m_xBinaryFile << GetSymbol( xParser.GetSymbol() ) << '\n';
				break;
			}

			case Parser::COMMAND_C:
			{
				string xCode;
				COMPUTATION_TYPE eCompType = xParser.GetComputation();
				if( eCompType != COMP_ERROR )
				{
					xCode = GetComputation( eCompType );
				}
				else
				{
					cout << "Comp error: " << xParser.GetCommand() << '\n';
				}

				DESTINATION_TYPE eDestType = xParser.GetDestination();
				if( eDestType != DEST_ERROR )
				{
					xCode += GetDestination( eDestType );
				}
				else
				{
					// output error
				    cout << "Dest error: " << xParser.GetCommand() << '\n';
				}

				JUMP_TYPE eJumpType = xParser.GetJump();
				if( eJumpType != JUMP_ERROR )
				{
					xCode += GetJump( eJumpType );
				}
				else
				{
					// output error
					cout << "Jump error: " << xParser.GetCommand() << '\n';
				}
		
				if( (eCompType != COMP_ERROR) && (eDestType != DEST_ERROR) && (eJumpType != JUMP_ERROR) )
				{
					m_xBinaryFile << xCode << '\n';
				}
				break;
			}

			case Parser::COMMAND_ERROR:
			{
				cout << "Syntax error: " << xParser.GetCommand() << '\n';
				break;
			}
		}
	}
}