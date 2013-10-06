// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: dictionary.cpp - dictionary class source file
//	Copyright (C) 2013  Thunder422
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
//	2013-09-27	initial version

#include "dictionary.h"
#include "token.h"


Dictionary::Dictionary(void)
{
}

quint16 Dictionary::add(Token *token, Dictionary::EntryType *returnNewEntry)
{
	EntryType newEntry;

	int index = m_keyMap.value(token->string(), -1);
	if (index == -1)  // string not present?
	{
		if (m_freeStack.empty())  // no free indexes available?
		{
			index = m_keyList.count();
			m_keyList.append(token->string());
			m_useCount.append(1);
			newEntry = New_Entry;
		}
		else  // use a previously freed index
		{
			index = m_freeStack.pop();
			m_keyList[index] = token->string();
			m_useCount[index] = 1;
			newEntry = Reused_Entry;
		}
		m_keyMap[token->string()] = index;  // save key/index in map
	}
	else  // string already present, update use count
	{
		m_useCount[index]++;
		newEntry = Exists_Entry;
	}
	if (returnNewEntry)
	{
		*returnNewEntry = newEntry;
	}
	return index;
}


void Dictionary::remove(quint16 index)
{
	if (--m_useCount[index] == 0)  // update use count, if zero then remove it
	{
		m_keyMap.remove(m_keyList.at(index));  // remove key/index from map
		m_keyList[index].clear();
		m_freeStack.push(index);
	}
}


// end: dictionary.cpp
