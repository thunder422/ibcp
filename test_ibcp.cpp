// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_ibcp.cpp - contains code for testing
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

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTextStream>

#include "test_ibcp.h"
#include "commandline.h"
#include "table.h"
#include "parser.h"
#include "translator.h"


// function to process a test input file specified on the command line
// or accept input lines from the user
Tester::Tester(const QStringList &args)
{
	QString name[OptSizeOf];
	name[OptParser] = "parser";
	name[OptExpression] = "expression";
	name[OptTranslator] = "translator";

	// get base file name of program from first argument
	m_programName = QFileInfo(args.at(0)).baseName();

	// scan arguments for test options (ignore others)
	m_option = OptNone;
	switch (args.count())
	{
	case 2:
		if (isOption(args.at(1), "-tp", OptParser, name[OptParser])
			|| isOption(args.at(1), "-te", OptExpression, name[OptExpression])
			|| isOption(args.at(1), "-tt", OptTranslator, name[OptTranslator]))
		{
			break;
		}
		// fall through
	case 3:
		if (args.at(1) == "-t")
		{
			if (args.count() == 2)
			{
				m_option = OptError;
				m_errorMessage = tr("%1: missing test file name")
					.arg(m_programName);
			}
			else
			{
				// find start of file name less path
				m_testFileName = args.at(2);
				QString baseName = QFileInfo(m_testFileName).baseName();

				for (int i = OptFirst; i < OptSizeOf; i++)
				{
					// check beginning of file name
					if (baseName.startsWith(name[i], Qt::CaseInsensitive))
					{
						m_option = i;
						m_testName = name[i];
						break;
					}
				}
				if (m_option == OptNone)  // no matching names?
				{
					m_option = OptError;
					m_errorMessage = QString("%1: %2 -t (%3|%4|%5)[xx]")
						.arg(tr("usage")).arg(m_programName)
						.arg(name[OptParser]).arg(name[OptExpression])
						.arg(name[OptTranslator]);
				}
			}
		}
	}
	// ignore non-test or invalid test options
}


// function to see if argument is expected option
bool Tester::isOption(const QString &arg, const QString &exp,
	enum Option option, QString name)
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
QStringList Tester::options(void)
{
	return QStringList() << QString("-t <%1>").arg(tr("test_file")) << "-tp"
		<< "-te" << "-tt";
}


bool Tester::run(QTextStream &cout, CommandLine *commandLine)
{
	QFile file;
	QTextStream input(&file);
	QString inputLine;

	bool inputMode = m_testFileName.isEmpty();
	if (inputMode)
	{
		file.open(stdin, QIODevice::ReadOnly);
	}
	else
	{
		file.setFileName(m_testFileName);
		if (!file.open(QIODevice::ReadOnly))
		{
			m_errorMessage = tr("%1: error opening '%2'").arg(m_programName)
				.arg(m_testFileName);
			return false;
		}
	}

	// initialize the worker classes
	if (Table::hasErrors())
	{
		int n = 0;
		foreach (QString error, Table::errorList())
		{
			qCritical("%s", qPrintable(tr("Error #%1: %2").arg(++n)
				.arg(error)));
		}
		qFatal("%s", qPrintable(tr("Program aborting!")));
	}

	if (inputMode)
	{
		cout << endl;

		const char *copyright = commandLine->copyrightStatement();
		QString line = tr(copyright).arg(commandLine->programName())
			.arg(commandLine->copyrightYear());
		cout << line << endl;

		const char **warranty = commandLine->warrantyStatement();
		for (int i = 0; warranty[i]; i++)
		{
			QString line = tr(warranty[i]);
			cout << line << endl;
		}

		cout << endl << tr("Table initialization successful.") << endl;
	}

	Translator translator(Table::instance());

	if (inputMode)
	{
		cout << endl << tr("Testing %1...").arg(m_testName);
	}

	for (int lineno = 1;; lineno++)
	{
		if (inputMode)
		{
			cout << endl << tr("Input: ") << flush;
			inputLine = input.readLine();
			if (inputLine.isEmpty() || inputLine[0] == '\n')
			{
				break;
			}
		}
		else
		{
			if (input.atEnd())
			{
				break;
			}
			inputLine = input.readLine();
			if (inputLine.isEmpty() || inputLine[0] == '#')
			{
				continue;  // skip blank and comment lines
			}
			// no 'tr()' for this string - must match expected results file
			cout << endl << "Input: " << inputLine << endl;
		}

		switch (m_option)
		{
		case OptParser:
			parseInput(cout, inputLine);
			break;
		case OptExpression:
			translateInput(cout, translator, inputLine, true);
			break;
		case OptTranslator:
			translateInput(cout, translator, inputLine, false);
			break;
		}
	}
	if (!inputMode)
	{
		file.close();
		if (m_option != OptParser)
		{
			cout << endl;  // not for parser testing
		}
	}
	return true;
}


// function to parse an input line and print the resulting tokens
void Tester::parseInput(QTextStream &cout, const QString &testInput)
{
	Parser parser(Table::instance());
	Token *token;
	bool more;

	parser.setInput(QString(testInput));
	do {
		token = parser.token();
		more = printToken(cout, token, true);
		if (more && token->isType(Operator_TokenType)
			&& token->isCode(EOL_Code))
		{
			more = false;
		}
		delete token;
	}
	while (more);
}


// function to parse an input line, translate to an RPN list
// and output the resulting RPN list
void Tester::translateInput(QTextStream &cout, Translator &translator,
	const QString &testInput, bool exprMode)
{
	RpnList *rpnList = translator.translate(testInput, exprMode);
	if (!rpnList->hasError())
	{
		cout << "Output: " << rpnList->text() << ' ' << endl;
	}
	else  // translate error occurred
	{
		Token *token = rpnList->errorToken();
		printError(cout, token, rpnList->errorMessage());
		if (!token->isType(Error_TokenType))
		{
			cout << endl;  // FIXME not needed, here to match current results
		}
	}
	delete rpnList;
}


// function to print the contents of a token
bool Tester::printToken(QTextStream &cout, Token *token, bool tab)
{
	// include the auto-generated enumeration name text arrays
	#include "test_names.h"

	if (token->isType(Error_TokenType))
	{
		printError(cout, token, token->string());
		return false;
	}
	QString info("  ");
	if (token->hasParen())
	{
		info = token->isOperator() ? "??" : "()";
	}
	else if (token->isOperator())
	{
		info = "Op";
	}
	if (tab)
	{
		cout << '\t';
	}
	cout << qSetFieldWidth(2) << right << token->column() << qSetFieldWidth(0)
		<< left << ": " << qSetFieldWidth(10) << tokentype_name[token->type()]
		<< qSetFieldWidth(0) << info;
	switch (token->type())
	{
	case Remark_TokenType:
		cout << ' ' << code_name[token->code()];
		// fall thru
	case DefFuncN_TokenType:
	case NoParen_TokenType:
		cout << ' ' << qSetFieldWidth(7) << datatype_name[token->dataType()]
			<< qSetFieldWidth(0) << " |" << token->string() << '|';
		break;
	case DefFuncP_TokenType:
	case Paren_TokenType:
		cout << ' ' << qSetFieldWidth(7) << datatype_name[token->dataType()]
			<< qSetFieldWidth(0) << " |" << token->string() << "(|";
		break;
	case Constant_TokenType:
		cout << ' ' << qSetFieldWidth(7) << datatype_name[token->dataType()]
			<< qSetFieldWidth(0);
		switch (token->dataType())
		{
		case Integer_DataType:
			cout << ' ' << token->valueInt() << " |" << token->string() << '|';
			break;
		case Double_DataType:
			cout << ' ' << token->valueDbl() << " |" << token->string() << '|';
			break;
		case String_DataType:
			cout << " |" << token->string() << '|';
			break;
		}
		break;
	case Operator_TokenType:
	case IntFuncN_TokenType:
	case IntFuncP_TokenType:
		cout << ' ' << qSetFieldWidth(7) << datatype_name[token->dataType()]
			<< qSetFieldWidth(0);
		// fall thru
	case Command_TokenType:
		cout << " " << code_name[token->code()];
		if (token->isCode(Rem_Code) || token->isCode(RemOp_Code))
		{
			cout << " |" << token->string() << '|';
		}
		break;
	default:
		// nothing more to output
		break;
	}
	cout << endl;
	return true;
}


// function to print a token with an error
void Tester::printError(QTextStream &cout, Token *token, const QString &error)
{
	int col = token->column();
	int len = token->length();
	if (len < 0)  // alternate column?
	{
		col = -len;
		len = 1;
	}
	cout << QString(" ").repeated(7 + col) << QString("^").repeated(len)
		<< "-- " << error << endl;
}


// end: test_ibcp.cpp
