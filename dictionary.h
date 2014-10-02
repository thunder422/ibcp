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

#include <memory>
#include <stack>
#include <vector>

#include <QHash>

class Token;
using TokenPtr = std::shared_ptr<Token>;

enum class CaseSensitive {No, Yes};


class Dictionary
{
public:
	enum class EntryType
	{
		New,
		Reused,
		Exists
	};

	void clear(void);
	uint16_t add(const TokenPtr &token,
		CaseSensitive cs = CaseSensitive::No,
		EntryType *returnNewEntry = nullptr);
	int remove(uint16_t index, CaseSensitive cs = CaseSensitive::No);
	std::string string(int index) const
	{
		return m_keyList[index];
	}
	QString debugText(const QString header);

private:
	std::stack<uint16_t> m_freeStack;	// stack of free items
	std::vector<std::string> m_keyList;	// list of keys
	QHash<QString, int> m_keyHash;		// hash map of keys to indexes
	std::vector<uint16_t> m_useCount;	// list of key use counts
};


// interface class for info dictionary
class AbstractInfo
{
public:
	virtual ~AbstractInfo(void) {}
	virtual void clear(void) {}
	virtual void addElement(void) {}
	virtual void setElement(int index, const TokenPtr &token)
	{
		(void)index;
		(void)token;
	}
	virtual void clearElement(int index)
	{
		(void)index;
	}
};


class InfoDictionary : public Dictionary
{
public:
	InfoDictionary() {}

	void clear(void);
	uint16_t add(const TokenPtr &token,
		CaseSensitive cs = CaseSensitive::No);
	void remove(uint16_t index, CaseSensitive cs = CaseSensitive::No);

protected:
	std::unique_ptr<AbstractInfo> m_info;	// pointer to additional information
};


#endif // DICTIONARY_H
