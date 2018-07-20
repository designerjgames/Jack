#ifndef _CODE_X86_WIN32_
#define _CODE_X86_WIN32_

#include <string>
#include "Code.h"
#include "SymbolTable.h"

class Code_x86_Win32 : public Code
{
public:
	Code_x86_Win32(const std::string& xFileName);
	~Code_x86_Win32();

	virtual void AddBootStrapCode();
	virtual void AddAddCode();
	virtual void AddSubCode();
	virtual void AddNegCode();
	virtual void AddAndCode();
	virtual void AddOrCode();
	virtual void AddNotCode();

	virtual void AddComparisonCode(const std::string& xSourceFile, VMTokenizer::KEYWORD_TYPE eCommandType);
	virtual void AddPushSegmentCode(VMTokenizer::KEYWORD_TYPE eSegType, unsigned int uSegIndex, const std::string& xSourceFile, bool bAddress = false, bool bMemory = false);
	virtual void AddPopSegmentCode(VMTokenizer::KEYWORD_TYPE eSegType, unsigned int uSegIndex, const std::string& xSourceFile);
	virtual void AddLabelCode(const std::string& xLabel, const std::string& xCurrentFuntion);
	virtual void AddIfCode(const std::string& xLabel, const std::string& xCurrentFuntion);
	virtual void AddGotoCode(const std::string& xLabel, const std::string& xCurrentFuntion);
	virtual void AddCallCode(const std::string& xSourceFileName, const std::string& xFunction, unsigned int uArgs, bool bVirtual = false, unsigned int bVirtualFunctionIndex = 0);
	virtual void AddFunctionCode(const std::string& xCurrentFuntion, unsigned int uNumberOfLocals);
	virtual void AddReturnCode();
	virtual void AddPushFunctionCode(const std::string& xFunction);
	virtual void AddComment(const std::string& xComment);

	virtual void OnPostGeneration();

private:
	void AddCommonUnaryArithmeticCode();

	// Assembly routine that get called whenever a function call is made
	virtual void AddInternalCallRoutine() {};

	// Assembly routine that get called whenever a return is made
	virtual void AddInternalReturnRoutine();

	// Assembly routine that get called whenever a comparison is made
	virtual void AddInternalComparisonRoutine();

	const std::string m_xDWORD;

	SymbolTable<std::string> m_xStaticVariableTable;

	static string s_xStaticsMarker;

	typedef Code PARENT;
};

#endif //_CODE_X86_WIN32_