// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_parser.cpp - contains code for testing the parser class
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

#include <stdio.h>
#include <stdlib.h>
#include "../ibcp.h"


void parse_input(Parser &parser, Table *table, const char *testinput);


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
	if (argc == 2)
	{
		testno = atoi(argv[1]) - 1;
		inputmode = strcmp(argv[1], "-i") == 0;
	}
	if (argc != 2 || (testno < 0 || testno >= ntests) && inputmode == 0)
	{ 
		printf("usage: test_parser <test number 1-%d>|-i\n", ntests);
		return 0;
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
		"Add", "Sub", "Mul", "Div", "IntDiv", "Mod", "Power",
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
		"List", "Edit", "Delete", "Run", "Renum", "Save", "Load", "New", "Auto",
		"Cont", "Quit",
	};

	Token *token;
	CmdArgs *args;

	parser.start((char *)testinput);
	while ((token = parser.get_token()) != NULL)
	{
		if (token->type == Error_TokenType)
		{
			// 2010-03-07: modified to use new error length
			printf("       %*s", token->column, "");
			for (int j = 0; j < token->length; j++)
			{
				putchar('^');
			}
			printf("-- %s\n", token->string->get_str());
			delete token;
			break;  // stop processing input
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
		printf("\t%2d: %-9s %s", token->column, tokentype_name[token->type],
			info);
		switch (token->type)
		{
		case ImmCmd_TokenType:
			printf(" %3d-%s", token->code,
				code_name[table->code(token->code)]);
			if (token->datatype == CmdArgs_DataType)
			{
				args = (CmdArgs *)token->string->get_data();
				printf(" Args: begin=%d end=%d start=%d incr=%d",
					args->begin, args->end, args->start, args->incr);
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
			printf(" %d-%s", token->code,
				code_name[table->code(token->code)]);
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
		case IntFuncN_TokenType:  // 2010-03-13: replaced IncFunc entry
		case IntFuncP_TokenType:
			printf(" %-7s", datatype_name[token->datatype]);
		case Command_TokenType:
			printf(" %d-%s", token->code,
				code_name[table->code(token->code)]);
			if (table->code(token->code) == Rem_Code
				|| table->code(token->code) == RemOp_Code)
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
		delete token;
	}
}