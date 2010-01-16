// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: string.h - contains string class
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
//  1.0  2010-01-15  initial release
//

#ifndef STRING_H
#define STRING_H

#include <string.h>


// This class give BASIC string like functionality to C++.
//
//    - empty strings, i.e. "", are defined with len = 0 and str = NULL
//    - special strings for holding const char * are defined with len = -1;
//      these are not copied or deallocated (since they were not allocated)
//      they must not be used with the various functions
//      only valid functions are ~String() and get_str()

class String {
	int len;			// length of string
	char *str;			// pointer to string

public:
	String(void)
	{
		len = 0;
		str = NULL;
	}
	String(String &s);
	String(const char *s, const char *p);
	String(const char *s, int l);
	String(int l);
	String(const char *s)  // special holder for C const char * strings
	{
		len = -1;
		str = (char *)s;
	}
	~String()
	{
		if (len > 0)  // only delete allocated strings
		{
			delete str;
		}
	}
	char *get_str(void)
	{
		return str;
	}
	int get_len(void)
	{
		return len;
	}

	bool equalcase(String *s);
	bool equalcase(const char *s);
	int equal(String *s);
	int compare(String *s);

	bool set(int pos, char c);
	String *getref(int start, int count);
	void setref(String *s);
	void reset(void)  // reset string with no deallocation
	{
		len = 0;
		str = NULL;
	}

	void copy(String *s);
	void move(String *s);
	void cat(String *s);
	friend String *String_cat(String *s1, String *s2);
};


#endif  // STRING_H
