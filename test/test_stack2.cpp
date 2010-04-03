// vim:ts=4:sw=4

//	Interactive BASIC Compiler Project
//	File: test_stack.cpp - contains test code for testing list class
//	Copyright (C) 2010-2010  Thunder422
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
//  1.0  2010-04-01  initial release

#include <stdio.h>
#include "../stack.h"


void print_gpl_header(void)
{
	printf("test_stack.exe  Copyright (C) 2009  Thunder422\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
    printf("This is free software, and you are welcome to\n");
    printf("redistribute it under certain conditions.\n\n");
}


int main(void)
{
	print_gpl_header();

	SimpleStack<int> count_stack;

	count_stack.push(1);
	printf("%d\n", count_stack.top());
	count_stack.top()++;
	count_stack.top()++;
	printf("%d\n", count_stack.top());
	count_stack.top()++;
	printf("%d\n\n", count_stack.pop());
	for (int i = 0; i < 15; i++)
	{
		count_stack.push(i + 100);
	}
	while (!count_stack.empty())
	{
		printf("%d\n", count_stack.pop());
	}
}
