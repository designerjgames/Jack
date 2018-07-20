#include "Tokenizer.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <regex>
#include <functional> 
#include <locale>

using namespace std;

Tokenizer::Tokenizer( const string& xSource, bool bIsString /*false*/ )
: m_xCode()
, m_xFile()
, m_uCurrentIndex( 0 )
, m_AwaitingClosingComment( false )
, m_uLineNumber(0)
, m_bIsString( bIsString )
, m_eTokenType(TOKEN_UNKNOWN)
, m_xTokenValue()
, m_xFileName()
, m_bAllIdentifiersConform(false)
{
	if( bIsString )
	{
		m_xCode = xSource;
	}
	else
	{
		m_xFileName = xSource;
		m_xFile.open(m_xFileName, ios::binary);
		if (m_xFile.fail())
		{
			cout << "file opening failed" << endl;
		} 
	}
}

Tokenizer::Tokenizer(const Tokenizer& xTokenizer)
: m_xCode(xTokenizer.m_xCode)
, m_uCurrentIndex(xTokenizer.m_uCurrentIndex)
, m_AwaitingClosingComment(xTokenizer.m_AwaitingClosingComment)
, m_uLineNumber(xTokenizer.m_uLineNumber)
, m_bIsString(xTokenizer.m_bIsString)
, m_eTokenType(xTokenizer.m_eTokenType)
, m_xTokenValue(xTokenizer.m_xTokenValue)
, m_xFileName(xTokenizer.m_xFileName)
{
	if (!m_bIsString)
	{
		m_xFile.open(m_xFileName, ios::binary);
		if (m_xFile.fail())
		{
			cout << "file opening failed" << endl;
		}
		else
		{
			SetStreamReadingPosition(const_cast<Tokenizer&>(xTokenizer).GetStreamPosition());
		}
	}
}

Tokenizer::~Tokenizer( )
{
	if( !m_bIsString )
	{
		m_xFile.close();
	}
}

void Tokenizer::Reset()
{
	m_xCode                  = "";
	m_uCurrentIndex          = 0;
	m_AwaitingClosingComment = false;
	m_uLineNumber            = 0;
	m_eTokenType             = TOKEN_UNKNOWN;
	m_xTokenValue			 = "";

	ResetStreamReadingPosition();
}

bool Tokenizer::IsNumber( const string& xSymbol ) const
{
    std::string::const_iterator it = xSymbol.begin();
    while (it != xSymbol.end() && std::isdigit(*it)) ++it;
    return !xSymbol.empty() && it == xSymbol.end();
}

bool Tokenizer::IsIdentifierConform( const string& xIdentifier ) const
{
	if (m_bAllIdentifiersConform)
		return true;

	// It shouldn't start with a number
	if( isdigit( xIdentifier[0] ) )
	{
		return false;
	}

	std::regex xRgx;

	if( m_bIsString )
		xRgx.assign("[a-zA-Z0-9_.$@]+");
	else
		xRgx.assign("[a-zA-Z0-9_.$]+");

	if( !std::regex_match( xIdentifier, xRgx ) )
    {
		return false;
    }

    return true;
}

void Tokenizer::SetStreamReadingPosition(long long llPosition)
{
	m_xFile.clear(); // clear fail and eof bits
	m_xFile.seekg(llPosition, ios::beg) ;
}

bool Tokenizer::AdvanceToNextCodeLine()
{
	while (m_uCurrentIndex == m_xCode.length())
	{
		if( m_bIsString )
			return false; // No next line in a string

		m_uCurrentIndex = 0;
		if (!m_xFile.eof())
		{
			m_xCode = "";
			getline(m_xFile, m_xCode);

			++m_uLineNumber;

			//Remove all space from the code on both ends
			//Start
			m_xCode.erase(m_xCode.begin(), std::find_if(m_xCode.begin(), m_xCode.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
			//End
			m_xCode.erase(std::find_if(m_xCode.rbegin(), m_xCode.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), m_xCode.end());

			if (m_AwaitingClosingComment)
			{
				// Get */ index
				int iCCIndex = GetClosingCommentIndex();
				if (iCCIndex != -1)
				{
					m_uCurrentIndex = iCCIndex + 2;
					m_AwaitingClosingComment = false;
				}
				else
				{
					m_uCurrentIndex = m_xCode.length();
				}
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool Tokenizer::GetNextToken()
{
	m_eTokenType  = Tokenizer::TOKEN_UNKNOWN;
	m_xTokenValue = "";

	__pragma(warning(suppress:4127))
	while (true)
	{
		if (AdvanceToNextCodeLine())
		{
			GetNextToken_Internal();
			switch (m_eTokenType)
			{
				case TOKEN_COMMENT_TO_END_OF_LINE:
				case TOKEN_COMMENT_TO_CLOSING:
				{
					break;
				}
				default:
				{
					return true;
				}
			}
		}
		else
		{
			return false;
		}
	}
}

bool Tokenizer::GetNextToken_Internal()
{
	//Assume m_xCode has tokens;
	for( ; m_uCurrentIndex < m_xCode.length( ); ++m_uCurrentIndex )
	{
		TOKEN_TYPE eTMPTokenType = FindTokenType(std::string( 1, m_xCode[m_uCurrentIndex] ));
		switch( eTMPTokenType )
		{
			case TOKEN_SPACE:
			{
				break;
			}

			case TOKEN_DQUOTE:
			{
				m_eTokenType            = TOKEN_STRING_CONSTANT;
				unsigned int uOddQuotes = 0;
				// advance until we hit an odd number of double quotes
				for( ++m_uCurrentIndex; m_uCurrentIndex < m_xCode.length( ); ++m_uCurrentIndex )
				{
					if( std::string(1, m_xCode[m_uCurrentIndex]) != "\"" )
					{
						m_xTokenValue += m_xCode[m_uCurrentIndex];
					}
					else
					{
						++uOddQuotes;
						for( ++m_uCurrentIndex; m_uCurrentIndex < m_xCode.length( ); ++m_uCurrentIndex )
						{
							if( std::string( 1, m_xCode[m_uCurrentIndex]) != "\"" )
							{
								if( ( uOddQuotes & 1 ) )
								{
									return true;
								}
								uOddQuotes = 0;
							}

							m_xTokenValue += m_xCode[m_uCurrentIndex];
						}
					}
				}
				return false;
			}

			case TOKEN_SYMBOL:
			{	
				if( std::string( 1, m_xCode[m_uCurrentIndex] ) == "/" )
				{
					// Might be a comment, handle it
					m_xTokenValue = m_xCode[m_uCurrentIndex];
					if( m_uCurrentIndex < m_xCode.length( ) - 1 )
					{
						string tmpString         = m_xTokenValue + m_xCode[m_uCurrentIndex + 1];

						eTMPTokenType = FindTokenType( tmpString );
						if( eTMPTokenType == TOKEN_COMMENT_TO_END_OF_LINE )
						{
							m_eTokenType = TOKEN_COMMENT_TO_END_OF_LINE;
							m_uCurrentIndex = m_xCode.length( );
							return true;
						}

						if( eTMPTokenType == TOKEN_COMMENT_TO_CLOSING )
						{
							m_eTokenType = TOKEN_COMMENT_TO_CLOSING;
							// Get */ index
							int iCCIndex = GetClosingCommentIndex( );
							if( iCCIndex != -1 )
							{
								m_uCurrentIndex = iCCIndex + 2;
							}
							else
							{
								m_AwaitingClosingComment = true;
								m_uCurrentIndex          = m_xCode.length( );
							}
							return true;
						}
					}
				}

				m_eTokenType  = TOKEN_SYMBOL;
				m_xTokenValue = m_xCode[m_uCurrentIndex++];
				return true;
			}

			case TOKEN_KEYWORD:
			{
				m_eTokenType  = TOKEN_KEYWORD;
				m_xTokenValue = m_xCode[m_uCurrentIndex++];
				return true;
			}

			case TOKEN_UNKNOWN:
			{
				m_xTokenValue = m_xCode[m_uCurrentIndex];
				for( ++m_uCurrentIndex; m_uCurrentIndex < m_xCode.length( ); ++m_uCurrentIndex )
				{
					eTMPTokenType = FindTokenType( std::string( 1, m_xCode[m_uCurrentIndex] ) );
					switch( eTMPTokenType )
					{
						case TOKEN_SPACE:
						case TOKEN_SYMBOL:
						case TOKEN_KEYWORD:
						{
							if (DetectTokenType())
							return true;
						}
					}
					m_xTokenValue += m_xCode[m_uCurrentIndex];
				}

				// hitting return, check if we have a valid number, identifier, or keyword
				if (DetectTokenType())
					return true;

				break;
			}	
		}
	}
	return false;
}

bool Tokenizer::DetectTokenType()
{
	if (IsNumber(m_xTokenValue))
	{
		m_eTokenType = TOKEN_INT_CONSTANT;
		return true;
	}

	if (FindTokenType(m_xTokenValue) == TOKEN_KEYWORD)
	{
		m_eTokenType = TOKEN_KEYWORD;
		return true;
	}

	if (IsIdentifierConform(m_xTokenValue))
	{
		m_eTokenType = TOKEN_INDENTIFIER;
		return true;
	}
	return false;
}

int Tokenizer::GetClosingCommentIndex( ) const
{
	size_t iCCfound = m_xCode.find( "*/", m_uCurrentIndex );

	if( iCCfound != std::string::npos )
	{
		return iCCfound;
	}

	return -1;
}

Tokenizer::TOKEN_TYPE Tokenizer::FindTokenType( const string& xToken ) const
{
	//Check for symbols
	if (xToken.size() == 1)
	{
		if (xToken == "\"")
		{
			return TOKEN_DQUOTE;
		}

		if (xToken == " " || xToken == "\t")
		{
			return TOKEN_SPACE;
		}
	}
	else
	{
		//Check if comments
		if (xToken == "//")
		{
			return TOKEN_COMMENT_TO_END_OF_LINE;
		}

		if (xToken == "/*")
		{
			return TOKEN_COMMENT_TO_CLOSING;
		}
	}

	return TOKEN_UNKNOWN;
}