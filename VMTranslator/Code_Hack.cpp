#include "Code_Hack.h"

#include <bitset>
#include <iostream>
#include <cctype>

using namespace std;

Code_Hack::Code_Hack(const string& xFileName)
: PARENT(xFileName)
{
}

Code_Hack::~Code_Hack()
{
}

void Code_Hack::AddCommonBinaryArithmeticCode()
{
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "AM=M-1\n";
	m_xAssemblyCodeStream << "D=M\n";
	m_xAssemblyCodeStream << "A=A-1\n";
}

void Code_Hack::AddCommonUnaryArithmeticCode()
{
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "AM=M-1\n";
	m_xAssemblyCodeStream << "D=M\n";
}

void Code_Hack::AddComparisonCode(const string& xSourceFile, VMTokenizer::KEYWORD_TYPE eCommandType)
{
	// Put return address in R13
	string xReturnLabel = xSourceFile + "_" + "RT_" + std::to_string(m_uReturnCount++);
	m_xAssemblyCodeStream << "@" << xReturnLabel << '\n';
	m_xAssemblyCodeStream << "D=A\n"; // D has return address now

								// Jump to comp routine
	switch (eCommandType)
	{
	case VMTokenizer::COMMAND_ARITHMETIC_EQ:
	{
		m_xAssemblyCodeStream << "@" << m_xInternalComparisonRoutineEQ << '\n';
		break;
	}
	case VMTokenizer::COMMAND_ARITHMETIC_GT:
	{
		m_xAssemblyCodeStream << "@" << m_xInternalComparisonRoutineGT << '\n';
		break;
	}
	case VMTokenizer::COMMAND_ARITHMETIC_LT:
	{
		m_xAssemblyCodeStream << "@" << m_xInternalComparisonRoutineLT << '\n';
		break;
	}
	case VMTokenizer::COMMAND_ARITHMETIC_GE:
	{
		m_xAssemblyCodeStream << "@" << m_xInternalComparisonRoutineGE << '\n';
		break;
	}
	case VMTokenizer::COMMAND_ARITHMETIC_LE:
	{
		m_xAssemblyCodeStream << "@" << m_xInternalComparisonRoutineLE << '\n';
		break;
	}
	}

	m_xAssemblyCodeStream << "0;JMP\n";

	m_xAssemblyCodeStream << "(" << xReturnLabel << ")\n";
}

void Code_Hack::AddInternalComparisonRoutine()
{
	// Comparison routine label
	m_xAssemblyCodeStream << "(" << m_xInternalComparisonRoutineEQ << ")\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[R13] << '\n';
	m_xAssemblyCodeStream << "M=D\n"; // R13 has return address now

	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "AM=M-1\n";
	m_xAssemblyCodeStream << "D=M\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "A=M-1\n";
	m_xAssemblyCodeStream << "D=M-D\n";
	m_xAssemblyCodeStream << "@__COMP_INTERNAL_SUCCESS__" << "\n";
	m_xAssemblyCodeStream << "D;JEQ\n";
	m_xAssemblyCodeStream << "@__COMP_INTERNAL_FAILURE__\n";
	m_xAssemblyCodeStream << "0;JMP\n";

	m_xAssemblyCodeStream << "(" << m_xInternalComparisonRoutineGT << ")\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[R13] << '\n';
	m_xAssemblyCodeStream << "M=D\n"; // R13 has return address now

	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "AM=M-1\n";
	m_xAssemblyCodeStream << "D=M\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "A=M-1\n";
	m_xAssemblyCodeStream << "D=M-D\n";
	m_xAssemblyCodeStream << "@__COMP_INTERNAL_SUCCESS__" << "\n";
	m_xAssemblyCodeStream << "D;JGT\n";
	m_xAssemblyCodeStream << "@__COMP_INTERNAL_FAILURE__\n";
	m_xAssemblyCodeStream << "0;JMP\n";

	m_xAssemblyCodeStream << "(" << m_xInternalComparisonRoutineLT << ")\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[R13] << '\n';
	m_xAssemblyCodeStream << "M=D\n"; // R13 has return address now

	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "AM=M-1\n";
	m_xAssemblyCodeStream << "D=M\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "A=M-1\n";
	m_xAssemblyCodeStream << "D=M-D\n";
	m_xAssemblyCodeStream << "@__COMP_INTERNAL_SUCCESS__" << "\n";
	m_xAssemblyCodeStream << "D;JLT\n";
	m_xAssemblyCodeStream << "@__COMP_INTERNAL_FAILURE__\n";
	m_xAssemblyCodeStream << "0;JMP\n";

	m_xAssemblyCodeStream << "(" << m_xInternalComparisonRoutineGE << ")\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[R13] << '\n';
	m_xAssemblyCodeStream << "M=D\n"; // R13 has return address now

	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "AM=M-1\n";
	m_xAssemblyCodeStream << "D=M\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "A=M-1\n";
	m_xAssemblyCodeStream << "D=M-D\n";
	m_xAssemblyCodeStream << "@__COMP_INTERNAL_SUCCESS__" << "\n";
	m_xAssemblyCodeStream << "D;JGE\n";
	m_xAssemblyCodeStream << "@__COMP_INTERNAL_FAILURE__\n";
	m_xAssemblyCodeStream << "0;JMP\n";

	m_xAssemblyCodeStream << "(" << m_xInternalComparisonRoutineLE << ")\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[R13] << '\n';
	m_xAssemblyCodeStream << "M=D\n"; // R13 has return address now

	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "AM=M-1\n";
	m_xAssemblyCodeStream << "D=M\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "A=M-1\n";
	m_xAssemblyCodeStream << "D=M-D\n";
	m_xAssemblyCodeStream << "@__COMP_INTERNAL_SUCCESS__" << "\n";
	m_xAssemblyCodeStream << "D;JLE\n";
	m_xAssemblyCodeStream << "@__COMP_INTERNAL_FAILURE__\n";
	m_xAssemblyCodeStream << "0;JMP\n";

	m_xAssemblyCodeStream << "(__COMP_INTERNAL_FAILURE__)\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "A=M-1\n";
	m_xAssemblyCodeStream << "M=0\n";
	m_xAssemblyCodeStream << "@__COMP_INTERNAL_END__\n";
	m_xAssemblyCodeStream << "0;JMP\n";
	m_xAssemblyCodeStream << "(__COMP_INTERNAL_SUCCESS__)\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "A=M-1\n";
	m_xAssemblyCodeStream << "M=-1\n";
	m_xAssemblyCodeStream << "(__COMP_INTERNAL_END__)\n";

	// Jump to return address in R13
	m_xAssemblyCodeStream << "@" << RamReservedStrings[R13] << '\n';
	m_xAssemblyCodeStream << "A=M\n";
	m_xAssemblyCodeStream << "0;JMP\n";
}

void Code_Hack::AddPushSegmentCode(VMTokenizer::KEYWORD_TYPE eSegType, unsigned int uSegIndex, const string& xSourceFile, bool bAddress, bool bMemory)
{
	// bMemory is ignored here
	bMemory;

	switch (eSegType)
	{
	case VMTokenizer::RAM_SEGMENT_CONSTANT:
	{
		if (uSegIndex == 0)
		{
			m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
			m_xAssemblyCodeStream << "AM=M+1\n";
			m_xAssemblyCodeStream << "A=A-1\n";
			m_xAssemblyCodeStream << "M=0\n";
			return;
		}
		else if (uSegIndex == 1)
		{
			m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
			m_xAssemblyCodeStream << "AM=M+1\n";
			m_xAssemblyCodeStream << "A=A-1\n";
			m_xAssemblyCodeStream << "M=1\n";
			return;
		}
		else
		{
			m_xAssemblyCodeStream << "@" << uSegIndex << '\n';
		}

		break;
	}
	case VMTokenizer::RAM_SEGMENT_LOCAL:
	case VMTokenizer::RAM_SEGMENT_THIS:
	case VMTokenizer::RAM_SEGMENT_ARGUMENT:
	case VMTokenizer::RAM_SEGMENT_THAT:
	case VMTokenizer::RAM_SEGMENT_TEMP:
	{
		if (uSegIndex > 1)
		{
			m_xAssemblyCodeStream << "@" << uSegIndex << '\n';
			m_xAssemblyCodeStream << "D=A\n";
		}

		m_xAssemblyCodeStream << "@" << RamReservedStrings[GetRamReservedSymbolFromSegmentType(eSegType)] << '\n';
		if (eSegType == VMTokenizer::RAM_SEGMENT_TEMP)
		{
			if (uSegIndex > 0)
			{
				if (uSegIndex == 1)
					m_xAssemblyCodeStream << "A=A+1" << "\n";
				else
					m_xAssemblyCodeStream << "A=A+D" << "\n";
			}
		}
		else
		{
			if (uSegIndex > 0)
			{
				if (uSegIndex == 1)
					m_xAssemblyCodeStream << "A=M+1" << "\n";
				else
					m_xAssemblyCodeStream << "A=M+D" << "\n";
			}
			else
				m_xAssemblyCodeStream << "A=M" << "\n";
		}

		break;
	}
	case VMTokenizer::RAM_SEGMENT_POINTER:
	{
		string xThisOrThat = (uSegIndex == 0) ? "THIS" : "THAT";
		m_xAssemblyCodeStream << "@" << xThisOrThat << '\n';
		break;
	}
	case VMTokenizer::RAM_SEGMENT_STATIC:
	{
		m_xAssemblyCodeStream << "@" << xSourceFile << "." << uSegIndex << '\n';
		break;
	}
	default:
	{
		bAddress = bAddress;
	}
	}

	if (bAddress || eSegType == VMTokenizer::RAM_SEGMENT_CONSTANT)
		m_xAssemblyCodeStream << "D=A\n";
	else
		m_xAssemblyCodeStream << "D=M\n";

	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "AM=M+1\n";
	m_xAssemblyCodeStream << "A=A-1\n";
	m_xAssemblyCodeStream << "M=D\n";
}

void Code_Hack::AddPopSegmentCode(VMTokenizer::KEYWORD_TYPE eSegType, unsigned int uSegIndex, const string& xSourceFile)
{
	// Get segment index
	switch (eSegType)
	{
	case VMTokenizer::RAM_SEGMENT_LOCAL:
	case VMTokenizer::RAM_SEGMENT_THIS:
	case VMTokenizer::RAM_SEGMENT_ARGUMENT:
	case VMTokenizer::RAM_SEGMENT_THAT:
	case VMTokenizer::RAM_SEGMENT_TEMP:
	{
		if (uSegIndex <= 6)
		{
			m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << "\n";
			m_xAssemblyCodeStream << "AM=M-1\n";
			m_xAssemblyCodeStream << "D=M\n"; // D has stack
			m_xAssemblyCodeStream << "@" << RamReservedStrings[GetRamReservedSymbolFromSegmentType(eSegType)] << "\n";
			if (eSegType == VMTokenizer::RAM_SEGMENT_TEMP)
			{
				if (uSegIndex > 0)
				{
					for (unsigned int u = 0; u < uSegIndex; ++u)
						m_xAssemblyCodeStream << "A=A+1\n";
				}

			}
			else
			{
				if (uSegIndex > 0)
				{
					m_xAssemblyCodeStream << "A=M+1\n";
					for (unsigned int u = 1; u < uSegIndex; ++u)
						m_xAssemblyCodeStream << "A=A+1\n";
				}
				else
				{
					m_xAssemblyCodeStream << "A=M\n";
				}
			}
			m_xAssemblyCodeStream << "M=D\n";
		}
		else
		{
			m_xAssemblyCodeStream << "@" << uSegIndex << '\n';
			m_xAssemblyCodeStream << "D=A\n";

			m_xAssemblyCodeStream << "@" << RamReservedStrings[GetRamReservedSymbolFromSegmentType(eSegType)] << '\n';

			if (eSegType == VMTokenizer::RAM_SEGMENT_TEMP)
			{
				m_xAssemblyCodeStream << "D=A+D" << "\n";
			}
			else
			{
				m_xAssemblyCodeStream << "D=M+D" << "\n";
			}

			m_xAssemblyCodeStream << "@R13\n";
			m_xAssemblyCodeStream << "M=D\n"; // R13 has local index
			m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << "\n";
			m_xAssemblyCodeStream << "AM=M-1\n";
			m_xAssemblyCodeStream << "D=M\n"; // D has stack
			m_xAssemblyCodeStream << "@" << RamReservedStrings[R13] << "\n";
			m_xAssemblyCodeStream << "A=M\n";
			m_xAssemblyCodeStream << "M=D\n";
		}
		break;
	}
	case VMTokenizer::RAM_SEGMENT_POINTER:
	{
		m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << "\n";
		m_xAssemblyCodeStream << "AM=M-1\n";
		m_xAssemblyCodeStream << "D=M\n";
		string xThisOrThat = (uSegIndex == 0) ? "THIS" : "THAT";
		m_xAssemblyCodeStream << "@" << xThisOrThat << '\n';
		m_xAssemblyCodeStream << "M=D\n";
		break;
	}
	case VMTokenizer::RAM_SEGMENT_STATIC:
	{
		m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << "\n";
		m_xAssemblyCodeStream << "AM=M-1\n";
		m_xAssemblyCodeStream << "D=M\n";
		m_xAssemblyCodeStream << "@" << xSourceFile << "." << uSegIndex << '\n';
		m_xAssemblyCodeStream << "M=D\n";
		break;
	}
	}
}

void Code_Hack::AddLabelCode(const string& xLabel, const string& xCurrentFuntion)
{
	m_xAssemblyCodeStream << "(" << xCurrentFuntion << "$" << xLabel << ")\n";
}

void Code_Hack::AddGotoCode(const string& xLabel, const string& xCurrentFuntion)
{
	m_xAssemblyCodeStream << "@" << xCurrentFuntion << "$" << xLabel << '\n';
	m_xAssemblyCodeStream << "0;JMP\n";
}

void Code_Hack::AddIfCode(const string& xLabel, const string& xCurrentFuntion)
{
	AddCommonUnaryArithmeticCode();
	m_xAssemblyCodeStream << "@" << xCurrentFuntion << "$" << xLabel << '\n';
	m_xAssemblyCodeStream << "D;JNE\n";
}

void Code_Hack::AddCallCode(const string& xSourceFileName, const string& xFunction, unsigned int uArgs, bool bVirtual /*=false*/, unsigned int bVirtualFunctionIndex /*=0*/)
{
	// Save function address in R14
	if (bVirtual)
	{
		// Stack has function address
		m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
		m_xAssemblyCodeStream << "AM=M-1\n";  // stack "pop" of the virtual function baseaddress
		m_xAssemblyCodeStream << "A=M\n";     // A has vpointer address
		m_xAssemblyCodeStream << "D=M\n";     // D has Array address 

		if (bVirtualFunctionIndex != 0)
		{
			m_xAssemblyCodeStream << "@" << bVirtualFunctionIndex << '\n';
			m_xAssemblyCodeStream << "D=A+D\n";   // vfunction var 
		}

		m_xAssemblyCodeStream << "A=D\n";     // A has array address of virtual function
		m_xAssemblyCodeStream << "D=M\n";     // D has virtual function address
	}
	else
	{
		m_xAssemblyCodeStream << "@" << xFunction << '\n';
		m_xAssemblyCodeStream << "D=A\n";
	}

	// R14 has jump to function now
	m_xAssemblyCodeStream << "@" << RamReservedStrings[R14] << '\n';
	m_xAssemblyCodeStream << "M=D\n";

	//If the number of arguments is 0, increase SP to allow for a slot
	//on stack for the function return value
	if (uArgs == 0)
	{
		m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
		m_xAssemblyCodeStream << "M=M+1\n";
	}

	// Save the number of arguments in R13, so they can be accessed by the call routine
	// If the function is called with no arguments, one argument was "pushed" to the stack
	// anyway to store the return value.
	unsigned int uNumberOfArguments = uArgs;
	uNumberOfArguments = (uNumberOfArguments == 0) ? 1 : uNumberOfArguments;
	m_xAssemblyCodeStream << "@" << uNumberOfArguments << '\n';
	m_xAssemblyCodeStream << "D=A\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[R13] << '\n';
	m_xAssemblyCodeStream << "M=D\n";

	// Put return address in D
	string xReturnLabel = xSourceFileName + "_" + "RT_" + std::to_string(m_uReturnCount++);
	m_xAssemblyCodeStream << "@" << xReturnLabel << '\n';
	m_xAssemblyCodeStream << "D=A\n"; // D has return address now

								// Jump to call assembly routine
	m_xAssemblyCodeStream << "@" << m_xInternalCallRoutine << '\n';
	m_xAssemblyCodeStream << "0;JMP\n";

	m_xAssemblyCodeStream << "(" << xReturnLabel << ")\n";
}

void Code_Hack::AddInternalCallRoutine()
{
	// call routine label
	m_xAssemblyCodeStream << "(" << m_xInternalCallRoutine << ")\n";

	// D has the return value from the caller
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "A=M\n";
	m_xAssemblyCodeStream << "M=D\n";

	m_xAssemblyCodeStream << "@" << RamReservedStrings[LCL] << '\n';
	m_xAssemblyCodeStream << "D=M\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "AM=M+1\n";
	m_xAssemblyCodeStream << "M=D\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[ARG] << '\n';
	m_xAssemblyCodeStream << "D=M\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "AM=M+1\n";
	m_xAssemblyCodeStream << "M=D\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[THIS] << '\n';
	m_xAssemblyCodeStream << "D=M\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "AM=M+1\n";
	m_xAssemblyCodeStream << "M=D\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[THAT] << '\n';
	m_xAssemblyCodeStream << "D=M\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "AM=M+1\n";
	m_xAssemblyCodeStream << "M=D\n";

	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "M=M+1\n";

	m_xAssemblyCodeStream << "@" << RamReservedStrings[R13] << '\n';
	m_xAssemblyCodeStream << "D=M\n"; // D has number of arguments

	m_xAssemblyCodeStream << "@5\n";
	m_xAssemblyCodeStream << "D=D+A\n"; // D has number of arguments + 5

	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "D=M-D\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[ARG] << '\n';
	m_xAssemblyCodeStream << "M=D\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "D=M\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[LCL] << '\n';
	m_xAssemblyCodeStream << "M=D\n";

	m_xAssemblyCodeStream << "@" << RamReservedStrings[R14] << '\n';
	m_xAssemblyCodeStream << "A=M\n";
	m_xAssemblyCodeStream << "0;JMP\n";
}

void Code_Hack::AddFunctionCode(const string& xCurrentFuntion, unsigned int uNumberOfLocals)
{
	//m_xAssemblyCodeStream << "(" << xParser.GetFunction() << ")\n";

	//unsigned int uNumberOfLocals = xParser.GetNumberOfArguments();
	//if (uNumberOfLocals > 0)
	//{
	//	m_xAssemblyCodeStream << "@" << uNumberOfLocals << '\n';
	//	m_xAssemblyCodeStream << "D=A\n";
	//	// Reposition SP
	//	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	//	m_xAssemblyCodeStream << "M=M+D\n";
	//}


	m_xAssemblyCodeStream << "(" << xCurrentFuntion << ")\n";

	unsigned int uNumberOfArguments = uNumberOfLocals;
	if (uNumberOfArguments > 0)
	{
		m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
		m_xAssemblyCodeStream << "AD=M\n";
		m_xAssemblyCodeStream << "M=0\n";

		for (unsigned int u = 1; u < uNumberOfLocals; ++u)
		{
			m_xAssemblyCodeStream << "AD=D+1\n";
			m_xAssemblyCodeStream << "M=0\n";
		}

		// Reposition SP
		m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
		m_xAssemblyCodeStream << "M=D+1\n";
	}
}

void Code_Hack::AddReturnCode()
{
	AddCommonUnaryArithmeticCode();
	// Jump to return assembly routine
	m_xAssemblyCodeStream << "@" << m_xInternalReturnRoutine << '\n';
	m_xAssemblyCodeStream << "0;JMP\n";
}

void Code_Hack::AddInternalReturnRoutine()
{
	// call routine label
	m_xAssemblyCodeStream << "(" << m_xInternalReturnRoutine << ")\n";

	//put return value in *ARG
	m_xAssemblyCodeStream << "@" << RamReservedStrings[ARG] << '\n';
	m_xAssemblyCodeStream << "A=M\n";
	m_xAssemblyCodeStream << "M=D\n";

	//Restore SP for calling function
	m_xAssemblyCodeStream << "@" << RamReservedStrings[ARG] << '\n';
	m_xAssemblyCodeStream << "D=M\n"; // D has SP address
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "M=D+1\n";

	//Save LCL address
	m_xAssemblyCodeStream << "@" << RamReservedStrings[LCL] << '\n';
	m_xAssemblyCodeStream << "D=M\n";

	//Save return address in R13
	m_xAssemblyCodeStream << "@5\n";
	m_xAssemblyCodeStream << "A=D-A\n";
	m_xAssemblyCodeStream << "D=M\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[R13] << '\n';
	m_xAssemblyCodeStream << "M=D\n";

	m_xAssemblyCodeStream << "@" << RamReservedStrings[LCL] << '\n';
	m_xAssemblyCodeStream << "AM=M-1\n";
	m_xAssemblyCodeStream << "D=M\n";

	m_xAssemblyCodeStream << "@" << RamReservedStrings[THAT] << '\n';
	m_xAssemblyCodeStream << "M=D\n";

	m_xAssemblyCodeStream << "@" << RamReservedStrings[LCL] << '\n';
	m_xAssemblyCodeStream << "AM=M-1\n";
	m_xAssemblyCodeStream << "D=M\n";

	m_xAssemblyCodeStream << "@" << RamReservedStrings[THIS] << '\n';
	m_xAssemblyCodeStream << "M=D\n";

	m_xAssemblyCodeStream << "@" << RamReservedStrings[LCL] << '\n';
	m_xAssemblyCodeStream << "AM=M-1\n";
	m_xAssemblyCodeStream << "D=M\n";

	m_xAssemblyCodeStream << "@" << RamReservedStrings[ARG] << '\n';
	m_xAssemblyCodeStream << "M=D\n";

	m_xAssemblyCodeStream << "@" << RamReservedStrings[LCL] << '\n';
	m_xAssemblyCodeStream << "AM=M-1\n";
	m_xAssemblyCodeStream << "D=M\n";

	m_xAssemblyCodeStream << "@" << RamReservedStrings[LCL] << '\n';
	m_xAssemblyCodeStream << "M=D\n";

	// Jump to return address
	m_xAssemblyCodeStream << "@" << RamReservedStrings[R13] << '\n';
	m_xAssemblyCodeStream << "A=M\n";
	m_xAssemblyCodeStream << "0;JMP\n";
}

void Code_Hack::AddAddCode()
{
	AddCommonBinaryArithmeticCode();
	m_xAssemblyCodeStream << "M=M+D\n";
}

void Code_Hack::AddSubCode()
{
	AddCommonBinaryArithmeticCode();
	m_xAssemblyCodeStream << "M=M-D\n";
}

void Code_Hack::AddNegCode()
{
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "A=M-1\n";
	m_xAssemblyCodeStream << "M=-M\n";
}

void Code_Hack::AddAndCode()
{
	AddCommonBinaryArithmeticCode();
	m_xAssemblyCodeStream << "M=M&D\n";
}

void Code_Hack::AddOrCode()
{
	AddCommonBinaryArithmeticCode();
	m_xAssemblyCodeStream << "M=M|D\n";
}

void Code_Hack::AddNotCode()
{
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "A=M-1\n";
	m_xAssemblyCodeStream << "M=!M\n";
}

void Code_Hack::AddPushFunctionCode(const string& xFunction)
{
	// push function address
	m_xAssemblyCodeStream << "@" << xFunction << '\n';
	m_xAssemblyCodeStream << "D=A\n";

	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "AM=M+1\n";
	m_xAssemblyCodeStream << "A=A-1\n";
	m_xAssemblyCodeStream << "M=D\n";
}

void Code_Hack::AddComment(const string& xComment)
{
	m_xAssemblyCodeStream << "// " << xComment << '\n';
}

void Code_Hack::AddBootStrapCode()
{
	//Set SP to point to memory address 261 from the initial 256 (to simulate a call to Sys.init)
	m_xAssemblyCodeStream << "@261\n";
	m_xAssemblyCodeStream << "D=A\n";
	m_xAssemblyCodeStream << "@" << RamReservedStrings[SP] << '\n';
	m_xAssemblyCodeStream << "M=D\n";

	//Jump to Sys.init
	m_xAssemblyCodeStream << "@Sys.init\n";
	m_xAssemblyCodeStream << "0;JMP\n";
}

void Code_Hack::OnPostGeneration()
{
	PARENT::OnPostGeneration();
}