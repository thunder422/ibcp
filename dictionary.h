// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: dictionary.h - dictionary class header file
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

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <QHash>
#include <QStack>
#include <QStringList>

class Token;


class Dictionary
{
public:
	Dictionary(void);

	enum EntryType
	{
		New_Entry,
		Reused_Entry,
		Exists_Entry,
		sizeof_Entry
	};

	void clear(void);
	quint16 add(Token *token, Qt::CaseSensitivity cs = Qt::CaseInsensitive,
		EntryType *returnNewEntry = NULL);
	int remove(quint16 index);
	QString string(int index) const
	{
		return m_keyList.at(index);
	}
	QString debugText(const QString header);

private:
	QStack<quint16> m_freeStack;		// stack of free items
	QStringList m_keyList;				// list of keys
	QHash<QString, int> m_keyHash;		// hash map of keys to indexes
	QList<quint16> m_useCount;			// list of key use counts
};


// interface class for info dictionary
class AbstractInfo
{
public:
	virtual ~AbstractInfo(void) {}
	virtual void clear(void) {}
	virtual void addElement(void) {}
	virtual void setElement(int index, Token *token) {}
	virtual void clearElement(int index) {}
};


class InfoDictionary : public Dictionary
{
public:
	InfoDictionary(void);
	~InfoDictionary(void);

	void clear(void);
	quint16 add(Token *token, Qt::CaseSensitivity cs = Qt::CaseInsensitive);
	void remove(quint16 index);

protected:
	AbstractInfo *m_info;				// pointer to additional information
};


#endif // DICTIONARY_H
