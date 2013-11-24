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

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTextStream>

#include "test_ibcp.h"
#include "commandline.h"
#include "table.h"
#include "parser.h"
#include "programmodel.h"
#include "recreator.h"
#include "translator.h"


// function to process a test input file specified on the command line
// or accept input lines from the user
Tester::Tester(const QStringList &args, QTextStream &cout) :
	m_cout(cout),
	m_translator(new Translator),
	m_programUnit(new ProgramModel),
	m_recreator(new Recreator)
{
	QString name[OptSizeOf];
	name[OptParser] = "parser";
	name[OptExpression] = "expression";
	name[OptTranslator] = "translator";
	name[OptEncoder] = "encoder";
	name[OptRecreator] = "recreator";

	// get base file name of program from first argument
	m_programName = QFileInfo(args.at(0)).baseName();

	// scan arguments for test options (ignore others)
	m_option = OptNone;
	m_recreate = false;
	switch (args.count())
	{
	case 2:
		if (isOption(args.at(1), "-tp", OptParser, name[OptParser])
			|| isOption(args.at(1), "-te", OptExpression, name[OptExpression])
			|| isOption(args.at(1), "-tt", OptTranslator, name[OptTranslator])
			|| isOption(args.at(1), "-tc", OptEncoder, name[OptEncoder])
			|| isOption(args.at(1), "-tr", OptRecreator, name[OptRecreator]))
		{
			break;
		}
		// not one of the above options, fall through
	case 3:
		if (args.at(1) == "-to")
		{
			m_recreate = true;
		}
		else if (args.at(1) != "-t")
		{
			break;  // no test option found
		}
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

			for (int i = OptFirst; i < OptNumberOf; i++)
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
				QString parser = m_recreate
					? "" : QString("%1|").arg(name[OptParser]);
				m_errorMessage = QString("%1: %2 -t%3 (%4%5|%6)[xx]")
					.arg(tr("usage")).arg(m_programName).arg(parser)
					.arg(m_recreate ? "o" : "").arg(name[OptExpression])
					.arg(name[OptTranslator]);
			}
			else if (m_option == OptParser && m_recreate)
			{
				m_option = OptError;
				m_errorMessage = QString("%1: cannot use -to with %2 files")
					.arg(m_programName).arg(name[OptParser]);
			}
		}
	}
	// ignore non-test or invalid test options
}


Tester::~Tester(void)
{
	delete m_translator;
	delete m_programUnit;
	delete m_recreator;
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
		<< "-te" << "-tt" << "-tc" << "-tr"
		<< QString("-to <%1>").arg(tr("test_file"));
}


bool Tester::run(CommandLine *commandLine)
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

	if (inputMode)
	{
		m_cout << endl;

		const char *copyright = commandLine->copyrightStatement();
		QString line = tr(copyright).arg(commandLine->programName())
			.arg(commandLine->copyrightYear());
		m_cout << line << endl;

		const char **warranty = commandLine->warrantyStatement();
		for (int i = 0; warranty[i]; i++)
		{
			QString line = tr(warranty[i]);
			m_cout << line << endl;
		}

		m_cout << endl << tr("Table initialization successful.") << endl;
	}

	if (inputMode)
	{
		m_cout << endl << tr("Testing %1...").arg(m_testName);
	}

	for (int lineno = 1;; lineno++)
	{
		if (inputMode)
		{
			m_cout << endl << tr("Input: ") << flush;
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
			if (inputLine[0] == '#'
				|| m_option != OptEncoder && inputLine.isEmpty())
			{
				continue;  // skip blank and comment lines
			}
			if (m_option != OptEncoder)
			{
				printInput(inputLine);
			}
		}

		switch (m_option)
		{
		case OptParser:
			parseInput(inputLine);
			break;
		case OptExpression:
			translateInput(inputLine, true);
			break;
		case OptTranslator:
			translateInput(inputLine, false);
			break;
		case OptEncoder:
			encodeInput(inputLine);
			break;
		case OptRecreator:
			recreateInput(inputLine);
			break;
		}
		// report any token leaks and extra token deletes
		// FIXME temporary disable for encoder testing since program model
		//       currently holds on to the rpn lists
		if (m_option != OptEncoder)
		{
			Token::reportErrors();
		}
	}
	if (!inputMode)
	{
		file.close();
		if (m_option != OptParser)
		{
			m_cout << endl;  // not for parser testing
		}
	}

	if (m_option == OptEncoder)
	{
		// for encoder testing, output program lines
		m_cout << "Program:" << endl;
		for (int i = 0; i < m_programUnit->rowCount(); i++)
		{
			m_cout << i << ": " << m_programUnit->debugText(i, true) << endl;
		}
		m_cout << m_programUnit->dictionariesDebugText();

		if (m_recreate)
		{
			m_cout << endl << "Output:" << endl;
			for (int i = 0; i < m_programUnit->rowCount(); i++)
			{
				m_cout << i << ": " << m_programUnit->lineText(i) << endl;
			}
		}
	}

	return true;
}


// function to parse an input line and print the resulting tokens
void Tester::parseInput(const QString &testInput)
{
	Parser parser;
	Token *token;
	bool more;

	parser.setInput(QString(testInput));
	do {
		token = parser.token();
		more = printToken(token, true);
		if (more && token->isCode(EOL_Code))
		{
			more = false;
		}
		delete token;
	}
	while (more);
}


// function to parse an input line, translate to an RPN list
// and output the resulting RPN list
RpnList *Tester::translateInput(const QString &testInput, bool exprMode,
	const char *header)
{
	RpnList *rpnList = m_translator->translate(testInput, exprMode
		? Translator::Expression_TestMode : Translator::Yes_TestMode);
	if (rpnList->hasError())
	{
		printError(rpnList->errorColumn(), rpnList->errorLength(),
			rpnList->errorMessage());
		delete rpnList;
		return NULL;
	}
	else  // no error, translate line and if selected recreate it
	{
		QString output;
		if (m_recreate)
		{
			// recreate text from rpn list
			output = m_recreator->recreate(rpnList, exprMode);
		}
		else
		{
			output = rpnList->text();
		}
		if (header == NULL)
		{
			header = "Output";
			delete rpnList;
			rpnList = NULL;
		}
		m_cout << header << ": " << output << ' ' << endl;
		return rpnList;
	}
}


// function to parse an input line, translate to an RPN list,
// recreate the line and output the resulting recreated text
void Tester::recreateInput(const QString &testInput)
{
	RpnList *rpnList = translateInput(testInput, false, "Tokens");
	if (rpnList != NULL)
	{
		// recreate text from rpn list
		QString output = m_recreator->recreate(rpnList);
		delete rpnList;
		m_cout << "Output: " << output << ' ' << endl;
	}
}


// function to parse an input line, translate to an RPN list
// and output the resulting RPN list
void Tester::encodeInput(QString &testInput)
{
	// parse beginning of line for line number program operation
	Operation operation = Change_Operation;
	int pos = 0;
	if (pos < testInput.length())
	{
		if (testInput.at(pos) == '+')
		{
			operation = Insert_Operation;
			pos++;
		}
		else if (testInput.at(pos) == '-')
		{
			operation = Remove_Operation;
			pos++;
		}
	}
	int digitCount = 0;
	int lineIndex = 0;
	while (pos < testInput.length() && testInput.at(pos).isDigit())
	{
		int digit = testInput.at(pos).toAscii() - '0';
		lineIndex = lineIndex * 10 + digit;
		pos++;
		digitCount++;
	}
	if (digitCount == 0)  // no number at beginning?
	{
		if (operation == Change_Operation)
		{
			// no number at beginning, insert at end of program
			operation = Insert_Operation;
			lineIndex = m_programUnit->rowCount();
		}
		else if (operation == Insert_Operation)
		{
			// no number at beginning, insert at end of program
			lineIndex = m_programUnit->rowCount();
		}
		else  // no line index number after + or -
		{
			printInput(testInput);
			m_cout << QString("        ^-- expected line index number") << endl;
			return;
		}
	}
	else
	{
		if (lineIndex > m_programUnit->rowCount()
			|| operation != Insert_Operation
			&& lineIndex == m_programUnit->rowCount())
		{
			printInput(testInput);
			m_cout << QString("       %1^-- line index number out of range")
				.arg(operation == Change_Operation ? "" : " ") << endl;
			return;
		}
		if (operation == Remove_Operation && pos < testInput.length())
		{
			printInput(testInput);
			m_cout << QString(" ").repeated(7 + pos)
				<< QString("^-- no statement expected with remove line")
				<< endl;
			return;
		}
	}
	// skip spaces
	while (pos < testInput.length() && testInput.at(pos).isSpace())
	{
		pos++;
	}
	testInput.remove(0, pos);  // remove operation, number and any spaces

	// call update with arguments dependent on operation
	if (operation == Remove_Operation)
	{
		m_programUnit->update(lineIndex, 1, 0, QStringList());
	}
	else  // Change_Operation or Insert_Operation
	{
		m_programUnit->update(lineIndex, 0,
			operation == Insert_Operation ? 1 : 0, QStringList() << testInput);
	}

	const ErrorItem *errorItem = m_programUnit->lineError(lineIndex);
	// only output line if no operation/line number or has an error
	if (pos == 0 || errorItem != NULL)
	{
		printInput(testInput);
		if (errorItem != NULL)
		{
			printError(errorItem->column(), errorItem->length(),
				errorItem->message());
		}
		else  // get text of encoded line and output it
		{
			m_cout << "Output: " << m_programUnit->debugText(lineIndex) << endl;
		}
	}
}


// function to print the contents of a token
bool Tester::printToken(Token *token, bool tab)
{
	// include the auto-generated enumeration name text arrays
	#include "test_names.h"

	if (token->isType(Error_TokenType))
	{
		printError(token->column(), token->length(), token->string());
		return false;
	}
	QString info("  ");
	if (token->hasParen())
	{
		info = "()";
	}
	else if (token->isType(Operator_TokenType)
		|| token->isType(Command_TokenType))
	{
		info = "Op";
	}
	if (tab)
	{
		m_cout << '\t';
	}
	m_cout << qSetFieldWidth(2) << right << token->column() << qSetFieldWidth(0)
		<< left << ": " << qSetFieldWidth(10) << tokentype_name[token->type()]
		<< qSetFieldWidth(0) << info;
	switch (token->type())
	{
	case DefFuncN_TokenType:
	case NoParen_TokenType:
		m_cout << ' ' << qSetFieldWidth(7) << datatype_name[token->dataType()]
			<< qSetFieldWidth(0) << " |" << token->string() << '|';
		break;
	case DefFuncP_TokenType:
	case Paren_TokenType:
		m_cout << ' ' << qSetFieldWidth(7) << datatype_name[token->dataType()]
			<< qSetFieldWidth(0) << " |" << token->string() << "(|";
		break;
	case Constant_TokenType:
		m_cout << ' ' << qSetFieldWidth(7) << datatype_name[token->dataType()]
			<< qSetFieldWidth(0);
		switch (token->dataType(true))
		{
		case Integer_DataType:
			m_cout << ' ' << token->valueInt();
			if (token->hasSubCode(Double_SubCode))
			{
				m_cout << "," << token->value();
			}
			m_cout << " |" << token->string() << '|';
			break;
		case Double_DataType:
			m_cout << ' ' << token->value() << " |" << token->string() << '|';
			break;
		case String_DataType:
			m_cout << " |" << token->string() << '|';
			break;
		}
		break;
	case Operator_TokenType:
	case IntFuncN_TokenType:
	case IntFuncP_TokenType:
		m_cout << ' ' << qSetFieldWidth(7) << datatype_name[token->dataType()]
			<< qSetFieldWidth(0);
		// fall thru
	case Command_TokenType:
		m_cout << " " << code_name[token->code()];
		if (token->isCode(Rem_Code) || token->isCode(RemOp_Code))
		{
			m_cout << " |" << token->string() << '|';
		}
		break;
	default:
		// nothing more to output
		break;
	}
	m_cout << endl;
	return true;
}


// function to print a token with an error
void Tester::printError(int column, int length,
	const QString &error)
{
	if (length < 0)  // alternate column?
	{
		column = -length;
		length	= 1;
	}
	m_cout << QString(" ").repeated(7 + column) << QString("^").repeated(length)
		<< "-- " << error << endl;
}


// end: test_ibcp.cpp
