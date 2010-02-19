// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_cons.cpp - contains code for testing constructor exceptions
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
//  2010-01-27  initial release
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void print_gpl_header(void)
{
	printf("test_cons.exe  Copyright (C) 2010  Thunder422\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
    printf("This is free software, and you are welcome to\n");
    printf("redistribute it under certain conditions.\n\n");
}


class Test {
	int a;
public:
	Test(void)
	{
		a = 0;
	}
	Test(int b)
	{
		switch (b)
		{
		case 1:
			throw 10;
			break;
		case 2:
			a = b;
			break;
		case 3:
			throw 20;
			break;
		case 4:
			a = b * b;
			break;
		}
	}
	int get_a(void)
	{
		return a;
	}
};


class Test2 {
	int a;
public:
	Test2(int b)
	{
		switch (b)
		{
		case 1:
			throw 10;
			break;
		case 2:
			a = b;
			break;
		case 3:
			throw 20;
			break;
		case 4:
			a = b * b;
			break;
		}
	}
	int get_a(void)
	{
		return a;
	}
};



int main(void)
{
	print_gpl_header();

	Test test1;
	printf("Test 1 (each value)...\n");
	for (int i = 1; i <= 4; i++)
	{
		bool fail = false;
		try
		{
			test1 = Test(i);
		}
		catch (int n)
		{
			printf("Failed: %d\n", n);
			fail = true;
		}
		if (!fail)
		{
			printf("Good: %d\n", test1.get_a());
		}
	}

	printf("\nTest 2 (should succeed)...\n");
	Test2 *test2;
	try
	{
		test2 = new Test2(2);
	}
	catch (int n)
	{
		printf("Failed: %d\n", n);
		exit(1);
	}
	printf("Good: %d\n", test2->get_a());

	printf("\nTest 3 (should fail)...\n");
	Test2 *test3;
	try
	{
		test3 = new Test2(3);
	}
	catch (int n)
	{
		printf("Failed: %d\n", n);
		exit(1);
	}
	printf("Good: %d\n", test3->get_a());
}
