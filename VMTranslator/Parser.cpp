#include "Parser.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

Parser::Parser(const string& xFilename, VMTokenizer& xTokenizer, Code& xCode )
: m_xError()
, m_xTokenizer(xTokenizer)
, m_xCode(xCode)
, m_xFile()
, m_xSourceFileNameWithoutExtention()
, m_xCurrentFunctionName()
{
	m_xSourceFileNameWithoutExtention = xFilename.substr(0, xFilename.length() - 3);
	m_xFile.open(xFilename);
	if (m_xFile.fail())
	{
		cout << "file opening failed" << endl;
	}
}

Parser::~Parser()
{
	m_xFile.close();
}

bool Parser::Compile()
{
	bool bNeedToGetNextToken = true;
	while( !bNeedToGetNextToken || m_xTokenizer.GetNextToken() )
	{
		bNeedToGetNextToken = true;

		m_xCode.AddComment(m_xTokenizer.GetTokenValue() );
		Tokenizer::TOKEN_TYPE eCmdType = m_xTokenizer.GetTokenType();

		switch (eCmdType)
		{
			case Tokenizer::TOKEN_KEYWORD:
			{
				VMTokenizer::KEYWORD_TYPE eKeywordType = VMTokenizer::GetKeywordType(m_xTokenizer.GetTokenValue());
				switch (eKeywordType)
				{
					case VMTokenizer::COMMAND_PUSH:
					case VMTokenizer::COMMAND_PUSH_ADDRESS:
					{
						m_xTokenizer.GetNextToken();
						// This should be segment

						if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_KEYWORD)
						{
							// Check it's a segment type
							if (!IsKeywordCommand(m_xTokenizer.GetTokenValue()))
							{
								// Good
								VMTokenizer::KEYWORD_TYPE eSegType = m_xTokenizer.GetKeywordType(m_xTokenizer.GetTokenValue());

								// Index, constant should follow
								m_xTokenizer.GetNextToken();
								if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_INT_CONSTANT)
								{
									// All good
									bool bAddress = (eKeywordType == VMTokenizer::COMMAND_PUSH_ADDRESS) ? true : false;

									string xIndex = m_xTokenizer.GetTokenValue();

									bool bMemory = false;
									// Special case for statics, statics have their own filename if accessed as fields
									if (eSegType == VMTokenizer::RAM_SEGMENT_STATIC)
									{
										m_xTokenizer.GetNextToken();
										if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_INDENTIFIER)
										{
											string xClassName = m_xTokenizer.GetTokenValue().substr(1, m_xTokenizer.GetTokenValue().size());
											m_xTokenizer.GetNextToken();
											if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_KEYWORD && VMTokenizer::GetKeywordType(m_xTokenizer.GetTokenValue()) == VMTokenizer::COMMAND_MEMORY)
											{
												bMemory = true;
											}
											else
											{
												bNeedToGetNextToken = false;
											}

											m_xCode.AddPushSegmentCode(eSegType, stoi(xIndex), xClassName, bAddress, bMemory);
											break;
										}
										return false;
									}

									m_xTokenizer.GetNextToken();
									if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_KEYWORD && VMTokenizer::GetKeywordType(m_xTokenizer.GetTokenValue()) == VMTokenizer::COMMAND_MEMORY)
									{
										bMemory = true;
									}
									else
									{
										bNeedToGetNextToken = false;
									}

									m_xCode.AddPushSegmentCode(eSegType, stoi(xIndex), m_xSourceFileNameWithoutExtention, bAddress, bMemory);
									break;
								}
							}
						}
						return false;
					}
					case VMTokenizer::COMMAND_POP:
					{
						m_xTokenizer.GetNextToken();
						// This should be segment

						if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_KEYWORD)
						{
							// Check it's a segment type
							if (!IsKeywordCommand(m_xTokenizer.GetTokenValue()))
							{
								// Good
								VMTokenizer::KEYWORD_TYPE eSegType = m_xTokenizer.GetKeywordType(m_xTokenizer.GetTokenValue());

								// Index, constant should follow
								m_xTokenizer.GetNextToken();
								if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_INT_CONSTANT)
								{
									// All good
									// Special case for statics, statics have their own filename if accessed as fields
									if (eSegType == VMTokenizer::RAM_SEGMENT_STATIC)
									{
										string xIndex = m_xTokenizer.GetTokenValue();
										m_xTokenizer.GetNextToken();
										if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_INDENTIFIER)
										{
											string xClassName = m_xTokenizer.GetTokenValue().substr(1, m_xTokenizer.GetTokenValue().size());
											m_xCode.AddPopSegmentCode(eSegType, stoi(xIndex), xClassName);
											break;
										}
										return false;
									}

									m_xCode.AddPopSegmentCode(eSegType, stoi(m_xTokenizer.GetTokenValue()), m_xSourceFileNameWithoutExtention);
									break;
								}
							}
						}
						return false;
					}
					case VMTokenizer::COMMAND_ARITHMETIC_ADD:
					{
						m_xCode.AddAddCode();
						break;
					}
					case VMTokenizer::COMMAND_ARITHMETIC_SUB:
					{
						m_xCode.AddSubCode();
						break;
					}
					case VMTokenizer::COMMAND_ARITHMETIC_NEG:
					{
						m_xCode.AddNegCode();
						break;
					}
					case VMTokenizer::COMMAND_ARITHMETIC_AND:
					{
						m_xCode.AddAndCode();
						break;
					}
					case VMTokenizer::COMMAND_ARITHMETIC_OR:
					{
						m_xCode.AddOrCode();
						break;
					}
					case VMTokenizer::COMMAND_ARITHMETIC_NOT:
					{
						m_xCode.AddNotCode();
						break;
					}
					case VMTokenizer::COMMAND_ARITHMETIC_EQ:
					case VMTokenizer::COMMAND_ARITHMETIC_GT:
					case VMTokenizer::COMMAND_ARITHMETIC_LT:
					case VMTokenizer::COMMAND_ARITHMETIC_GE:
					case VMTokenizer::COMMAND_ARITHMETIC_LE:
					{
						m_xCode.AddComparisonCode(m_xSourceFileNameWithoutExtention, eKeywordType);
						break;
					}
					case VMTokenizer::COMMAND_LABEL:
					{
						m_xTokenizer.GetNextToken();
						// This should be identifier

						if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_INDENTIFIER)
						{
							m_xCode.AddLabelCode(m_xTokenizer.GetTokenValue(), m_xCurrentFunctionName);
							break;
						}

						return false;
					}
					case VMTokenizer::COMMAND_GOTO:
					{
						m_xTokenizer.GetNextToken();
						// This should be identifier

						if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_INDENTIFIER)
						{
							m_xCode.AddGotoCode(m_xTokenizer.GetTokenValue(), m_xCurrentFunctionName);
							break;
						}

						return false;
					}

					case VMTokenizer::COMMAND_IF:
					{
						m_xTokenizer.GetNextToken();
						// This should be identifier

						if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_INDENTIFIER)
						{
							m_xCode.AddIfCode(m_xTokenizer.GetTokenValue(), m_xCurrentFunctionName);
							break;
						}

						return false;
					}

					case VMTokenizer::COMMAND_CALL:
					{
						m_xTokenizer.GetNextToken();
						// This should be identifier

						if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_INDENTIFIER)
						{
							// Function name
							string xFunctionToCall = m_xTokenizer.GetTokenValue();

							// Next are number of args
							m_xTokenizer.GetNextToken();
							if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_INT_CONSTANT)
							{
								// Good, call code
								m_xCode.AddCallCode(m_xSourceFileNameWithoutExtention, xFunctionToCall, stoi(m_xTokenizer.GetTokenValue()));
								break;
							}
						}

						return false;
					}

					case VMTokenizer::COMMAND_CALL_VIRTUAL:
					{
						// Next are number of args
						m_xTokenizer.GetNextToken();
						if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_INT_CONSTANT)
						{
							unsigned int uArgs = stoi(m_xTokenizer.GetTokenValue());
							// Next is virtual function index on the Vtable
							m_xTokenizer.GetNextToken();
							if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_INT_CONSTANT)
							{
								// Good, call code
								m_xCode.AddCallCode(m_xSourceFileNameWithoutExtention, "", uArgs, true, stoi(m_xTokenizer.GetTokenValue()));
								break;
							}
						}

						return false;
					}

					case VMTokenizer::COMMAND_FUNCTION:
					{
						m_xTokenizer.GetNextToken();
						// This should be identifier

						if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_INDENTIFIER)
						{
							// class name
							m_xCurrentFunctionName = m_xTokenizer.GetTokenValue();

							// Next are number of locals
							m_xTokenizer.GetNextToken();
							if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_INT_CONSTANT)
							{
								// Good, call code
								m_xCode.AddFunctionCode(m_xCurrentFunctionName, stoi(m_xTokenizer.GetTokenValue()));
								break;
							}
						}

						return false;
					}

					case VMTokenizer::COMMAND_RETURN:
					{
						m_xCode.AddReturnCode();
						break;
					}

					case VMTokenizer::COMMAND_ASM:
					{
						bool bWroteASM = false;

						//write everithing until hittin }
						m_xTokenizer.GetNextToken();
						if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_SYMBOL && m_xTokenizer.GetTokenValue() == "{")
						{
							m_xTokenizer.SetAllIdentifiersConform(true);
							string xAsm;
							while (m_xTokenizer.GetNextToken())
							{
								if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_SYMBOL && m_xTokenizer.GetTokenValue() == ";")
								{
									m_xCode.AddString(xAsm);
									xAsm = "";
									continue;
								}
								else if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_SYMBOL && m_xTokenizer.GetTokenValue() == "}")
								{
									m_xTokenizer.SetAllIdentifiersConform(false);
									bWroteASM = true;
									break;
								}
								xAsm += m_xTokenizer.GetTokenValue();
								xAsm += " ";
							}
						}
						else
						{
							return false;
						}

						if (bWroteASM)
							break;
						else
							return false;
					}

					case VMTokenizer::COMMAND_PUSH_FUNCTION:
					{
						m_xTokenizer.GetNextToken();
						// This should be identifier

						if (m_xTokenizer.GetTokenType() == Tokenizer::TOKEN_INDENTIFIER)
						{
							// Good, call code
							m_xCode.AddPushFunctionCode(m_xTokenizer.GetTokenValue());
							break;
						}

						return false;
					}

					default:
					{
						return false;
					}
				}

				break;
			}
			default:
			{
				return false;
			}
		}
	}

	return true;
}

bool Parser::IsKeywordCommand(const string& xKeyword) const
{
	if ((xKeyword == "static")
		|| (xKeyword == "this")
		|| (xKeyword == "local")
		|| (xKeyword == "argument")
		|| (xKeyword == "that")
		|| (xKeyword == "constant")
		|| (xKeyword == "pointer")
		|| (xKeyword == "temp"))
		return false;

	return true;
}