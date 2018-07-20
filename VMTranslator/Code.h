#ifndef _CODE_
#define _CODE_

#include <sstream>
#include <string>
#include "VMTokenizer.h"

class Code
{
public:
	enum RAM_RESERVED_SYMBOLS
	{
		SP,
		LCL,
		ARG,
		THIS,
		THAT,
		R0,
		R1,
		R2,
		R3,
		R4,
		R5,
		R6,
		R7,
		R8,
		R9,
		R10,
		R11,
		R12,
		R13,
		R14,
		R15,
		SCREEN,
		KBD,
		TOTAL_RAM_RESERVED_SYMBOLS
	};

	std::string RamReservedStrings[TOTAL_RAM_RESERVED_SYMBOLS];
	Code(const std::string& xFileName);
	~Code();

	virtual void AddBootStrapCode() = 0;
	void AddInternalRoutinesCode();

	virtual void AddAddCode() = 0;
	virtual void AddSubCode() = 0;
	virtual void AddNegCode() = 0;
	virtual void AddAndCode() = 0;
	virtual void AddOrCode () = 0;
	virtual void AddNotCode() = 0;

	virtual void AddComparisonCode	(const std::string& xSourceFile, VMTokenizer::KEYWORD_TYPE eCommandType) = 0;
	virtual void AddPushSegmentCode	(VMTokenizer::KEYWORD_TYPE eSegType, unsigned int uSegIndex, const std::string& xSourceFile, bool bAddress = false, bool bMemory = false) = 0;
	virtual void AddPopSegmentCode	(VMTokenizer::KEYWORD_TYPE eSegType, unsigned int uSegIndex, const std::string& xSourceFile) = 0;
	virtual void AddLabelCode		(const std::string& xLabel, const std::string& xCurrentFuntion) = 0;
	virtual void AddIfCode			(const std::string& xLabel, const std::string& xCurrentFuntion) = 0;
	virtual void AddGotoCode		(const std::string& xLabel, const std::string& xCurrentFuntion) = 0;
	virtual void AddCallCode		(const std::string& xSourceFileName, const std::string& xFunction, unsigned int uArgs, bool bVirtual = false, unsigned int bVirtualFunctionIndex = 0) = 0;
	virtual void AddFunctionCode    (const std::string& xCurrentFuntion, unsigned int uNumberOfLocals) = 0;
	virtual void AddReturnCode      () = 0;
	virtual void AddPushFunctionCode(const std::string& xFunction) = 0;
	virtual void AddComment         (const std::string& xComment)  = 0;
    void AddString					(const std::string& xString);
	
	virtual void OnPostGeneration   () = 0;

	        bool WriteAssemblyFile();

protected:
	// Assembly routine that get called whenever a function call is made
	virtual void AddInternalCallRoutine() = 0;

	// Assembly routine that get called whenever a return is made
	virtual void AddInternalReturnRoutine() = 0;

	// Assembly routine that get called whenever a comparison is made
	virtual void AddInternalComparisonRoutine() = 0;

	Code::RAM_RESERVED_SYMBOLS GetRamReservedSymbolFromSegmentType(VMTokenizer::KEYWORD_TYPE eSegType) const;

	std::stringstream m_xAssemblyCodeStream;
	std::string       m_xAssemblyCode;
	std::string		  m_xFileName;
	unsigned int	  m_uReturnCount;
	std::string		  m_xInternalCallRoutine;
	std::string		  m_xInternalReturnRoutine;
	std::string		  m_xInternalComparisonRoutineEQ;
	std::string		  m_xInternalComparisonRoutineGT;
	std::string		  m_xInternalComparisonRoutineLT;
	std::string		  m_xInternalComparisonRoutineGE;
	std::string		  m_xInternalComparisonRoutineLE;
	unsigned int	  m_uEndLabelCount;
	unsigned int	  m_uCompLabelCount;
};

#endif //Code