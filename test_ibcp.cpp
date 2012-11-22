// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_ibcp.cpp - contains code for testing
//	Copyright (C) 2010-2012  Thunder422
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
#include "table.h"
#include "parser.h"
#include "translator.h"


extern void printGplHeader(QTextStream &cout, const QString &name);


// function to process a test input file specified on the command line
// or accept input lines from the user
Tester::Tester(QStringList &args)
{
	QMap<enum Option, QString> name;
	name[OptParser] = "parser";
	name[OptExpression] = "expression";
	name[OptTranslator] = "translator";

	// get base file name of program from first argument
	m_programName = QFileInfo(args.at(0)).baseName();

	// scan arguments for test options (ignore others)
	m_option = OptNone;
	for (int i = 1; i < args.count(); i++)
	{
		if (!isOption(args.at(i), "-tp", OptParser, name[OptParser])
			&& !isOption(args.at(i), "-te", OptExpression, name[OptExpression])
			&& !isOption(args.at(i), "-tt", OptTranslator, name[OptTranslator])
			&& args.at(i).compare("-t") == 0)
		{
			if (m_option & OptAny)
			{
				m_option |= OptError;
				m_errorMessage = tr("%1: multiple test options not allowed")
					.arg(m_programName);
			}
			else
			{
				i++;  // advance to next argument
				if (i >= args.count())
				{
					m_option |= OptError;
					m_errorMessage = tr("%1: missing test file name")
						.arg(m_programName);
				}
				else
				{
					// find start of file name less path
					m_testFileName = args.at(i);
					QString baseName = QFileInfo(m_testFileName).baseName();

					QMapIterator<enum Option, QString> it(name);
					while (it.hasNext())
					{
						it.next();
						// check beginning of file name
						if (baseName.startsWith(it.value(),
							Qt::CaseInsensitive))
						{
							m_option |= it.key();
							m_testName = name[it.key()];
							break;
						}
					}
					if ((m_option & OptAny) == 0)  // no matching names?
					{
						m_option |= OptError;
						m_errorMessage = QString("%1: %2 -t (%3|%4|%5)[xx]")
							.arg(tr("usage")).arg(m_programName)
							.arg(name[OptParser]).arg(name[OptExpression])
							.arg(name[OptTranslator]);
					}
					else
					{
						m_option |= OptFile;
					}
				}
			}
		}
		// ignore non-test options
	}
}


bool Tester::isOption(const QString &arg, const QString &exp,
	enum Option option, QString name)
{
	if (arg.compare(exp) == 0)
	{
		if (m_option & OptAny)
		{
			m_option |= OptError;
			m_errorMessage = tr("%1: multiple test options not allowed")
				.arg(m_programName);
		}
		else
		{
			m_option |= option;
			m_testName = name;
		}
		return true;
	}
	return false;
}


bool Tester::run(QTextStream &cout)
{
	QFile file;
	QTextStream input(&file);
	QString inputLine;

	bool inputMode = (m_option & OptFile) == 0;
	if (inputMode)
	{
		cout << endl << tr("Testing %1...").arg(m_testName);
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
	Token::initialize();
	QStringList errors = Table::create();
	if (!errors.isEmpty())
	{
		int n = 0;
		foreach (QString error, errors)
		{
			qWarning("%s", qPrintable(tr("Error #%1: %2").arg(++n).arg(error)));
		}
		qFatal("%s", qPrintable(tr("Program aborting!")));
	}
	printGplHeader(cout, m_programName);
	cout << "Table initialization successful." << endl;

	Translator translator(Table::instance());

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

		if (m_option & OptParser)
		{
			parseInput(cout, inputLine);
		}
		else if (m_option & OptExpression)
		{
			translateInput(cout, translator, inputLine, true);
		}
		else if (m_option & OptTranslator)
		{
			translateInput(cout, translator, inputLine, false);
		}
	}
	if (!inputMode)
	{
		file.close();
		if ((m_option & OptParser) == 0)
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
	if (translator.setInput(testInput, exprMode))
	{
		QList<RpnItem *> *rpnList = translator.output();
		printOutput(cout, "Output", *rpnList);
		while (!rpnList->isEmpty())
		{
			delete rpnList->takeLast();
		}
		delete rpnList;
	}
	else  // translate error occurred
	{
		Token *token = translator.errorToken();
		printError(cout, token, translator.errorMessage());
		if (!token->isType(Error_TokenType))
		{
			cout << endl;  // FIXME not needed, here to match current results
		}
	}
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


// function to print entire output rpn list
void Tester::printOutput(QTextStream &cout, const QString &header,
	QList<RpnItem *> &rpnList)
{
	cout << header << ": ";
	foreach (RpnItem *rpnItem, rpnList)
	{
		printSmallToken(cout, rpnItem->token());
		if (rpnItem->nOperands() > 0)
		{
			QChar separator('[');
			for (int i = 0; i < rpnItem->nOperands(); i++)
			{
				cout << separator;
				printSmallToken(cout, rpnItem->operand(i)->token());
				separator = ',';
			}
			cout << ']';
		}
		cout << ' ';
	}
	cout << endl;
}


// function to print the abbreviated contents of a token
bool Tester::printSmallToken(QTextStream &cout, Token *token)
{
	Table &table = Table::instance();

	switch (token->type())
	{
	case Remark_TokenType:
		// fall thru
	case DefFuncN_TokenType:
	case NoParen_TokenType:
		// TODO
		cout << token->string();
		break;
	case DefFuncP_TokenType:
	case Paren_TokenType:
		cout << token->string() << '(';  // TODO
		break;
	case Constant_TokenType:
		switch (token->dataType())
		{
		case Integer_DataType:
		case Double_DataType:
			cout << token->string();  // TODO
			break;
		case String_DataType:
			cout << '"' << token->string() << '"';  // TODO
			break;
		}
		break;
	case Operator_TokenType:
		if (token->isCode(RemOp_Code))
		{
			cout << table.name(token->code()) << '|' << token->string() << '|';
		}
		else
		{
			cout << table.debugName(token->code());
		}
		break;
	case IntFuncN_TokenType:
	case IntFuncP_TokenType:
		cout << table.debugName(token->code());
		break;
	case Command_TokenType:
		if (token->isCode(Rem_Code))
		{
			cout << table.name(token->code()) << '|' << token->string() << '|';
		}
		else
		{
			cout << table.name(token->code());
			if (table.name2(token->code()) != NULL)
			{
				cout << '-' << table.name2(token->code());
			}
		}
		break;
	default:
		// nothing more to output
		break;
	}
	if (token->reference())
	{
		cout << "<ref>";
	}
	if (token->isSubCode(~Used_SubCode))
	{
		cout << '\'';
		if (token->isSubCode(Paren_SubCode))
		{
			cout << ')';
		}
		if (token->isSubCode(Let_SubCode))
		{
			cout << "LET";
		}
		if (token->isSubCode(SemiColon_SubCode))
		{
			cout << ';';
		}
		if (token->isSubCode(Keep_SubCode))
		{
			cout << "Keep";
		}
		if (token->isSubCode(End_SubCode))
		{
			cout << "End";
		}
		if (token->isSubCode(Question_SubCode))
		{
			cout << "Question";
		}
		cout << '\'';
	}
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
