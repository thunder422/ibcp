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

#include <memory>

#include <QCoreApplication>
#include <QStringList>

#include "programmodel.h"
#include "recreator.h"
#include "translator.h"

class QTextStream;

class CommandLine;
class Token;
using TokenPtr = std::shared_ptr<Token>;
class RpnItem;
class RpnList;

class Tester
{
	Q_DECLARE_TR_FUNCTIONS(Test)

public:
	explicit Tester(const QStringList &args, QTextStream &cout);

	static QStringList options(void);
	bool run(CommandLine *commandLine);
	bool hasOption(void) const  // has a test option been specified?
	{
		return m_option != OptNone;
	}
	bool hasError(void) const  // does test arguments contain an error?
	{
		return m_option == OptError;
	}
	QString errorMessage(void) const  // message of error
	{
		return m_errorMessage;
	}

private:
	enum Option {
		OptNone = -1,
		OptFirst,
		OptParser = OptFirst,
		OptExpression,
		OptTranslator,
		OptEncoder,
		OptNumberOf,
		OptRecreator,
		OptSizeOf,
		OptError = OptSizeOf
	};

	bool isOption(const QString &arg, const QString &exp, Option option,
		QString name);
	void parseInput(const QString &testInput);
	RpnList translateInput(const QString &testInput, bool exprMode,
		const char *header = NULL);
	void recreateInput(const QString &testInput);
	void encodeInput(QString &testInput);
	void printInput(const QString &inputLine)
	{
		// no 'tr()' for this string - must match expected results file
		m_cout << endl << "Input: " << inputLine << endl;
	}
	bool printToken(const TokenPtr &token, bool tab);
	void printError(int column, int length, const QString &error);

	QString m_programName;			// name of program
	int m_option;					// selection option
	bool m_recreate;				// recreate testing
	QString m_testName;				// name of test
	QString m_testFileName;			// name of test file (OptFile only)
	QTextStream &m_cout;			// reference to output device
	std::unique_ptr<Translator> m_translator;		// translator instance
	std::unique_ptr<ProgramModel> m_programUnit; 	// program unit
	std::unique_ptr<Recreator> m_recreator;			// recreator instance
	QString m_errorMessage;			// message if error occurred
};


#endif // TEST_IBCP_H
