// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: dictionary.h - dictionary class header file
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

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

#include "utility.h"

class Token;
using TokenPtr = std::shared_ptr<Token>;


class Dictionary
{
public:
	enum class EntryType
	{
		New,
		Reused,
		Exists
	};

	Dictionary(CaseSensitive caseSensitive = CaseSensitive::No);
	virtual ~Dictionary();

	virtual void clear(void);
	virtual std::pair<uint16_t, EntryType> add(Token *token);
	virtual bool remove(uint16_t index);
	virtual std::string string(uint16_t index) const;

	friend std::ostream &operator<<(std::ostream &os,
		const Dictionary *const dictionary);

private:
	struct EntryValue
	{
		EntryValue(uint16_t index) : m_index {index}, m_useCount {1} {}

		uint16_t m_index;
		uint16_t m_useCount;
	};

	using KeyMap = std::unordered_map<std::string, EntryValue, CaseOptionalHash,
		CaseOptionalEqual>;

	std::stack<uint16_t> m_freeStack;
	std::vector<KeyMap::iterator> m_iterator;
	KeyMap m_keyMap;
};

inline Dictionary::Dictionary(CaseSensitive caseSensitive) :
	m_keyMap {10, CaseOptionalHash {caseSensitive},
		CaseOptionalEqual {caseSensitive}} {}

inline Dictionary::~Dictionary()
{
}

inline std::string Dictionary::string(uint16_t index) const
{
	return m_iterator[index]->first;
}


class AbstractInfo
{
public:
	virtual ~AbstractInfo(void) {}
	virtual void clear(void) {}
	virtual void addElement(Token *token)
	{
		(void)token;
	}
	virtual void setElement(int index, Token *token)
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
	InfoDictionary(CaseSensitive caseSensitive = CaseSensitive::No);

	void clear(void) override;
	std::pair<uint16_t, EntryType> add(Token *token) override;
	bool remove(uint16_t index) override;

protected:
	std::unique_ptr<AbstractInfo> m_info;
};

inline InfoDictionary::InfoDictionary(CaseSensitive caseSensitive) :
	Dictionary {caseSensitive} {}


#endif  // DICTIONARY_H
