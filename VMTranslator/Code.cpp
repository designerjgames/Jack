#include "Code.h"

#include <bitset>
#include <iostream>
#include <cctype>

using namespace std;

Code::Code(const string& xFileName)
: m_xAssemblyCodeStream()
, m_xAssemblyCode()
, m_xFileName(xFileName)
, m_uReturnCount(0)
, m_xInternalCallRoutine()
, m_xInternalReturnRoutine()
, m_xInternalComparisonRoutineEQ()
, m_xInternalComparisonRoutineGT()
, m_xInternalComparisonRoutineLT()
, m_xInternalComparisonRoutineGE()
, m_xInternalComparisonRoutineLE()
, m_uEndLabelCount(0)
, m_uCompLabelCount(0)
{
	RamReservedStrings[SP] = "SP";
	RamReservedStrings[LCL] = "LCL";
	RamReservedStrings[ARG] = "ARG";
	RamReservedStrings[THIS] = "THIS";
	RamReservedStrings[THAT] = "THAT";
	RamReservedStrings[R0] = "R0";
	RamReservedStrings[R1] = "R1";
	RamReservedStrings[R2] = "R2";
	RamReservedStrings[R3] = "R3";
	RamReservedStrings[R4] = "R4";
	RamReservedStrings[R5] = "R5";
	RamReservedStrings[R6] = "R6";
	RamReservedStrings[R7] = "R7";
	RamReservedStrings[R8] = "R8";
	RamReservedStrings[R9] = "R9";
	RamReservedStrings[R10] = "R10";
	RamReservedStrings[R11] = "R11";
	RamReservedStrings[R12] = "R12";
	RamReservedStrings[R13] = "R13";
	RamReservedStrings[R14] = "R14";
	RamReservedStrings[R15] = "R15";
	RamReservedStrings[SCREEN] = "SCREEN";
	RamReservedStrings[KBD] = "KBD";
}

Code::~Code()
{

}

Code::RAM_RESERVED_SYMBOLS Code::GetRamReservedSymbolFromSegmentType(VMTokenizer::KEYWORD_TYPE eSegType) const
{
	switch (eSegType)
	{
		case VMTokenizer::RAM_SEGMENT_LOCAL:
		{
			return LCL;
		}
		case VMTokenizer::RAM_SEGMENT_THIS:
		{
			return THIS;
		}
		case VMTokenizer::RAM_SEGMENT_ARGUMENT:
		{
			return ARG;
		}
		case VMTokenizer::RAM_SEGMENT_THAT:
		{
			return THAT;
		}
		case VMTokenizer::RAM_SEGMENT_TEMP:
		{
			return R5;
		}
	}

	return TOTAL_RAM_RESERVED_SYMBOLS;
}

void Code::AddInternalRoutinesCode()
{
	// Set the internal call routine name
	m_xInternalCallRoutine = "__INTERNAL_CALL_ROUTINE__";
	// Add call routine code
	AddInternalCallRoutine();

	// Set the internal return routine name
	m_xInternalReturnRoutine = "__INTERNAL_RETURN_ROUTINE__";
	// Add return routine code
	AddInternalReturnRoutine();

	// Set the internal comparison routine name
	m_xInternalComparisonRoutineEQ = "__INTERNAL_COMP_EQ__";
	m_xInternalComparisonRoutineGT = "__INTERNAL_COMP_GT__";
	m_xInternalComparisonRoutineLT = "__INTERNAL_COMP_LT__";
	m_xInternalComparisonRoutineGE = "__INTERNAL_COMP_GE__";
	m_xInternalComparisonRoutineLE = "__INTERNAL_COMP_LE__";

	// Add comparison routine code
	AddInternalComparisonRoutine();
}

void Code::OnPostGeneration()
{
	m_xAssemblyCode = m_xAssemblyCodeStream.str();
}

bool Code::WriteAssemblyFile()
{
	ofstream xAsmFile;
	xAsmFile.open(m_xFileName);
	if (xAsmFile.fail())
	{
		cout << "file opening failed" << endl;
		return false;
	}

	xAsmFile << m_xAssemblyCode;
	xAsmFile.close();

	return true;
}

void Code::AddString(const std::string& xString)
{
	m_xAssemblyCodeStream << xString << '\n';
}