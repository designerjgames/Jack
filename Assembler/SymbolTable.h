#ifndef _SYMBOL_TABLE_
#define _SYMBOL_TABLE_

#include <unordered_map>

using namespace std;

class SymbolTable
{
public:
	SymbolTable();
	~SymbolTable() {};

	bool		 Add( const string& xSymbol, unsigned int uAddress );
	bool		 Has( const string& xSymbol ) const;
	unsigned int GetAddress( const string& xSymbol ) const;

private:
	unordered_map<string, unsigned int> m_xTable;
};

#endif //_SYMBOL_TABLE_