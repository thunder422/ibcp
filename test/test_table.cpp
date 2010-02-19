// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_table.cpp - contains code for testing the table class
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
//  2010-02-18  initial release
//

#include <stdio.h>
#include <stdlib.h>
#include "../ibcp.h"

void print_gpl_header(void)
{
	printf("test_table.exe  Copyright (C) 2010  Thunder422\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
    printf("This is free software, and you are welcome to\n");
    printf("redistribute it under certain conditions.\n\n");
}


int main(void)
{
	print_gpl_header();

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
		exit(1);
	}
	printf("Table initialization successful.\n");
	
	int index;
	printf("\nTesting search command functionality:\n");
	index = table->search('L', Null_Flag);
	printf("\t'L', Null (finds LIST): %d\n", index);
	index = table->search('L', Line_Flag);
	printf("\t'L', Line (finds LIST): %d\n", index);
	index = table->search('l', String_Flag);
	printf("\t'l', String (finds LOAD): %d\n", index);
	index = table->search('L', RangeIncr_Flag);
	printf("\t'L', RangeIncr (invalid arguments): %d\n", index);
	index = table->search('r', RangeIncr_Flag);
	printf("\t'r', RangeIncr (finds RENUM): %d\n", index);
	index = table->search('q', Blank_Flag);
	printf("\t'q', Blank (finds QUIT): %d\n", index);
	index = table->search('k', Null_Flag);
	printf("\t'k', Blank (invalid command): %d\n", index);

	printf("\nTesting search plain word functionality:\n");
	char *word = new char[10];
	strcpy(word, "let");
	index = table->search(PlainWord_TableSearch, word, strlen(word));
	printf("\t'%s', (first plain word): %d\n", word, index);
	strcpy(word, "InPut");
	index = table->search(PlainWord_TableSearch, word, strlen(word));
	printf("\t'%s', (middle plain word mix case): %d\n", word, index);
	strcpy(word, "inputa");
	index = table->search(PlainWord_TableSearch, word, strlen(word));
	printf("\t'%s', (too many characters): %d\n", word, index);
	strcpy(word, "inpu");
	index = table->search(PlainWord_TableSearch, word, strlen(word));
	printf("\t'%s', (too few characters): %d\n", word, index);
	strcpy(word, "XOR");
	index = table->search(PlainWord_TableSearch, word, strlen(word));
	printf("\t'%s', (last plain word): %d\n", word, index);
	strcpy(word, "SIN(");
	index = table->search(PlainWord_TableSearch, word, strlen(word));
	printf("\t'%s', (not plain word): %d\n", word, index);

	printf("\nTesting search parentheses word functionality:\n");
	strcpy(word, "ABS(");
	index = table->search(ParenWord_TableSearch, word, strlen(word));
	printf("\t'%s', (first paren word): %d\n", word, index);
	strcpy(word, "sin(");
	index = table->search(ParenWord_TableSearch, word, strlen(word));
	printf("\t'%s', (middle paren word): %d\n", word, index);
	strcpy(word, "sin$(");
	index = table->search(PlainWord_TableSearch, word, strlen(word));
	printf("\t'%s', (bad paren word): %d\n", word, index);
	strcpy(word, "sin");
	index = table->search(PlainWord_TableSearch, word, strlen(word));
	printf("\t'%s', (bad paren word): %d\n", word, index);
	strcpy(word, "Mid$(");
	index = table->search(ParenWord_TableSearch, word, strlen(word));
	printf("\t'%s', (paren word with data type): %d\n", word, index);
	strcpy(word, "val(");
	index = table->search(ParenWord_TableSearch, word, strlen(word));
	printf("\t'%s', (last plain word): %d\n", word, index);
	strcpy(word, "input");
	index = table->search(ParenWord_TableSearch, word, strlen(word));
	printf("\t'%s', (not paren word): %d\n", word, index);

	printf("\nTesting search data type word functionality:\n");
	strcpy(word, "sin(");
	index = table->search(DataTypeWord_TableSearch, word, strlen(word));
	printf("\t'%s', (bad data type): %d\n", word, index);
	strcpy(word, "date$");
	index = table->search(DataTypeWord_TableSearch, word, strlen(word));
	printf("\t'%s', (bad data type): %d\n", word, index);
	strcpy(word, "input");
	index = table->search(DataTypeWord_TableSearch, word, strlen(word));
	printf("\t'%s', (bad data type): %d\n", word, index);

	printf("\nTesting search symbol functionality:\n");
	strcpy(word, "+");
	index = table->search(Symbol_TableSearch, word, 1);
	printf("\t'%s', (first symbol): %d\n", word, index);
	strcpy(word, "'");
	index = table->search(Symbol_TableSearch, word, 1);
	printf("\t'%s', (last symbol): %d\n", word, index);
	strcpy(word, "<");
	index = table->search(Symbol_TableSearch, word, 1);
	printf("\t'%s', (one character symbol): %d\n", word, index);
	strcpy(word, "<=");
	index = table->search(Symbol_TableSearch, word, 1);
	printf("\t'%s', (one character symbol): %d\n", word, index);
	strcpy(word, "<=");
	index = table->search(Symbol_TableSearch, word, 2);
	printf("\t'%s', (two character symbol): %d\n", word, index);
	strcpy(word, "&");
	index = table->search(Symbol_TableSearch, word, 1);
	printf("\t'%s', (bad symbol): %d\n", word, index);
	strcpy(word, "sin(");
	index = table->search(Symbol_TableSearch, word, strlen(word));
	printf("\t'%s', (bad symbol): %d\n", word, index);
}
