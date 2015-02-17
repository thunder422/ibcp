// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: programwriter.h - program writer class header file
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

#ifndef PROGRAMWRITER_H
#define PROGRAMWRITER_H

#include <programcode.h>

class ProgramModel;
class Token;


class ProgramWriter
{
public:
	ProgramWriter(const ProgramModel *unit, ProgramBackInserter backInserter);

	void writeCode(uint16_t codeIndex, uint16_t subCode);
	void generateAndWriteOperand(Token *token, OperandType operandType);

private:
	const ProgramModel *m_unit;
	ProgramBackInserter m_backInserter;
};


inline ProgramWriter::ProgramWriter(const ProgramModel *unit,
	ProgramBackInserter backInserter) :
	m_unit {unit},
	m_backInserter {backInserter}
{
}

inline void ProgramWriter::writeCode(uint16_t codeIndex, uint16_t subCode)
{
	m_backInserter = ProgramWord {codeIndex, subCode};
}


#endif  // PROGRAMWRITER_H
