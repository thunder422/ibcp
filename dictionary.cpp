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


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                            DICTIONARY FUNCTIONS                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


void Dictionary::clear(void)
{
	m_freeStack = {};
	m_iterator.clear();
	m_keyMap.clear();
}


uint16_t Dictionary::add(const TokenPtr &token,
	Dictionary::EntryType *returnNewEntry)
{
	EntryType newEntry;

	// if requested, store upper case of key in hash to make search case
	// insensitive (actual string will be stored in key list)
	std::string key {token->string()};
	auto iterator = m_keyMap.find(key);
	int index;
	if (iterator == m_keyMap.end())  // string not present?
	{
		if (m_freeStack.empty())  // no free indexes available?
		{
			index = m_iterator.size();
			newEntry = EntryType::New;
			m_iterator.emplace_back(m_keyMap.emplace(key, index).first);
		}
		else  // use a previously freed index
		{
			index = m_freeStack.top();
			m_freeStack.pop();
			newEntry = EntryType::Reused;
			m_iterator[index] = m_keyMap.emplace(key, index).first;
		}
	}
	else  // string already present, update use count
	{
		index = iterator->second.m_index;
		iterator->second.m_useCount++;
		newEntry = EntryType::Exists;
	}
	if (returnNewEntry)
	{
		*returnNewEntry = newEntry;
	}
	return index;
}


int Dictionary::remove(uint16_t index)
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


// function to reset the dictionary
void InfoDictionary::clear(void)
{
	m_info->clear();
	Dictionary::clear();
}


// function to possibly add a new dictionary entry and return its index
uint16_t InfoDictionary::add(const TokenPtr &token)
{
	EntryType returnNewEntry;
	int index {Dictionary::add(token, &returnNewEntry)};
	if (returnNewEntry == EntryType::New)
	{
		// a new entry was added to the dictionary
		// so add a new element to the additional information
		m_info->addElement(token);
	}
	else if (returnNewEntry == EntryType::Reused)
	{
		// for a new entry or a reused entry,
		// set the additional information from the token
		m_info->setElement(index, token);
	}
	return index;
}


// function to remove an entry from the dictionary
void InfoDictionary::remove(uint16_t index)
{
	if (Dictionary::remove(index))
	{
		// clear the additional information if the entry was removed
		m_info->clearElement(index);
	}
}


// end: dictionary.cpp
