// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: ibcp.h - miscellaneous functions
//	Copyright (C) 2010-2011  Thunder422
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
//	2010-03-13	initial release, created to hold variables and functions
//				related to definitions in the ibcp.h definitions file that have
//				no other file into; and main, which includes the GPL header
//				output, Token initialization call, Table initialization code,
//				and a call to the test_parser() function
//
//	2010-03-17	changed return value of test_parser from int to bool (the
//				  command line options are passed to each test routine and if
//				  the options are not for them, they return false so the next
//				  test routine can be called)
//				removed includes that weren't needed,
//				changed exit() to a return
//
//	2010-03-18	added support for test_translator
//
//	2010-03-20	added translator instantiation to main() and translator
//				argument to translator function calls
//
//	2010-04-02	added support for obtaining precedence from token types that
//				don't have table entries (DefFuncP and Paren)
//
//	2010-04-25	added "-t" to usage string
//				added check if program name does not have full path
//
//	2010-05-20	added maximum operands and maximum associate code table error
//				reporting
//
//	2010-05-28	replaced strrchr(argv[1],...) call with 'name' in usage
//				message when command line options are not recognized, this
//				cause the program to crash when run from the Windows cmd line
//	2010-05-29	added the initialization of the new token has table entry flags
//
//	2010-06-25	added TokenStatus initialization to Token::initialize()
//				 for new token status message structure array
//				replaced TokenStsMgsErr and TableError with generic Error
//				  template
//	2010-06-28	added NULL string check to Token::initialize() message array
//				  initialization to catch missing entries
//				added missing entry for Null_TokenStatus to
//				  Token::message_array[], which was not detected by the error
//				  checking because there was a null entry at the end of the
//				  array that happen to have a 0 (Null_TokenStatus) in it
//	2010-06-29	updated expected expression error messages
//	2010-07-01/05  updates to token status messages for changes
//
//	2011-01-02	added for error codes
//	2011-01-27	corrected some memory leaks with error lists and table instance
//	2011-01-29	implemented Token new and delete operator functions to detect
//				  memory leaks of tokens
//
//	2011-02-10	added BUG_CmdStackEmptyCmd token status and message
//	2011-02-11	added BUG_UnexpToken token status and message
//	2011-02-12	renamed ExpStatement_TokenStatus to ExpCommand and message
//	2011-03-05	removed PrintOnlyIntFunc, added ExpSemiCommaOrEnd_TokenStatus
//	2011-03-10	renamed ExpCommand_TokenStatus to ExpCmd_TokenStatus
//	2011-03-13	removed ExpAssignRef_TokenStatus, UnExpCommand_TokenStatus
//				changed message to NoOpenParen_TokenStatus
//	2011-03-19	added ExpEndStmt_TokenStatus
//	2011-03-24	added ExpOpSemiOrComma_TokenStatus
//
//	2011-03-26	enum TokenStatus is now automatically generated from the
//				  Token::message_array[] is this source file, added comments
//				  were captured from ibcp.h since they were being deleted
//				removed TokenStatus error checking and index_status[] array
//				  initialization from Token::initialize()
//	2011-06-11	added ibcp_config.h for receiving values from cmake
//				added ibcp_version() for outputting current version
//				corrected executable name to allow for '\' (windows) and
//				  '/' (linux) in the path
//
//	2012-10-10	replaced calls to test_parser() and test_translator() with
//				  call to new function test_ibcp() updated usage message
//	2012-10-11	store program name as global along with length of name, which
//				  does not include extension (.exe) if present to make
//				  consistent between windows and linux; also fixed bug for
//				  linux name (was incorrectly defining new variable within if)
//
//	2012-10-23	get full release string from cmake (ibcp_config.h) instead of
//				  individual major, minor and patch numbers so that during
//				  development and git release number is used (to produce a
//				  unique release number at each commit)
//				moved output of version number before gpl header eliminating
//				  gpl header to match other programs
//	2012-10-23	modified ibcp_version to output program name correctly by
//				  using length (to work correctly on Windows)
//	2012-10-28	removed token lists and new/delete overload functions
//	2012-10-28	removed exception handling
//				changed how table is initialized
//	2012-10-29	replaced c stdio with Qt output (main output via text stream)
//				replaced char arrays with QString (program name)
//				renamed variables and functions to Qt naming convention
//	2012-11-01	changed char* to QString, removed immediate command support

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include "ibcp.h"
#include "ibcp_config.h"  // for cmake
#include "token.h"
#include "table.h"
#include "parser.h"
#include "translator.h"


void printGplHeader(QTextStream &cout, const QString &name)
{
	cout << endl << name << "  Copyright (C) 2010-" << ibcp_COPYRIGHT_YEAR
		<< "  Thunder422" << endl
		<< "This program comes with ABSOLUTELY NO WARRANTY." << endl
		<< "This is free software, and you are welcome to" << endl
		<< "redistribute it under certain conditions." << endl << endl;
}

// Program Name and Length less extension
// TODO this needs to be put into a class somewhere)
QString programName;

// function to print version number
bool ibcpVersion(QTextStream &cout, const QString &name, int argc, char *argv[])
{
	if (argc != 2 || QString::compare(argv[1], "-v") != 0)
	{
		return false;  // not our options
	}
	cout << name << QObject::tr(" version %1").arg(ibcp_RELEASE_STRING + 7)
		<< endl;
	return true;
}


// prototype for test function
bool ibcpTest(QTextStream &cout, Translator &translator, int argc,
	char *argv[]);


int main(int argc, char *argv[])
{
	// get base file name of program from first argument
	programName = QFileInfo(argv[0]).baseName();

	// setup standard output stream
	QFile output;
	output.open(stdout, QIODevice::WriteOnly | QIODevice::Unbuffered);
	QTextStream cout(&output);

	if (ibcpVersion(cout, programName, argc, argv))
	{
		return 0;
	}
	printGplHeader(cout, programName);

	Token::initialize();

	QStringList errors = Table::create();
	if (!errors.isEmpty())
	{
		int n = 0;
		foreach (QString error, errors)
		{
			qWarning("%s", qPrintable(QObject::tr("Error #%1: %2").arg(++n)
				.arg(error)));
		}
		qWarning("%s", qPrintable(QObject::tr("Program aborting!")));
		return 1;
	}
	cout << "Table initialization successful." << endl;

	Translator translator(Table::instance());

	if (!ibcpTest(cout, translator, argc, argv))
	{
		qWarning("%s: %s -v -t <%s>|-tp|-te|-tt",
			qPrintable(QObject::tr("usage")), qPrintable(programName),
			qPrintable(QObject::tr("test_file")));
	}
	return 0;
}


// end: ibcp.cpp
