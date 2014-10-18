// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_ibcp.cpp - tester class source file
//	Copyright (C) 2010-2013  Thunder422
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
#include "parser.h"
#include "statusmessage.h"
#include "utility.h"


// overloaded output stream operator for abbreviated contents of a token
std::ostream &operator<<(std::ostream &os, const TokenPtr &token)
{
	Table &table = Table::instance();
	bool second {};

	switch (token->type())
	{
	case Token::Type::DefFuncN:
		os << token->string().toStdString();
		break;

	case Token::Type::NoParen:
		if (token->code() == Invalid_Code)
		{
			os << '?';
		}
		os << token->string().toStdString();
		if (table.hasFlag(token->code(), Reference_Flag))
		{
			os << "<ref>";
		}
		break;

	case Token::Type::DefFuncP:
	case Token::Type::Paren:
		os << token->string().toStdString() << '(';
		break;

	case Token::Type::Constant:
		if (token->code() == Invalid_Code)
		{
			os << '?';
		}
		switch (token->dataType())
		{
		case DataType::Integer:
		case DataType::Double:
			os << token->string().toStdString();
			if (token->dataType() == DataType::Integer)
			{
				os << "%";
			}
			break;

		case DataType::String:
			os << '"' << token->string().toStdString() << '"';
			break;
		default:
			break;
		}
		break;

	case Token::Type::Operator:
		if (token->isCode(RemOp_Code))
		{
			os << table.name(token->code()).toStdString();
			second = true;
		}
		else
		{
			os << table.debugName(token->code()).toStdString();
		}
		break;

	case Token::Type::IntFuncN:
	case Token::Type::IntFuncP:
		os << table.debugName(token->code()).toStdString();
		break;

	case Token::Type::Command:
		if (token->isCode(Rem_Code))
		{
			os << table.name(token->code()).toStdString();
			second = true;
		}
		else
		{
			os << table.name(token->code()).toStdString();
			if (!table.name2(token->code()).isEmpty())
			{
				os << '-' << table.name2(token->code()).toStdString();
			}
		}
		break;

	default:
		// nothing more to output
		break;
	}
	if (token->reference())
	{
		os << "<ref>";
	}
	if (token->hasSubCode())
	{
		os << '\'';
		if (token->hasSubCode(Paren_SubCode))
		{
			os << ')';
		}
		if (token->hasSubCode(Option_SubCode))
		{
			std::string option {table.optionName(token->code()).toStdString()};
			if (option.empty())
			{
				os << "BUG";
			}
			else
			{
				os << option;
			}
		}
		if (token->hasSubCode(Colon_SubCode))
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
		os << '|' << token->string().toStdString() << '|';
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
	m_translator {new Translator},
	m_programUnit {new ProgramModel},
	m_recreator {new Recreator}
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
			m_errorMessage = m_programName + ": missing test file name";
		}
		else
		{
			// find start of file name less path
			m_testFileName = args.back();
			std::string baseName {Utility::baseFileName(m_testFileName)};

			for (auto iterator : name)
			{
				auto noCaseCompare = [](char c1, char c2)
				{
					return toupper(c1) == toupper(c2);
				};

				// check beginning of file name
				if (baseName.size() >= iterator.second.size()
					&& std::equal(iterator.second.begin(),
					iterator.second.end(), baseName.begin(), noCaseCompare))
				{
					m_option = iterator.first;
					m_testName = iterator.second;
					break;
				}
			}
			if (m_option == Option{})  // no matching names?
			{
				m_errorMessage = "usage: " + m_programName + " -t";
				if (m_recreate)
				{
					m_errorMessage += 'o';
				}
				m_errorMessage += " (";
				if (m_recreate)
				{
					m_errorMessage += name[Option::Parser] + '|';
				}
				m_errorMessage += name[Option::Expression] + '|'
					+ name[Option::Translator] + ")[xx]";
			}
			else if (m_option == Option::Parser && m_recreate)
			{
				m_errorMessage = m_programName + ": cannot use -to with "
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
std::string Tester::options(void)
{
	return "-t <test_file>|-tp|-te|-tt|-tc|-tr|-to <test_file>";
}


bool Tester::run(std::string copyrightStatement)
{
	std::ifstream ifs;

	bool inputMode {m_testFileName.empty()};
	if (!inputMode)
	{
		ifs.open(m_testFileName);
		if (!ifs.is_open())
		{
			m_errorMessage = m_programName + ": error opening '"
				 + m_testFileName + '\'';
			return false;
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
				<< m_programUnit->debugText(i, true).toStdString() << '\n';
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
				m_cout << i << ": " << m_programUnit->lineText(i).toStdString()
					<< '\n';
			}
		}
	}

	return true;
}


// function to parse an input line and print the resulting tokens
void Tester::parseInput(const std::string &testInput)
{
	bool more;

	Parser parse {testInput.c_str()};
	do
	{
		TokenPtr token {parse()};
		more = printToken(token, parse.errorStatus(), true)
			&& !token->isCode(EOL_Code);
	}
	while (more);
}


// function to parse an input line, translate to an RPN list
// and output the resulting RPN list
RpnList Tester::translateInput(const std::string &testInput, bool exprMode,
	const char *header)
{
	QString tmp {testInput.c_str()};
	RpnList rpnList {m_translator->translate(tmp, exprMode
		? Translator::TestMode::Expression : Translator::TestMode::Yes)};
	if (rpnList.hasError())
	{
		printError(rpnList.errorColumn(), rpnList.errorLength(),
			rpnList.errorStatus());
		rpnList.clear();  // return an empty list
	}
	else  // no error, translate line and if selected recreate it
	{
		m_cout << (header ? header : "Output") << ": ";
		if (m_recreate)
		{
			// recreate text from rpn list
			QString output = m_recreator->recreate(rpnList, exprMode);
			m_cout << output.toStdString();
		}
		else
		{
			m_cout << rpnList;
		}
		m_cout << " \n";
	}
	return rpnList;
}


// function to parse an input line, translate to an RPN list,
// recreate the line and output the resulting recreated text
void Tester::recreateInput(const std::string &testInput)
{
	RpnList rpnList {translateInput(testInput, false, "Tokens")};
	if (!rpnList.empty())
	{
		// recreate text from rpn list
		QString output {m_recreator->recreate(rpnList)};
		m_cout << "Output: " << output.toStdString() << " \n";
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
		m_programUnit->update(lineIndex, 1, 0, QStringList());
	}
	else  // Change_Operation or Insert_Operation
	{
		m_programUnit->update(lineIndex, 0,
			operation == Operation::Insert ? 1 : 0,
			QStringList() << testInput.c_str());
	}

	const ErrorItem *errorItem {m_programUnit->lineError(lineIndex)};
	// only output line if no operation/line number or has an error
	if (pos == 0 || errorItem)
	{
		printInput(testInput);
		if (errorItem)
		{
			printError(errorItem->column(), errorItem->length(),
				errorItem->status());
		}
		else  // get text of encoded line and output it
		{
			m_cout << "Output: "
				<< m_programUnit->debugText(lineIndex).toStdString() << '\n';
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
static const char *tokenTypeName(Token::Type type)
{
	switch (type)
	{
	case Token::Type::Command:
		return "Command";
	case Token::Type::Operator:
		return "Operator";
	case Token::Type::IntFuncN:
		return "IntFuncN";
	case Token::Type::IntFuncP:
		return "IntFuncP";
	case Token::Type::Constant:
		return "Constant";
	case Token::Type::DefFuncN:
		return "DefFuncN";
	case Token::Type::DefFuncP:
		return "DefFuncP";
	case Token::Type::NoParen:
		return "NoParen";
	case Token::Type::Paren:
		return "Paren";
	case Token::Type::Error:
		return "Error";
	}
	return "";  // silence compiler warning (doesn't reach here at run-time)
}


// function to print the contents of a token
bool Tester::printToken(const TokenPtr &token, Status errorStatus, bool tab)
{
	// include the auto-generated enumeration name text arrays
	#include "test_names.h"

	if (token->isType(Token::Type::Error))
	{
		printError(token->column(), token->length(), errorStatus);
		return false;
	}
	std::string info {"  "};
	if (token->hasParen())
	{
		info = "()";
	}
	else if (token->isType(Token::Type::Operator)
		|| token->isType(Token::Type::Command))
	{
		info = "Op";
	}
	if (tab)
	{
		m_cout << '\t';
	}
	m_cout << std::setw(2) << std::right << token->column() << std::left << ": "
		<< std::setw(10) << tokenTypeName(token->type()) << info;
	switch (token->type())
	{
	case Token::Type::DefFuncN:
	case Token::Type::NoParen:
		m_cout << ' ' << std::setw(7) << dataTypeName(token->dataType())
			<< " |" << token->string().toStdString() << '|';
		break;
	case Token::Type::DefFuncP:
	case Token::Type::Paren:
		m_cout << ' ' << std::setw(7) << dataTypeName(token->dataType())
			<< " |" << token->string().toStdString() << "(|";
		break;
	case Token::Type::Constant:
		m_cout << ' ' << std::setw(7) << dataTypeName(token->dataType());
		switch (token->dataType(true))
		{
		case DataType::Integer:
			m_cout << ' ' << token->valueInt();
			if (token->hasSubCode(Double_SubCode))
			{
				m_cout << "," << token->value();
			}
			m_cout << " |" << token->string().toStdString() << '|';
			break;
		case DataType::Double:
			m_cout << ' ' << token->value() << " |"
				<< token->string().toStdString() << '|';
			break;
		case DataType::String:
			m_cout << " |" << token->string().toStdString() << '|';
			break;
		default:
		    break;
		}
		break;
	case Token::Type::Operator:
	case Token::Type::IntFuncN:
	case Token::Type::IntFuncP:
		m_cout << ' ' << std::setw(7) << dataTypeName(token->dataType());
		// fall thru
	case Token::Type::Command:
		m_cout << " " << code_name[token->code()];
		if (token->isCode(Rem_Code) || token->isCode(RemOp_Code))
		{
			m_cout << " |" << token->string().toStdString() << '|';
		}
		break;
	default:
		// nothing more to output
		break;
	}
	m_cout << '\n';
	return true;
}


// function to print a token with an error
void Tester::printError(int column, int length, Status status)
{
	if (length < 0)  // alternate column?
	{
		column = -length;
		length = 1;
	}
	m_cout << std::string(7 + column, ' ') << std::string(length, '^')
		<< "-- " << StatusMessage::text(status).toStdString() << '\n';
}


// end: test_ibcp.cpp
