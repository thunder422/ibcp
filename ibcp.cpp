// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: ibcp.h - miscellaneous functions
//	Copyright (C) 2010  Thunder422
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
//  Change History:
//
//  2010-03-13  initial release, created to hold variables and functions
//              related to definitions in the ibcp.h definitions file that have
//              no other file into; and main, which includes the GPL header
//              output, Token initialization call, Table initialization code,
//              and a call to the test_parser() function
//
//  2010-03-17  changed return value of test_parser from int to bool (the
//                command line options are passed to each test routine and if
//                the options are not for them, they return false so the next
//                test routine can be called)
//              removed includes that weren't needed,
//              changed exit() to a return
//
//  2010-03-18  added support for test_translator
//
//  2010-03-20  added translator instantiation to main() and translator
//              argument to translator function calls
//
//  2010-04-02  added support for obtaining precedence from token types that
//              don't have table entries (DefFuncP and Paren)
//
//  2010-04-25  added "-t" to usage string
//              added check if program name does not have full path
//
//  2010-05-20  added maximum operands and maximum associate code table error
//              reporting
//
//  2010-05-28  replaced strrchr(argv[1],...) call with 'name' in usage
//              message when command line options are not recognized, this
//              cause the program to crash when run from the Windows cmd line
//  2010-05-29  added the initialization of the new token has table entry flags
//
//  2010-06-25  added TokenStatus initialization to Token::initialize()
//               for new token status message structure array
//              replaced TokenStsMgsErr and TableError with generic Error
//                template
//  2010-06-28  added NULL string check to Token::initialize() message array
//                initialization to catch missing entries
//              added missing entry for Null_TokenStatus to
//                Token::message_array[], which was not detected by the error
//                checking because there was a null entry at the end of the
//                array that happen to have a 0 (Null_TokenStatus) in it
//  2010-06-29  updated expected expression error messages
//

#include <stdio.h>
#include <stdarg.h>  // 2010-06-25: for generic print function
#include "ibcp.h"

void print_gpl_header(char *name)
{
    printf("\n%s  Copyright (C) 2010  Thunder422\n", name);
    printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
    printf("This is free software, and you are welcome to\n");
    printf("redistribute it under certain conditions.\n\n");
}


// Token
bool Token::paren[sizeof_TokenType];
bool Token::op[sizeof_TokenType];
int Token::prec[sizeof_TokenType];  // 2010-04-02
bool Token::table[sizeof_TokenType];  // 2010-05-29
TokenStsMsg Token::message_array[sizeof_TokenStatus] = { // 2010-06-25
	{Null_TokenStatus,
		"BUG: Null_TokenStaus"},  // should not see this (2010-06-28)
	{Good_TokenStatus,
		"BUG: Good_TokenStaus"},  // should not see this
	{Done_TokenStatus,
		"BUG: Done_TokenStaus"},  // should not see this
	{ExpStatement_TokenStatus,
		"expected statement"},
	{ExpExpr_TokenStatus,
		"expected expression"},
	{ExpExprOrEnd_TokenStatus,
		"expected expression or end-of-statement"},
	{ExpOpOrEnd_TokenStatus,
		"expected operator or end-of-statement"},
	{ExpBinOpOrEnd_TokenStatus,
		"expected binary operator or end-of-statement"},
	{ExpEqualOrComma_TokenStatus,
		"expected equal or comma for assignment"},
	{ExpAssignItem_TokenStatus,
		"expected item for assignment"},
	{ExpOpOrComma_TokenStatus,
		"expected operator or comma"},
	{ExpOpCommaOrParen_TokenStatus,
		"expected operator, comma or closing parentheses"},
	{NoOpenParen_TokenStatus,
		"missing opening parentheses"},
	{ExpOpOrParen_TokenStatus,
		"expected operator or closing parentheses"},
	{UnexpAssignComma_TokenStatus,
		"expected operator in assignment"},
	{ExpAssignRef_TokenStatus,
		"item cannot be assigned"},
	{ExpAssignListRef_TokenStatus,
		"list item cannot be assigned"},
	{UnexpParenInCmd_TokenStatus,
		"unexpected parentheses in command"},
	{UnexpParenInComma_TokenStatus,
		"unexpected parentheses in assignment list"},
	{ExpDouble_TokenStatus,
		"expected double expression"},
	{ExpInteger_TokenStatus,
		"expected integer expression"},
	{ExpString_TokenStatus,
		"expected string expression (old)"},
	{ExpNumExpr_TokenStatus,
		"expected numeric expression"},
	{ExpStrExpr_TokenStatus,
		"expected string expression"},
	{UnExpCommand_TokenStatus,
		"unexpected command"},
	{PrintOnlyIntFunc_TokenStatus,
		"invalid use of print function"},
	{ExpDblVar_TokenStatus,
		"expected double variable"},
	{ExpIntVar_TokenStatus,
		"expected integer variable"},
	{ExpStrVar_TokenStatus,
		"expected string variable"},
	{BUG_NotYetImplemented,
		"BUG: not yet implemented"},
	{BUG_InvalidMode,
		"BUG: invalid mode"},
	{BUG_HoldStackEmpty,
		"BUG: hold stack empty, expected Null"},
	{BUG_HoldStackNotEmpty,
		"BUG: hold stack not empty"},
	{BUG_DoneStackNotEmpty,
		"BUG: done stack not empty"},
	{BUG_DoneStackEmptyParen,
		"BUG: done stack empty, expected token for ')'"},
	{BUG_DoneStackEmptyArrFunc,
		"BUG: done stack empty, expected token for array/function"},
	{BUG_UnexpectedCloseParen,
		"BUG: unexpected closing parentheses"},
	{BUG_UnexpectedToken,
		"BUG: expected token on stack for array/function"},
	{BUG_DoneStackEmpty,
		"BUG: expected operand on done stack"},
	{BUG_CmdStackNotEmpty,
		"BUG: command stack not empty"},
	{BUG_CmdStackEmpty,
		"BUG: command stack empty"},
	{BUG_Debug,
		"BUG: debug"}
};
int Token::index_status[sizeof_TokenStatus];


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
	List<Error<TokenStatus> > *error_list = new List<Error<TokenStatus> >;
	int i;
	for (i = 0; i < sizeof_TokenStatus; i++)  // initialize index array
	{
		index_status[i] = -1;
	}
	for (i = 0; i < sizeof_TokenStatus; i++)  // fill in index array
	{
		TokenStatus status = message_array[i].status;
		// 2010-06-28: added missing string check
		if (message_array[i].string == NULL)
		{
			continue;  // will be caught in the missing check below
		}
		else if (index_status[status] == -1)
		{
			index_status[status] = i;
		}
		else  // already assigned
		{
			Error<TokenStatus> error(status, index_status[status], i);
			error_list->append(&error);
		}
	}
	for (i = 0; i < sizeof_TokenStatus; i++)  // check for missing statuses
	{
		if (index_status[i] == -1)
		{
			Error<TokenStatus> error((TokenStatus)i);
			error_list->append(&error);
		}
	}

	// throw exception if error_list is not empty
	if (!error_list->empty())
	{
		throw error_list;
	}

	delete error_list;
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
	if (!test_parser(parser, table, argc, argv)
		&& !test_translator(translator, parser, table, argc, argv))
	{
		// 2010-05-28: replaced strrchr(argv[1],...) call with 'name'
		printf("usage: %s -p|-t <options>\n", name);
	}
	return 0;
}
	

// end: ibcp.cpp
