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

#include <stdio.h>
#include <stdlib.h>
#include "ibcp.h"

void parse_input(Parser &parser, Table *table, const char *testinput);
void translate_input(Translator &translator, Parser &parser, Table *table,
	const char *testinput, bool exprmode = false);
bool print_token(Token *token, Table *table, bool tab);
bool print_small_token(Token *token, Table *table);
void print_error(Token *token, const char *error);
void print_token_leaks(Table *table, const char *testinput);


// 2012-10-11: new function to replace test_parser() and test_translator()
bool test_ibcp(Translator &translator, Parser &parser, Table *table, int argc,
	char *argv[])
{
	extern char *program_name;
	extern int program_name_len;

	const int max_inputline = 200;
	char parser_name[] = "parser";
	const size_t parser_len = sizeof(parser_name) - 1;
	char expression_name[] = "expression";
	const size_t expression_len = sizeof(expression_name) - 1;
	char translator_name[] = "translator";
	const size_t translator_len = sizeof(translator_name) - 1;

	enum {
		test_parser, test_expression, test_translator
	} testmode;
	char *name[] = {  // must match enumeration above
		parser_name, expression_name, translator_name
	};
	bool inputmode;

	if (argc == 2 && strcmp(argv[1], "-tp") == 0)
	{
		testmode = test_parser;
		inputmode = true;
	}
	else if (argc == 2 && strcmp(argv[1], "-te") == 0)
	{
		testmode = test_expression;
		inputmode = true;
	}
	else if (argc == 2 && strcmp(argv[1], "-tt") == 0)
	{
		testmode = test_translator;
		inputmode = true;
	}
	else if (argc == 3 && strcmp(argv[1], "-t") == 0)
	{
		// find start of file name less path
		char *filnam = strrchr(argv[2], '\\');  // dos path character
		if (filnam == NULL)
		{
			filnam = strrchr(argv[2], '/');  // linux path character
		}
		filnam = filnam == NULL ? argv[2] : filnam + 1;

		// get and check beginning of file name
		if (strncmp(filnam, parser_name, parser_len) == 0)
		{
			testmode = test_parser;
		}
		else if (strncmp(filnam, expression_name, expression_len) == 0)
		{
			testmode = test_expression;
		}
		else if (strncmp(filnam, translator_name, translator_len) == 0)
		{
			testmode = test_translator;
		}
		else
		{
			printf("usage: %.*s -t (%s|%s|%s)[XX]\n", program_name_len,
				program_name, parser_name, expression_name, translator_name);
			return true;  // our options were bad
		}
		inputmode = false;
	}
	else
	{
		return false;  // not our options
	}

	char inputline[max_inputline];
	FILE *fd;
	if (inputmode)
	{
		printf("\nTesting %s...", name[testmode]);
	}
	else
	{
		fd = fopen(argv[2], "r");
		if (fd == NULL)
		{
			printf("%.*s: error opening '%s'\n", program_name_len, program_name,
				argv[2]);
			return true;
		}
	}
	for (int lineno = 1;; lineno++)
	{
		if (inputmode)
		{
			printf("\nInput: ");
			if (gets(inputline) == NULL)
			{
				printf("\n%.*s: gets() failure - returned null\n",
					program_name_len, program_name);
				return true;
			}
			if (inputline[0] == '\0')
			{
				break;
			}
		}
		else
		{
			if (feof(fd))
			{
				break;
			}
			if (fgets(inputline, max_inputline, fd) == NULL)
			{
				if (feof(fd))
				{
					break;
				}
				printf("%.*s: error reading line %d from file '%s'\n",
					program_name_len, program_name, lineno, argv[2]);
				fclose(fd);
				return true;
			}
			int nl = strlen(inputline) - 1;
			if (inputline[nl] != '\n')
			{
				printf("%.*s: expected newline on line %d from file '%s'\n"
					"    (line too long?  maximum = %d\n", program_name_len,
					program_name, lineno, argv[2], max_inputline);
				fclose(fd);
				return true;
			}
			inputline[nl] = '\0';  // remove newline
			if (inputline[0] == '\0' || inputline[0] == '#')
			{
				continue;  // skip blank and comment lines
			}
			printf("\nInput: %s\n", inputline);
		}

		switch (testmode)
		{
		case test_parser:
			parse_input(parser, table, inputline);
			break;
		case test_expression:
			translate_input(translator, parser, table, inputline, true);
			print_token_leaks(table, inputline);
			break;
		case test_translator:
			translate_input(translator, parser, table, inputline, false);
			print_token_leaks(table, inputline);
			break;
		}
	}
	if (!inputmode)
	{
		fclose(fd);
		if (testmode != test_parser)
		{
			printf("\n");  // not for parser testing
		}
	}
	return true;
}


// 2010-03-11: created from parts of main
void parse_input(Parser &parser, Table *table, const char *testinput)
{
	Token *token;
	bool more;

	parser.start((char *)testinput);
	// 2010-03-18: fix loop since get_token() no longer returns null
	do {
		token = parser.get_token();
		more = print_token(token, table, true);
		if (more && token->type == Operator_TokenType
			&& token->code == EOL_Code)
		{
			more = false;
		}
		delete token;
	}
	while (more);
}


// 2010-03-18: new function for testing translator
// 2010-04-16: added new expression mode flag argument
void translate_input(Translator &translator, Parser &parser, Table *table,
	const char *testinput, bool exprmode)
{
	Token *token;
	Token *org_token;
	TokenStatus status;

	translator.start(exprmode);
	parser.start((char *)testinput);
	do {
        // set parser operand state from translator (2011-03-27)
        parser.set_operand_state(translator.get_operand_state());
		org_token = token = parser.get_token();
		// 2010-03-18: need to check for a parser error
		if (token->type == Error_TokenType)
		{
			print_error(token, token->string->get_ptr());
			delete token;
			translator.clean_up();
			return;
		}
		//print_token(token, table);
		status = translator.add_token(token);
	}
	while (status == Good_TokenStatus);
	if (status == Done_TokenStatus)
	{
		// 2010-05-15: change rpn_list from Token pointers
		List<RpnItem *> *rpn_list = translator.get_result();
		List<RpnItem *>::Element *element;
		printf("Output: ");
		// 2010-05-15: added separate print loop so operands can also be printed
		// 2011-01-29: modified for updated List class functions
		for (element = rpn_list->first(); element != NULL;
			rpn_list->next(element))
		{
			print_small_token(element->value->token, table);
			//print_token(token, table);
			if (element->value->noperands > 0)
			{
				char separator = '[';
				for (int i = 0; i < element->value->noperands; i++)
				{
					printf("%c", separator);
					print_small_token(element->value->operand[i]->value->token,
						table);
					separator = ',';
				}
				printf("]");
			}
			printf(" ");
		}
		// 2010-03-21: corrected to handle an empty RPN list
		// 2011-01-29: rewrote to remove last item instead of first item
		RpnItem *rpn_item;
		while (rpn_list->pop(&rpn_item))
		{
			delete rpn_item;
		}
		// 2012-10-24: fix memory leak
		delete rpn_list;
	}
	else  // error occurred, output it
	{
		// 2010-06-25: replaced status switch with token->message(status)
		// token pointer is set to cause of error
		print_error(token, token->message(status));
		if (token == org_token)
		{
			// 2010-04-14: only deleted error token if it's the original token
			//             returned from the parser, if not then this token is
			//             in the output list and will be deleted by the
			//             clean_up() function (the original token has been
			//             already been deleted by the Translator) XXX
			delete token;
		}
		else  // check if token is open paren (2011-01-30 leak)
		{
			translator.delete_open_paren(token);
		}
		translator.clean_up();
	}
	printf("\n");
}


// 2010-03-11: created from parts parse_input()
// 2011-01-29: added argument flag for printing out leading tab character
bool print_token(Token *token, Table *table, bool tab)
{
	// 2012-10-12: replaced all name text arrays with auto-generated file
	#include "test_names.h"

	CmdArgs *args;
	char bfr[20];
	char *exp;

	if (token->type == Error_TokenType)
	{
		// 2010-03-20: moved code to print_error()
		print_error(token, token->string->get_ptr());
		return false;
	}
	// 2010-03-13: test new Token static functions
	const char *info = "  ";
	if (token->has_paren())
	{
		info = token->is_operator() ? "??" : "()";
	}
	else if (token->is_operator())
	{
		info = "Op";
	}
	if (tab)
	{
		printf("\t");
	}
	printf("%2d: %-9s %s", token->column, tokentype_name[token->type], info);
	switch (token->type)
	{
	case ImmCmd_TokenType:
		// 2011-03-08: removed output of token code

		printf(" %s", code_name[token->code]);
		if (token->datatype == CmdArgs_DataType)
		{
			args = (CmdArgs *)token->string->get_data();
			printf(" Args: begin=%d end=%d start=%d incr=%d", args->begin,
				args->end, args->start, args->incr);
		}
		else if (token->datatype == String_DataType)
		{
			printf(" String Arg: |%.*s|", token->string->get_len(),
				token->string->get_ptr());
		}
		else
		{
			printf(" !Invalid Data Type!");
		}
		break;
	case Remark_TokenType:
		// 2011-03-08: removed output of token code
		printf(" %s", code_name[token->code]);
		// fall thru
	case DefFuncN_TokenType:
	case NoParen_TokenType:
		printf(" %-7s", datatype_name[token->datatype]);
		printf(" |%.*s|", token->string->get_len(),
			token->string->get_ptr());
		break;
	// 2011-03-26: separated tokens with parens, add paren to output
	case DefFuncP_TokenType:
	case Paren_TokenType:
		printf(" %-7s", datatype_name[token->datatype]);
		printf(" |%.*s(|", token->string->get_len(),
			token->string->get_ptr());
		break;
	case Constant_TokenType:
		printf(" %-7s", datatype_name[token->datatype]);
		switch (token->datatype)
		{
		case Integer_DataType:
			printf(" %d |%.*s|", token->int_value,
				token->string->get_len(), token->string->get_ptr());
			break;
		case Double_DataType:
			// only output 2 exponents digits unless 3 are needed (2012-10-14)
			sprintf(bfr, "%g", token->dbl_value);
			exp = strchr(bfr, 'e');
			if (exp != NULL)  // contains an exponent?
			{
				if (exp[4] != '\0' && exp[2] == '0')  // 3 digits and first '0'?
				{
					strcpy(exp + 2, exp + 3);  // move last 2 digits over first
				}
			}
			printf(" %s |%.*s|", bfr, token->string->get_len(),
				token->string->get_ptr());
			break;
		case String_DataType:
			printf(" |%.*s|", token->string->get_len(),
				token->string->get_ptr());
			break;
		}
		break;
	case Operator_TokenType:
	case IntFuncN_TokenType:
	case IntFuncP_TokenType:
		printf(" %-7s", datatype_name[token->datatype]);
	case Command_TokenType:
		// 2011-03-08: removed output of token code
		printf(" %s", code_name[token->code]);
		if (token->code == Rem_Code || token->code == RemOp_Code)
		{
			printf(" |%.*s|", token->string->get_len(),
				token->string->get_ptr());
		}
		break;
	default:
		// nothing more to output
		break;
	}
	printf("\n");
	return true;
}


// 2010-03-20: reimplemented print_token for small output
bool print_small_token(Token *token, Table *table)
{
	CmdArgs *args;

	// 2010-03-13: test new Token static functions
	switch (token->type)
	{
	case ImmCmd_TokenType:
		printf("%s:", table->name(token->code));
		if (token->datatype == CmdArgs_DataType)
		{
			args = (CmdArgs *)token->string->get_data();
			printf("%d,%d,%d,%d", args->begin, args->end, args->start,
				args->incr);
		}
		else if (token->datatype == String_DataType)
		{
			printf("\"%.*s\"", token->string->get_len(),
				token->string->get_ptr());
		}
		else
		{
			printf("?");
		}
		break;
	case Remark_TokenType:
		// fall thru
	case DefFuncN_TokenType:
	case NoParen_TokenType:
		printf("%.*s", token->string->get_len(), token->string->get_ptr());
		break;
	// 2011-03-26: separated tokens with parens, add paren to output
	case DefFuncP_TokenType:
	case Paren_TokenType:
		printf("%.*s(", token->string->get_len(), token->string->get_ptr());
		break;
	case Constant_TokenType:
		switch (token->datatype)
		{
		case Integer_DataType:
			printf("%.*s", token->string->get_len(), token->string->get_ptr());
			break;
		case Double_DataType:
			printf("%.*s", token->string->get_len(), token->string->get_ptr());
			break;
		case String_DataType:
			printf("\"%.*s\"", token->string->get_len(),
				token->string->get_ptr());
			break;
		}
		break;
	case Operator_TokenType:
		if (token->code == RemOp_Code)
		{
			printf("%s|%.*s|", table->name(token->code),
				token->string->get_len(), token->string->get_ptr());
		}
		else
		{
			// 2010-04-02: output name2 (if set) for debug output string
			// 2010-04-04: replaced with debug_name call
			printf("%s", table->debug_name(token->code));
		}
		break;
	case IntFuncN_TokenType:
	case IntFuncP_TokenType:
		// 2010-04-04: replaced with debug_name call
		printf("%s", table->debug_name(token->code));
		break;
	case Command_TokenType:
		if (token->code == Rem_Code)
		{
			printf("%s|%.*s|", table->name(token->code),
				token->string->get_len(), token->string->get_ptr());
		}
		else
		{
			printf("%s", table->name(token->code));
			// 2010-06-06: call name2() instead of name()
			if (table->name2(token->code) != NULL)
			{
				printf("-%s", table->name2(token->code));
			}
		}
		break;
	default:
		// nothing more to output
		break;
	}
	// 2010-04-12: output reference identifier
	if (token->reference)
	{
		printf("<ref>");
	}
	// 2010-05-29: output sub-code flags
	// 2011-01-22: ignore used sub-code
	if (token->subcode & ~Used_SubCode)
	{
		printf("'");
		if (token->subcode & Paren_SubCode)
		{
			printf(")");
		}
		if (token->subcode & Let_SubCode)
		{
			printf("LET");
		}
		// 2010-08-01: removed Comma_SubCode
		// 2010-06-08: added semicolon subcode flag
		if (token->subcode & SemiColon_SubCode)
		{
			printf(";");
		}
		// 2011-03-20: added keep and end subcodes
		if (token->subcode & Keep_SubCode)
		{
			printf("Keep");
		}
		if (token->subcode & End_SubCode)
		{
			printf("End");
		}
		// 2011-03-22: added question subcodes
		if (token->subcode & Question_SubCode)
		{
			printf("Question");
		}
		printf("'");
	}
	return true;
}


// 2010-03-20: created from parts print_token()
void print_error(Token *token, const char *error)
{
	int len;

	// 2010-03-07: modified to use new error length
	printf("       %*s", token->column, "");
	// 2011-01-11: removed extra code, token now contains correct length
	for (int j = 0; j < token->length; j++)
	{
		putchar('^');
	}
	printf("-- %s\n", error);
}


void print_token_leaks(Table *table, const char *testinput)
{
	// 2011-01-29: check for undeleted tokens
	if (!Token::list.empty())
	{
		printf("Leaks: %s\n", testinput);
		int len = strlen(testinput) + 1;  // one for EOL that may be at end
		char *leaks = new char[len + 1];  // one for '\0' terminator
		memset(leaks, ' ', len);
		leaks[len] = '\0';
		char id = '1';
		List<Token *>::Element *element;
		for (element = Token::list.first(); element != NULL;
			Token::list.next(element))
		{
			for (int i = element->value->length; --i >= 0; )
			{
				leaks[element->value->column + i] = id;
			}
			switch (id)
			{
			case '9':
				id = 'A';
				break;
			case 'Z':
				id = 'a';
				break;
			case 'z':
				id = '*';
				break;
			default:
				id++;
			}
		}
		printf("Leaks: %s\n", leaks);
		delete[] leaks;
		id = '1';
		while ((element = Token::list.first()) != NULL)
		{
			printf("Token Leak '%c' ", id);
			print_token(element->value, table, false);
			delete element->value;  // this will also delete it from list
			switch (id)
			{
			case '9':
				id = 'A';
				break;
			case 'Z':
				id = 'a';
				break;
			case 'z':
				id = '*';
				break;
			default:
				id++;
			}
		}
	}
	if (!Token::del_list.empty())
	{
		printf("\nExtra Token Deletes\n");
		Token token;
		for (int i = 1; Token::del_list.remove(NULL, &token); i++)
		{
			printf("Token #%d ", i);
			print_token(&token, table, false);
		}
	}
}
