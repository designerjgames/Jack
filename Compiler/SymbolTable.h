#ifndef _SYMBOL_TABLE_
#define _SYMBOL_TABLE_

#include <unordered_map>

using namespace std;

template < class SYMBOL >
class SymbolTable
{
public:
	SymbolTable();
	~SymbolTable() {};

	bool		    Add(const string& xID, const SYMBOL& xSymbol);

	void            Clear() { m_xTable.clear(); }

	const SYMBOL* GetSymbol(const string& xSymbol) const;

	const unordered_map<string, const SYMBOL>& GetTable() const { return m_xTable; }

private:
	unordered_map<string, const SYMBOL> m_xTable;
};

template < class SYMBOL >
SymbolTable<SYMBOL>::SymbolTable()
{

}

template < class SYMBOL >
bool SymbolTable<SYMBOL>::Add(const string& xID, const SYMBOL& xSymbol )
{
	auto result = m_xTable.insert(std::make_pair(xID, xSymbol));

	return result.second;
}

template < class SYMBOL >
const SYMBOL* SymbolTable<SYMBOL>::GetSymbol(const string& xID) const
{
	std::unordered_map<std::string, const SYMBOL >::const_iterator xIt = m_xTable.find(xID);

	if( xIt == m_xTable.end() )
		return NULL;

	return &xIt->second;
}

#endif //_SYMBOL_TABLE_