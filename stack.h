// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: stack.h - contains template for generic simple stack class
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
//  2010-04-02  initial release
//

#ifndef STACK_H
#define STACK_H

// Note: template functions don't actually generate code, so they are
//       located below (not in the template so that they don't get
//       generated as in-line functions).

// The SimpleStack class contains a simple implementation of a stack
// where an allocated array is used to contain the stack entries instead
// of the more powerful doubly linked list of the List class.  This
// class is used in placed where insert and remove from any place within
// the stack is not necessary.  Only the basic stack functions push, pop
// and top are supported.  
//
// Because arrays are allocated, a size is needed to allocate the array.
// The initial size is specified with an argument to the template.
// Since arrays are limited in size, this class provides a mechanism for
// automatically growing the array when necessary.  The size that the
// array is increased is specified with an argument to the template.
// Defaults of 10 are provided for these sizes.
//
// This class template does not generate any code until it is
// instantiated for a particular type.  The type must be a simple C++
// type like "int" or "double" or pointer.  For instance, to declare an
// integer stack, use the following:
//
//      SimpleStack<int> int_stack;
//
// To declare a stack for an enumeration, use the following:
//
//      enum Operator {plus, minus, multiply, divide};
//      SimpleStack<Operator> operator_stack;
//
// To declare a stack for a structure, use the following:
//
//      struct Item {...};
//      SimpleStack<Item *> item_stack;


template <class T, int initial_size = 10, int increase_size = 10>
class SimpleStack {
	T *stack;					// pointer to array holding stack
	int size;					// current size of the array
	int index;					// index to the top element in the stack
public:
	SimpleStack(void)
	{
		size = initial_size;
		stack = new T[size];	// allocate stack array
		index = -1;				// indicates stack is empty
	}
	~SimpleStack()
	{
		delete[] stack;			// de-allocate stack array
	}

	// function to check if list is empty 
	bool empty(void)
	{
		return index == -1;
	}

	// function to push a value on the top of a stack
	void push(T value);

	// function to pop a constant generic value from top of a stack
	// - stack must not be empty - no protection is provided, use empty()
	T pop(void)
	{
		return stack[index--];
	}

	// function to get a reference to the generic value on top of stack
	T &top(void)
	{
		// must have a value on the stack
		return stack[index];
	}
};


// This function appends a value passed as a pointer to the end of the
// list.  A new element is allocated and is linked in after the element
// argument in the list.  A pointer to the new element is returned.

template <class T, int initial_size, int increase_size>
void SimpleStack<T, initial_size, increase_size>::push(T value)
{
	if (++index >= size)
	{
		// stack array is not big enough, increase size
		int newsize = size + increase_size;
		T* newstack = new T[newsize];

		// move elements from current array to new array
		// (C++ provides no realloc() functionality with new)
		for (int i = 0; i < size; i++)
		{
			newstack[i] = stack[i];
		}

		delete[] stack;
		stack = newstack;
		size = newsize;
	}
	// add value to stack array
	stack[index] = value;
}


#endif  // STACK_H
