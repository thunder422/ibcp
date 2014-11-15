// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: programcode.h - program word and code class header file
//	Copyright (C) 2014  Thunder422
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
// WORK moved to new programcode.h
class ProgramWord
{
public:
	// instruction access functions
	ProgramWord(Code code, unsigned subCode) :
		// FIXME remove c-style type case
		m_word((unsigned)code | (subCode & ProgramMask_SubCode)) {}

	Code instructionCode() const
	{
		// FIXME remove c-style type case
		return (Code)(m_word & ProgramMask_Code);
	}
	int instructionSubCode() const
	{
		return m_word & ProgramMask_SubCode;
	}
	bool instructionHasSubCode(int subCode) const
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
	using const_iterator = std::vector<ProgramWord>::const_iterator;
	using reference = std::vector<ProgramWord>::reference;

	ProgramCode() {}

	// code vector pass through access functions
	const_iterator begin() const
	{
		return m_code.begin();
	}
	const_iterator end() const
	{
		return m_code.end();
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
	reference operator[](size_t index)
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
	std::vector<ProgramWord> m_code;	// vector of program words
};


#endif // PROGRAMCODE_H
