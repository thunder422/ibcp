// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_stack.cpp - contains test code for testing list class
//	Copyright (C) 2009-2010  Thunder422
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
//
//  2010-01-15  added 'const' to op_str[]
//
//  2010-01-30  added test_list() function;
//              renamed print_stack functions to print_list
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


// 2010-01-30: renamed from print_stack, renamed argument
void print_list(const char *str, List<int> &IntList)
{
	List<int>::Element *element;

	printf("%s: ", str);
	for (element = IntList.first(); IntList.not_end(element);
		IntList.next(element))
	{
		printf("%d ", element->value);
	}
	printf("EOL\n");
}


void test_int_stack(void)
{
	List<int> IntStack;

	print_list("Begin", IntStack);

	IntStack.push(1);
	IntStack.push(2);
	print_list("1&2", IntStack);

	IntStack.push(8);
	print_list("1&2&8", IntStack);

	int a = IntStack.pop();
	int b = IntStack.pop();
	printf("a=%d b=%d\n", a, b);
	IntStack.push(a+b);
	print_list("1&(2+8)", IntStack);

	IntStack.push(3);
	print_list("1&(2+8)&3", IntStack);

	a = IntStack.pop();
	b = IntStack.pop();
	printf("a=%d b=%d\n", a, b);
	IntStack.push(a+b);
	print_list("1&((2+8)+3)", IntStack);

	IntStack.push(4);
	print_list("1&((2+8)+3)&4", IntStack);

	while (!IntStack.empty())
	{
		printf(" %d", IntStack.pop());
	}
	printf("\n");
	pstring class functionsrint_list("End", IntStack);
}


struct Item {
	int num;
	const char *str;

	Item(void) {}
	Item(int n, const char *s)
	{
		num = n;
		str = s;
	}
};


// 2010-01-30: renamed from print_stack, renamed argument
void print_list(const char *str, List<Item> &ItemList)
{
	List<Item>::Element *element;

	printf("%s: ", str);
	for (element = ItemList.first(); ItemList.not_end(element);
		ItemList.next(element))
	{
		printf("<%d,%s> ", element->value.num, element->value.str);
	}
	printf("EOL\n");
}


void test_item_stack(void)
{
	Item item;
	List<Item> ItemStack;

	print_list("Begin", ItemStack);

	item.num = 1;
	item.str = "PRINT";
	ItemStack.push(&item);
	item.num = 7;
	item.str = "A";
	ItemStack.push(&item);
	item.num = 8;
	item.str = ";";
	ItemStack.push(&item);
	print_list("PRINT A;", ItemStack);

	item = ItemStack.pop();
	printf("item=<%d,%s>\n", item.num, item.str);
	print_list("PRINT A", ItemStack);

	while (ItemStack.pop(&item))
	{
		printf(" <%d,%s>", item.num, item.str);
	}
	printf("\n");
	print_list("End", ItemStack);
}


enum Operator {ADD, SUB, MUL, DIV};
const char *op_str[] = {"+", "-", "*", "/"};

void print_list(const char *str, List<Operator> &OpStack)
{
	List<Operator>::Element *element;

	printf("%s: ", str);
	for (element = OpStack.first(); OpStack.not_end(element);
		OpStack.next(element))
	{
		printf("%s ", op_str[element->value]);
	}
	printf("EOL\n");
}


void test_enum_stack(void)
{
	List<Operator> OpStack;

	print_list("Begin", OpStack);

	OpStack.push(ADD);
	OpStack.push(SUB);
	print_list("ADD&SUB", OpStack);

	OpStack.push(MUL);
	print_list("ADD&SUB&MUL", OpStack);

	Operator a = OpStack.pop();
	printf("popped=%s\n", op_str[a]);

	OpStack.push(DIV);
	print_list("ADD&SUB&DIV", OpStack);

	while (OpStack.pop(&a))
	{
		printf(" %s", op_str[a]);
	}
	printf("\n");
	print_list("End", OpStack);
}


void test_list(void)
{
	Item item;
	List<Item>::Element *element;
	List<Item> ItemList;

	item = Item(1, "AAA");
	ItemList.append(&item);
	item = Item(2, "BBB");
	element = ItemList.append(&item);
	item = Item(3, "CCC");
	ItemList.append(&item);
	print_list("append-end 123", ItemList);

	item = Item(25, "BCD");
	ItemList.append(element, &item);
	print_list("append after 2", ItemList);

	item = Item(15, "ABC");
	ItemList.insert(element, &item);
	print_list("insert before 2", ItemList);

	item = Item(0, "000");
	ItemList.insert(&item);
	print_list("insert begin", ItemList);

	bool flag = ItemList.remove(&item);
	printf("remove item: <%d,%s> (%d)\n", item.num, item.str, flag);
	print_list("remove end", ItemList);

	flag = ItemList.remove(element, &item);
	printf("remove item 2: <%d,%s> (%d)\n", item.num, item.str, flag);
	print_list("remove 2", ItemList);

	printf("remove all items from begin of list...\n");
	do
	{
		element = ItemList.first();
		flag = ItemList.remove(element, &item);
		printf(" <%d,%s>", item.num, item.str);
	}
	while (flag);
	printf("\n");
	print_list("End", ItemList);
}


int main(void)
{
	print_gpl_header();

	printf("-- Integer Stack Test --\n");
	test_int_stack();
	printf("\n");
	printf("-- Struct Stack Test --\n");
	test_item_stack();
	printf("\n");
	printf("-- Enum Stack Test --\n");
	test_enum_stack();
	printf("\n");
	printf("-- List Test --\n");
	test_list();
}
