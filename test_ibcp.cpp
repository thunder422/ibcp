// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_ibcp.cpp - tester class source file
//	Copyright (C) 2010-2014  Thunder422
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	For a copy of the GNU General Public License,
//	see <http://www.gnu.org/licenses/>.
//
//
//	Change History:
//
//	2010-03-01	initial version

#include <fstream>
#include <iomanip>
#include <iostream>

#include "test_ibcp.h"
#include "table.h"
#include "token.h"
#include "parser.h"
#include "statusmessage.h"
#include "utility.h"


// overloaded output stream operator for abbreviated contents of a token
std::ostream &operator<<(std::ostream &os, const TokenPtr &token)
{
	bool second {};

	if (token->isCommand())
	{
		if (token->isCode(Rem_Code))
		{
			os << token->name();
			second = true;
		}
		else
		{
			os << token->name();
			if (!token->name2().empty())
			{
				os << '-' << token->name2();
			}
		}
	}
	else if (token->isOperator())
	{
		if (token->isCode(RemOp_Code))
		{
			os << token->name();
			second = true;
		}
		else
		{
			os << token->debugName();
		}
	}
	else
	{
		switch (token->type())
		{
		case Type::NoParen:
			os << token->stringWithDataType();
			if (token->hasFlag(Reference_Flag))
			{
				os << "<ref>";
			}
			break;

		case Type::DefFuncNoArgs:
			os << token->stringWithDataType();
			break;

		case Type::DefFunc:
		case Type::Paren:
			os << token->stringWithDataType() << '(';
			break;

		case Type::Constant:
			switch (token->dataType())
			{
			case DataType::Integer:
			case DataType::Double:
				os << token->string();
				if (token->dataType() == DataType::Integer)
				{
					os << "%";
				}
				break;

			case DataType::String:
				os << '"' << token->string() << '"';
				break;
			default:
				break;
			}
			break;

		default:
			// output debug name by default
			os << token->debugName();
			break;
		}
	}
	if (token->reference())
	{
		os << "<ref>";
	}
	if (token->hasSubCode())
	{
		os << '\'';
		if (!token->hasFlag(Command_Flag) && token->hasSubCode(Paren_SubCode))
		{
			os << ')';
		}
		if (token->hasSubCode(Option_SubCode))
		{
			std::string option {token->optionName()};
			if (option.empty())
			{
				os << "BUG";
			}
			else
			{
				os << option;
			}
		}
		if (token->hasFlag(Command_Flag) && token->hasSubCode(Colon_SubCode))
		{
			os << ':';
		}
		if (token->hasSubCode(Double_SubCode))
		{
			os << "Double";
		}
		os << '\'';
	}
	if (second)
	{
		os << '|' << token->string() << '|';
	}
	return os;
}


// overloaded output stream operator for abbreviated contents of rpn list
std::ostream &operator<<(std::ostream &os, const RpnList &rpnList)
{
	std::unordered_map<RpnItemPtr, int> itemIndex;
	int index {};

	for (RpnItemPtr rpnItem : rpnList)
	{
		if (index > 0)
		{
			os << ' ';
		}
		itemIndex[rpnItem] = index++;
		os << rpnItem->token();
		if (rpnItem->attachedCount() > 0)
		{
			char separator {'['};
			for (RpnItemPtr item : rpnItem->attached())
			{
				if (item)
				{
					os << separator << itemIndex[item] << ':' << item->token();
					separator = ',';
				}
			}
			if (separator != '[')
			{
				os << ']';
			}
		}
	}
	return os;
}


// overloaded output stream operator for abbreviated contents of rpn list
std::ostream &operator<<(std::ostream &os, const Dictionary *const dictionary)
{
	for (size_t i {}; i < dictionary->m_iterator.size(); i++)
	{
		if (dictionary->m_iterator[i] != dictionary->m_keyMap.end())
		{
			auto iterator = dictionary->m_iterator[i];
			os << i << ": " << iterator->second.m_useCount << " |"
				<< iterator->first << "|\n";
		}
	}
	os << "Free:";
	if (dictionary->m_freeStack.empty())
	{
		os << " none";
	}
	else
	{
		std::stack<uint16_t> tempStack = dictionary->m_freeStack;
		while (!tempStack.empty())
		{
			int index {tempStack.top()};
			tempStack.pop();
			os << ' ' << index;
			if (dictionary->m_iterator[index] != dictionary->m_keyMap.end())
			{
				os << '|' << dictionary->m_iterator[index]->first << '|';
			}
		}
	}
	os << '\n';
	return os;
}


// function to process a test input file specified on the command line
// or accept input lines from the user
Tester::Tester(const std::string &programName,
	const std::list<std::string> &args, std::ostream &cout) :
	m_programName {programName},
	m_cout(cout),
	m_programUnit {new ProgramModel}
{
	std::unordered_map<Option, std::string, EnumClassHash> name {
		{Option::Parser,     "parser"},
		{Option::Expression, "expression"},
		{Option::Translator, "translator"},
		{Option::Encoder,    "encoder"}
	};

	// scan arguments for test options (ignore others)
	m_option = Option{};
	m_recreate = false;
	switch (args.size())
	{
	case 1:
		if (isOption(args.front(), "-tp", Option::Parser, name[Option::Parser])
			|| isOption(args.front(), "-te", Option::Expression,
				name[Option::Expression])
			|| isOption(args.front(), "-tt", Option::Translator,
				name[Option::Translator])
			|| isOption(args.front(), "-tc", Option::Encoder,
				name[Option::Encoder])
			|| isOption(args.front(), "-tr", Option::Recreator, "recreator"))
		{
			break;
		}
		// not one of the above options, fall through
	case 2:
		if (args.front() == "-to")
		{
			m_recreate = true;
		}
		else if (args.front() != "-t")
		{
			break;  // no test option found
		}
		if (args.size() == 1)
		{
			throw m_programName + ": missing test file name";
		}
		else
		{
			// find start of file name less path
			m_testFileName = args.back();
			std::string baseName {Utility::baseFileName(m_testFileName)};

			for (auto iterator : name)
			{
				// check beginning of file name
				if (noCaseStringBeginsWith(baseName, iterator.second))
				{
					m_option = iterator.first;
					m_testName = iterator.second;
					break;
				}
			}
			if (m_option == Option{})  // no matching names?
			{
				std::string errorMessage = "usage: " + m_programName + " -t";
				if (m_recreate)
				{
					errorMessage += 'o';
				}
				errorMessage += " (";
				if (m_recreate)
				{
					errorMessage += name[Option::Parser] + '|';
				}
				errorMessage += name[Option::Expression] + '|'
					+ name[Option::Translator] + ")[xx]";
				throw errorMessage;
			}
			else if (m_option == Option::Parser && m_recreate)
			{
				throw m_programName + ": cannot use -to with "
					+ name[Option::Parser] + " files";
			}
		}
	}
	// ignore non-test or invalid test options
}


// function to see if argument is expected option
bool Tester::isOption(const std::string &arg, const std::string &exp,
	Option option, std::string name)
{
	if (arg == exp)
	{
		m_option = option;
		m_testName = name;
		return true;
	}
	return false;
}


// function to return list of valid options
std::string Tester::options()
{
	return "-t <test_file>|-tp|-te|-tt|-tc|-tr|-to <test_file>";
}


void Tester::operator()(std::string copyrightStatement)
{
	std::ifstream ifs;

	bool inputMode {m_testFileName.empty()};
	if (!inputMode)
	{
		ifs.open(m_testFileName);
		if (!ifs.is_open())
		{
			throw m_programName + ": error opening '" + m_testFileName + '\'';
		}
	}

	if (inputMode)
	{
		m_cout << m_programName << copyrightStatement << "\n\n";

		m_cout << "This program comes with ABSOLUTELY NO WARRANTY.\n";
		m_cout << "This is free software, and you are welcome to\n";
		m_cout << "redistribute it under certain conditions.\n";

		m_cout << "\nTable initialization successful.\n";

		m_cout << "\nTesting " << m_testName;
	}

	for (int lineno {1};; lineno++)
	{
		std::string inputLine;

		if (inputMode)
		{
			m_cout << "\nInput: " << std::flush;
			std::getline(std::cin, inputLine);
			if (inputLine.empty() || inputLine[0] == '\n')
			{
				break;
			}
		}
		else
		{
			if (!std::getline(ifs, inputLine))
			{
				break;  // no more lines
			}
			if (m_option != Option::Encoder)
			{
				if (inputLine.empty() || inputLine[0] == '#')
				{
					continue;  // skip blank and comment lines
				}
				printInput(inputLine);
			}
			else if (!inputLine.empty() && inputLine[0] == '#')
			{
				continue;  // skip comment lines
			}
		}

		switch (m_option)
		{
		case Option::Parser:
			parseInput(inputLine);
			break;
		case Option::Expression:
			translateInput(inputLine, true);
			break;
		case Option::Translator:
			translateInput(inputLine, false);
			break;
		case Option::Encoder:
			encodeInput(std::move(inputLine));
			break;
		case Option::Recreator:
			recreateInput(inputLine);
			break;
		}
	}
	if (!inputMode && m_option != Option::Parser)
	{
		m_cout << '\n';  // not for parser testing
	}

	if (m_option == Option::Encoder)
	{
		// for encoder testing, output program lines
		m_cout << "Program:\n";
		for (int i {}; i < m_programUnit->rowCount(); i++)
		{
			m_cout << i << ": "
				<< m_programUnit->debugText(i, true) << '\n';
		}

		m_cout << "\nRemarks:\n" << m_programUnit->remDictionary();
		m_cout << "\nNumber Constants:\n"
			<< m_programUnit->constNumDictionary();
		m_cout << "\nString Constants:\n"
			<< m_programUnit->constStrDictionary();
		m_cout << "\nDouble Variables:\n" << m_programUnit->varDblDictionary();
		m_cout << "\nInteger Variables:\n" << m_programUnit->varIntDictionary();
		m_cout << "\nString Variables:\n" << m_programUnit->varStrDictionary();

		if (m_recreate)
		{
			m_cout << "\nOutput:\n";
			for (int i {}; i < m_programUnit->rowCount(); i++)
			{
				m_cout << i << ": " << m_programUnit->lineText(i) << '\n';
			}
		}
	}
}


// function to parse an input line and print the resulting tokens
void Tester::parseInput(const std::string &testInput)
try
{
	Parser parse {testInput};
	for (;;)
	{
		TokenPtr token {parse(DataType::Any, Reference::None)};
		printToken(token);
		if (token->isType(Type::DefFunc) || token->isType(Type::Paren))
		{
			parse.getParen();
		}
		if (token->isCode(EOL_Code))
		{
			return;
		}
	}
}
catch (TokenError &error)
{
	printError(error);
}


// function to parse an input line, translate to an RPN list
// and output the resulting RPN list
RpnList Tester::translateInput(const std::string &testInput, bool exprMode,
	std::string &&header)
try
{
	RpnList rpnList {Translator{testInput}(exprMode
		? Translator::TestMode::Expression : Translator::TestMode::Yes)};

	m_cout << (!header.empty() ? header : "Output") << ": ";
	if (m_recreate)
	{
		// recreate text from rpn list
		m_cout << Recreator{}(rpnList, exprMode);
	}
	else
	{
		m_cout << rpnList;
	}
	m_cout << ' ' << std::endl;
	return rpnList;
}
catch (TokenError &error)
{
	printError(error);
	return RpnList{};  // return an empty list
}


// function to parse an input line, translate to an RPN list,
// recreate the line and output the resulting recreated text
void Tester::recreateInput(const std::string &testInput)
{
	RpnList rpnList {translateInput(testInput, false, "Tokens")};
	if (!rpnList.empty())
	{
		// recreate text from rpn list
		m_cout << "Output: " << Recreator{}(std::move(rpnList)) << " \n";
	}
}


// function to parse an input line, translate to an RPN list
// and output the resulting RPN list
void Tester::encodeInput(std::string testInput)
{
	// parse beginning of line for line number program operation
	Operation operation {Operation::Change};
	size_t pos = 0;
	if (pos < testInput.length())
	{
		if (testInput[pos] == '+')
		{
			operation = Operation::Insert;
			pos++;
		}
		else if (testInput[pos] == '-')
		{
			operation = Operation::Remove;
			pos++;
		}
	}
	int digitCount = 0;
	int lineIndex = 0;
	while (pos < testInput.length() && isdigit(testInput[pos]))
	{
		int digit {testInput[pos] - '0'};
		lineIndex = lineIndex * 10 + digit;
		pos++;
		digitCount++;
	}
	if (digitCount == 0)  // no number at beginning?
	{
		if (operation == Operation::Change)
		{
			// no number at beginning, insert at end of program
			operation = Operation::Insert;
			lineIndex = m_programUnit->rowCount();
		}
		else if (operation == Operation::Insert)
		{
			// no number at beginning, insert at end of program
			lineIndex = m_programUnit->rowCount();
		}
		else  // no line index number after + or -
		{
			printInput(testInput);
			m_cout << "        ^-- expected line index number\n";
			return;
		}
	}
	else
	{
		if (lineIndex > m_programUnit->rowCount()
			|| (operation != Operation::Insert
			&& lineIndex == m_programUnit->rowCount()))
		{
			printInput(testInput);
			m_cout << "       " << (operation == Operation::Change ? "" : " ")
				<< "^-- line index number out of range\n";
			return;
		}
		if (operation == Operation::Remove && pos < testInput.length())
		{
			printInput(testInput);
			m_cout << std::string(7 + pos, ' ')
				<< "^-- no statement expected with remove line\n";
			return;
		}
	}
	// skip spaces
	while (pos < testInput.length() && isspace(testInput[pos]))
	{
		pos++;
	}
	testInput.erase(0, pos);  // remove operation, number and any spaces

	// call update with arguments dependent on operation
	if (operation == Operation::Remove)
	{
		m_programUnit->update(lineIndex, 1, 0, std::vector<std::string>{});
	}
	else  // Change_Operation or Insert_Operation
	{
		m_programUnit->update(lineIndex, 0,
			operation == Operation::Insert ? 1 : 0,
			std::vector<std::string>{testInput});
	}

	const ErrorItem *errorItem {m_programUnit->lineError(lineIndex)};
	// only output line if no operation/line number or has an error
	if (pos == 0 || errorItem)
	{
		printInput(testInput);
		if (errorItem)
		{
			TokenError error {errorItem->status(), errorItem->column(),
				errorItem->length()};
			printError(error);
		}
		else  // get text of encoded line and output it
		{
			m_cout << "Output: " << m_programUnit->debugText(lineIndex) << '\n';
		}
	}
}


// function to convert data type enumerator to string
static const char *dataTypeName(DataType dataType)
{
	switch (dataType)
	{
	case DataType::Double:
		return "Double";
	case DataType::Integer:
		return "Integer";
	case DataType::String:
		return "String";
	case DataType::None:
		return "None";
	case DataType::Number:
		return "Number";
	case DataType::Any:
		return "Any";
	}
	return "";  // silence compiler warning (doesn't reach here at run-time)
}


// function to convert token type enumerator to string
static const char *tokenTypeName(const TokenPtr &token)
{
	if (token->isCommand())
	{
		return "Command";
	}
	else if (token->isOperator())
	{
		return "Operator";
	}
	else
	{
		switch (token->type())
		{
		case Type::IntFunc:
			return "IntFunc";
		case Type::Constant:
			return "Constant";
		case Type::DefFunc:
			return "DefFunc";
		case Type::DefFuncNoArgs:
			return "DefFuncN";
		case Type::NoParen:
			return "NoParen";
		case Type::Paren:
			return "Paren";
		}
	}
	return "";  // silence compiler warning (doesn't reach here at run-time)
}


// function to print the contents of a token
void Tester::printToken(const TokenPtr &token)
{
	m_cout << '\t' << std::setw(2) << std::right << token->column() << std::left
		<< ": " << std::setw(10) << tokenTypeName(token);
	if (token->isCommand())
	{
		m_cout << "Op " << token->debugName();
		if (token->isCode(Rem_Code))
		{
			m_cout << " |" << token->string() << '|';
		}
	}
	else if (token->isOperator())
	{
		m_cout << "Op " << std::setw(7) << dataTypeName(token->dataType())
			<< ' ' << token->debugName();
		if (token->isCode(RemOp_Code))
		{
			m_cout << " |" << token->string() << '|';
		}
	}
	else
	{
		switch (token->type())
		{
		case Type::IntFunc:
			m_cout << (token->operandCount() == 0 ? "  " : "()");
			break;
		case Type::Constant:
		case Type::NoParen:
		case Type::DefFuncNoArgs:
			m_cout << "  ";
			break;
		case Type::Paren:
		case Type::DefFunc:
			m_cout << "()";
			break;
		}
		m_cout << ' ' << std::setw(7) << dataTypeName(token->dataType());
		switch (token->type())
		{
		case Type::DefFuncNoArgs:
		case Type::NoParen:
			m_cout << " |" << token->stringWithDataType() << '|';
			break;
		case Type::DefFunc:
		case Type::Paren:
			m_cout << " |" << token->stringWithDataType() << "(|";
			break;
		case Type::Constant:
			switch (token->dataType())
			{
			case DataType::Integer:
				m_cout << ' ' << token->valueInt();
				break;
			case DataType::Double:
				m_cout << ' ';
				if (token->hasSubCode(IntConst_SubCode))
				{
					m_cout << token->valueInt() << ",";
				}
				m_cout << token->value();
				break;
			default:
				break;
			}
			m_cout << " |" << token->string() << '|';
			break;
		case Type::IntFunc:
			m_cout << " " << token->debugName();
			break;
		}
	}
	m_cout << std::endl;
}


// function to print a token with an error
void Tester::printError(TokenError &error)
{
	m_cout << std::string(7 + error.m_column, ' ')
		<< std::string(error.m_length, '^') << "-- "
		<< StatusMessage::text(error.m_status).toStdString() << std::endl;
}


// end: test_ibcp.cpp
