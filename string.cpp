// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: string.cpp - contains functions for the string class
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
//  2010-01-15  initial release
//
//  2010-01-17  added length of zero check to constructors
//
//  2010-02-01  changed delete to delete[] because new[] used
//  2010-02-09  allow string to hold generic data
//

#include <ctype.h>
#include "string.h"


// *****************************
// **  CONSTRUCTOR FUNCTIONS  **
// *****************************


// constructor for creating string from another string
//
//    - don't use if source string is a temporary

String::String(String &s)
{
	len = s.len;
	if (len <= 0)  // < 0 prevents copy of special const char * holders
	{
		str = NULL;
	}
	else
	{
		str = new char[len];
		memcpy(str, s.str, len);
	}
}


// constructor for creating string from char array and pointer
//
//     - pointer must be greater than char array pointer
//     - pointer points to next character not to be in string

String::String(const char *s, const char *p)
{
	len = p - s;
	// 2010-01-17: only allocate if length is non-zero
	if (len <= 0)
	{
		str = NULL;
	}
	else
	{
		str = new char[len];
		strncpy(str, s, len);
	}
}


// constructor for creating string from char array and length

String::String(const char *s, int l)
{
	len = l;
	// 2010-01-17: only allocate if length is non-zero
	if (len <= 0)
	{
		str = NULL;
	}
	else
	{
		str = new char[len];
		strncpy(str, s, l);
	}
}


// constructor for creating string for holding generic data

String::String(void *s, int l)
{
	len = l;
	// 2010-01-17: only allocate if length is non-zero
	if (len <= 0)
	{
		str = NULL;
	}
	else
	{
		str = new char[len];
		memcpy(str, s, l);
	}
}


// constructor for creating allocated string
//
//     - the string is allocated but not initialized
//     - no string is allocated is length is zero

String::String(int l)
{
	len = l;
	// 2010-01-17: only allocate if length is non-zero
	if (len <= 0)
	{
		str = NULL;
	}
	else
	{
		str = new char[len];
	}
}


// *************************
// **  COMPARE FUNCTIONS  **
// *************************


// function for comparing strings regardless of case
//
//    - strings with unequal lengths are not equal

bool String::equalcase(String *s)
{
	if (len != s->len)
	{
		return false;
	}
	for (int i = 0; i < len; i++)
	{
		if (toupper(str[i]) != toupper(s->str[i]))
		{
			return false;
		}
	}
	return true;
}


// function for comparing a string with a char array regardless of case
//
//    - strings with unequal lengths are not equal

bool String::equalcase(const char *s)
{
	int i;
	for (i = 0; i < len && *s; i++, s++)
	{
		if (toupper(str[i]) != toupper(*s))
		{
			return false;
		}
	}
	return i == len && *s == 0;
}


// function for comparing if strings are equal
//
//    - optimized relative for compare
//    - strings not same length are not equal
//    - returns: 1 if equal, 0 if not

int String::equal(String *s)
{
	if (len != s->len)
	{
		return 0;
	}
	char *s1 = str;
	char *s2 = s->str;
	for (int i = 0; i < len; i++)
	{
		if (*s1++ != *s2++)
		{
			return 0;
		}
	}
	return 1;
}


// function for comparing strings
//
//    - returns: -1 if string less than argument, 0 if equal, 1 if greater than

int String::compare(String *s)
{
	for (int i = 0; i < len && i < s->len; i++)
	{
		if (str[i] < s->str[i])
		{
			return -1;
		}
		if (str[i] > s->str[i])
		{
			return 1;
		}
	}
	// strings equal up to length of shorter string
	if (len < s->len)
	{
		// string length is shorter, therefore string less than argument
		return -1;
	}
	if (len > s->len)
	{
		// string length is longer, therefore string greater than argument
		return 1;
	}
	// strings are equal length, therefore are equal
	return 0;
}


// ****************************
// **  SUB-STRING FUNCTIONS  **
// ****************************


//  function for setting a character within a string
//
//      - the string must already be allocated, like with String(int l)
//      - the position is checked to make sure it is within the string

bool String::set(int pos, char c)
{
	if (pos >= 0 && pos < len)
	{
		str[pos] = c;
		return true;
	}
	else
	{
		return false;
	}
}


// function got getting a mid string reference
//
//     - no allocation is performed
//     - the reference should not be deleted before calling reset()
//     - the references should be set using setref()

String *String::getref(int start, int count)
{
	String *result = new String;

	if (start > 0 && start <= len)
	{
		if (start + count - 1 > len)
		{
			count = len - start + 1;
		}
		result->str = str + start - 1;
		result->len = count;
	}
	return result;
}

// function for setting a mid string reference
//
//     - no allocation is performed
//     - no more characters are copied than are in the reference
//     - the reference is obtained from getref()

void String::setref(String *s)
{
	int l = s->len < len ? s->len : len;
	if (l > 0)
	{
		memcpy(str, s->str, l);
	}
}


// *****************************
// **  STRING WORK FUNCTIONS  **
// *****************************


// function for copying one string to another
//
//     - old string is released
//     - new string is allocated
//     - used when source string is not a temporary

void String::copy(String *s)
{
	if (len > 0)  // only delete allocated strings
	{
		delete[] str;
	}
	len = s->len;
	if (len <= 0)  // < 0 prevents copy of special const char * holders
	{
		str = NULL;
	}
	else
	{
		str = new char[len];
		memcpy(str, s->str, len);
	}
}


// function for moving one string to another
//
//     - old string is released
//     - no string is allocated
//     - string is transfered from source
//     - source string is reset
//     - used when source string is a temporary

void String::move(String *s)
{
	if (len > 0)  // only delete allocated strings
	{
		delete[] str;
	}
	len = s->len;
	str = s->str;
	s->reset();
}


// function for concatenating two strings
//
//    - the string argument is appended to the string
//    - a new character array is allocated, the old one released
//    - used when first string is already a temporary

void String::cat(String *s)
{
	char *p = new char[len + s->len];
	memcpy(p, str, len);
	memcpy(p + len, s->str, s->len);
	delete[] str;
	str = p;
	len += s->len;
}


// function for concatenating two strings
//
//    - a new string object is allocated
//    - used when both strings are not temporaries

String *String_cat(String *s1, String *s2)
{
	String *result = new String(s1->len + s2->len);
	memcpy(result->str, s1->str, s1->len);
	memcpy(result->str + s1->len, s2->str, s2->len);
	return result;
}


// end: string.cpp
