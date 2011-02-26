// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_operators.cpp - contains code for operators at runtime
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
//	Change History:
//
//	2010-07-17	initial release
//

#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

void print_gpl_header(void)
{
	printf("test_operators.exe  Copyright (C) 2010  Thunder422\n");
	printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	printf("This is free software, and you are welcome to\n");
	printf("redistribute it under certain conditions.\n\n");
}


union EvalItem {
	double dblval;
	int intval;
};



void add(Stack<EvalItem, 1> &eval_stack)
{
	double &operand2 = eval_stack.pop().dblval;
	eval_stack.top().dblval += operand2;
}


void add_i1(Stack<EvalItem, 1> &eval_stack)
{
	double &operand2 = eval_stack.pop().dblval;
	eval_stack.top().dblval = eval_stack.top().intval + operand2;
}


void add_i2(Stack<EvalItem, 1> &eval_stack)
{
	int &operand2 = eval_stack.pop().intval;
	eval_stack.top().dblval += operand2;
}


void add_int(Stack<EvalItem, 1> &eval_stack)
{
	int &operand2 = eval_stack.pop().intval;
	eval_stack.top().intval += operand2;
}


int main(void)
{
	Stack<EvalItem, 1> eval_stack;

	print_gpl_header();

	eval_stack.push().dblval = 1.1;
	eval_stack.push().dblval = 2.2;
	add(eval_stack);
	printf("1.1 + 2.2 = %g\n", eval_stack.pop().dblval);

	eval_stack.push().intval = 1;
	eval_stack.push().dblval = 2.2;
	add_i1(eval_stack);
	printf("1 + 2.2 = %g\n", eval_stack.pop().dblval);

	eval_stack.push().dblval = 1.1;
	eval_stack.push().intval = 2;
	add_i2(eval_stack);
	printf("1.1 + 2 = %g\n", eval_stack.pop().dblval);

	eval_stack.push().intval = 1;
	eval_stack.push().intval = 2;
	add_int(eval_stack);
	printf("1 + 2 = %d\n", eval_stack.pop().intval);

	eval_stack.push().dblval = 3.3;
	eval_stack.push().dblval = 4.4;
	add(eval_stack);
	printf("3.3 + 4.4 = %g\n", eval_stack.pop().dblval);

	eval_stack.push().intval = 3;
	eval_stack.push().dblval = 4.4;
	add_i1(eval_stack);
	printf("3 + 4.4 = %g\n", eval_stack.pop().dblval);

	eval_stack.push().dblval = 3.3;
	eval_stack.push().intval = 4;
	add_i2(eval_stack);
	printf("3.3 + 4 = %g\n", eval_stack.pop().dblval);

	eval_stack.push().intval = 3;
	eval_stack.push().intval = 4;
	add_int(eval_stack);
	printf("3 + 4 = %d\n", eval_stack.pop().intval);
}
