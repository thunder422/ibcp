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
	m_keyList.clear();
	m_keyHash.clear();
	m_useCount.clear();
}


uint16_t Dictionary::add(const TokenPtr &token, CaseSensitive cs,
	Dictionary::EntryType *returnNewEntry)
{
	EntryType newEntry;

	// if requested, store upper case of key in hash to make search case
	// insensitive (first actual string will be stored in key list)
	QString hashKey {cs == CaseSensitive::No
		? token->string().toUpper() : token->string()};
	int index {m_keyHash.value(hashKey, -1)};
	if (index == -1)  // string not present?
	{
		if (m_freeStack.empty())  // no free indexes available?
		{
			index = m_keyList.size();
			m_keyList.emplace_back(token->string().toStdString());
			m_useCount.emplace_back(1);
			newEntry = EntryType::New;
		}
		else  // use a previously freed index
		{
			index = m_freeStack.top();
			m_freeStack.pop();
			m_keyList[index] = token->string().toStdString();
			m_useCount[index] = 1;
			newEntry = EntryType::Reused;
		}
		m_keyHash[hashKey] = index;  // save key/index in map
	}
	else  // string already present, update use count
	{
		m_useCount[index]++;
		newEntry = EntryType::Exists;
	}
	if (returnNewEntry)
	{
		*returnNewEntry = newEntry;
	}
	return index;
}


int Dictionary::remove(uint16_t index, CaseSensitive cs)
{
	if (--m_useCount[index] == 0)  // update use count, if zero then remove it
	{
		QString hashKey {QString::fromStdString(m_keyList[index])};
		if (cs == CaseSensitive::No)
		{
			hashKey = hashKey.toUpper();
		}
		m_keyHash.remove(hashKey);  // remove key/index from hash map
		m_keyList[index].clear();
		m_freeStack.emplace(index);
		return true;
	}
	return false;
}


QString Dictionary::debugText(const QString header)
{
	QString string {QString("\n%1:\n").arg(header)};
	for (size_t i {}; i < m_keyList.size(); i++)
	{
		if (m_useCount[i] != 0)
		{
			string.append(QString("%1: %2 |%3|\n").arg(i).arg(m_useCount[i])
				.arg(QString::fromStdString(m_keyList[i])));
		}
	}
	string.append("Free:");
	if (m_freeStack.empty())
	{
		string.append(" none");
	}
	else
	{
		std::stack<uint16_t> tempStack = m_freeStack;
		while (!tempStack.empty())
		{
			int index {tempStack.top()};
			tempStack.pop();
			string.append(QString(" %1").arg(index));
			if (m_useCount[index] != 0)
			{
				string.append(QString("'%1'").arg(m_useCount[index]));
			}
			if (!m_keyList[index].empty())
			{
				string.append(QString("|%1|")
					.arg(QString::fromStdString(m_keyList[index])));
			}
		}
	}
	string.append("\n");
	return string;
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
uint16_t InfoDictionary::add(const TokenPtr &token, CaseSensitive cs)
{
	EntryType returnNewEntry;
	int index {Dictionary::add(token, cs, &returnNewEntry)};
	if (returnNewEntry == EntryType::New)
	{
		// a new entry was added to the dictionary
		// so add a new element to the additional information
		m_info->addElement();
	}
	if (returnNewEntry != EntryType::Exists)
	{
		// for a new entry or a reused entry,
		// set the additional information from the token
		m_info->setElement(index, token);
	}
	return index;
}


// function to remove an entry from the dictionary
void InfoDictionary::remove(uint16_t index, CaseSensitive cs)
{
	if (Dictionary::remove(index, cs))
	{
		// clear the additional information if the entry was removed
		m_info->clearElement(index);
	}
}


// end: dictionary.cpp
