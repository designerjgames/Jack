#ifndef _VMWriter_
#define _VMWriter_

#include <string>
#include <sstream>
#include "CompilerTokenizer.h"

class Parser;

class VMWriter
{
public:
	enum TERM
	{
		TERM_INT_CONSTANT,
		TERM_STRING_CONSTANT,
		TERM_TRUE,
		TERM_FALSE,
		TERM_NULL,
		TERM_THIS,
		TERM_INDENTIFIER,
		TERM_FUNCTION_ADDRESS
	};

	VMWriter( const Parser& xParser );
	~VMWriter();

	bool WriteTerm(TERM eTerm, const std::string* pxName = NULL, bool bAddress = false);
	void WriteArray( bool bPushArray);
	bool WriteOperator( const std::string& xOperator );
	bool WriteAssignment(const std::string& xIdentifierName, bool bArray);
	void WriteSubroutineCall(const std::string& xIdentifierName, unsigned int uNumberOfArgs, bool bVirtual = false, unsigned int uVFIndex = 0, std::string* VMCodeLine = NULL);
	void WriteZeroReturn();
	void WriteReturn();
	void WriteRemoveZeroReturn();
	void WriteFunctionDeclaration( const std::string& xIdentifierName, const std::string& xLocalsMarker );
	bool WriteFunctionType( CompilerTokenizer::KEYWORD_TYPE eKeywordType, unsigned int uObjectSize = 0 );

	void WriteIfGotoTrue( unsigned int uNumberOfFunctionConditionals );
	void WriteGotoFalse( unsigned int uNumberOfFunctionConditionals );
	void WriteTrueLabel( unsigned int uNumberOfFunctionConditionals );
	void WriteGotoEnd( unsigned int uNumberOfFunctionConditionals );
	void WriteFalseLabel( unsigned int uNumberOfFunctionConditionals );
	void WriteEndLabel( unsigned int uNumberOfFunctionConditionals );

	void WriteWhileGotoTrue(unsigned int uNumberOfFunctionConditionals);
	void WriteWhileGotoFalse(unsigned int uNumberOfFunctionConditionals);
	void WriteWhileTrueLabel(unsigned int uNumberOfFunctionConditionals);
	void WriteWhileFalseLabel(unsigned int uNumberOfFunctionConditionals);
	void WriteWhileLabel( unsigned int uNumberOfFunctionConditionals );
	void WriteWhile( unsigned int uNumberOfFunctionConditionals );
	void WriteString(const std::string& xString);

	const std::stringstream& GetVMCode() const { return m_xVMCode; }
          std::stringstream& GetVMCode()       { return m_xVMCode; }

	unsigned int GetSkippedAllocs() const                 { return m_uSkippedAllocs;    }
	void         SetSkippedAllocs( unsigned int uAllocs ) { m_uSkippedAllocs = uAllocs; }

private:
	const Parser& m_xParser;
	std::stringstream  m_xVMCode;

	unsigned int  m_uSkippedAllocs; // Used to label contructor allocation skips

	static unsigned int s_uFunctionCalls;
};

#endif //_VMWriter_