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

#include <stdio.h>
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

void Token::initialize(void)
{
	// set true for types that contain an opening parentheses
	paren[IntFuncP_TokenType] = true;
	paren[DefFuncP_TokenType] = true;
	paren[Paren_TokenType] = true;

	// set true for types that are considered an operator
	op[Command_TokenType] = true;
	op[Operator_TokenType] = true;
}


bool test_parser(Parser &parser, Table *table, int argc, char *argv[]);


int main(int argc, char *argv[])
{
	print_gpl_header(strrchr(argv[0], '\\') + 1);
	
	Token::initialize();
	Table *table;
	try
	{
		table = new Table();
	}
	catch (List<TableError> *error_list)
	{
		TableError error;

		fprintf(stderr, "Error(s) found in Table:\n");
		int n = 0;
		bool more;
		do
		{
			more = error_list->remove(NULL, &error);
			fprintf(stderr, "Error #%d: ", ++n);
			switch (error.type)
			{
			case Duplicate_TableErrType:
				fprintf(stderr, "Code %d in table more than once "
					"at entries %d and %d\n", error.duplicate.code,
					error.duplicate.ifirst, error.duplicate.idup);
				break;
			case Missing_TableErrType:
				fprintf(stderr, "Code %d missing from table\n",
					error.missing.code);
				break;
			case Range_TableErrType:
				fprintf(stderr, "Search type %d indexes (%d, %d) not correct\n",
					error.range.type, error.range.ibeg, error.range.iend);
				break;
			case Overlap_TableErrType:
				fprintf(stderr, "Search type %d indexes (%d, %d) overlap with "
					"search type %d\n", error.overlap.type1, error.overlap.ibeg,
					error.overlap.iend, error.overlap.type2);
				break;
			default:
				fprintf(stderr, "Unknown error %d\n", error.type);
				break;
			}
		}
		while (more);
		fprintf(stderr, "Program aborting!\n");
		return 1;
	}
	printf("Table initialization successful.\n");
		
	Parser parser(table);

	if (!test_parser(parser, table, argc, argv))
	{
		printf("usage: %s -p <options>\n", strrchr(argv[0], '\\') + 1);
	}
	return 0;
}
	

// end: ibcp.cpp
