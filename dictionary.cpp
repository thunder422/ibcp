// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: dictionary.cpp - dictionary class source file
//	Copyright (C) 2013-2015  Thunder422
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

#include "dictionary.h"
#include "token.h"


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                            DICTIONARY FUNCTIONS                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


void Dictionary::clear(void)
{
	m_freeStack = std::stack<uint16_t>{};
	m_iterator.clear();
	m_keyMap.clear();
}


std::pair<uint16_t, Dictionary::EntryType> Dictionary::add(Token *token)
{
	int index = m_freeStack.empty() ? m_iterator.size() : m_freeStack.top();
	auto iteratorAndIsNew = m_keyMap.emplace(token->string(), index);
	EntryType entryType;
	if (iteratorAndIsNew.second)
	{
		if (m_freeStack.empty())
		{
			m_iterator.emplace_back(iteratorAndIsNew.first);
			entryType = EntryType::New;
		}
		else
		{
			m_freeStack.pop();
			m_iterator[index] = iteratorAndIsNew.first;
			entryType = EntryType::Reused;
		}
	}
	else
	{
		iteratorAndIsNew.first->second.m_useCount++;
		index = iteratorAndIsNew.first->second.m_index;
		entryType = EntryType::Exists;
	}
	return std::make_pair(index, entryType);
}


bool Dictionary::remove(uint16_t index)
{
	auto iterator = m_iterator[index];
	if (iterator != m_keyMap.end() && --iterator->second.m_useCount == 0)
	{
		m_freeStack.emplace(iterator->second.m_index);
		m_keyMap.erase(iterator);
		m_iterator[index] = m_keyMap.end();
		return true;
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                      INFORMATION DICTIONARY FUNCTIONS                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


void InfoDictionary::clear(void)
{
	m_info->clear();
	Dictionary::clear();
}


std::pair<uint16_t, Dictionary::EntryType> InfoDictionary::add(Token *token)
{
	auto indexEntryType = Dictionary::add(token);
	if (indexEntryType.second == EntryType::New)
	{
		m_info->addElement(token);
	}
	else if (indexEntryType.second == EntryType::Reused)
	{
		m_info->setElement(indexEntryType.first, token);
	}
	return indexEntryType;
}


bool InfoDictionary::remove(uint16_t index)
{
	if (Dictionary::remove(index))
	{
		m_info->clearElement(index);
		return true;
	}
	return false;
}


// end: dictionary.cpp
