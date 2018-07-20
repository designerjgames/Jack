// Compiler.cpp : Defines the entry point for the console application.
//
#include "Parser.h"
#include "Resolver.h"
#include "CompilerTokenizer.h"
#include <windows.h> 
#include <string>
#include <vector>
#include <iostream> 

using namespace std;

void BuildJackFilesList();

std::vector<std::string> gJackFileNames;

int main(/*int argc, char* argv[]*/)
{
	BuildJackFilesList();

	bool bCompileSuccess = true;
	for( unsigned int u = 0; u < gJackFileNames.size(); ++u )
	{
		//Check if we didn't compile this one already
		const Parser* pxParser = Parser::GetParsersSymbolTable().GetSymbol(gJackFileNames[u]);
		if (!pxParser)
		{
			CompilerTokenizer m_xTokenizer(gJackFileNames[u]);
			Parser			  m_xParser(gJackFileNames[u], m_xTokenizer);

			bCompileSuccess &= m_xParser.Compile();

			if (bCompileSuccess)
			{
				VMCode xVMCode;
				xVMCode.m_xName = gJackFileNames[u];
				xVMCode.m_xCode = m_xParser.GetVMCode().str();
				Resolver::AddVMCode(xVMCode);
			}

			// print out error
			cout << m_xParser.GetError();
		}
	}

	// Time to resolve symbols, and check types
	if (bCompileSuccess)
	{
		// Write unresolved vm files here for debug
		Resolver::WriteVMFiles(true);

		if (Resolver::Resolve())
		{
			// Write the vm files
			Resolver::WriteVMFiles();
		}
		else
		{
			Resolver::PrintErrors();
		}
	}

	return 0;
}

void BuildJackFilesList()
{
	HANDLE hFind;
	WIN32_FIND_DATA FindData;

	cout << "Build VM file list\n" << endl;

	// Find the first file
	hFind = FindFirstFile( L"*.jack", &FindData );
	if( hFind != INVALID_HANDLE_VALUE )
	{
		//std::cout << FindData.cFileName << endl;

		char ch[260];
		char defChar = ' ';
		WideCharToMultiByte( CP_ACP, 0, FindData.cFileName, -1, ch, 260, &defChar, NULL );

		gJackFileNames.push_back( ch );
	}

	// Look for more
	while( FindNextFile( hFind, &FindData ) )
	{
		//cout << FindData.cFileName << endl;

		char ch[260];
		char defChar = ' ';
		WideCharToMultiByte( CP_ACP, 0, FindData.cFileName, -1, ch, 260, &defChar, NULL );

		gJackFileNames.push_back( ch );
	}

	FindClose( hFind );
}

