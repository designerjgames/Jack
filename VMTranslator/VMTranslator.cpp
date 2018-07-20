// Assembler.cpp : Defines the entry point for the console application.
//
#include "Code_Hack.h"
#include "Code_x86_Win32.h"
#include "Parser.h"
#include <windows.h> 
#include <string>
#include <vector>
#include <iostream> 

using namespace std;

void BuildVMFilesList();

std::vector<std::string> gVMFileNames;
int main(/*int argc, char* argv[]*/)
{
	BuildVMFilesList();

	if( gVMFileNames.size() > 0 )
	{
		Code_x86_Win32 xCode("program.asm");

		xCode.AddBootStrapCode();
		xCode.AddInternalRoutinesCode();

		for( unsigned int u = 0; u < gVMFileNames.size(); ++u )
		{
			VMTokenizer xTokenizer(gVMFileNames[u]);
			Parser		xParser(gVMFileNames[u], xTokenizer, xCode);

			if (!xParser.Compile())
				return 0;
		}

		xCode.OnPostGeneration();
		xCode.WriteAssemblyFile();
	}

	return 0;
}

void BuildVMFilesList()
{
	HANDLE hFind;
	WIN32_FIND_DATA FindData;

	cout << "Build VM file list\n" << endl;

	// Find the first file
	hFind = FindFirstFile( L"*.vm", &FindData );
	if( hFind != INVALID_HANDLE_VALUE )
	{
		std::cout << FindData.cFileName << endl;

		char ch[260];
		char defChar = ' ';
		WideCharToMultiByte( CP_ACP, 0, FindData.cFileName, -1, ch, 260, &defChar, NULL );

		gVMFileNames.push_back( ch );
	}
	
	// Look for more
	while( FindNextFile( hFind, &FindData ) )
	{
		cout << FindData.cFileName << endl;

		char ch[260];
		char defChar = ' ';
		WideCharToMultiByte( CP_ACP, 0, FindData.cFileName, -1, ch, 260, &defChar, NULL );

		gVMFileNames.push_back( ch );
	}

	FindClose( hFind );
}

