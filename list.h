// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: list.h - contains template for list generic list class
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
//  2009-12-28  initial version
//

#ifndef LIST_H
#define LIST_H

// Note: template functions don't actually generate code, so they are
//       located below (not in the template so that they don't get
//       generated as in-line functions).

// This class template does not generate any code until it is
// instantiated for a particular type.  The type can be a simple C++
// type like "int" or "double" or could be a structure.  For instance,
// to declare an integer stack, use the following:
//
//      List<int> int_stack;
//
// To declare a stack for an enumeration, use the following:
//
//      enum Operator {plus, minus, multiply, divide};
//      List<Operator> operator_stack;
//
// To delete a stack for a structure, use the following:
//
//      struct Item {...};
//      List<Item> item_stack;


template <class T> class List {
public:
	struct Element {
		Element *prev;
		Element *next;
		T value;
	};
private:
	Element *master;
public:
	List(void)
	{
		master = (Element *)new char[sizeof(Element) - sizeof(T)];
		master->prev = master->next = master;
	}
	~List();

	// function to check if list is empty 
	bool empty(void)
	{
		return master->prev == master;
	}
	// function get a reference to the current top (end) of list
	Element *top(void)
	{
		return master->prev;
	}

	// function to push (append) a constant generic value to end of list
	void push(T value);
	// function to pop (remove) a generic value from end of list
	T pop(void);

	// function to push (append) a generic value to end of list
	void push(T *value);
	// function to pop (remove) a generic value from end of list
	bool pop(T *value);

	// the following functions are used to scan a list from the beginning
	// to the end using a for statement, for example:
	//
	//      List<int> int_stack;
	//      List<int>element *element;
	//      for (int_stack.first(element); int_stack.not_end(element);
	//          int_stack.next(element)) {...}

	// function to set element pointer to first item (bottom) of list
	void first(Element *&element)
	{
		element = master->next;
	}
	// function to set element pointer to the next item of list
	void next(Element *&element)
	{
		element = element->next;
	}
	// function to see if the element pointer is not at the end of the list
	bool not_end(Element *&element)
	{
		return element != master;
	}
};


// This function pushes a value passes by value to the top of the stack
// (appends to the end of the list).  This function is meant to be used
// for lists containing simple type (e.g. int, double, etc.).  A new
// element is allocated and is linked in to the end of th list.
template <class T> void List<T>::push(T value)
{
	Element *element = new Element;
	element->next = master;
	element->prev = master->prev;
	element->value = value;
	(master->prev)->next = element;
	master->prev = element;
}

template <class T> T List<T>::pop(void)
{
	Element *element = master->prev;
	T value = element->value;
	(element->prev)->next = element->next;
	master->prev = element->prev;
	delete element;
	return value;
}


// This function pushes a value passes as a pointer to the top of the
// stack (appends to the end of the list).  This function is meant to be
// used for lists containing structures.  A new element is allocated and
// is linked in to the end of th list.
template <class T> void List<T>::push(T *value)
{
	Element *element = new Element;
	element->next = master;
	element->prev = master->prev;
	element->value = *value;
	(master->prev)->next = element;
	master->prev = element;
}

template <class T> bool List<T>::pop(T *value)
{
	if (empty())
	{
		return false;
	}
	Element *element = master->prev;
	*value = element->value;
	(element->prev)->next = element->next;
	master->prev = element->prev;
	delete element;
	return true;
}

template <class T> List<T>::~List()
{
	Element *element = master->next;
	while (element != master)
	{
		delete element;
		element = element->next;
	}
	delete master;
}

#endif  // LIST_H
