// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: programcode.h - program word and code class header file
//	Copyright (C) 2014-2015  Thunder422
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
//	2014-11-13	initial version (from parts of programmmodel.h)

#ifndef PROGRAMCODE_H
#define PROGRAMCODE_H

#include <cstdint>
#include <vector>

#include "ibcp.h"


// class for holding and accessing a program word
class ProgramWord
{
public:
	// instruction access functions
	ProgramWord(uint16_t code, uint16_t subCode) :
		m_word(code | (subCode & ProgramMask_SubCode)) {}

	uint16_t instructionCode() const
	{
		// FIXME remove c-style type cast
		return m_word & ProgramMask_Code;
	}
	SubCode instructionSubCode() const
	{
		return SubCode(m_word & ProgramMask_SubCode);
	}
	bool instructionHasSubCode(SubCode subCode) const
	{
		return (m_word & subCode) != 0;
	}

	// operand word access functions
	ProgramWord(uint16_t operand) : m_word(operand) {}

	uint16_t operand() const
	{
		return m_word;
	}

private:
	uint16_t m_word;					// one program word
};


// class for holding the program code vector
class ProgramCode
{
public:
	using Vector = std::vector<ProgramWord>;
	using ConstIterator = Vector::const_iterator;
	using Reference = Vector::reference;
	using BackInserter = std::back_insert_iterator<Vector>;

	ProgramCode() {}

	// code vector pass through access functions
	ConstIterator begin() const
	{
		return m_code.begin();
	}
	ConstIterator end() const
	{
		return m_code.end();
	}
	BackInserter backInserter()
	{
		return std::back_inserter(m_code);
	}
	bool empty() const
	{
		return m_code.empty();
	}
	size_t size() const
	{
		return m_code.size();
	}
	void clear()
	{
		m_code.clear();
	}
	Reference operator[](size_t index)
	{
		return m_code[index];
	}
	template <typename... Args>
	void emplace_back(Args&&... args)
	{
		m_code.emplace_back(std::forward<Args>(args)...);
	}

	// program line access functions
	void insertLine(int offset, const ProgramCode &line)
	{
		if (!line.empty())
		{
			m_code.insert(m_code.begin() + offset, line.begin(),
				line.end());
		}
	}

	void removeLine(int offset, int oldSize)
	{
		if (oldSize > 0)  // something to remove?
		{
			auto start = m_code.begin() + offset;
			m_code.erase(start, start + oldSize);
		}
	}

	void replaceLine(int offset, size_t oldSize, const ProgramCode &line)
	{
		if (line.empty())
		{
			// no new line, just remove old line
			removeLine(offset, oldSize);
		}
		else if (line.size() >= oldSize)  // new line larger or same as old?
		{
			// copy part of new line that will fit to replace old line
			auto endCopy = std::copy(line.begin(), line.begin() + oldSize,
				m_code.begin() + offset);

			if (line.size() > oldSize)  // more of new line?
			{
				// insert remaining part of new line
				m_code.insert(endCopy, line.begin() + oldSize, line.end());
			}
		}
		else  // new line smaller
		{
			// copy new line into program
			auto endCopy = std::copy(line.begin(), line.end(),
				m_code.begin() + offset);

			// remove extra part of old line
			m_code.erase(endCopy, endCopy + oldSize - line.size());
		}
	}

private:
	Vector m_code;
};


class ProgramModel;

class ProgramLineReader
{
public:
	ProgramLineReader(const ProgramModel *unit,
		ProgramCode::ConstIterator begin, int offset, int size) : m_unit {unit},
		m_iterator {begin + offset}, m_end {m_iterator + size} {}

	uint16_t operator()()
	{
		return (*m_iterator++).operand();
	}
	uint16_t previous() const
	{
		return (*(m_iterator - 1)).operand();
	}
	bool hasMoreWords() const
	{
		return m_iterator != m_end;
	}
	const ProgramModel *unit() const
	{
		return m_unit;
	}

private:
	const ProgramModel *m_unit;
	ProgramCode::ConstIterator m_iterator;
	ProgramCode::ConstIterator m_end;
};


#endif // PROGRAMCODE_H
