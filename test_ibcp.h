// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: test_ibcp.h - tester class header file
//	Copyright (C) 2012-2013  Thunder422
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
//	2012-11-20	initial version

#ifndef TEST_IBCP_H
#define TEST_IBCP_H

#include <list>
#include <memory>
#include <string>

#include "programmodel.h"
#include "recreator.h"
#include "translator.h"

class CommandLine;
class Token;
using TokenPtr = std::shared_ptr<Token>;
class RpnItem;
class RpnList;

class Tester
{
public:
	explicit Tester(const std::string &programName,
		const std::list<std::string> &args, std::ostream &cout);

	static std::string options(void);
	bool run(std::string copyrightStatement);
	bool hasOption(void) const  // has a test option been specified?
	{
		return m_option != Option{};
	}
	bool hasError(void) const  // does test arguments contain an error?
	{
		return !m_errorMessage.empty();
	}
	std::string errorMessage(void) const  // message of error
	{
		return m_errorMessage;
	}

private:
	enum class Option {
		Parser = 1,
		Expression,
		Translator,
		Encoder,
		Recreator
	};

	bool isOption(const std::string &arg, const std::string &exp, Option option,
		std::string name);
	void parseInput(const std::string &testInput);
	RpnList translateInput(const std::string &testInput, bool exprMode,
		const char *header = nullptr);
	void recreateInput(const std::string &testInput);
	void encodeInput(std::string testInput);
	void printInput(const std::string &inputLine)
	{
		m_cout << "\nInput: " << inputLine << '\n';
	}
	void printToken(const TokenPtr &token);
	void printError(Error &error);

	std::string m_programName;		// name of program
	Option m_option;				// selection option
	bool m_recreate;				// recreate testing
	std::string m_testName;			// name of test
	std::string m_testFileName;		// name of test file (OptFile only)
	std::ostream &m_cout;			// reference to output stream
	std::unique_ptr<Translator> m_translator;		// translator instance
	std::unique_ptr<ProgramModel> m_programUnit; 	// program unit
	std::unique_ptr<Recreator> m_recreator;			// recreator instance
	std::string m_errorMessage;		// message if error occurred
};


#endif // TEST_IBCP_H
