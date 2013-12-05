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


Dictionary::Dictionary(void)
{
}


quint16 Dictionary::add(Token *token, Qt::CaseSensitivity cs,
	Dictionary::EntryType *returnNewEntry)
{
	EntryType newEntry;

	// if requested, store upper case of key in hash to make search case
	// insensitive (first actual string will be stored in key list)
	QString hashKey = cs == Qt::CaseInsensitive
		? token->string().toUpper() : token->string();
	int index = m_keyHash.value(hashKey, -1);
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
		m_keyHash[hashKey] = index;  // save key/index in map
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


int Dictionary::remove(quint16 index)
{
	if (--m_useCount[index] == 0)  // update use count, if zero then remove it
	{
		m_keyHash.remove(m_keyList.at(index));  // remove key/index from map
		m_keyList[index].clear();
		m_freeStack.push(index);
		return true;
	}
	return false;
}


QString Dictionary::debugText(const QString header)
{
	QString string = QString("\n%1:\n").arg(header);
	for (int i = 0; i < m_keyList.count(); i++)
	{
		if (m_useCount.at(i) != 0)
		{
			string.append(QString("%1: %2 |%3|\n").arg(i).arg(m_useCount.at(i))
				.arg(m_keyList.at(i)));
		}
	}
	string.append("Free:");
	if (m_freeStack.isEmpty())
	{
		string.append(" none");
	}
	else
	{
		for (int i = 0; i < m_freeStack.count(); i++)
		{
			int index = m_freeStack.at(i);
			string.append(QString(" %1").arg(index));
			if (m_useCount.at(index) != 0)
			{
				string.append(QString("'%1'").arg(m_useCount.at(index)));
			}
			if (!m_keyList.at(index).isEmpty())
			{
				string.append(QString("|%1|").arg(m_keyList.at(index)));
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


// constructor function - save the information instance pointer
InfoDictionary::InfoDictionary(void) :
	m_info(NULL)
{

}

InfoDictionary::~InfoDictionary(void)
{
	delete m_info;
}


// function to possibly add a new dictionary entry and return its index
quint16 InfoDictionary::add(Token *token, Qt::CaseSensitivity cs)
{
	EntryType returnNewEntry;
	int index = Dictionary::add(token, cs, &returnNewEntry);
	if (returnNewEntry == New_Entry)
	{
		// a new entry was added to the dictionary
		// so add a new element to the additional information
		m_info->addElement();
	}
	if (returnNewEntry != Exists_Entry)
	{
		// for a new entry or a reused entry,
		// set the additional information from the token
		m_info->setElement(index, token);
	}
	return index;
}


// function to remove an entry from the dictionary
void InfoDictionary::remove(quint16 index)
{
	if (Dictionary::remove(index))
	{
		// clear the additional information if the entry was removed
		m_info->clearElement(index);
	}
}


// end: dictionary.cpp
