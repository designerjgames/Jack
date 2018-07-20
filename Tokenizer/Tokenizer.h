#ifndef _TOKENIZER_
#define _TOKENIZER_

#include <fstream>
#include <string>

class Tokenizer
{
public:
	enum TOKEN_TYPE
	{
		TOKEN_UNKNOWN,
		TOKEN_COMMENT_TO_END_OF_LINE,
		TOKEN_COMMENT_TO_CLOSING,
		TOKEN_DQUOTE,
		TOKEN_SPACE,
		TOKEN_KEYWORD,
		TOKEN_SYMBOL,
		TOKEN_INDENTIFIER,
		TOKEN_INT_CONSTANT,
		TOKEN_STRING_CONSTANT
	};

	Tokenizer( const std::string& xSource, bool bIsString = false );
	Tokenizer( const Tokenizer& xTokenizer );
	virtual ~Tokenizer( );
	
	TOKEN_TYPE    GetTokenType()  const { return m_eTokenType; }
	bool          GetNextToken();
	unsigned int  GetLineNumber() const { return m_uLineNumber; }
	const std::string& GetTokenValue() const { return m_xTokenValue; }

	long long	  GetStreamPosition() { return m_xFile.tellg(); }
	void          Reset();
	void		  ResetStreamReadingPosition() { SetStreamReadingPosition(0); }

	void          SetAllIdentifiersConform(bool bConform) { m_bAllIdentifiersConform = bConform;  }

protected:
	virtual TOKEN_TYPE FindTokenType(const std::string& xToken) const;

	void       SetStreamReadingPosition(long long llPosition);
	bool       AdvanceToNextCodeLine();

	bool	   IsNumber(const std::string& xSymbol)                    const;
	bool	   IsSymbolConform(const std::string& xSymbol)             const;
	bool       IsIdentifierConform( const std::string& xIdentifier )   const;
	int        GetClosingCommentIndex( )                          const;
	bool	   GetNextToken_Internal();
	bool	   DetectTokenType();

	std::string   m_xCode;
	std::ifstream m_xFile;
	unsigned int  m_uCurrentIndex;
	bool		  m_AwaitingClosingComment;
	unsigned int  m_uLineNumber;
	bool		  m_bIsString;
	TOKEN_TYPE	  m_eTokenType;
	std::string   m_xTokenValue;
	std::string   m_xFileName;
	bool          m_bAllIdentifiersConform;
};

#endif //_TOKENIZER_