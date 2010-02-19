// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: list.h - contains template for list generic list class
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
//  2009-12-28  initial version
//
//  2010-01-30  added more "list" like functions;
//              made stack calls inline to list functions;
//              removed partial support for simple type list (values to
//                append/push and remove/pop can be done by pointer) to reduce
//                the number of functions;
//              changed first() to return report;
//              renamed top() to last();
//              implemented top()
//
//  2010-02-13  allow NULL for element pointer of remove to remove the first
//              element of the list
//  2010-02-15  added constructor for element to initialize value from a
//              pointer, added value argument to new Element and removed
//              setting of element->value in append and insert functions
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

		Element(T *v)
		{
			value = *v;
		}
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

	// function get pointer to first element of list
	Element *first(void)
	{
		return master->next;
	}

	// function get pointer to the last element of list
	Element *last(void)
	{
		return master->prev;
	}

	// 2010-01-29: added list-like functions
	//
	// function to append a generic value to the end of list
	Element *append(T *value);

	// function to append a generic value after an element
	Element *append(Element *element, T *value);

	// function to insert a generic value to begin of list
	Element *insert(T *value);

	// function to insert a generic value before an element
	Element *insert(Element *element, T *value);

	// function to remove a generic value from the end of list
	bool remove(T *value);

	// function to remove a generic value at an element
	bool remove(Element *element, T *value);

	// 2010-01-29: made stack functions inline calls to list functions
	//
	// function to push a generic value on the top of a stack
	void push(T *value)
	{
		append(value);
	}

	// function to push a constant value on the top of a stack
	void push(T value)
	{
		append(&value);
	}

	// function to pop a generic value from top of a stack
	bool pop(T *value)
	{
		return remove(value);
	}

	// function to pop a constant generic value from top of a stack
	T pop(void)
	{
		T value;
		remove(&value);
		return value;
	}

	// function to get the generic value from top of a stack
	bool top(T *value)
	{
		if (empty())
		{
			return false;
		}
		*value = master->prev->value;
		return true;
	}

	// function to get the generic value from top of stack
	T top(void)
	{
		// must have a value on the stack
		return master->prev->value;
	}

	// the following functions are used to scan a list from the
	// beginning to the end using a for statement, for example:
	//
	//      List<int> int_list;
	//      List<int>Element *element;
	//      for (element = int_list.first(); int_list.not_end(element);
	//          int_list.next(element)) {...}

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


// This function appends a value passed as a pointer to the end of the
// list.  A new element is allocated and is linked in after the element
// argument in the list.  A pointer to the new element is returned.

// 2010-01-29: renamed from push, added return value
template <class T> typename List<T>::Element *List<T>::append(T *value)
{
	Element *element = new Element(value);
	element->next = master;
	element->prev = master->prev;
	master->prev->next = element;
	master->prev = element;
	return element;
}


// This function appends a value passed as a pointer after the element
// argument.  A new element is allocated and is linked in after the
// element argument in the list.  A pointer to the new element is
// returned.

// 2010-01-29: renamed from push, added return value
template <class T>
typename List<T>::Element *List<T>::append(List<T>::Element *element, T *value)
{
	Element *new_elem = new Element(value);
	new_elem->next = element->next;
	new_elem->prev = element;
	element->next = new_elem;
	new_elem->next->prev = new_elem;
	return new_elem;
}


// This function inserts a value passed as a pointer to the beginning of
// the list.  A new element is allocated and is linked at the beginning
// of the list (after the master element).  A pointer to the new element
// is returned.

template <class T> typename List<T>::Element *List<T>::insert(T *value)
{
	Element *new_elem = new Element(value);
	new_elem->next = master->next;
	new_elem->prev = master;
	master->next = new_elem;
	new_elem->next->prev = new_elem;
	return new_elem;
}


// This function inserts a value passed as a pointer before the element
// argument.  A new element is allocated and is linked in before the
// element argument in the list.  A pointer to the new element is
// returned.

template <class T>
typename List<T>::Element *List<T>::insert(List<T>::Element *element, T *value)
{
	Element *new_elem = new Element(value);
	new_elem->next = element;
	new_elem->prev = element->prev;
	element->prev = new_elem;
	new_elem->prev->next = new_elem;
	return new_elem;
}


// This function removes a value from the end of the list.  The function
// is called with a pointer to the value in which to place (copy) the
// value that is at the end of the list.
//
// If this function is called and the list empty, a false is returned
// and nothing is copied.  The value is copied from the element at the
// end of the list, which is de-linked from the list and deallocated.  A
// true value is then returned.

// 2010-01-29: renamed from pop
template <class T> bool List<T>::remove(T *value)
{
	if (empty())
	{
		return false;
	}
	Element *element = master->prev;
	*value = element->value;
	element->prev->next = element->next;
	master->prev = element->prev;
	delete element;
	return true;
}


// This function removes a value pointer to by the element argument.
// The function is called with a pointer to the value in which to place
// (copy) the value that is at the element argument.  After the value is
// copied from the element argument, the element is de-linked from the
// list and deallocated.  Returns whether the list still contains
// elements after the element argument is removed.

template <class T> bool List<T>::remove(Element *element, T *value)
{
	// 2010-02-13: get first element if argument is NULL
	if (element == NULL)
	{
		element = first();
	}
	*value = element->value;
	element->prev->next = element->next;
	element->next->prev = element->prev;
	delete element;
	return !empty();
}


// This destructor function deallocates any elements remaing in the list
// and then deallocates the master element.

template <class T> List<T>::~List()
{
	Element *element = master->next;
	while (element != master)
	{
		delete element;
		element = element->next;
	}
	// 2010-01-30: must use delete[] since new[] was used to allocate
	delete[] (char *)master;
}


#endif  // LIST_H
