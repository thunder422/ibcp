// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_string.cpp - contains test code for testing string class
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
//	2010-01-15	initial release
//
//	2010-04-20	fixed include so that program will compile in test directory
//
//	2011-02-06	changed get_str() to get_ptr()
//
//	2012-10-16	removed output of string pointer so that results can be compared

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../string.h"

void print_gpl_header(void)
{
	printf("test_sting.exe  Copyright (C) 2010  Thunder422\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
    printf("This is free software, and you are welcome to\n");
    printf("redistribute it under certain conditions.\n\n");
}

void print_string(const char *name, String *s)
{
	int len = s->get_len();
	char *str = s->get_ptr();
	if (len == 0)
	{
		printf("%s = NULL (0)\n", name);
	}
	else
	{
		printf("%s = '%.*s' (%d)\n", name, len, str, len);
	}
}

void print_strings(String *s1, String *s2, int result)
{
	printf("'%.*s' vs '%.*s' = %d\n", s1->get_len(), s1->get_ptr(),
		s2->get_len(), s2->get_ptr(), result);
}

void print_strings(String *s1, const char *s2, int result)
{
	printf("'%.*s' vs '%s' = %d\n", s1->get_len(), s1->get_ptr(), s2, result);
}

int main(void)
{
	print_gpl_header();

	printf("Testing constructors...\n");
	String *string1 = new String;
	print_string("blank", string1);

	String *string2 = new String("abcde", 5);
	print_string("'abcde',5", string2);

	String *string3 = new String("fghij", 3);
	print_string("'fghij',3", string3);

	const char *s = "ABCDEFG";
	const char *p = s + 4;
	String *string4 = new String(s, p);
	print_string("'ABCDEFG', p='EFG'", string4);

	printf("\nTesting functions...\n");
	string1->copy(string2);
	print_string("copy 'abcdef' to blank", string1);

	string1->copy(string3);
	print_string("copy 'fgh' to 'abcde'", string1);

	string1->move(string4);
	print_string("move 'ABCD' to 'fgh'", string1);
	print_string("move from 'fgh'", string4);
	delete string4;

	string1->cat(string2);
	print_string("'ABCD' + 'abcde'", string1);
	
	string4 = new String(4);
	string4->set(0, '1');
	string4->set(1, '2');
	string4->set(2, '3');
	string4->set(3, '4');
	print_string("4, set = '1234'", string4);

	delete string2;
	string2 = String_cat(string1, string4);
	print_string("'ABCDabcde' + '1234'", string2);
	delete string1;

	printf("\nTesting getref/setref...\n");
	string1 = string2->getref(5, 3);
	print_string("mid$('ABCDabcde1234', 5, 3)", string1);
	string1->setref(string3);
	print_string("set to 'fgh'", string2);
	string1->setref(string4);
	print_string("set to '1234'", string2);
	delete string3;
	string3 = new String("z", 1);
	string1->setref(string3);
	print_string("set to 'z'", string2);
	string1->reset();
	delete string1;
	delete string2;
	delete string3;
	delete string4;

	string1 = new String("print", 5);
	string2 = new String("PRINT", 5);
	string3 = new String("print1", 6);
	string4 = new String("INPUT", 5);
	String *string5 = new String("print", 5);
	printf("\nTesting equalcase(String)...\n");
	print_strings(string1, string2, string1->equalcase(string2));
	print_strings(string1, string3, string1->equalcase(string3));
	print_strings(string1, string4, string1->equalcase(string4));
	print_strings(string1, string5, string1->equalcase(string5));

	printf("\nTesting equalcase(char *)...\n");
	print_strings(string1, "PRINT", string1->equalcase("PRINT"));
	print_strings(string1, "print", string1->equalcase("print"));
	print_strings(string1, "INPUT", string1->equalcase("INPUT"));
	print_strings(string3, "PRINT", string3->equalcase("PRINT"));

	printf("\nTesting equal(String)...\n");
	print_strings(string1, string2, string1->equal(string2));
	print_strings(string1, string3, string1->equal(string3));
	print_strings(string1, string4, string1->equal(string4));
	print_strings(string1, string5, string1->equal(string5));

	printf("\nTesting compare(String)...\n");
	print_strings(string1, string2, string1->compare(string2));
	print_strings(string1, string3, string1->compare(string3));
	print_strings(string1, string4, string1->compare(string4));
	print_strings(string1, string5, string1->compare(string5));
	print_strings(string2, string1, string2->compare(string1));
	print_strings(string2, string3, string2->compare(string3));
	print_strings(string2, string4, string2->compare(string4));
	print_strings(string2, string5, string2->compare(string5));
	print_strings(string3, string1, string3->compare(string1));
	print_strings(string3, string2, string3->compare(string2));
	print_strings(string3, string4, string3->compare(string4));
	print_strings(string3, string5, string3->compare(string5));
	print_strings(string4, string1, string4->compare(string1));
	print_strings(string4, string2, string4->compare(string2));
	print_strings(string4, string3, string4->compare(string3));
	print_strings(string4, string5, string4->compare(string5));
	print_strings(string5, string1, string5->compare(string1));
	print_strings(string5, string2, string5->compare(string2));
	print_strings(string5, string3, string5->compare(string3));
	print_strings(string5, string4, string5->compare(string4));
}
