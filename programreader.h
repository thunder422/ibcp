// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: programreader.h - program reader class header file
//	Copyright (C) 2015  Thunder422
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

#ifndef PROGRAMREADER_H
#define PROGRAMREADER_H

#include <string>

#include "programcode.h"

class ProgramModel;


class ProgramReader
{
public:
	ProgramReader(const ProgramModel *unit, ProgramConstIterator begin,
		int offset, int size);

	uint16_t operator()();
	uint16_t previous() const;
	bool hasMoreWords() const;
	std::string readOperandAndGetString(OperandType operandType);
	void readOperandAndRemoveReference(OperandType operandType);

private:
	uint16_t readOperand();

	const ProgramModel *m_unit;
	ProgramConstIterator m_iterator;
	ProgramConstIterator m_end;
};


inline ProgramReader::ProgramReader(const ProgramModel *unit,
	ProgramConstIterator begin, int offset, int size) :
	m_unit {unit},
	m_iterator {begin + offset},
	m_end {m_iterator + size}
{
}


inline uint16_t ProgramReader::readOperand()
{
	return (*m_iterator++).operand();
}

inline uint16_t ProgramReader::operator()()
{
	return readOperand();
}

inline uint16_t ProgramReader::previous() const
{
	return (*(m_iterator - 1)).operand();
}

inline bool ProgramReader::hasMoreWords() const
{
	return m_iterator != m_end;
}


#endif  // PROGRAMREADER_H
