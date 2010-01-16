// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_stack.cpp - contains test code for testing list class
//	Copyright (C) 2009  Thunder422
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
//  2009-12-28  initial release
//  2010-01-15  added 'const' to op_str[]
//

#include <stdio.h>
#include "list.h"

void print_gpl_header(void)
{
	printf("test_stack.exe  Copyright (C) 2009  Thunder422\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
    printf("This is free software, and you are welcome to\n");
    printf("redistribute it under certain conditions.\n\n");
}

void print_stack(const char *str, List<int> &IntStack)
{
	List<int>::Element *element;

	printf("%s: ", str);
	for (IntStack.first(element); IntStack.not_end(element);
		IntStack.next(element))
	{
		printf("%d ", element->value);
	}
	printf("EOS\n");
}

void test_int_stack(void)
{
	List<int> IntStack;

	print_stack("Begin", IntStack);

	IntStack.push(1);
	IntStack.push(2);
	print_stack("1&2", IntStack);

	IntStack.push(8);
	print_stack("1&2&8", IntStack);

	int a = IntStack.pop();
	int b = IntStack.pop();
	printf("a=%d b=%d\n", a, b);
	IntStack.push(a+b);
	print_stack("1&(2+8)", IntStack);

	IntStack.push(3);
	print_stack("1&(2+8)&3", IntStack);

	a = IntStack.pop();
	b = IntStack.pop();
	printf("a=%d b=%d\n", a, b);
	IntStack.push(a+b);
	print_stack("1&((2+8)+3)", IntStack);

	IntStack.push(4);
	print_stack("1&((2+8)+3)&4", IntStack);

	while (!IntStack.empty())
	{
		printf(" %d", IntStack.pop());
	}
	printf("\n");
	print_stack("End", IntStack);
}

struct Item {
	int num;
	const char *str;
};

void print_stack(const char *str, List<Item> &ItemStack)
{
	List<Item>::Element *element;

	printf("%s: ", str);
	for (ItemStack.first(element); ItemStack.not_end(element);
		ItemStack.next(element))
	{
		printf("<%d,%s> ", element->value.num, element->value.str);
	}
	printf("EOS\n");
}

void test_item_stack(void)
{
	Item item;
	List<Item> ItemStack;

	print_stack("Begin", ItemStack);

	item.num = 1;
	item.str = "PRINT";
	ItemStack.push(&item);
	item.num = 7;
	item.str = "A";
	ItemStack.push(&item);
	item.num = 8;
	item.str = ";";
	ItemStack.push(&item);
	print_stack("PRINT A;", ItemStack);

	item = ItemStack.pop();
	printf("item=<%d,%s>\n", item.num, item.str);
	print_stack("PRINT A", ItemStack);

	while (ItemStack.pop(&item))
	{
		printf(" <%d,%s>", item.num, item.str);
	}
	printf("\n");
	print_stack("End", ItemStack);
}

enum Operator {ADD, SUB, MUL, DIV};
const char *op_str[] = {"+", "-", "*", "/"};

void print_stack(const char *str, List<Operator> &OpStack)
{
	List<Operator>::Element *element;

	printf("%s: ", str);
	for (OpStack.first(element); OpStack.not_end(element);
		OpStack.next(element))
	{
		printf("%s ", op_str[element->value]);
	}
	printf("EOS\n");
}

void test_enum_stack(void)
{
	List<Operator> OpStack;

	print_stack("Begin", OpStack);

	OpStack.push(ADD);
	OpStack.push(SUB);
	print_stack("ADD&SUB", OpStack);

	OpStack.push(MUL);
	print_stack("ADD&SUB&MUL", OpStack);

	Operator a = OpStack.pop();
	printf("poped=%s\n", op_str[a]);

	OpStack.push(DIV);
	print_stack("ADD&SUB&DIV", OpStack);

	while (OpStack.pop(&a))
	{
		printf(" %s", op_str[a]);
	}
	printf("\n");
	print_stack("End", OpStack);
}

int main(void)
{
	print_gpl_header();

	printf("-- Interger Stack Test --\n");
	test_int_stack();
	printf("\n");
	printf("-- Struct Stack Test --\n");
	test_item_stack();
	printf("\n");
	printf("-- Enum Stack Test --\n");
	test_enum_stack();
}
