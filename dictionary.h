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
#include <unordered_map>
#include <vector>

#include <QString>

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

	Dictionary(CaseSensitive caseSensitive = CaseSensitive::No) :
		m_keyMap {10, KeyHash {caseSensitive}, KeyEqual {caseSensitive}} {}

	void clear(void);
	uint16_t add(const TokenPtr &token, EntryType *returnNewEntry = nullptr);
	int remove(uint16_t index);
	std::string string(int index) const
	{
		return m_iterator[index]->first;
	}
	QString debugText(const QString header);

private:
	struct EntryValue
	{
		EntryValue(uint16_t index) : m_index {index}, m_useCount {1} {}

		uint16_t m_index;						// index of entry
		uint16_t m_useCount;					// use count of entry
	};

	// case sensitive optional key hash function operator
	struct KeyHash
	{
		CaseSensitive caseSensitive;

		size_t operator()(const std::string &s) const
		{
			if (caseSensitive != CaseSensitive::No)
			{
				return std::hash<std::string>{}(s);
			}
			std::string s2;
			std::transform(s.begin(), s.end(), std::back_inserter(s2), toupper);
			return std::hash<std::string>{}(s2);
		}
	};

	// case sensitive optional key equal function operator
	struct KeyEqual
	{
		CaseSensitive caseSensitive;

		bool operator()(const std::string &s1, const std::string &s2) const
		{
			if (caseSensitive != CaseSensitive::No)
			{
				return s1 == s2;
			}
			if (s1.size() != s2.size())
			{
				return false;
			}
			for (size_t i = 0; i < s1.size(); ++i)
			{
				if (toupper(s1[i]) != toupper(s2[i]))
				{
					return false;
				}
			}
			return true;
		}
	};

	using KeyMap = std::unordered_map<std::string, EntryValue, KeyHash,
		KeyEqual>;
	using KeyIterator = KeyMap::iterator;

	std::stack<uint16_t> m_freeStack;		// stack of free entries
	std::vector<KeyIterator> m_iterator;	// iterators to entries (by index)
	KeyMap m_keyMap;						// map of key entries
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
	InfoDictionary(CaseSensitive caseSensitive = CaseSensitive::No) :
		Dictionary {caseSensitive} {}

	void clear(void);
	uint16_t add(const TokenPtr &token);
	void remove(uint16_t index);

protected:
	std::unique_ptr<AbstractInfo> m_info;	// pointer to additional information
};


#endif // DICTIONARY_H
