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

#include <QCoreApplication>
#include <QStringList>

class CommandLine;
class Token;
class RpnItem;
class RpnList;
class Translator;
class ProgramModel;

class Tester
{
	Q_DECLARE_TR_FUNCTIONS(Test)

public:
	explicit Tester(const QStringList &args);
	~Tester(void) {}

	static QStringList options(void);
	bool run(QTextStream &cout, CommandLine *commandLine);
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
		OptSizeOf,
		OptError = OptSizeOf
	};

	bool isOption(const QString &arg, const QString &exp,
		enum Option option, QString name, bool newTrans = false);
	void parseInput(QTextStream &cout, const QString &testInput);
	void translateInput(QTextStream &cout, Translator &translator,
		const QString &testInput, bool exprMode);
	void encodeInput(QTextStream &cout, ProgramModel *programModel,
		QString &testInput);
	void printInput(QTextStream &cout, const QString &inputLine)
	{
		// no 'tr()' for this string - must match expected results file
		cout << endl << "Input: " << inputLine << endl;
	}
	bool printToken(QTextStream &cout, Token *token, bool tab);
	void printError(QTextStream &cout, int column, int length,
		const QString &error);

	QString m_programName;		// name of program
	int m_option;				// selection option
	bool m_recreate;			// recreate testing
	QString m_testName;			// name of test
	QString m_testFileName;		// name of test file (OptFile only)
	QString m_errorMessage;		// message if error occurred
};


#endif // TEST_IBCP_H
