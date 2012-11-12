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
//	2010-03-01	initial release
//
//	2010-03-06	added support for a command line argument to indicate which test
//				to run, added test input lines for testing identifiers, split
//				DefFunc_TokenType into DefFuncN_TokenType and
//				DefFuncP_TokenType
//
//	2010-03-07	added test input lines for testing numbers,
//				changed error display to use new length value in token
//
//	2010-03-08	added test input lines for testing strings
//
//	2010-03-10	added test input lines for testing operators
//
//	2010-03-11	added command line option to input lines interactively,
//				separated code into new parse_input() function
//
//	2010-03-13	replaced IncFunc with IntFuncN and IntFuncP token types
//				added new Token static functions that indicate whether token
//				  has a parentheses or is and operator
//				changed main() to test_parser(), moved print_gpl_header, and
//				  table initialization to new ibcp.cpp source file
//
//	2010-03-17	renamed file from test_parser.cpp - will now contain all test
//				  code
//				separated print_token() from parse_input() so that it can be
//				  used from other functions
//				added string for new Code values to print_token
//				changed all token->code to token->index
//
//	2010-03-18	fixed loop since get_token() no longer returns null
//				added new functions test_translator() and translate_input() for
//				  testing translator
//
//	2010-03-20	implemented test translator code, made print_small_token() so
//				that RPN list could be output in a compact form
//
//	2010-03-21	added check for parser errors in translator test code
//				corrected output RPN list to handle an empty list
//				added bad length check to print_error()
//				corrected print_error() to handle error tokens
//				added more simple expressions test inputs
//
//	2010-03-25	added expressions for testing expressions with parentheses
//				added additional errors for parentheses errors
//
//	2010-03-26	added more parentheses test expressions
//
//	2010-04-02	added expressions for testing expressions with arrays functions
//				added additional errors for arrays/function errors
//				in print_small_token() for operators, use name2 for output if
//				  set, otherwise use name
//
//	2010-04-04	added expressions for testing the number of arguments
//				check for internal functions
//				added an additional error
//				added calls to debug_name() in print_small_token()
//
//	2010-04-11	replaced unexpected comma error with two errors
//				added assignment errors
//				added new assignment codes to print_token()
//	2010-04-12	added output of "<ref>" if token reference flag set
//	2010-04-14	correct token memory allocation problem (only delete token if
//				it is the original token passed to the Translator)
//	2010-04-16	added assignment/reference and parentheses errors
//				modified translator_input() to set expression test mode in
//				  Translator for previous test inputs
//				added assignment statement test inputs
//	2010-04-17	added another unexpected comma error
//
//	2010-04-25	added errors for data type handling
//				return get_str() to get_ptr()
//				added new data handling errors
//	2010-04-27	corrected previous translation test inputs
//				added data type handling test inputs
//
//	2010-05-09	corrected expresion in translator test input set 6
//				added assignment data type handling test inputs (set 7)
//
//	2010-05-15	modified code because output list from Translator now contains
//				  RpnItem* instead of Token*
//				added additional loop to output list from Translator with no
//				  deleting, necessary so that the operands of tokens saved by
//				  the Translator can also be printed
//
//	2010-05-19	created testinput8 for translator substring tests
//	2010-05-21	added rest of test inputs for substring assignment tests
//
//	2010-05-22	added more of test inputs for substring assignment tests that
//				contain mix string list assignments
//
//	2010-05-28	added static in front of all test input definitions to prevent
//				code from being generated
//	2010-05-29	added support for sub-code output to print_small_token()
//				renamed Translator::Status names to _TokenStatus names
//				added support for unexpected command error
//				updated datatype_name[] and code_name[] arrays for changes made
//				  since parser testing was last used
//				corrected output of double and integer token output to used
//				  originally entered string instead of stored value
//				added '0' constants tests to parser test input set #3
//				added message to identifier Parser or Translator testing for
//				  input mode testing
//				added testinput9 for translator command testing with tests for
//				  LET command
//	2010-05-30	added command stack not empty bug error
//
//	2010-06-02	updated print_token() for new codes
//	2010-06-06	corrected to call name2() in print_small_token()
//	2010-06-08	added PRINT statement test inputs
//				added semicolon sub-code flag support to print_small_token()
//	2010-06-09	added unexpected comma in arguments error
//				renamed bug errors for clarity
//	2010-06-10	added new translator test sets (10 and 11)
//	2010-06-10	added expected unary or operand error
//	2010-06-01/14  updated many error enumeration names and string messages for
//				   clarity
//
//	2010-06-25	removed token status switch (messages moved to ibcp.cpp)
//	2010-06-26	added more error test inputs
//	2010-06-29	corrected print_error() to take into account double quotes
//				  surrounding and used internally on string constants
//				added new translator expression type test set (12)
//	2010-07-04	moved semicolon error tests from testinput11 to new testinput13
//				added more error tests to testinput11
//				added more error tests in testinput12
//
//	2010-08-01	removed comma sub-code
//	2010-10-05	added more translator tests
//				modified to get code_name[] contents from auto-generated include
//				  file
//
//	2011-01-04	added translator test 14 - expression type tests
//	2011-01-11	removed redundant code in print_error() for determining length
//				  of token (can now just use token length member)
//	2011-01-22	ignore Used_SubCode in print_small_token()
//				added more statements to translator test 14
//	2011-01-29	modified for updated List class functions
//				implemented memory leak detection, including new outputting
//				  leaks with new print_token_leaks
//	2011-01-30	corrected memory leak for errors at open parentheses
//	2011-02-01	added additional array tests to translator test 12
//	2011-02-05	added temporary strings tests in new translator test 15
//
//	2011-02-26	updated for change of table index to code
//	2011-03-03	added more print function expression tests
//	2011-03-08	removed output of token codes in print_token() to prevent
//				  parser test failures due to code/table entry changes
//	2011-03-20	added translator test 16 for input statment tests
//				added keep and end  subcodes to print_small_token()
//	2011-03-22	added question subcode to print_small_token()
//
//	2011-03-26	modified print_token() and print_small_token() to output an
//				  open parentheses for DefFuncP and Paren token types that are
//				  no longer stored in the tokens string
//	2011-03-27	set parser operand state from translator before each token
//				added translator test 17 for negative constant tests
//	2011-06-07	added checks for gets() return value to remove compiler warnings
//
//	2012-10-11	implemented new function test_ibcp() to replace test_parser()
//				  and test_translator(), which reads the test input from a file
//				  specified on the command line (all test inputs moved to test
//				  input data files from the code here)
//	2012-10-14	added code to parser constant double output to remove third
//				  exponent digit if present and is zero - this is for Windows
//				  that outputs 3 digits all the time (Linux does not)
//
//	2012-10-24	deleted the rpnlist object after deleting all the list members
//				  (to fix a memory leak)
//	2012-10-27	changed translator output from List class to QList
//				created separate print_output() so can be used for debugging
//	2012-10-28	removed token leak output routine
//	2012-10-29	converted strings to QString (and QByteArray)
//				converted file I/O to QFile (and QFileInfo)
//				converted output to QTextStream
//				renamed variables and functions to Qt naming convention
//	2012-10-31	changed from reading file directory with QByteArray to using a
//				  QTextStream so that QString can be read
//	2012-11-01	renamed Parser functions to Qt Style naming
//				pass QString to Parser instead of char*
//				changed QByteArray to QString
//				removed immediate command support

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTextStream>

#include "table.h"
#include "parser.h"
#include "translator.h"

void parseInput(QTextStream &cout, const QString &testInput);
void translateInput(QTextStream &cout, Translator &translator,
	const QString &testInput, bool exprMode);
bool printToken(QTextStream &cout, Token *token, bool tab);
void printOutput(QTextStream &cout, const QString &header,
	QList<RpnItem *> &output);
bool printSmallToken(QTextStream &cout, Token *token);
void printError(QTextStream &cout, Token *token, const QString &error);


// function to process a test input file specified on the command line
// or accept input lines from the user
bool ibcpTest(QTextStream &cout, Translator &translator, int argc, char *argv[])
{
	extern char *programName;

	enum testModeEnum {
		testParser, testExpression, testTranslator, sizeofTestMode
	} testMode;
	QVector<QString> name(sizeofTestMode);
	name[testParser] = "parser";
	name[testExpression] = "expression";
	name[testTranslator] = "translator";
	bool inputMode;

	if (argc == 2 && QString::compare(argv[1], "-tp") == 0)
	{
		testMode = testParser;
		inputMode = true;
	}
	else if (argc == 2 && QString::compare(argv[1], "-te") == 0)
	{
		testMode = testExpression;
		inputMode = true;
	}
	else if (argc == 2 && QString::compare(argv[1], "-tt") == 0)
	{
		testMode = testTranslator;
		inputMode = true;
	}
	else if (argc == 3 && QString::compare(argv[1], "-t") == 0)
	{
		// find start of file name less path
		QString fileName = QFileInfo(argv[2]).baseName();

		// get and check beginning of file name
		if (fileName.startsWith(name[testParser], Qt::CaseInsensitive))
		{
			testMode = testParser;
		}
		else if (fileName.startsWith(name[testExpression], Qt::CaseInsensitive))
		{
			testMode = testExpression;
		}
		else if (fileName.startsWith(name[testTranslator], Qt::CaseInsensitive))
		{
			testMode = testTranslator;
		}
		else
		{
			qCritical("usage: %s -t (%s|%s|%s)[XX]\n", qPrintable(programName),
				qPrintable(name[testParser]), qPrintable(name[testExpression]),
				qPrintable(name[testTranslator]));
			return true;  // our options were bad
		}
		inputMode = false;
	}
	else
	{
		return false;  // not our options
	}

	QFile file;
	QTextStream input(&file);
	QString inputLine;

	if (inputMode)
	{
		cout << endl << "Testing " << name[testMode] << "...";
		file.open(stdin, QIODevice::ReadOnly);
	}
	else
	{
		file.setFileName(argv[2]);
		if (!file.open(QIODevice::ReadOnly))
		{
			qCritical("%s: error opening '%s'\n", qPrintable(programName),
				argv[2]);
			return true;
		}
	}
	for (int lineno = 1;; lineno++)
	{
		if (inputMode)
		{
			cout << endl << "Input: " << flush;
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
			cout << endl << "Input: " << inputLine << endl;
		}

		switch (testMode)
		{
		case testParser:
			parseInput(cout, inputLine);
			break;
		case testExpression:
			translateInput(cout, translator, inputLine, true);
			break;
		case testTranslator:
			translateInput(cout, translator, inputLine, false);
			break;
		}
	}
	if (!inputMode)
	{
		file.close();
		if (testMode != testParser)
		{
			cout << endl;  // not for parser testing
		}
	}
	return true;
}


// function to parse an input line and print the resulting tokens
void parseInput(QTextStream &cout, const QString &testInput)
{
	Parser parser(Table::instance());
	Token *token;
	bool more;

	parser.setInput(QString(testInput));
	// 2010-03-18: fix loop since get_token() no longer returns null
	do {
		token = parser.getToken();
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
void translateInput(QTextStream &cout, Translator &translator,
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
bool printToken(QTextStream &cout, Token *token, bool tab)
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
void printOutput(QTextStream &cout, const QString &header,
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
bool printSmallToken(QTextStream &cout, Token *token)
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
void printError(QTextStream &cout, Token *token, const QString &error)
{
	int col = token->column();
	int len = token->length();
	cout << QString(" ").repeated(7 + col) << QString("^").repeated(len)
		<< "-- " << error << endl;
}
