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

#include <stdio.h>
#include <stdarg.h>  // 2010-06-25: for generic print function
#include "ibcp.h"
#include "ibcp_config.h"  // 2011-06-11: for cmake

void print_gpl_header(char *name)
{
	printf("\n%s  Copyright (C) 2010-%d  Thunder422\n", name,
		ibcp_COPYRIGHT_YEAR);
	printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	printf("This is free software, and you are welcome to\n");
	printf("redistribute it under certain conditions.\n\n");
}

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
List<Token *> Token::list;  // 2011-01-29
List<Token> Token::del_list;  // 2011-01-29


// This function overrides the default Token new operator function and
// maintains a list of tokens that have been allocated.  When a new Token
// is requested, it is added to the list of allocated tokens, the pointer
// to the list element is put into the token.

void *Token::operator new(size_t size)
{
	// allocate the memory for the token
	Token *token = (Token *)new char[size];

	// append token to list of tokens and save its element pointer in token
	token->element = list.append(&token);

	// return pointer to new token
	return token;
}


// This function overrides the default Token delete operator function
// and maintains list of tokens that have been allocated.  When a Token
// is deleted, it is removed from the list of allocated tokens using the
// element pointer put into the token when it was allocated.  If the
// Token was already deleted (its element pointer is NULL), it is added
// to the deleted list so that multiple deleted tokens can be reported.

void Token::operator delete(void *ptr)
{
	Token *token = (Token *)ptr;

	// check if token has already been deleted
	if (token->element == NULL)
	{
		// append a copy of the token to the multiple deleted token list
		del_list.append(token);
	}
	else
	{
		// remove element from the list of tokens
		list.remove(token->element);

		// mark the token as deleted (in case of a second delete attempt)
		token->element = NULL;

		// delete the token
		delete[] (char *)token;
	}
}


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

	// 2010-06-25: build message index and check message array
	// 2010-06-25: replaced with Error template
    // 2011-03-26: removed error checking (done by enums.awk) and index_status[]
}


// function to print version number (2011-06-11)
bool ibcp_version(char *name, int argc, char *argv[])
{
	if (argc != 2 || strcmp(argv[1], "-v") != 0)
	{
		return false;  // not our options
	}
	// 2010-03-13: changed to output actual program name
	// 2010-04-02: added + 1 so that '\' is no output
	printf("%s version %d.%d.%d\n", name, ibcp_VERSION_MAJOR,
		ibcp_VERSION_MINOR, ibcp_VERSION_PATCH);
	return true;
}


bool test_parser(Parser &parser, Table *table, int argc, char *argv[]);
bool test_translator(Translator &translator, Parser &parser, Table *table,
	int argc, char *argv[]);


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
	bool error_occurred = false;  // 2010-06-25

	// 2010-04-25: added check if program name does not have path
	char *name = strrchr(argv[0], '\\');
	// 2011-06-11: if not found then type '/' for linux
	if (name == NULL)
	{
		char *name = strrchr(argv[0], '/');
	}
	name = name == NULL ? argv[0] : name + 1;
	print_gpl_header(name);

	// 2010-06-25: added try block for token initialization
	try
	{
		Token::initialize();
	}
	// 2010-06-25: replaced with Error template
	catch (List<Error<TokenStatus> > *error_list)
	{
		// 2010-06-25: code replaced with function call
		Error<TokenStatus>::report(error_list, &print_stderr, "TokenStsMsg",
			"Status");
		delete error_list;  // 2011-01-27: eliminate memory leak
		error_occurred = true;
	}
	Table *table;
	try
	{
		table = new Table();
	}
	// 2010-06-25: replaced with Error template
	catch (List<Error<Code> > *error_list)
	{
		// 2010-06-25: code replaced with function call (code now in Error)
		Error<Code>::report(error_list, print_stderr, "Table", "Code");
		delete error_list;  // 2011-01-27: eliminate memory leak
		error_occurred = true;
	}
	if (error_occurred)  // 2010-06-25
	{
		fprintf(stderr, "Program aborting!\n");
		return 1;
	}
	printf("Table initialization successful.\n");

	Translator translator(table);
	Parser parser(table);

	// 2010-03-18: added call to test_translator
	// 2010-04-25: added "-t" to usage string
	// 2011-06-11: added check for version option
	if (!ibcp_version(name, argc, argv)
		&& !test_parser(parser, table, argc, argv)
		&& !test_translator(translator, parser, table, argc, argv))
	{
		// 2010-05-28: replaced strrchr(argv[1],...) call with 'name'
		// 2011-06-11: added "-v" to usage string
		printf("usage: %s -v -p|-t <options>\n", name);
	}
	delete table;  // 2011-01-27: eliminate memory leak
	return 0;
}


// end: ibcp.cpp
