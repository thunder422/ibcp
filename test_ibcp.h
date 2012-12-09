// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: test_ibcp.h - test  header file
//	Copyright (C) 2012  Thunder422
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
class Translator;

class Tester
{
	Q_DECLARE_TR_FUNCTIONS(Test)

	enum Option {  // contains bit masks
		OptNone = -1,
		OptFirst,
		OptParser = OptFirst,
		OptExpression,
		OptTranslator,
		OptSizeOf,
		OptError = OptSizeOf
	};

	bool isOption(const QString &arg, const QString &exp,
		enum Option option, QString name);
	void parseInput(QTextStream &cout, const QString &testInput);
	void translateInput(QTextStream &cout, Translator &translator,
		const QString &testInput, bool exprMode);
	bool printToken(QTextStream &cout, Token *token, bool tab);
	void printOutput(QTextStream &cout, const QString &header,
		QList<RpnItem *> &output);
	bool printSmallToken(QTextStream &cout, Token *token);
	void printError(QTextStream &cout, Token *token, const QString &error);

	QString m_programName;		// name of program
	int m_option;				// option bit masks
	QString m_testName;			// name of test
	QString m_testFileName;		// name of test file (OptFile only)
	QString m_errorMessage;		// message if error occurred
public:
	Tester(const QStringList &args);
	~Tester(void) {}

	QStringList options(void) const;
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
};


#endif // TEST_IBCP_H
