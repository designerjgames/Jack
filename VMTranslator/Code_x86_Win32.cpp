#include "Code_x86_Win32.h"

#include <bitset>
#include <iostream>
#include <cctype>
#include <algorithm>

using namespace std;

string Code_x86_Win32::s_xStaticsMarker = "##_statics_marker_##";

Code_x86_Win32::Code_x86_Win32(const string& xFileName)
: PARENT(xFileName)
, m_xDWORD("dword")
{
	RamReservedStrings[LCL]  = "ebp";
	RamReservedStrings[ARG]  = "ecx";
	RamReservedStrings[THIS] = "edx";
	RamReservedStrings[THAT] = "edi";
	RamReservedStrings[R13]  = "esi";
}

Code_x86_Win32::~Code_x86_Win32()
{
}

void Code_x86_Win32::AddCommonUnaryArithmeticCode()
{
	// pop stack into eax
	m_xAssemblyCodeStream << "pop eax\n";
}

void Code_x86_Win32::AddComparisonCode(const string& xSourceFile, VMTokenizer::KEYWORD_TYPE eCommandType)
{
	// Put return address in R13
	string xReturnLabel = xSourceFile + "_" + "RT_" + std::to_string(m_uReturnCount++);
	m_xAssemblyCodeStream << "mov eax, offset " << xReturnLabel << '\n'; // eax has return address now

	// Jump to comp routine
	switch (eCommandType)
	{
		case VMTokenizer::COMMAND_ARITHMETIC_EQ:
		{
			m_xAssemblyCodeStream << "jmp " << m_xInternalComparisonRoutineEQ << '\n';
			break;
		}
		case VMTokenizer::COMMAND_ARITHMETIC_GT:
		{
			m_xAssemblyCodeStream << "jmp " << m_xInternalComparisonRoutineGT << '\n';
			break;
		}
		case VMTokenizer::COMMAND_ARITHMETIC_LT:
		{
			m_xAssemblyCodeStream << "jmp " << m_xInternalComparisonRoutineLT << '\n';
			break;
		}
		case VMTokenizer::COMMAND_ARITHMETIC_GE:
		{
			m_xAssemblyCodeStream << "jmp " << m_xInternalComparisonRoutineGE << '\n';
			break;
		}
		case VMTokenizer::COMMAND_ARITHMETIC_LE:
		{
			m_xAssemblyCodeStream << "jmp " << m_xInternalComparisonRoutineLE << '\n';
			break;
		}
	}

	m_xAssemblyCodeStream << xReturnLabel << ":\n";
}

void Code_x86_Win32::AddInternalComparisonRoutine()
{
	// Comparison routine label
	m_xAssemblyCodeStream << m_xInternalComparisonRoutineEQ << ":\n";
	m_xAssemblyCodeStream << "mov " << RamReservedStrings[R13] << ", eax\n"; // R13 has return address now

	m_xAssemblyCodeStream << "pop eax\n";
	m_xAssemblyCodeStream << "pop ebx\n";
	m_xAssemblyCodeStream << "cmp ebx, eax\n";
	m_xAssemblyCodeStream << "je  __COMP_INTERNAL_SUCCESS__\n";
	m_xAssemblyCodeStream << "jmp __COMP_INTERNAL_FAILURE__\n";

	m_xAssemblyCodeStream << m_xInternalComparisonRoutineGT << ":\n";
	m_xAssemblyCodeStream << "mov " << RamReservedStrings[R13] << ", eax\n"; // R13 has return address now

	m_xAssemblyCodeStream << "pop eax\n";
	m_xAssemblyCodeStream << "pop ebx\n";
	m_xAssemblyCodeStream << "cmp ebx, eax\n";
	m_xAssemblyCodeStream << "jg  __COMP_INTERNAL_SUCCESS__\n";
	m_xAssemblyCodeStream << "jmp __COMP_INTERNAL_FAILURE__\n";

	m_xAssemblyCodeStream << m_xInternalComparisonRoutineLT << ":\n";
	m_xAssemblyCodeStream << "mov " << RamReservedStrings[R13] << ", eax\n"; // R13 has return address now

	m_xAssemblyCodeStream << "pop eax\n";
	m_xAssemblyCodeStream << "pop ebx\n";
	m_xAssemblyCodeStream << "cmp ebx, eax\n";
	m_xAssemblyCodeStream << "jl  __COMP_INTERNAL_SUCCESS__\n";
	m_xAssemblyCodeStream << "jmp __COMP_INTERNAL_FAILURE__\n";

	m_xAssemblyCodeStream << m_xInternalComparisonRoutineGE << ":\n";
	m_xAssemblyCodeStream << "mov " << RamReservedStrings[R13] << ", eax\n"; // R13 has return address now

	m_xAssemblyCodeStream << "pop eax\n";
	m_xAssemblyCodeStream << "pop ebx\n";
	m_xAssemblyCodeStream << "cmp ebx, eax\n";
	m_xAssemblyCodeStream << "jge  __COMP_INTERNAL_SUCCESS__\n";
	m_xAssemblyCodeStream << "jmp __COMP_INTERNAL_FAILURE__\n";

	m_xAssemblyCodeStream << m_xInternalComparisonRoutineLE << ":\n";
	m_xAssemblyCodeStream << "mov " << RamReservedStrings[R13] << ", eax\n"; // R13 has return address now

	m_xAssemblyCodeStream << "pop eax\n";
	m_xAssemblyCodeStream << "pop ebx\n";
	m_xAssemblyCodeStream << "cmp ebx, eax\n";
	m_xAssemblyCodeStream << "jle  __COMP_INTERNAL_SUCCESS__\n";
	m_xAssemblyCodeStream << "jmp __COMP_INTERNAL_FAILURE__\n";

	m_xAssemblyCodeStream << "__COMP_INTERNAL_FAILURE__:\n";
	m_xAssemblyCodeStream << "push 0\n";
	m_xAssemblyCodeStream << "jmp __COMP_INTERNAL_END__\n";
	m_xAssemblyCodeStream << "__COMP_INTERNAL_SUCCESS__:\n";
	m_xAssemblyCodeStream << "push -1\n";;
	m_xAssemblyCodeStream << "__COMP_INTERNAL_END__:\n";

	// Jump to return address in R13
	m_xAssemblyCodeStream << "jmp " << RamReservedStrings[R13] << '\n';
}

void Code_x86_Win32::AddPushSegmentCode(VMTokenizer::KEYWORD_TYPE eSegType, unsigned int uSegIndex, const string& xSourceFile, bool bAddress /*false*/, bool bMemory /*false*/)
{
	switch (eSegType)
	{
		case VMTokenizer::RAM_SEGMENT_CONSTANT:
		{
			if (bMemory)
			{
				m_xAssemblyCodeStream << "push edx\n";
				m_xAssemblyCodeStream << "xor  edx, edx\n";
				m_xAssemblyCodeStream << "mov  eax, " << uSegIndex << '\n';
				m_xAssemblyCodeStream << "imul eax, 4\n";
				m_xAssemblyCodeStream << "pop  edx\n";
				m_xAssemblyCodeStream << "push eax\n";
			}
			else
			{
				m_xAssemblyCodeStream << "push " << uSegIndex << '\n';
			}

			break;
		}
		case VMTokenizer::RAM_SEGMENT_LOCAL:
		{
			if (bAddress)
			{
				m_xAssemblyCodeStream << "mov  eax, " << RamReservedStrings[LCL] << '\n';
				m_xAssemblyCodeStream << "sub  eax, " << (uSegIndex + 1) * 4 << '\n'; // * 4?
				m_xAssemblyCodeStream << "push eax\n";
			}
			else
			{
				if (bMemory)
				{
					m_xAssemblyCodeStream << "push edx\n";
					m_xAssemblyCodeStream << "xor  edx, edx\n";
					m_xAssemblyCodeStream << "mov  ebx,  [" << RamReservedStrings[LCL] << '-' << (uSegIndex + 1) * 4 << "]\n";
					m_xAssemblyCodeStream << "imul eax, ebx, 4\n";
					m_xAssemblyCodeStream << "pop  edx\n";
					m_xAssemblyCodeStream << "push eax\n";
					break;
				}
			}
			m_xAssemblyCodeStream << "push [" << RamReservedStrings[LCL] << '-' << (uSegIndex + 1) * 4 << "]\n";

			break;
		}
		case VMTokenizer::RAM_SEGMENT_THIS:
		case VMTokenizer::RAM_SEGMENT_THAT:
		{
			m_xAssemblyCodeStream << "mov  eax, " << RamReservedStrings[GetRamReservedSymbolFromSegmentType(eSegType)] << '\n';
			m_xAssemblyCodeStream << "add  eax, " << uSegIndex * 4 << '\n'; // * 4?

			if (bAddress)
				m_xAssemblyCodeStream << "push eax\n";
			else
			{
				if (bMemory)
				{
					m_xAssemblyCodeStream << "push edx\n";
					m_xAssemblyCodeStream << "xor  edx, edx\n";
					m_xAssemblyCodeStream << "imul ebx, [eax], 4\n";
					m_xAssemblyCodeStream << "pop  edx\n";
					m_xAssemblyCodeStream << "push ebx\n";
					break;
				}

				m_xAssemblyCodeStream << "push [eax]\n";
			}
				
			break;
		}
		case VMTokenizer::RAM_SEGMENT_ARGUMENT:
		{
			m_xAssemblyCodeStream << "mov  eax, " << RamReservedStrings[ARG] << '\n';
			m_xAssemblyCodeStream << "sub  eax, " << uSegIndex * 4 << '\n'; // * 4?
			if (bAddress)
			{
				m_xAssemblyCodeStream << "push eax\n";
			}
			else
			{
				if (bMemory)
				{
					m_xAssemblyCodeStream << "push edx\n";
					m_xAssemblyCodeStream << "xor  edx, edx\n";
					m_xAssemblyCodeStream << "imul ebx, [eax], 4\n";
					m_xAssemblyCodeStream << "pop  edx\n";
					m_xAssemblyCodeStream << "push ebx\n";
					break;
				}
				m_xAssemblyCodeStream << "push [eax]\n";
			}
				
			break;
		}
		case VMTokenizer::RAM_SEGMENT_TEMP:
		{
			m_xAssemblyCodeStream << "mov  eax, offset " << RamReservedStrings[GetRamReservedSymbolFromSegmentType(eSegType)] << '\n';
			m_xAssemblyCodeStream << "add  eax, " << uSegIndex * 4 << '\n'; // * 4?

			if (bAddress)
				m_xAssemblyCodeStream << "push eax\n";
			else
			{
				if (bMemory)
				{
					m_xAssemblyCodeStream << "push edx\n";
					m_xAssemblyCodeStream << "xor  edx, edx\n";
					m_xAssemblyCodeStream << "imul ebx, [eax], 4\n";
					m_xAssemblyCodeStream << "pop  edx\n";
					m_xAssemblyCodeStream << "push ebx\n";
					break;
				}

				m_xAssemblyCodeStream << "push [eax]\n";
			}

			break;
		}
		case VMTokenizer::RAM_SEGMENT_POINTER:
		{
			string xThisOrThat = (uSegIndex == 0) ? RamReservedStrings[THIS] : RamReservedStrings[THAT];

			if (bAddress)
				m_xAssemblyCodeStream << "push offset" << xThisOrThat << '\n';
			else
				m_xAssemblyCodeStream << "push " << xThisOrThat << '\n';
			break;
		}
		case VMTokenizer::RAM_SEGMENT_STATIC:
		{
			string xVar = xSourceFile;
			xVar += "$";
			xVar += to_string(uSegIndex);
			
			if (bAddress)
				m_xAssemblyCodeStream << "push offset" << xVar << '\n';
			else
				m_xAssemblyCodeStream << "push " << xVar << '\n';

			m_xStaticVariableTable.Add(xVar, xVar);
			break;
		}
	}
}

void Code_x86_Win32::AddPopSegmentCode(VMTokenizer::KEYWORD_TYPE eSegType, unsigned int uSegIndex, const string& xSourceFile)
{
	m_xAssemblyCodeStream << "pop eax" << '\n';

	// Get segment index
	switch (eSegType)
	{
	case VMTokenizer::RAM_SEGMENT_LOCAL:
	{
		m_xAssemblyCodeStream << "mov  [" << RamReservedStrings[LCL] << '-' << (uSegIndex+1) * 4 << "], eax\n";
		break;
	}
	case VMTokenizer::RAM_SEGMENT_THIS:
	case VMTokenizer::RAM_SEGMENT_THAT:
	{
		m_xAssemblyCodeStream << "mov  ebx, " << RamReservedStrings[GetRamReservedSymbolFromSegmentType(eSegType)] << '\n';
		m_xAssemblyCodeStream << "add  ebx, " << uSegIndex * 4 << '\n'; // * 4?
		m_xAssemblyCodeStream << "mov [ebx], eax\n";
		break;
	}
	case VMTokenizer::RAM_SEGMENT_ARGUMENT:
	{
		m_xAssemblyCodeStream << "mov  ebx, " << RamReservedStrings[ARG] << '\n';
		m_xAssemblyCodeStream << "sub  ebx, " << uSegIndex * 4 << '\n'; // * 4?
		m_xAssemblyCodeStream << "mov [ebx], eax\n";
		break;
	}
	case VMTokenizer::RAM_SEGMENT_TEMP:
	{
		m_xAssemblyCodeStream << "mov  ebx, offset " << RamReservedStrings[GetRamReservedSymbolFromSegmentType(eSegType)] << '\n';
		m_xAssemblyCodeStream << "add  ebx, " << uSegIndex * 4 << '\n'; // * 4?
		m_xAssemblyCodeStream << "mov [ebx], eax\n";
		break;
	}
	case VMTokenizer::RAM_SEGMENT_POINTER:
	{
		string xThisOrThat = (uSegIndex == 0) ? RamReservedStrings[THIS] : RamReservedStrings[THAT];
		m_xAssemblyCodeStream << "mov " << xThisOrThat << ", eax\n";
		break;
	}
	case VMTokenizer::RAM_SEGMENT_STATIC:
	{
		string xVar = xSourceFile;
		xVar       += "$";
		xVar       += to_string(uSegIndex);

		m_xAssemblyCodeStream << "mov " << xVar << ", eax\n";
		m_xStaticVariableTable.Add(xVar, xVar);
		break;
	}
	}
}

void Code_x86_Win32::AddLabelCode(const string& xLabel, const string& xCurrentFuntion)
{
	string xFunction = xCurrentFuntion;
	replace(xFunction.begin(), xFunction.end(), '.', '$');

	m_xAssemblyCodeStream << xFunction << "$" << xLabel << ":\n";
}

void Code_x86_Win32::AddGotoCode(const string& xLabel, const string& xCurrentFuntion)
{
	string xFunction = xCurrentFuntion;
	replace(xFunction.begin(), xFunction.end(), '.', '$');

	m_xAssemblyCodeStream << "jmp " << xFunction << "$" << xLabel << '\n';
}

void Code_x86_Win32::AddIfCode(const string& xLabel, const string& xCurrentFuntion)
{
	string xFunction = xCurrentFuntion;
	replace(xFunction.begin(), xFunction.end(), '.', '$');

	AddCommonUnaryArithmeticCode();
	m_xAssemblyCodeStream << "cmp eax, -1\n";
	m_xAssemblyCodeStream << "je offset " << xFunction << "$" << xLabel << '\n';
}

void Code_x86_Win32::AddCallCode(const string& xSourceFileName, const string& xFunction, unsigned int uArgs, bool bVirtual /*=false*/, unsigned int bVirtualFunctionIndex /*=0*/)
{
	string xFixedFunction = xFunction;
	replace(xFixedFunction.begin(), xFixedFunction.end(), '.', '$');

	// Save function address in eax
	if (bVirtual)
	{
		// Stack has function address
		m_xAssemblyCodeStream << "pop eax\n";// stack "pop" of the virtual function baseaddress
		m_xAssemblyCodeStream << "mov ebx, [eax]\n"; // ebx has vpointer addres
		m_xAssemblyCodeStream << "mov eax, [ebx]\n"; // eax has Array addres

		if (bVirtualFunctionIndex != 0)
			m_xAssemblyCodeStream << "add eax, " << bVirtualFunctionIndex * 4 << '\n'; // eax has Array element

		m_xAssemblyCodeStream << "mov eax, [eax]\n";//  eax has virtual function address
	}
	else
	{
		m_xAssemblyCodeStream << "mov eax, offset " << xFixedFunction << '\n';//  eax has virtual function address
	}

	// ebx has jump to function now
	m_xAssemblyCodeStream << "mov ebx, eax\n";

	//If the number of arguments is 0, increase SP to allow for a slot
	//on stack for the function return value
	if (uArgs == 0)
	{
		m_xAssemblyCodeStream << "push 0\n";
	}

	// Save the number of arguments in R13, so they can be accessed by the call routine
	// If the function is called with no arguments, one argument was "pushed" to the stack
	// anyway to store the return value.
	unsigned int uNumberOfArguments = uArgs;
	uNumberOfArguments = (uNumberOfArguments == 0) ? 1 : uNumberOfArguments;

	m_xAssemblyCodeStream << "mov " << RamReservedStrings[R13] << ", " << uNumberOfArguments * 4 << '\n';

	// Put return address in eax
	string xReturnLabel = xSourceFileName + "_" + "RT_" + std::to_string(m_uReturnCount++);
	m_xAssemblyCodeStream << "mov eax, offset " << xReturnLabel << '\n';//  eax has virtual function address

	// Jump to call assembly routine
	/*m_xAssemblyCodeStream << "jmp " << m_xInternalCallRoutine << '\n';*/
	m_xAssemblyCodeStream << "jmp ebx \n";

	m_xAssemblyCodeStream << xReturnLabel << ":\n";
}

void Code_x86_Win32::AddFunctionCode(const string& xCurrentFuntion, unsigned int uNumberOfLocals)
{
	string xFunction = xCurrentFuntion;
	replace(xFunction.begin(), xFunction.end(), '.', '$');

	m_xAssemblyCodeStream << xFunction << ":\n";

	// eax has the return address from the caller
	m_xAssemblyCodeStream << "push eax\n";

	// push local
	m_xAssemblyCodeStream << "push " << RamReservedStrings[LCL] << '\n';
	// push arg
	m_xAssemblyCodeStream << "push " << RamReservedStrings[ARG] << '\n';
	// push this
	m_xAssemblyCodeStream << "push " << RamReservedStrings[THIS] << '\n';
	// push that
	m_xAssemblyCodeStream << "push " << RamReservedStrings[THAT] << '\n';

	m_xAssemblyCodeStream << "mov eax, " << RamReservedStrings[R13] << '\n';

	m_xAssemblyCodeStream << "add eax, 20\n"; // eax has number of arguments + 5 or 5*4 ?

											  // set current frame for ARG and LCL
	m_xAssemblyCodeStream << "mov ebx, esp\n";
	m_xAssemblyCodeStream << "add ebx, eax\n";

	m_xAssemblyCodeStream << "mov " << RamReservedStrings[ARG] << ", ebx\n";
	m_xAssemblyCodeStream << "sub " << RamReservedStrings[ARG] << ", 4\n";
	m_xAssemblyCodeStream << "mov " << RamReservedStrings[LCL] << ", esp\n";

	for (unsigned int u = 0; u < uNumberOfLocals; ++u)
	{
		// push locals
		m_xAssemblyCodeStream << "push 0\n";
	}
}

void Code_x86_Win32::AddReturnCode()
{
	AddCommonUnaryArithmeticCode();

	// Jump to return assembly routine
	m_xAssemblyCodeStream << "jmp " << m_xInternalReturnRoutine << '\n';
}

void Code_x86_Win32::AddInternalReturnRoutine()
{
	// call routine label
	m_xAssemblyCodeStream << m_xInternalReturnRoutine << ":\n";

	// put return value that is on eax in *ARG
	m_xAssemblyCodeStream << "mov ebx, " << RamReservedStrings[ARG] << '\n';
	m_xAssemblyCodeStream << "mov [ebx], eax\n";

	// save new SP for calling function in ebx
	m_xAssemblyCodeStream << "mov ebx, " << RamReservedStrings[ARG] << '\n';

	// Set SP as local
	m_xAssemblyCodeStream << "mov esp, " << RamReservedStrings[LCL] << '\n';

	// pop that
	m_xAssemblyCodeStream << "pop " << RamReservedStrings[THAT] << '\n';

	// pop this
	m_xAssemblyCodeStream << "pop " << RamReservedStrings[THIS] << '\n';

	// pop arg
	m_xAssemblyCodeStream << "pop " << RamReservedStrings[ARG] << '\n';

	// pop lcl
	m_xAssemblyCodeStream << "pop " << RamReservedStrings[LCL] << '\n';

	// pop return address into eax
	m_xAssemblyCodeStream << "pop eax\n";

	// set correct esp now
	m_xAssemblyCodeStream << "mov esp, ebx\n";

	// Jump to return address
	m_xAssemblyCodeStream << "jmp eax \n";
}

void Code_x86_Win32::AddAddCode()
{
	// Pop both operands into EAX and EBX registers
	m_xAssemblyCodeStream << "pop  eax\n";
	m_xAssemblyCodeStream << "pop  ebx\n";
	
	// Add
	m_xAssemblyCodeStream << "add  eax, ebx\n";

	// Push result into stack
	m_xAssemblyCodeStream << "push eax\n";
}

void Code_x86_Win32::AddSubCode()
{
	// Pop both operands into EAX and EBX registers
	m_xAssemblyCodeStream << "pop  eax\n";
	m_xAssemblyCodeStream << "pop  ebx\n";

	// Add
	m_xAssemblyCodeStream << "sub  ebx, eax\n";

	// Push result into stack
	m_xAssemblyCodeStream << "push ebx\n";
}

void Code_x86_Win32::AddNegCode()
{
	m_xAssemblyCodeStream << "mov eax, [esp]\n";
	m_xAssemblyCodeStream << "neg eax\n";
	m_xAssemblyCodeStream << "mov [esp], eax\n";
}

void Code_x86_Win32::AddAndCode()
{
	// Pop both operands into EAX and EBX registers
	m_xAssemblyCodeStream << "pop  eax\n";
	m_xAssemblyCodeStream << "pop  ebx\n";

	// Add
	m_xAssemblyCodeStream << "and  eax, ebx\n";

	// Push result into stack
	m_xAssemblyCodeStream << "push eax\n";
}

void Code_x86_Win32::AddOrCode()
{
	// Pop both operands into EAX and EBX registers
	m_xAssemblyCodeStream << "pop  eax\n";
	m_xAssemblyCodeStream << "pop  ebx\n";

	// Add
	m_xAssemblyCodeStream << "or   eax, ebx\n";

	// Push result into stack
	m_xAssemblyCodeStream << "push eax\n";
}

void Code_x86_Win32::AddNotCode()
{
	m_xAssemblyCodeStream << "mov eax, [esp]\n";
	m_xAssemblyCodeStream << "not eax\n";
	m_xAssemblyCodeStream << "mov [esp], eax\n";
}

void Code_x86_Win32::AddPushFunctionCode(const string& xFunction)
{
	string xFixedFunction = xFunction;
	replace(xFixedFunction.begin(), xFixedFunction.end(), '.', '$');

	m_xAssemblyCodeStream << "push offset " << xFixedFunction << '\n';
}

void Code_x86_Win32::AddComment(const string& xComment)
{
	m_xAssemblyCodeStream << "; " << xComment << '\n';
}

void Code_x86_Win32::AddBootStrapCode()
{
	m_xAssemblyCodeStream << ".386\n";
	m_xAssemblyCodeStream << ".model flat, stdcall\n";
	m_xAssemblyCodeStream << "option casemap : none\n";
	m_xAssemblyCodeStream << "include \\masm32\\include\\windows.inc\n";
	m_xAssemblyCodeStream << "include \\masm32\\include\\kernel32.inc\n";
	m_xAssemblyCodeStream << "include \\masm32\\include\\gdi32.inc\n";
	m_xAssemblyCodeStream << "include \\masm32\\include\\user32.inc\n";

	m_xAssemblyCodeStream << "includelib \\masm32\\lib\\kernel32.lib\n";
	m_xAssemblyCodeStream << "includelib \\masm32\\lib\\gdi32.lib\n";
	m_xAssemblyCodeStream << "includelib \\masm32\\lib\\user32.lib\n";

	m_xAssemblyCodeStream << "WinMain PROTO : DWORD, : DWORD, : DWORD, : DWORD\n";
	m_xAssemblyCodeStream << "WndProc PROTO : DWORD, : DWORD, : DWORD, : DWORD\n";

	m_xAssemblyCodeStream << ".data\n";

	m_xAssemblyCodeStream << RamReservedStrings[R0]   << " " << m_xDWORD << " 0" << '\n';
	m_xAssemblyCodeStream << RamReservedStrings[R1]   << " " << m_xDWORD << " 0" << '\n';
	m_xAssemblyCodeStream << RamReservedStrings[R2]   << " " << m_xDWORD << " 0" << '\n';
	m_xAssemblyCodeStream << RamReservedStrings[R3]   << " " << m_xDWORD << " 0" << '\n';
	m_xAssemblyCodeStream << RamReservedStrings[R4]   << " " << m_xDWORD << " 0" << '\n';
	m_xAssemblyCodeStream << RamReservedStrings[R5]   << " " << m_xDWORD << " 0" << '\n';
	m_xAssemblyCodeStream << RamReservedStrings[R6]   << " " << m_xDWORD << " 0" << '\n';
	m_xAssemblyCodeStream << RamReservedStrings[R7]   << " " << m_xDWORD << " 0" << '\n';
	m_xAssemblyCodeStream << RamReservedStrings[R8]   << " " << m_xDWORD << " 0" << '\n';
	m_xAssemblyCodeStream << RamReservedStrings[R9]   << " " << m_xDWORD << " 0" << '\n';
	m_xAssemblyCodeStream << RamReservedStrings[R10]	<< " " << m_xDWORD << " 0" << '\n';
	m_xAssemblyCodeStream << RamReservedStrings[R11]	<< " " << m_xDWORD << " 0" << '\n';
	m_xAssemblyCodeStream << RamReservedStrings[R12]	<< " " << m_xDWORD << " 0" << '\n';
	m_xAssemblyCodeStream << RamReservedStrings[R14]    << " " << m_xDWORD << " 0" << '\n';
	m_xAssemblyCodeStream << RamReservedStrings[R15]	<< " " << m_xDWORD << " 0" << '\n';

	// Windows vars
	
	m_xAssemblyCodeStream << "ScreenHandle" << " " << m_xDWORD << " 0 " << '\n';
	m_xAssemblyCodeStream << "HeapHandle" << " " << m_xDWORD << " 0 " << '\n';


	m_xAssemblyCodeStream << "ClassName db \"Jack\"  , 0\n";
	m_xAssemblyCodeStream << "AppName   db \"Jack on Win\", 0\n";
	m_xAssemblyCodeStream << "KeyState UCHAR  256 dup(0);\n";
	m_xAssemblyCodeStream << "ScanCode" << " " << m_xDWORD << " 0 " << '\n';
	m_xAssemblyCodeStream << "VKCode" << " " << m_xDWORD << " 0 " << '\n';

	m_xAssemblyCodeStream << s_xStaticsMarker << '\n';

	m_xAssemblyCodeStream << ".DATA?\n";
	m_xAssemblyCodeStream << "ScreenRect RECT <>\n";
	m_xAssemblyCodeStream << "DeviceContextHandle HDC ?\n";
	m_xAssemblyCodeStream << "BackBufferDeviceContextHandle HDC ?\n";
	m_xAssemblyCodeStream << "hBlackPixel HBRUSH ?\n";
	m_xAssemblyCodeStream << "hWhitePixel HBRUSH ?\n";
	m_xAssemblyCodeStream << "hBitmap HBITMAP ?\n";
	m_xAssemblyCodeStream << "msg MSG <>\n";
	m_xAssemblyCodeStream << "wc WNDCLASSEX <>\n";
	m_xAssemblyCodeStream << "hInst HINSTANCE ?\n";
	m_xAssemblyCodeStream << "Ps PAINTSTRUCT <>\n";
	m_xAssemblyCodeStream << "rct RECT <>\n";
	m_xAssemblyCodeStream << "layout HKL ?\n";

	m_xAssemblyCodeStream << ".code\n";

	m_xAssemblyCodeStream << "start proc\n";

	m_xAssemblyCodeStream << "invoke GetModuleHandle, NULL\n";
	m_xAssemblyCodeStream << "mov hInst, eax\n";

	m_xAssemblyCodeStream << "mov " << RamReservedStrings[LCL] << ", esp\n";
	m_xAssemblyCodeStream << "mov " << RamReservedStrings[ARG] << ", esp\n";
	m_xAssemblyCodeStream << "sub " << RamReservedStrings[ARG] << ", 4\n";

	m_xAssemblyCodeStream << "jmp Sys$init\n";
}

void Code_x86_Win32::OnPostGeneration()
{
	m_xAssemblyCodeStream << "start endp\n";

	m_xAssemblyCodeStream << "WndProc proc hWnd : HWND, uMsg : UINT, wParam : WPARAM, lParam : LPARAM\n";
	m_xAssemblyCodeStream << ".IF uMsg == WM_DESTROY\n";
	m_xAssemblyCodeStream << "invoke PostQuitMessage, NULL\n";

	m_xAssemblyCodeStream << ".ELSEIF uMsg == WM_KEYUP\n";
	m_xAssemblyCodeStream << "mov ScanCode, 0\n";

	m_xAssemblyCodeStream << ".ELSEIF uMsg == WM_KEYDOWN\n";
	m_xAssemblyCodeStream << "mov eax, wParam\n";
	m_xAssemblyCodeStream << "mov VKCode, eax\n";
	m_xAssemblyCodeStream << "mov eax, LPARAM\n";
	m_xAssemblyCodeStream << "mov ScanCode, eax\n";

	m_xAssemblyCodeStream << ".ELSEIF uMsg == WM_PAINT\n";
	m_xAssemblyCodeStream << "invoke BeginPaint, ScreenHandle, offset Ps\n";
	m_xAssemblyCodeStream << "invoke EndPaint, ScreenHandle, offset Ps\n";

	m_xAssemblyCodeStream << ".ELSE\n";
	m_xAssemblyCodeStream << "invoke DefWindowProc, hWnd, uMsg, wParam, lParam\n";
	m_xAssemblyCodeStream << "ret\n";
	m_xAssemblyCodeStream << ".ENDIF\n";
	m_xAssemblyCodeStream << "xor eax, eax\n";
	m_xAssemblyCodeStream << "ret\n";
	m_xAssemblyCodeStream << "WndProc endp\n";

	m_xAssemblyCodeStream << "end start\n";

	PARENT::OnPostGeneration();

	string xStatics;
	for (unordered_map<string, const string>::const_iterator it = m_xStaticVariableTable.GetTable().begin(); it != m_xStaticVariableTable.GetTable().end(); ++it)
	{
		// Replace the field marker now with the actual code
		// Get the VM file
		xStatics += it->second;
		xStatics += " ";
		xStatics += m_xDWORD;
		xStatics += " 0\n";
	}

	size_t iFoundPosition = m_xAssemblyCode.find(s_xStaticsMarker);
	if (iFoundPosition != std::string::npos)
		m_xAssemblyCode.replace(iFoundPosition, s_xStaticsMarker.length(), xStatics);
}