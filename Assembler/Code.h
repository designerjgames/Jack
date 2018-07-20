#ifndef _CODE_
#define _CODE_

#include <fstream>
#include <string>
#include "SymbolTable.h"

using namespace std;

class Parser;

class Code
{
public:
	enum DESTINATION_TYPE
	{
		DEST_NULL,
		DEST_M,
		DEST_D,
		DEST_MD,
		DEST_A,
		DEST_AM,
		DEST_AD,
		DEST_AMD,
		DEST_ERROR
	};

	enum COMPUTATION_TYPE
	{
		COMP_0,
		COMP_1,
		COMP_NEG_1,
		COMP_D,
		COMP_A,
		COMP_NOT_D,
		COMP_NOT_A,
		COMP_NEG_D,
		COMP_NEG_A,
		COMP_D_ADD_1,
		COMP_A_ADD_1,
		COMP_D_SUB_1,
		COMP_A_SUB_1,
		COMP_D_ADD_A,
		COMP_D_SUB_A,
		COMP_A_SUB_D,
		COMP_D_AND_A,
		COMP_D_OR_A,
		COMP_M,
		COMP_NOT_M,
		COMP_NEG_M,
		COMP_M_ADD_1,
		COMP_M_SUB_1,
		COMP_D_ADD_M,
		COMP_D_SUB_M,
		COMP_M_SUB_D,
		COMP_D_AND_M,
		COMP_D_OR_M,
		COMP_ERROR
	};

	enum JUMP_TYPE
	{
		JUMP_NULL,
		JUMP_JGT,
		JUMP_JEQ,
		JUMP_JGE,
		JUMP_JLT,
		JUMP_JNE,
		JUMP_JLE,
		JUMP_JMP,
		JUMP_ERROR
	};

	Code( const string& xFilename );
	~Code();

	void AddLabelsToSymbolTable( Parser& xParser );
	void Generate( Parser& xParser );

	const string& GetComputation( COMPUTATION_TYPE eType ) const { return m_xComputation[eType]; }
	const string& GetDestination( DESTINATION_TYPE eType ) const { return m_xDestination[eType]; }
	const string& GetJump       ( JUMP_TYPE        eType ) const { return m_xJump[eType];        }
	      string  GetSymbol     ( const string& xSymbol  );

	static const unsigned int USER_RAM_START = 16;

private:
	bool IsNumber(const string& xSymbol) const;

	string m_xComputation[28];
	string m_xDestination[8];
	string m_xJump[8];

	unsigned int m_uRamAddress;
	SymbolTable  m_xSymbolTable;
	ofstream     m_xBinaryFile;
};

#endif //Code