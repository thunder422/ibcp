// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: list.h - contains template for list generic list class
//	Copyright (C) 2009-2011  Thunder422
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
//	2009-12-28	initial version
//
//	2010-01-30	added more "list" like functions;
//				made stack calls inline to list functions;
//				removed partial support for simple type list (values to
//				  append/push and remove/pop can be done by pointer) to reduce
//				  the number of functions;
//				changed first() to return report;
//				renamed top() to last();
//				implemented top()
//
//	2010-02-13	allow NULL for element pointer of remove to remove the first
//				element of the list
//	2010-02-15	added constructor for element to initialize value from a
//				pointer, added value argument to new Element and removed
//				setting of element->value in append and insert functions
//
//	2010-02-29	updated List class by replacing the allocated master element
//				with a tail element pointer; this tail element pointer is set to
//				NULL when the list is empty; the various functions were updated
//				to maintain the tail element pointer; implemented new raw
//				remove() for removing an element from the list, the other
//				remove() functions now call this; removed not_end()
//
//	2011-01-29	re-implemented List class to maintain a tail element pointer
//				  instead of a master element
//				implemented new raw remove function used by other removes
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
// To declare a stack for a structure, use the following:
//
//      struct Item {...};
//      List<Item> item_stack;
//
// To scan the list, use the following:
//
//		List<Item> list;
//		List<Item>::Element *element;
//		for (element = list.first(); element != NULL; list.next(element))
//		{
//			Item item = element->value;
//			...
//		}


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
	Element *tail;
public:
	List(void)
	{
		tail = NULL;
	}
	~List();

	// function to check if list is empty
	bool empty(void)
	{
		return tail == NULL;
	}

	// function get pointer to first element of list
	Element *first(void)
	{
		return tail == NULL ? NULL : tail->next;
	}

	// function get pointer to the last element of list
	Element *last(void)
	{
		return tail;
	}

	// 2010-01-29: added list-like functions
	//
	// function to append a generic value to the end of list
	Element *append(T *value);

	// function to append a generic value after an element
	Element *append(Element *after, T *value);

	// function to insert a generic value to begin of list
	Element *insert(T *value);

	// function to insert a generic value before an element
	Element *insert(Element *before, T *value);

	// function to remove a generic value from the end of list
	bool remove(T *value);

	// function to remove a generic value at an element
	bool remove(Element *element, T *value);

	// function to remove an element from the list (2011-01-29)
	void remove(Element *element);

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
		*value = tail->value;
		return true;
	}

	// function to get the generic value from top of stack
	T top(void)
	{
		// must have a value on the stack
		return tail->value;
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
		// modified to return NULL at end of list (2011-01-29)
		element = element == tail ? NULL : element->next;
	}
};


// This function appends a value passed as a pointer to the end of the
// list.  A new element is allocated and is linked to the end of the
// list.  A pointer to the new element (new tail) is returned.

// 2010-01-29: renamed from push, added return value
template <class T> typename List<T>::Element *List<T>::append(T *value)
{
	Element *element = new Element(value);
	if (tail == NULL)
	{
		element->prev = element;
		element->next = element;
	}
	else
	{
		element->prev = tail;
		element->next = tail->next;
		tail->next->prev = element;
		tail->next = element;
	}
	return tail = element;
}


// This function appends a value passed as a pointer after the element
// argument.  A new element is allocated and is linked in after the
// element argument in the list.  A pointer to the new element is
// returned.

// 2010-01-29: renamed from push, added return value
template <class T>
typename List<T>::Element *List<T>::append(List<T>::Element *after, T *value)
{
	Element *element = new Element(value);
	element->next = after->next;
	element->prev = after;
	after->next = element;
	element->next->prev = element;
	if (tail == after)
	{
		tail = element;
	}
	return element;
}


// This function inserts a value passed as a pointer to the beginning of
// the list.  A new element is allocated and is linked at the beginning
// of the list.  A pointer to the new element (new head) is returned.

template <class T> typename List<T>::Element *List<T>::insert(T *value)
{
	Element *element = new Element(value);
	if (tail == NULL)
	{
		element->next = element;
		element->prev = element;
		tail = element;
	}
	else
	{
		element->next = tail->next;
		element->prev = tail;
		element->next->prev = element;
		tail->next = element;
	}
	return element;
}


// This function inserts a value passed as a pointer before the element
// argument.  A new element is allocated and is linked in before the
// element argument in the list.  A pointer to the new element is
// returned.

template <class T>
typename List<T>::Element *List<T>::insert(List<T>::Element *before, T *value)
{
	Element *element = new Element(value);
	element->next = before;
	element->prev = before->prev;
	before->prev = element;
	element->prev->next = element;
	return element;
}


// This function removes a value from the end (tail) of the list.  The
// function is called with a pointer to the value in which to place
// (copy) the value that is at the end of the list.
//
// If this function is called and the list empty, a false is returned
// and nothing is copied.  The value is copied from the tail element at
// the end of the list, which is removed (a new tail will be set).  A
// true is then returned.

// 2010-01-29: renamed from pop
// 2011-01-29: changed to call new raw remove function
template <class T> bool List<T>::remove(T *value)
{
	if (empty())
	{
		return false;
	}
	else
	{
		*value = tail->value;
		remove(tail);
		return true;
	}
}


// This function removes a value pointed to by the element argument.  If
// the element argument is NULL, then the first item in the list is
// removed.  The function is called with a pointer to the value in which
// to place (copy) the value that is at the element argument.  After the
// value is copied from the element argument, the element is removed.
// Returns whether the list still contains elements after the element
// argument is removed.

// 2011-01-29: changed to call new raw remove function
template <class T> bool List<T>::remove(Element *element, T *value)
{
	// 2010-02-13: get first element if argument is NULL
	if (element == NULL)
	{
		element = tail->next;
	}
	*value = element->value;
	remove(element);
	return !empty();
}


// This function removes an element from the list.  If the element is
// the only element in the list, then the list is now empty.  The
// element is de-linked from the list and deallocated.  If the element
// is the tail, then a new tail is set.

// 2011-01-29: new raw remove function
template <class T> void List<T>::remove(Element *element)
{
	if (element->prev == element)
	{
		tail = NULL;					// list now empty
	}
	else
	{
	element->prev->next = element->next;
	element->next->prev = element->prev;
		if (element == tail)
		{
			tail = element->prev;		// set new tail
		}
	}
	delete element;
}


// This destructor function deallocates any elements remaining in the list.

template <class T> List<T>::~List()
{
	while (!empty())
	{
		remove(tail);
	}
}


#endif  // LIST_H
