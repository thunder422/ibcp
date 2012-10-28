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

#include <stdio.h>
#include <stdarg.h>  // 2010-06-25: for generic print function
#include "ibcp.h"
#include "ibcp_config.h"  // 2011-06-11: for cmake

void print_gpl_header(char *name, int len)
{
	printf("\n%.*s  Copyright (C) 2010-%d  Thunder422\n", len, name,
		ibcp_COPYRIGHT_YEAR);
	printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	printf("This is free software, and you are welcome to\n");
	printf("redistribute it under certain conditions.\n\n");
}

// Program Name and Length less extension (2012-10-11)
char *program_name;
int program_name_len;

// Token
bool Token::paren[sizeof_TokenType];
bool Token::op[sizeof_TokenType];
int Token::prec[sizeof_TokenType];  // 2010-04-02
bool Token::table[sizeof_TokenType];  // 2010-05-29
// 2011-03-26: changed to a simply array of strings
const char *Token::message_array[sizeof_TokenStatus] = { // 2010-06-25
	"Null_TokenStatus (BUG)",							// Null
	"Good_TokenStatus (BUG)",							// Good
	"Done_TokenStatus (BUG)",							// Done
	"expected command",									// ExpCmd
	"expected expression",								// ExpExpr
	"expected expression or end-of-statement",			// ExpExprOrEnd
	"expected operator or end-of-statement",			// ExpOpOrEnd
	"expected binary operator or end-of-statement",		// ExpBinOpOrEnd
	"expected equal or comma for assignment",			// ExpEqualOrComma
	"expected comma",									// ExpComma
	"expected item for assignment",						// ExpAssignItem
	"expected operator or comma",						// ExpOpOrComma
	"expected operator, comma or closing parentheses",	// ExpOpCommaOrParen
	"expected operator or end-of-expression",			// NoOpenParen
	"expected operator or closing parentheses",			// ExpOpOrParen
	"expected double expression",						// ExpDouble
	"expected integer expression",						// ExpInteger
	"expected string expression (old)",					// ExpString
	"expected numeric expression",						// ExpNumExpr
	"expected string expression",						// ExpStrExpr
	"expected semicolon, comma or end-of-statement",	// ExpSemiCommaOrEnd
	"expected semicolon or comma",						// ExpSemiOrComma
	"expected operator, semicolon or comma",			// ExpOpSemiOrComma
	"expected double variable",							// ExpDblVar
	"expected integer variable",						// ExpIntVar
	"expected string variable",							// ExpStrVar
	"expected variable",								// ExpVar
	"expected string item for assignment",				// ExpStrItem
	"expected end-of-statement",						// ExpEndStmt
	// the following statuses used during development
	"BUG: not yet implemented",						// NotYetImplemented
	"BUG: invalid mode",							// InvalidMode
	"BUG: hold stack empty",						// HoldStackEmpty
	"BUG: hold stack not empty",					// HoldStackNotEmpty
	"BUG: done stack not empty",					// DoneStackNotEmpty
	"BUG: done stack empty - parentheses",			// DoneStackEmptyParen
	"BUG: done stack empty - operands",				// DoneStackEmptyOperands
	"BUG: done stack empty - operands 2",			// DoneStackEmptyOperands2
	"BUG: done stack empty - find code",			// DoneStackEmptyFindCode
	"BUG: unexpected closing parentheses",			// UnexpectedCloseParen
	"BUG: unexpected token on hold stack",			// UnexpectedToken
	"BUG: expected operand on done stack",			// DoneStackEmpty
	"BUG: command stack not empty",					// CmdStackNotEmpty
	"BUG: command stack empty",						// CmdStackEmpty
	"BUG: command stack empty for expression",		// CmdStackEmptyExpr
	"BUG: command stack empty for command",			// CmdStackEmptyCmd
	"BUG: no assign list code found",				// NoAssignListCode
	"BUG: invalid data type",						// InvalidDataType
	"BUG: count stack empty",						// CountStackEmpty
	"BUG: unexpected parentheses in expression",	// UnexpParenExpr
	"BUG: unexpected token",						// UnexpToken
	"BUG: debug #1",								// Debug1
	"BUG: debug #2",								// Debug2
	"BUG: debug #3",								// Debug3
	"BUG: debug #4",								// Debug4
	"BUG: debug #5",								// Debug5
	"BUG: debug #6",								// Debug6
	"BUG: debug #7",								// Debug7
	"BUG: debug #8",								// Debug8
	"BUG: debug #9",								// Debug9
	"BUG: debug"									// Debug
};


// This function initializes the static token data.
// This includes checking multiple defined and missing statuses

void Token::initialize(void)
{
	// set true for types that contain an opening parentheses
	paren[IntFuncP_TokenType] = true;
	paren[DefFuncP_TokenType] = true;
	paren[Paren_TokenType] = true;

	// set true for types that are considered an operator
	op[Command_TokenType] = true;
	op[Operator_TokenType] = true;

	// 2010-04-02: set precedence for non-table token types
	prec[Command_TokenType] = -1;  // use table precedence
	prec[Operator_TokenType] = -1;
	prec[IntFuncP_TokenType] = -1;
	prec[DefFuncP_TokenType] = 2;  // same as open parentheses
	prec[Paren_TokenType] = 2;

	// 2010-05-29: set token type has a table entry flags
	table[ImmCmd_TokenType] = true;
	table[Command_TokenType] = true;
	table[Operator_TokenType] = true;
	table[IntFuncN_TokenType] = true;
	table[IntFuncP_TokenType] = true;
	// FIXME should Remark_TokenType have a table entry?
}


// function to print version number (2011-06-11)
bool ibcp_version(char *name, int len, int argc, char *argv[])
{
	if (argc != 2 || strcmp(argv[1], "-v") != 0)
	{
		return false;  // not our options
	}
	// 2010-03-13: changed to output actual program name
	// 2012-10-23: changed to get release string from cmake without 'release'
	// 2012-10-23: added program name length
	printf("%.*s version %s\n", len, name, ibcp_RELEASE_STRING + 7);
	return true;
}


bool test_ibcp(Translator &translator, Parser &parser, Table *table, int argc,
	char *argv[]);


// 2010-06-25: generic print function for printing to stderr for Error
void print_stderr(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}


int main(int argc, char *argv[])
{
	// 2010-04-25: added check if program name does not have path
	// 2012-10-11: store pointer to name as a global
	program_name = strrchr(argv[0], '\\');
	// 2011-06-11: if not found then try '/' for linux
	if (program_name == NULL)
	{
		program_name = strrchr(argv[0], '/');
	}
	// 2012-10-11: find length of name not including extension
	program_name = program_name == NULL ? argv[0] : program_name + 1;
	char *ext = strrchr(program_name, '.');
	program_name_len = ext == NULL ? strlen(program_name) : ext - program_name;

	// 2012-10-23: moved version output before gpl output
	if (ibcp_version(program_name, program_name_len, argc, argv))
	{
		return 0;
	}
	print_gpl_header(program_name, program_name_len);

	Token::initialize();

	Table table;
	QStringList errors = table.initialize();
	if (!errors.isEmpty())
	{
		int n = 0;
		foreach (QString error, errors)
		{
			fprintf(stderr, "Error #%d: %s\n", ++n, qPrintable(error));
		}
		fprintf(stderr, "Program aborting!\n");
		return 1;
	}
	printf("Table initialization successful.\n");

	Translator translator(&table);
	Parser parser(&table);

	// 2010-03-18: added call to test_translator
	// 2010-04-25: added "-t" to usage string
	// 2011-06-11: added check for version option
	// 2012-10-10: replaced test_parser and test_translator with test_ibcp
	// 2012-10-23: moved version output before gpl output
	if (!test_ibcp(translator, parser, &table, argc, argv))
	{
		// 2010-05-28: replaced strrchr(argv[1],...) call with 'program_name'
		// 2011-06-11: added "-v" to usage string
		printf("usage: %s -v -t <test_file>|-tp|-te|-tt\n", program_name);
	}
	return 0;
}


// end: ibcp.cpp
