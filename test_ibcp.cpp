// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_ibcp.cpp - contains code for testing
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
//  2010-03-01  initial release
//
//  2010-03-06  added support for a command line argument to indicate which test
//              to run, added test input lines for testing identifiers, split
//              DefFunc_TokenType into DefFuncN_TokenType and
//              DefFuncP_TokenType
//
//  2010-03-07  added test input lines for testing numbers,
//              changed error display to use new length value in token
//
//  2010-03-08  added test input lines for testing strings
//
//  2010-03-10  added test input lines for testing operators
//
//  2010-03-11  added command line option to input lines interactively,
//              separated code into new parse_input() function
//
//  2010-03-13  replaced IncFunc with IntFuncN and IntFuncP token types
//              added new Token static functions that indicate whether token
//                has a parentheses or is and operator
//              changed main() to test_parser(), moved print_gpl_header, and
//                table initialization to new ibcp.cpp source file
//
//  2010-03-17  renamed file from test_parser.cpp - will now contain all test
//                code
//              separated print_token() from parse_input() so that it can be
//                used from other functions
//              added string for new Code values to print_token
//              changed all token->code to token->index
//
//  2010-03-18  fixed loop since get_token() no longer returns null
//              added new functions test_translator() and translate_input() for
//                testing translator
//
//  2010-03-20  implemented test translator code, made print_small_token() so
//              that RPN list could be output in a compact form
//
//  2010-03-21  added check for parser errors in translator test code
//              corrected output rpn list to handle an empty list
//              added bad length check to print_error()
//              corrected print_error() to handle error tokens
//              added more simple expressions test inputs
//
//  2010-03-25  added expressions for testing expressions with parentheses
//              added additional errors for parentheses errors
//
//  2010-03-26  added more parentheses test expressions 
//

#include <stdio.h>
#include <stdlib.h>
#include "ibcp.h"

void parse_input(Parser &parser, Table *table, const char *testinput);
void translate_input(Translator &translator, Parser &parser, Table *table,
	const char *testinput);
bool print_token(Token *token, Table *table);
bool print_small_token(Token *token, Table *table);
void print_error(Token *token, const char *error);


// 2010-03-13: changed from main()
bool test_parser(Parser &parser, Table *table, int argc, char *argv[])
{
	const char *testinput1[] = {  // immediate commands
		"L500=1000",
		" L500 = 1000 ",
		" L 500 = 1000 ",
		"L500-1000",
		" L 500 - 1000 ",
		"10L500-1000",
		" 10 L500 - 1000 ",
		" 10 L 500 - 1000 ",
		"l\"file",
		" l \"file\" ",
		" l ",
		"a", "A500,1000", "A,1000", "A500,", "A,", "A500;1000",
		"L500,1000",
		"  L500,1000",
		"  L500,L1000=5",
		"r", "r100", "R100,",
		"r100-", "r-200", "r-", "r100-200",
		"r100-,", "r-200,", "r-,", "r100-200,",
		"r100-,10", "r-200,10", "r-,10", "r100-200,10",
		"r100-,400,", "r-200,400,", "r-,400,", "r100-200,400,",
		"r100-,400,10", "r-200,400,10", "r-,400,10", "r100-200,400,10",
		"s\"file\",a",				// error: something after string
		"l444444444444",			// error: begin too big
		"l100-444444444444",		// error: end too big
		"l100-200;",				// error: wrong character after range
		"r100-200,4444444444",		// error: number after range too big
		"r100-200,4444444444,10",	// error: number after range too big
		"r100-200,l100",			// error: something invalid afer range
		"r100-200,400,0",			// error: invalid zero increment
		"a100,0",					// error: invalid zero increment
		"a100,10;L100",				// error: something after increment
		"r100,200,10",				// error: unexpected character
		"r100-200,400,10,20",		// error: something after increment
		"r100-200,400,4444444444",	// error: increment too big
		"r100-200,-10", 			// error: unexpected character
		"r100-200,400,-10",			// error: unexpected character
		NULL
	};

	const char *testinput2[] = {  // identifiers
		"fna FnB fNc FND fna$ fna% fnword# fnhello$ fnindex%",
		"fna( FnB( fNc(FND( fna$(fna%( fnword#( fnhello$( fnindex%",
		"a b(c$ D# asdf$ qwer( zxcv Asdf% QWER%( Nbb_34$( h_544_4#(",
		"LET PRINT end then xor Abs(CHR$(val( end if left$ left$(",
		"rem this should be a comment",
		"rem:this should be a comment",
		"rem-this should be a comment",
		NULL
	};

	const char *testinput3[] = {  // numbers
		"00", "01", ".A", ".e",	"..01",			// error tests
		"0.1 .1 .01 0.01 0.01000,1000A100.001",
		"100e10 100E+10 100e-10",
		"100er", "100E+r", "100E-r", "100+200", "100..001", "100e0",
		"2147483647 -2147483647 2147483648",	// integer limits tests
		"1.23456e308 1.234e309",				// double limits tests
		"1.2345e-307 1.234e-308",
		"1.23456e3081234",						// double limits tests
		"14234453464575678567846434234234534566",
		NULL
	};

	const char *testinput4[] = {  // strings
		"A$ = \"this is a test\"",
		"A$ = \"this is a test",			// no closing quote
		"A$ = \"this is a \"\"test\"\"\"",	// embedded quotes end
		"A$ = \"\"\"this\"\" is a test\"",	// embedded quotes begin
		"A$ = \"this is \"\"a\"\" test\"",	// embedded quotes middle
		"A$ = \"this is \"\" a test\"",		// single embedded quote
		"A$ = \"\"",						// empty string
		"A$ = \"",							// empty string/no closing quote
		"A$ = \"\"\"\"",					// single embedded quote
		"A$ = \"\"\"",						// embedded quote/no closing quote
		"A$ = LEFT$(\"TEST\", 1)",			// something after string
		NULL
	};

	const char *testinput5[] = {  // strings
		"A$ = mid$(\"b\",A+B*2,index)+LEFT$(\"TEST\", 1)",
		"if index>5 and subindex<=10 then",
		"var = 5'initialize variable + ",
		"if index<<5 && index><4 || n<4 then",					// error
		NULL
	};

	const char **test[] = {
		testinput1, testinput2, testinput3, testinput4, testinput5
	};
	const int ntests = sizeof(test) / sizeof(test[0]);

	int testno;
	int inputmode;
	if (argc < 2 || strcmp(argv[1], "-p") != 0)
	{
		return false;  // not our options
	}
	if (argc == 3)
	{
		testno = atoi(argv[2]) - 1;
		inputmode = strcmp(argv[2], "i") == 0;
	}
	if (argc != 3 || (testno < 0 || testno >= ntests) && inputmode == 0)
	{ 
		// 2010-03-13: changed to output actual program name
		printf("usage: %s -p <test number 1-%d>|i\n", strrchr(argv[0], '\\') + 1,
			ntests);
		return true;  // our options are bad
	}

	// 2010-03-13: removed Table initialization code
	
	if (inputmode)
	{
		char inputline[200];
		do {
			printf("\nInput: ");
			gets(inputline);
			parse_input(parser, table, inputline);
		} while (inputline[0] != '\0');
	}
	else
	{
		const char **testinput = test[testno];
		for (int i = 0; testinput[i] != NULL; i++)
		{
			printf("\nInput: %s\n", testinput[i]);
			parse_input(parser, table, testinput[i]);
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
		more = print_token(token, table);
		if (more && token->type == Operator_TokenType
			&& table->code(token->index) == EOL_Code)
		{
			more = false;
		}
		delete token;
	}
	while (more);
}


// 2010-03-18: added new function for testing translator
bool test_translator(Translator &translator, Parser &parser, Table *table,
	int argc, char *argv[])
{
	const char *testinput1[] = {  // simple expressions tests
		"A - B",
		"A + B",
		"A + B * C",
		"A * B + C",
		"A * -B + -C^D",
		"Value1 + Value2 - Value3 - Value4",
		"var_A * Var_A + Var_B * Var_B",
		"not A < 5 = B > 2",
		"not A < 5 and B > 2 or C < 1 and D <> 2",
		"A * B / c \\ D MOD E",
		"A mod B \\ c / D * E",
		"A * B ^ 2",
		"a$ = \"this\" + \"test\"",
		NULL
	};
	const char *testinput2[] = {  // parentheses expressions tests
		"(A + B) + C",
		"(A * B) + C",
		"(A + B) * C",
		"A + (B + C)",
		"A + (B + C) + D",
		"A + (B + C) * D",
		"(A + (B + C)) + D",
		"(A + (B + C)) * D",
		"A + (B * C)",
		"A + (B * C) + D",
		"A + (B * C) * D",
		"A + (B * C) ^ D",
		"A + ((B * C) + D)",
		"A + ((B * C) * D)",
		"A + ((B * C) ^ D)",
		"A * (B + C)",
		"A * (B + C) + D",
		"A * (B + C) * D",
		"A + (B) + C",
		"A + ((B)) + C",
		"A + B + ((C))",
		"(A + B + C)",
		"((A + B)) + C",
		"A + ((B * C))",
		"A + B * C ^ D",
		"((A + B) * C) ^ D",
		"-A ^ 2 + (-B) ^ 3",
		"(A + B) * (C + D)",
		"(A + B) + (C + D)",
		"A + B + (C + D)",
		"(A * B) + (C * D)",
		"(A + (B * (C ^ D))) + E",
		"(A ^ (B * (C + D))) + E",
		"(A + B",			// test missing close parentheses
		"A + B)",			// test missing open parentheses
		NULL
	};

	const char **test[] = {
		testinput1, testinput2
	};
	const int ntests = sizeof(test) / sizeof(test[0]);

	int testno;
	int inputmode;
	if (argc < 2 || strcmp(argv[1], "-t") != 0)
	{
		return false;  // not our options
	}
	if (argc == 3)
	{
		testno = atoi(argv[2]) - 1;
		inputmode = strcmp(argv[2], "i") == 0;
	}
	if (argc != 3 || (testno < 0 || testno >= ntests) && inputmode == 0)
	{ 
		// 2010-03-13: changed to output actual program name
		printf("usage: %s -t <test number 1-%d>|i\n", strrchr(argv[0], '\\') + 1,
			ntests);
		return true;  // our options are bad
	}

	if (inputmode)
	{
		char inputline[200];
		do {
			printf("\nInput: ");
			gets(inputline);
			translate_input(translator, parser, table, inputline);
		} while (inputline[0] != '\0');
	}
	else
	{
		const char **testinput = test[testno];
		for (int i = 0; testinput[i] != NULL; i++)
		{
			printf("\nInput: %s\n", testinput[i]);
			translate_input(translator, parser, table, testinput[i]);
		}
	}
	printf("\n");
	return true;
}


// 2010-03-18: new function for testing translator
void translate_input(Translator &translator, Parser &parser, Table *table,
	const char *testinput)
{
	Token *token;
	Translator::Status status;

	translator.start();
	parser.start((char *)testinput);
	do {
		token = parser.get_token();
		// 2010-03-18: need to check for a parser error
		if (token->type == Error_TokenType)
		{
			print_error(token, token->string->get_str());
			delete token;
			translator.clean_up();
			printf("\n");
			return;
		}
		//print_token(token, table);
		status = translator.add_token(token);
	}
	while (status == Translator::Good);
	if (status == Translator::Done)
	{
		List<Token *> *rpn_list = translator.get_result();
		printf("Output: ");
		// 2010-03-21: corrected to handle an empty rpn list
		while (!rpn_list->empty())
		{
			rpn_list->remove(NULL, &token);
			//print_token(token, table);
			print_small_token(token, table);
			delete token;
			printf(" ");
		}
	}
	else  // error occurred, output it
	{
		const char *error;

		switch (status)
		{
		case Translator::ExpectedOperand:
			error = "expected operand";
			break;
		case Translator::ExpectedOperator:
			error = "expected operator";
			break;
		case Translator::ExpectedBinOp:
			error = "expected binary operator";
			break;
		// 2010-03-25: added missing parentheses errors
		case Translator::MissingOpenParen:
			error = "missing opening parentheses";
			break;
		case Translator::MissingCloseParen:
			error = "missing closing parentheses";
			break;
		case Translator::NotYetImplemented:
			error = "not yet implemented";
			break;
		case Translator::StackEmpty:
			error = "hold stack empty, expected Null";
			break;
		case Translator::StackNotEmpty:
			error = "hold stack not empty";
			break;
		case Translator::StackEmpty1:
			error = "expected operand 1 on done stack";
			break;
		case Translator::StackEmpty2:
			error = "expected operand 2 on done stack";
			break;
		case Translator::StackNotEmpty2:
			error = "done stack not empty";
			break;
		case Translator::StackEmpty3:
			error = "done stack empty, expected result";
			break;
		// 2010-03-25: added error for parentheses support
		case Translator::StackEmpty4:
			error = "done stack empty, expected token for ')'";
			break;
		default:
			error = "UNEXPECTED ERROR";
			break;
		}
		// token pointer is set to cause of error
		print_error(token, error);
		delete token;
		translator.clean_up();
	}
	printf("\n");
}


// 2010-03-11: created from parts parse_input()
bool print_token(Token *token, Table *table)
{
	const char *tokentype_name[] = {
		"ImmCmd",
		"Command",
		"Operator",
		"IntFuncN",  // 2010-03-13: replaced IncFunc entry
		"IntFuncP",
		"Remark",
		"Constant",
		"DefFuncN",
		"DefFuncP",
		"NoParen",
		"Paren",
		"Error"
	};
	const char *datatype_name[] = {
		"None",
		"Double",
		"Integer",
		"String"
	};
	const char *code_name[] = {
		"Null",
		"Add", "Sub", "Neg", "Mul", "Div", "IntDiv", "Mod", "Power",
		"Eq", "Gt", "GtEq", "Lt", "LtEq", "NotEq",
		"And", "Or", "Not", "Eqv", "Imp", "Xor",
		"Abs", "Fix", "Int", "Rnd", "RndArg", "Sgn", "Cint",
		"Sqr", "Atn", "Cos", "Sin", "Tan", "Exp", "Log",
		"Asc", "Chr", "Instr", "Left", "Len", "Mid", "Repeat", "Right", "Space",
		"Str", "Val",
		"OpenParen", "CloseParen", "Comma", "SemiColon", "Colon", "RemOp",
		"Tab", "Spc",
		"Let", "Print", "Input", "Dim", "Def", "Rem",
		"If", "Then", "Else", "EndIf",
		"For", "To", "Step", "Next",
		"Do", "DoWhile", "DoUntil", "While", "Until", "Loop",
		"LoopWhile", "LoopUntil", "End",
		"EOL",
		"List", "Edit", "Delete", "Run", "Renum", "Save", "Load", "New", "Auto",
		"Cont", "Quit",
	};

	CmdArgs *args;

	if (token->type == Error_TokenType)
	{
		// 2010-03-20: moved code to print_error()
		print_error(token, token->string->get_str());
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
	printf("\t%2d: %-9s %s", token->column, tokentype_name[token->type], info);
	switch (token->type)
	{
	case ImmCmd_TokenType:
		printf(" %3d-%s", token->index,
			code_name[table->code(token->index)]);
		if (token->datatype == CmdArgs_DataType)
		{
			args = (CmdArgs *)token->string->get_data();
			printf(" Args: begin=%d end=%d start=%d incr=%d", args->begin,
				args->end, args->start, args->incr);
		}
		else if (token->datatype == String_DataType)
		{
			printf(" String Arg: |%.*s|", token->string->get_len(),
				token->string->get_str());
		}
		else
		{
			printf(" !Invalid Data Type!");
		}
		break;
	case Remark_TokenType:
		printf(" %d-%s", token->index,
			code_name[table->code(token->index)]);
		// fall thru
	case DefFuncN_TokenType:
	case DefFuncP_TokenType:
	case NoParen_TokenType:
	case Paren_TokenType:
		printf(" %-7s", datatype_name[token->datatype]);
		printf(" |%.*s|", token->string->get_len(),
			token->string->get_str());
		break;
	case Constant_TokenType:
		printf(" %-7s", datatype_name[token->datatype]);
		switch (token->datatype)
		{
		case Integer_DataType:
			printf(" %d |%.*s|", token->int_value,
				token->string->get_len(), token->string->get_str());
			break;
		case Double_DataType:
			printf(" %g |%.*s|", token->dbl_value,
				token->string->get_len(), token->string->get_str());
			break;
		case String_DataType:
			printf(" |%.*s|", token->string->get_len(),
				token->string->get_str());
			break;
		}
		break;
	case Operator_TokenType:
	case IntFuncN_TokenType:
	case IntFuncP_TokenType:
		printf(" %-7s", datatype_name[token->datatype]);
	case Command_TokenType:
		printf(" %d-%s", token->index,
			code_name[table->code(token->index)]);
		if (table->code(token->index) == Rem_Code
			|| table->code(token->index) == RemOp_Code)
		{
			printf(" |%.*s|", token->string->get_len(),
				token->string->get_str());
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
		printf("%s:", table->name(token->index));
		if (token->datatype == CmdArgs_DataType)
		{
			args = (CmdArgs *)token->string->get_data();
			printf("%d,%d,%d,%d", args->begin, args->end, args->start,
				args->incr);
		}
		else if (token->datatype == String_DataType)
		{
			printf("\"%.*s\"", token->string->get_len(),
				token->string->get_str());
		}
		else
		{
			printf("?");
		}
		break;
	case Remark_TokenType:
		// fall thru
	case DefFuncN_TokenType:
	case DefFuncP_TokenType:
	case NoParen_TokenType:
	case Paren_TokenType:
		printf("%.*s", token->string->get_len(), token->string->get_str());
		break;
	case Constant_TokenType:
		switch (token->datatype)
		{
		case Integer_DataType:
			printf("%d", token->int_value);
			break;
		case Double_DataType:
			printf("%g", token->dbl_value);
			break;
		case String_DataType:
			printf("\"%.*s\"", token->string->get_len(),
				token->string->get_str());
			break;
		}
		break;
	case Operator_TokenType:
		if (table->code(token->index) == RemOp_Code)
		{
			printf("%s|%.*s|", table->name(token->index),
				token->string->get_len(), token->string->get_str());
		}
		else
		{
			printf("%s", table->name(token->index));
		}
		break;
	case IntFuncN_TokenType:
	case IntFuncP_TokenType:
		printf("%s", table->name(token->index));
		break;
	case Command_TokenType:
		if (table->code(token->index) == Rem_Code)
		{
			printf("%s|%.*s|", table->name(token->index),
				token->string->get_len(), token->string->get_str());
		}
		else
		{
			printf("%s", table->name(token->index));
			if (table->name(token->index) != NULL)
			{
				printf("-%s", table->name2(token->index));
			}
		}
		break;
	default:
		// nothing more to output
		break;
	}
	return true;
}


// 2010-03-20: created from parts print_token()
void print_error(Token *token, const char *error)
{
	int len;

	// 2010-03-07: modified to use new error length
	printf("       %*s", token->column, "");
	if (token->type == Error_TokenType || token->string == NULL)
	{
		len = token->length;
		if (len > 20)  // should not happed
		{
			len = 1;
		}
	}
	else
	{
		len = token->string->get_len();
	}
	for (int j = 0; j < len; j++)
	{
		putchar('^');
	}
	printf("-- %s\n", error);
}
