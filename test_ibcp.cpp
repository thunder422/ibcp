// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_ibcp.cpp - contains code for testing
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
//

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


// 2010-03-13: changed from main()
bool test_parser(Parser &parser, Table *table, int argc, char *argv[])
{
	static const char *testinput1[] = {  // immediate commands
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

	static const char *testinput2[] = {  // identifiers
		"fna FnB fNc FND fna$ fna% fnword# fnhello$ fnindex%",
		"fna( FnB( fNc(FND( fna$(fna%( fnword#( fnhello$( fnindex%",
		"a b(c$ D# asdf$ qwer( zxcv Asdf% QWER%( Nbb_34$( h_544_4#(",
		"LET PRINT end then xor Abs(CHR$(val( end if left$ left$(",
		"rem this should be a comment",
		"rem:this should be a comment",
		"rem-this should be a comment",
		NULL
	};

	static const char *testinput3[] = {  // numbers
		"00", "01", ".A", ".e",	"..01",			// error tests
		"0.1 .1 .01 0.01 0.01000,1000A100.001",
		"100e10 100E+10 100e-10",
		"100er", "100E+r", "100E-r", "100+200", "100..001", "100e0",
		"2147483647 -2147483647 2147483648",	// integer limits tests
		"1.23456e308 1.234e309",				// double limits tests
		"1.2345e-307 1.234e-308",
		"1.23456e3081234",						// double limits tests
		"14234453464575678567846434234234534566",
		"0", "0+1",								// added 2010-05-29
		NULL
	};

	static const char *testinput4[] = {  // strings
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

	static const char *testinput5[] = {  // strings
		"A$ = mid$(\"b\",A+B*2,index)+LEFT$(\"TEST\", 1)",
		"if index>5 and subindex<=10 then",
		"var = 5'initialize variable + ",
		"if index<<5 && index><4 || n<4 then",					// error
		NULL
	};

	static const char **test[] = {
		testinput1, testinput2, testinput3, testinput4, testinput5
	};
	static const int ntests = sizeof(test) / sizeof(test[0]);

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
		printf("usage: %s -p <test number 1-%d>|i\n", strrchr(argv[0], '\\')
			+ 1, ntests);
		return true;  // our options are bad
	}

	// 2010-03-13: removed Table initialization code

	if (inputmode)
	{
		char inputline[200];
		printf("\nParser Testing...");  // 2010-05-29: added
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


// 2010-03-18: added new function for testing translator
bool test_translator(Translator &translator, Parser &parser, Table *table,
	int argc, char *argv[])
{
	static const char *testinput1[] = {  // simple expressions tests
		"A - B",
		"A + B",
		"A + B * C",
		"A * B + C",
		"A * -B + -C^D",
		"Value1 + Value2 - Value3 - Value4",
		"var_A * Var_A + Var_B * Var_B",
		"not A < 5 = B > 2",
		"not A < 5 and B > 2 or C < 1 and D <> 2",
		"A * B / C \\ D MOD E",  // 2010-01-27: rename c to C
		"A mod B \\ C / D * E",  // 2010-01-27: rename c to C
		"A * B ^ 2",
		"a$ = \"this\" + \"test\"",
		NULL
	};
	static const char *testinput2[] = {  // parentheses expressions tests
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
	static const char *testinput3[] = {  // parentheses expressions tests
		"MID$(A$+B$,(C+5)*D,4)+\" Test\"",
		"int(Arr(A,B*(C+2))+(D))",
		"Array(INT(B(X, Y * (-I + J), FNZ(I))), VAL(NUM$))",
		// begin of error tests
		"INT(A+B",
		"B,F+G",
		"Arr(B,,C)",
		"MID$(,,C)",
		"MID$(A,,C)",
		"MID$(A$,,C)",
		"MID$(A$,B$,C)",
		"MID$(A$,B,,C)",
		"MID$(A$,B,C$)",
		"MID$(A$,B,C,)",
		NULL
	};
	static const char *testinput4[] = {  // internal function tests
		"MID$(MID$(A$+B$,2),(C+5)*D,4)",
		"abs(A)+FIX(B)+INT(C)+RND(D)+SGN(E)+CINT(F)",
		"SQR(G)+ATN(H)+COS(I)+SIN(J)+TAN(K)+EXP(L)+LOG(M)",
		"ASC(A$)+ASC(B$,C)+INSTR(A$,B$)+INSTR(A$,B$,C)+LEN(D$)",
		"LEFT$(D$,E)+RIGHT$(F$,G)+REPEAT$(H$,5)+SPACE$(15)",
		"A=VAL(STR$(1.23))",  // 2010-01-27: corrected expression
		"MID$(A$)",
		"MID$(A$,1,2,3)",
		"INSTR(A$)",
		"INSTR(A$,B$,C,D)",
		"ASC(A$,B,C)",
		"INT(1.23,A)",
		NULL
	};
	static const char *testinput5[] = {  // assignment tests
		"A=3",
		"A,B=3",
		"A=B=3",
		"A=(B)=3",
		"A=(B=3)",
		"A,B=(C=3)",
		"A,B=C=4",
		"A(B,C)=D",
		"A(B,C),E=D",
		"E,A(B,C)=D",
		"E,A(B,C),F=D",
		"A(B,(C))=D",
		"A(B+C)=D",
		"A(B=C)=D",
		"A(B+C,D=E)=F",
		"A(B,C)=D(E)",
		"A(B,C),D(E)=INT(F)+Function(G)+Array(H)",
		// begin of error tests
		"A=B,C=4",
		"A=B+C,4",
		"A(B+C,(D,E)=F",
		"A(B+C,(D=E)=(F,G)",
		"A,B+C",
		"3=A",
		"A,3,B=4",
		"(A=B)",
		"A,B,(C)=4",
		"A+B",
		NULL
	};
	static const char *testinput6[] = {  // data type tests (2010-04-28)
		"Z = not A < 5.0 = B > 2.0",
		"Z% = not A% < 5 and B% > 2 or C% < 1 and D% <> 2",
		"Z$=STR$(VAL(\"1.23\"))",
		"Z$=MID$(A$+B$,(C%+5)*D%,4)+\" Test\"",
		"Z% = (a$ = \"this\" + \"test\")",  // 2010-05-09: corrected expression
		"Z = A% + B",
		"Z = A + B%",
		"Z% = A% + B%",
		"Z = A% + B% + C",
		"Z = A% - B",
		"Z = A - B%",
		"Z% = A% - B%",
		"Z = A% - B% * C",
		"Z = ABS(A%) + SGN(B%) + ABS(C) * SGN(D)",
		"Z = ABS(-A%) + SGN(-B%) + ABS(-C) * SGN(-D)",
		"Z$ = STR$(A) + STR$(A%)",
		"Z = A% + B * C ^ D",
		// begin of error tests
		"Z = A$ + B",
		"Z = A$ + B%",
		"Z = A + B$",
		"Z = A% + B$",
		"Z$ = A$ - B$",
		"Z$ = MID$(A$,B$,1)",
		"Z$ = MID$(A$,2,C$)",
		"Z$ = MID$(A,2,3)",
		"Z$ = MID$(A$,2,3) + B",
		"Z = B + MID$(A$,2,3)",
		"Z = A + (B$ + C$)",
		NULL
	};
	static const char *testinput7[] = {  // data type assign tests (2010-05-09)
		"A% = B% + 5",
		"A% = B + 5",
		"A% = B + 5.0",
		"A = B = C = 1.0",
		"A, B, C = 1.0",
		"A, B, C = 1",
		"A%,B% = C% + D%",
		"A% = B% = C% + D%",
		"A$,B$,C$ = \"Test\"",
		"A$ = B$ = C$ = \"Test\"",
		// begin of error tests
		"Z, Y = A$ + B%",
		"Z, Y = A$ + B$",
		"A,B%,C = 1.0",
		"A,B,C% = 1.0",
		"A%,B,C = 1.0",
		"A,\"T\",B = 1",
		"A = \"T\" = B = 1",
		"A,B,C = STR$(1)",
		"A,B,C$ = STR$(1)",
		"A,B,C$ = 1",
		"A,B$,C = 1",
		"A,B,1 = 1",
		"A,B%,1 = 1",
		"1,A,B% = 1",
		"A%,B,C% = 1",
		"A%,B$,C% = 1",
		"A$,B$,C% = 1",
		"A$,B$,C$ = 1",
		"Z = A$ + B * C",
		NULL
	};
	static const char *testinput8[] = {  // substring tests (2010-05-19)
		"LEFT$(A$,1) = B$",
		"LEFT$(A$,1) = B$ + C$",
		"MID$(A$,2) = B$",
		"MID$(A$,2) = B$ + C$",
		"MID$(A$,2,3) = B$",
		"MID$(A$,2,3) = B$ + C$",
		"RIGHT$(A$,2) = B$",
		"RIGHT$(A$,2) = B$ + C$",
		"LEFT$(A$,1) = RIGHT$(B$,1)",
		"LEFT$(A$,1) = RIGHT$(B$+C$,1)",
		// begin of error tests
		"RIGHT$(A$,2) = B",
		"RIGHT$(A$,2) = B%",
		"RIGHT$(A,2) = B$",
		"RIGHT$(A%,2) = B$",
		"RIGHT$(A$+B$,2) = C$",
		"RIGHT$(LEFT$(A$,1),2) = B$",
		"RIGHT$(LEFT$(A$+B$,1),2) = C$",
		// list assignments (2010-05-22)
		"LEFT$(A$,1),RIGHT$(B$,1) = C$",
		"LEFT$(A$,1),MID$(B$,2,1),RIGHT$(C$,1) = D$",
		"LEFT$(A$,1),B$ = C$",
		"LEFT$(A$,1),B$,C$ = D$",
		"A$,LEFT$(B$,1) = C$",
		"A$,LEFT$(B$,1),C$ = D$",
		"A$,B$,C$ = D$",
		"A$,B$,LEFT$(C$,1) = D$",
		"LEFT$(A$,1),B$,RIGHT$(C$,1) = D$",
		// more related tests (2010-10-05)
		"A(int(B))=C",
		NULL
	};
	static const char *testinput9[] = {  // command tests (2010-05-29)
		// LET tests
		"LET A=0.0",
		"LET A%=0",
		"LET A$=\"\"",
		"LET A=B=C=0.0",
		"LET A,B,C=0.0",
		"LET A%=B%=C%=0",
		"LET A%,B%,C%=0",
		"LET A$=B$=C$=\"\"",
		"LET A$,B$,C$=\"\"",
		"LET LEFT$(A$,1) = B$",
		"LET LEFT$(A$,1) = B$ + C$",
		"LET LEFT$(A$,1),RIGHT$(B$,1) = C$",
		"LET A$,LEFT$(B$,1) = C$",
		// LET error tests
		"LET LET A=0",
		"A = 0 LET 5",
		"PRINT",
		NULL
	};
	static const char *testinput10[] = {  // PRINT tests (2010-06-01)
		"PRINT A;B%;C$",
		"PRINT A;B%;C$;",
		"PRINT A,B%,C$",
		"PRINT A,B%,C$,",
		"PRINT A+B; C%+D%; E$+F$; MID$(E$,1); LEFT$(E$+F$,5)",
		"PRINT \"Test:\"; TAB(10); A%; SPC(5); B",
		"PRINT \"Test:\"; TAB(10); A; LOG(5); B",
		"PRINT A; TAB(10)",
		"PRINT A; TAB(10);",
		"PRINT A; SPC(10)",
		"PRINT A; SPC(10);",
		"PRINT ,,A",
		"PRINT ,,A,",
		"PRINT A,,B",
		"PRINT A,,B,",
		"PRINT A;;B",
		"PRINT;A;;B;;",
		"PRINT;",
		"PRINT,",
		// PRINT error tests
		"PRINT C(A,,B)",
		"PRINT C(A;B)",
		"A=SPC(10)",
		"A=TAB(10)",
		"PRINT (A,,B)",
		"PRINT A+;",
		"PRINT A+,",
		"PRINT A+",
		"PRINT INT(A+;",
		"PRINT INT(A+,",
		"PRINT INT(A+",
		"PRINT INT(B,,C)",
		"PRINT (A+B",
		"PRINT (A+B,",
		"PRINT MID$(A$+B$",
		// 2011-03-03: more PRINT error tests
		"PRINT (TAB(10)",
		"PRINT INT(TAB(10))",
		"PRINT VAL(TAB(10))",
		"PRINT A(TAB(10))",
		"PRINT A+TAB(10)",
		"PRINT A%+TAB(10)",
		"PRINT A$+TAB(10)",
		"PRINT TAB(10)+A",
		NULL
	};
	static const char *testinput11[] = {  // error tests (2010-06-13)
		"Z = C(A,,B)",
		"Z = INT(A,",
		"Z = (A+",
		"Z = A+",
		"Z = (A+B",
		"Z = (,",
		"Z = (A,",
		"Z = (A+,",
		"Z = (A+A+,",
		"Z = (A+B,",
		"Z = MID$(A$+B$",
		"Z = A NOT B",
		"Z = A + * B",
		"Z = A + B,",
		"Z = A,B = 1",
		"Z+A",
		"+Z",
		"-Z",
		"Z Y = 1",
		"Z , A B",
		"Z",
		",Z",
		"Z,",
		"Z,,",
		"Z,Y",
		"Z,Y,",
		"Z,Y,X",
		"Z,Y,X,",
		"Z=A,",
		"LET A",
		"LET A,",
		"Z=",
		"Z=,",
		"Z=+",
		"Z=Y=",
		"Z,Y=",
		"Z,Y=,",
		"LET A=",
		"Z(",
		"Z(,",
		"Z,Y(",
		"Z,Y(A",
		"Z,Y(A,",
		"Z,Y(A+",
		"Z = A A",
		"Z = (A A",
		"Z = Arr(A A",
		"Z = INT(A A",
		"Z = ASC(A$ A",
		"Z = ASC(A$,A A",
		"Z$ = MID$(A$ A",
		"Z$ = MID$(A$,A A",
		"Z$ = MID$(A$,A,A A",
		"MID$(",
		"MID$(A$",
		"MID$(A$,",
		"MID$(A$ B",
		"MID$(A$,B",
		"MID$(A$,B,",
		"MID$(A$,B C",
		"MID$(A$,B,C",
		"MID$(A$,B,C,",
		"MID$(A$,B,C D",
		"MID$(A$,B,C)",
		"MID$(A$,B,C)=",
		"INT(",
		"INT(A,",
		"INT(A)",
		"INT(A)=",
		"LET INT(A)=",
		"A,INT(A)=",
		"A=INT(A)=",
		"INT(A)=B",
		"LET ,",
		"LET A,,",
		"LET",
		"Z%,",
		"Z%,,",
		"Z%,Y%,",
		"Z%,Y%,X%,",
		"LET A%,",
		"LET A%,,",
		"Z$,",
		"Z$,,",
		"Z$,Y$,",
		"Z$,Y$,X,",
		"LET A$,",
		"LET A$,,",
		"Z%=",
		"Z%=,",
		"Z%=+",
		"Z%=Y%=",
		"Z%,Y%=",
		"Z%,Y%=,",
		"LET A%=",
		"Z$=",
		"Z$=,",
		"Z$=+",
		"Z$=Y$=",
		"Z$,Y$=",
		"Z$,Y$=,",
		"LET A$=",
		"MID$(A$+",
		"MID$(A$+B$",
		"MID$(A$+B$,",
		"A(INT(B+C))=4",
		"MID$(A$,LEFT$(B$,1)<C$)=D$",
		NULL
	};
	static const char *testinput12[] = {  // more error tests (2010-06-29)
		"Z = A$ + B$ + C$",
		"Z = MID$(A*B+C,1)",
		"Z$ = MID$(A*B+C,1)",
		"Z$ = MID$(Y$,A$ + B$ + C$)",
		"Z = A <> B$",
		"Z = A <> B$ < C$",
		"Z = A + B$ + C$",
		"Z = A < B$ + C$",
		"Z = A + B$ + C%",
		"Z = A < B$ + C%",
		"Z = A <> B$ + C$ < D$",
		"Z = A + B$ + C$ < D$",
		"Z = A + (B$ + C$ < D$)",
		"Z$ = A$ + B$ + C%",
		"Z$ = A$ + B * C",
		"A$,RIGHT$(B,2) = C$",
		"A$=RIGHT$(B,2) = C$",
		"Z=MID$(A$,2) < \"B\"",
		"Z=MID$(A$,2) = B$",
		"Z(INT(A+B))=C",
		"Z(A$)=C",
		"Z(A)=Y(A$)",
		// added more array assignments, non-errors (2011-02-01)
		"Z(A),Y(A)=X(A)",
		"Z(A,Y(B))=X(A)",
		"Z%(A%),Y%(A%)=X%(A%)",
		"Z$(A%),Y$(A%)=X$(A%)",
		// added more command error tests (2011-03-10)
		"TAB(10)=A",
		"+",
		"NOT",
		"A PRINT B",
		"A(I) PRINT B",
		"MID$(A$ PRINT,4)=\"\"",
		"LET (",
		"A,PRINT",
		"A = (0 LET 5)",
		"A = 0 PRINT 5",
		"A = (0 PRINT 5)",
		"A$ = MID$(B$ PRINT, 4)",
		"A = INT(0 LET 5)",
		"A = INT(0 PRINT 5)",
		"A$ = MID$(B$, 4 PRINT)",
		"A$ = MID$(B$, 4, 5 PRINT)",
		"MID$((A$),4)=B$",
		"MID$(-A$,4)=B$",
		"MID$(+A$,4)=B$",
		"FNA=B",
		"FNA(I)=B",
		"LET FNA=B",
		"LET FNA(I)=B",
		"C,FNA=B",
		"C,FNA(I)=B",
		"LET C,FNA=B",
		"LET C,FNA(I)=B",
		"C,FNA,D=B",
		"C,FNA(I),D=B",
		"LET C,FNA,D=B",
		"LET C,FNA(I),D=B",
		NULL
	};
	static const char *testinput13[] = {  // semicolon error tests (2010-07-04)
		"Z = C(A;B)",
		"Z = A;B",
		"Z = A+;",
		"Z = A+B;C",
		"Z = Y = A;B",
		"Z , Y = A;B",
		"Z , Y , A;B",
		"Z = INT(A;B)",
		"Z = ASC(A$;B)",
		"Z$ = MID$(A$;B)",
		"Z$ = MID$(A$,B;C)",
		"Z$ = MID$(A$,B,C;D)",
		"Z = INSTR(A$;B$)",
		"Z = INSTR(A$,B$;C)",
		"Z = INSTR(A$,B$,C;D)",
		"Z = C(A,;B)",
		"Z;",
		";Z",
		"Z,;",
		"Z,B;",
		"Z=B;",
		"LET A;",
		"Z=;",
		"Z,Y=;",
		"Z=Y=;",
		"Z=Y=(;",
		"Z=Y=(A;",
		"Z(;",
		"MID$(,",
		"MID$(;",
		"MID$(A$;",
		"MID$(A$,;",
		"MID$(A$,B;",
		"MID$(A$,B);",
		"Z$,MID$(,",
		"Z$,MID$(;",
		"Z$,MID$(A$;",
		"Z$,MID$(A$,;",
		"Z$,MID$(A$,B;",
		"Z$,MID$(A$,B);",
		"Z$=MID$(,",
		"Z$=MID$(;",
		"Z$=MID$(A$;",
		"Z$=MID$(A$,;",
		"Z$=MID$(A$,B;",
		"Z$=MID$(A$,B);",
		"Z(A;",
		"LET ;",
		"Z%,;",
		"LET A%,;",
		"Z$,;",
		"LET A$,;",
		"Z%=;",
		"Z%,Y%=;",
		"Z$=;",
		"Z$,Y$=;",
		NULL
	};
	static const char *testinput14[] = {  // expression error tests (2011-01-04)
		//
		"Z=(,",
		"Z=-,",
		"Z=A+,",
		"Z=(-,",
		"Z=(A+,",
		"Z=INT(,",
		"Z=INT(-,",
		"Z=INT(A+,",
		"Z=INT((,",
		"Z=INT((-,",
		"Z=INT((A+,",
		"Z=A(,",
		"Z=A(-,",
		"Z=A(B+,",
		"Z=A((,",
		"Z=A((-,",
		"Z=A((B+,",
		"Z=A(INT(,",
		"Z=A(INT(-,",
		"Z=A(INT(A+,",
		"Z=A(B(,",
		"Z=A(B(-,",
		"Z=A(B(C+,",
		"Z = -",
		"Z% = A%+",
		"Z$ = A$+",
		"Z = A(B+",
		"Z$ = A$ + -B",			// additional tests (2011-01-22)
		"Z$ = A$ + -(B+C)",
		"Z$ = A$ + INT(B+C)",
		"Z$ = A$ + Array(B+C)",
		NULL
	};
	static const char *testinput15[] = {  // temporary string tests (2011-02-05)
		"Z$ = A$ + B$",
		"Z$ = REPEAT$(A$,4) + B$",
		"Z$ = A$ + REPEAT$(B$+C$,3)",
		"Z$ = STR$(A) + LEFT$(B$+C$,5)",
		"Z% = A$ = B$",
		"Z% = A$ + B$ = C$",
		"Z% = A$ = B$ + C$",
		"Z% = A$ + B$ = C$ + D$",
		"Z% = A$ < B$",
		"Z% = A$ + B$ < C$",
		"Z% = A$ < B$ + C$",
		"Z% = A$ + B$ < C$ + D$",
		"Z% = A$ <= B$",
		"Z% = A$ + B$ <= C$",
		"Z% = A$ <= B$ + C$",
		"Z% = A$ + B$ <= C$ + D$",
		"Z% = A$ > B$",
		"Z% = A$ + B$ > C$",
		"Z% = A$ > B$ + C$",
		"Z% = A$ + B$ > C$ + D$",
		"Z% = A$ <= B$",
		"Z% = A$ + B$ <= C$",
		"Z% = A$ <= B$ + C$",
		"Z% = A$ + B$ <= C$ + D$",
		"Z% = A$ <> B$",
		"Z% = A$ + B$ <> C$",
		"Z% = A$ <> B$ + C$",
		"Z% = A$ + B$ <> C$ + D$",
		"Z$ = A$ + B$",
		"Z$,Y$ = A$ + B$",
		"LEFT$(Z$,1) = A$ + B$",
		"LEFT$(Z$,1),Y$,RIGHT$(X$,1) = A$ + B$",
		"Z% = ASC(A$) + ASC(B$+C$)",
		"Z% = ASC(A$,4) + ASC(B$+C$,4)",
		"Z% = INSTR(A$,B$)",
		"Z% = INSTR(A$+B$,C$)",
		"Z% = INSTR(A$,B$+C$)",
		"Z% = INSTR(A$+B$,C$+D$)",
		"Z% = INSTR(A$,B$,4)",
		"Z% = INSTR(A$+B$,C$,4)",
		"Z% = INSTR(A$,B$+C$,4)",
		"Z% = INSTR(A$+B$,C$+D$,4)",
		"Z% = LEN(A$) + LEN(B$+C$)",
		"Z = VAL(A$) + VAL(B$+C$)",
		"PRINT A$;B$+C$",
		NULL
	};
	static const char *testinput16[] = {  // INPUT tests (2011-03-20)
		"INPUT A",
		"INPUT A,B",
		"INPUT A,B%,C$",
		"INPUT A$;",
		"INPUT A(I%),B%(I%);",
		"INPUT PROMPT \"Enter Number: \"; N%",
		"INPUT PROMPT A$+\":\"; A",
		"INPUT PROMPT \"Two Values\", A, B;",
		"INPUT PROMPT P$(I%), S$(I%)",
		// error tests
		"INPUT",
		"INPUT,",
		"INPUT;",
		"INPUT; -A",
		"INPUT (A)",
		"INPUT INT(A)",
		"INPUT LEFT$(A$,1)",
		"INPUT FNA",
		"INPUT FNA(X)",
		"INPUT A B",
		"INPUT A+B",
		"INPUT A,",
		"INPUT PROMPT",
		"INPUT PROMPT A+B*C",
		"INPUT PROMPT A$",
		"INPUT PROMPT A$+",
		"INPUT PROMPT A$;",
		NULL
	};
	static const char *testinput17[] = {  // negative const tests (2011-03-27)
		"A=---B",
		"A=-1.0",
		"A%=--1",
		"A=B-1.5",
		"A=B+-1.5",
		"A=B+--1.5",
		"PRINT-1;--1;---1;- 1;",
		"A=- 1.0 + -.001 + -0.12",
		"A%=-1 + - 2",
		"A=-1.2E-23 + -1.2e+23 + -1.2e23",
		"A=- 1.2E-23 + - 1.2e+23 + - 1.2e23",
		"A% = NOT -B%",
		"A% = NOT -2147483648",  // negative integer limit
		"A% = - NOT 123",
		"A% = -2147483649 ",	// integer limit test
		"A = B^-C + B*-C",
		"A = -B^C + -B^-C",
		"A = -B*C + -B*-C",
		"A = (-B^NOT C) + -B*NOT C",  // TODO check with recreator
		"A = -B^NOT C + -B*NOT C",
		NULL
	};

	static const char **test[] = {
		testinput1, testinput2, testinput3, testinput4, testinput5, testinput6,
		testinput7, testinput8, testinput9, testinput10, testinput11,
		testinput12, testinput13, testinput14, testinput15, testinput16,
		testinput17
	};
	static const int ntests = sizeof(test) / sizeof(test[0]);

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
		// 2010-04-02: added + 1 so that '\' is no output
		printf("usage: %s -t <test number 1-%d>|i\n", strrchr(argv[0], '\\')
			+ 1, ntests);
		return true;  // our options are bad
	}

	if (inputmode)
	{
		char inputline[200];
		printf("\nTranslator Testing...");  // 2010-05-29: added
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
			translate_input(translator, parser, table, testinput[i],
				testno < 4);
			print_token_leaks(table, testinput[i]);
		}
	}
	printf("\n");
	return true;
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
		// 2010-05-29: updated for changes made to data type enumeration
		"Double",
		"Integer",
		"String",
		"TmpStr",
		"SubStr",
		"numberof",
		"None",
		"CmdArgs"
	};
	// 2010-04-04: updated list for new codes
	// 2010-04-11: updated list for new codes
	// 2010-05-29: updated list for new codes
	// 2010-06-02: updated list for new codes
	const char *code_name[] = {
		// replaced strings with automatically generated include file
		// (use "awk -f test_codes.awk <ibcp.h >test_codes.h" to create)
		#include "test_codes.h"
	};

	CmdArgs *args;

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
			printf(" %g |%.*s|", token->dbl_value,
				token->string->get_len(), token->string->get_ptr());
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
