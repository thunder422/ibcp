// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: parser.h - parser class header file
//	Copyright (C) 2012  Thunder422
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
//	2012-11-02	initial version (parts removed from ibcp.h)

#ifndef PARSER_H
#define PARSER_H

#include <sstream>

#include "ibcp.h"

class Table;
class Token;


class Parser
{
public:
	explicit Parser(const std::string &input) : m_input {input} {}
	Token *operator()(DataType dataType, Reference reference);

	// get and consume open parentheses if next character in input
	bool getParen()
	{
		if (m_input.peek() == '(')
		{
			m_input.get();
			return true;
		}
		return false;
	}

private:
	// main functions
	Token *getIdentifier(Reference reference) noexcept;
	Token *getNumber(DataType dataType);
	Token *getString() noexcept;
	Token *getOperator() noexcept;

	// support functions
	struct Word
	{
		std::string string;		// string of word
		DataType dataType;		// data type of word
		bool paren;				// word has an opening parentheses
	};
	enum class WordType
	{
		First,					// fully typed with optional parentheses word
		Second					// untyped no parentheses second word of command
	};

	Word getWord(WordType wordType) noexcept;

	std::istringstream m_input;	// input line being parsed
};


#endif  // PARSER_H
