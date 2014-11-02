// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: parser.cpp - parser class source file
//	Copyright (C) 2010-2011  Thunder422
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
//  Change History:
//
//	2010-03-01	initial version

#include "parser.h"
#include "table.h"


Parser::Parser(const QString &input) :
	m_table(Table::instance()),
	m_input {input},
	m_pos {}
{

}


// function to get a token at the current position
//
//   - a shared pointer to the token is returned
//   - after at time of return, member token is released (set to null)
//   - the token may contain an error message if an error was found

TokenUniquePtr Parser::operator()(Number number)
{
	skipWhitespace();
	if (m_input[m_pos].isNull())
	{
		return m_table.newToken(m_pos, 1, EOL_Code);
	}
	if (TokenUniquePtr token = getIdentifier())
	{
		return token;
	}
	if (number == Number::Yes)
	{
		if (TokenUniquePtr token = getNumber())
		{
			return token;
		}
	}
	if (TokenUniquePtr token = getString())
	{
		return token;
	}
	if (TokenUniquePtr token = getOperator())
	{
		return token;
	}
	// not a valid token, throw unknown token error
	throw Error {Status::UnknownToken, m_pos, 1};
}


// function to get an identifier at the current position, which may
// be a command (one or two words), internal function, operator, defined
// function, an identifier with a parenthesis, or an identifier with no
// parenthesis.  The internal function, defined function or identifier
// will have a data type
//
//   - returns default token pointer if no identifier (position not changed)
//   - returns token pointer to new token if there is

TokenUniquePtr Parser::getIdentifier()
{
	DataType dataType;		// data type of word
	bool paren;				// word has opening parenthesis flag

	// check to see if this is the start of a remark
	if (m_input.midRef(m_pos).startsWith(m_table.name(Rem_Code),
		Qt::CaseInsensitive))
	{
		// remark string is to end-of-line
		int pos {m_pos};
		int len = m_table.name(Rem_Code).length();
		m_pos = m_input.length();  // set to end-of-line
		return m_table.newToken(pos, len, Rem_Code, m_input.mid(pos + len));
	}

	int pos {scanWord(m_pos, dataType, paren)};
	if (pos == -1)
	{
		return TokenUniquePtr{};  // not an identifier
	}

	int len {pos - m_pos};
	Token::Type type {};
	Code code;
	// defined function?
	if (m_input.midRef(m_pos).startsWith("FN", Qt::CaseInsensitive))
	{
		type = paren ? Token::Type::DefFuncP : Token::Type::DefFuncN;
	}
	else
	{
		SearchType search= paren ? ParenWord_SearchType : PlainWord_SearchType;
		code = m_table.search(search, m_input.midRef(m_pos, len));
		if (code == Invalid_Code)
		{
			// word not found in table, therefore
			// must be variable, array, generic function, or subroutine
			// but that can't be determined here, so just generic token
			type = paren ? Token::Type::Paren : Token::Type::NoParen;
		}
	}
	std::swap(m_pos, pos);  // swap begin and end positions
	if (type != Token::Type{})
	{
		if (paren)
		{
			--len;  // don't store parentheses in token string
		}
		return TokenUniquePtr{new Token {pos, len, type, dataType, m_input}};
	}

	// found word in table (command, internal function, or operator)
	if (m_table.multiple(code) != Multiple::OneWord)
	{
		// command could be a two word command
		skipWhitespace();
		int pos2 {scanWord(m_pos, dataType, paren)};
		if (dataType == DataType::None && !paren)  // possible second word?
		{
			Code code2;
			if ((code2 = m_table.search(m_input.midRef(pos, len),
				m_input.midRef(m_pos, pos2 - m_pos))) != Invalid_Code)
			{
				// double word command found
				code = code2;
				len = pos2 - pos;
				m_pos = pos2;  // move position past second word
			}
		}
	}
	return m_table.newToken(pos, len, code);
}


// function to get a word at the position specified
//
//   - returns -1 if there is not an identifier at point
//   - returns index to character after identifier
//   - returns data type found or None if none was found
//   - returns flag if opening parenthesis at end of identifier

int Parser::scanWord(int pos, DataType &dataType, bool &paren)
{
	if (!m_input[pos].isLetter())
	{
		return -1;  // not an identifier
	}
	do
	{
		pos++;
	}
	while (m_input[pos].isLetterOrNumber() || m_input[pos] == '_');
	// pos now points to non-alnum or '_'

	// see if there is a data type symbol next
	switch (m_input[pos].unicode())
	{
	case '%':
		dataType = DataType::Integer;
		pos++;
		break;
	case '$':
		dataType = DataType::String;
		pos++;
		break;
	case '#':
		dataType = DataType::Double;
		pos++;
		break;
	default:
		dataType = DataType::None;
	}

	// see if there is an opening parenthesis
	if (m_input[pos] == '(')
	{
		paren = true;
		pos++;
	}
	else
	{
		paren = false;
	}

	// p now points to next character after identifier
	return pos;
}


// function to skip white space at the specified position
//
//   - for now the only white space is a space character
//   - returns pointer to next non-white space character
//   - if no white space found then argument is returned

void Parser::skipWhitespace(void)
{
	while (m_input[m_pos].isSpace())
	{
		m_pos++;
	}
}


// function to get numeric constant at the current position, which may
// be an integer or double constant.  If the value does not contain a
// decimal point or an exponent, and it fits into a 32-bit integer, then
// it will be returned as an integer, otherwise is will be returned as a
// double.
//
//   - numbers starting with zero must be followed by a decimal point
//   - returns default token pointer if no number (position not changed)
//   - returns token if there is a valid number
//   - throws an expection for errors
//   - string of the number is converted to a value
//   - string of the number is saved so it can be later reproduced

TokenUniquePtr Parser::getNumber()
{
	// TODO temporary to simulate m_input as input string stream
	std::string tmp {m_input.mid(m_pos).toStdString()};
	std::istringstream m_input {tmp};

	bool digits {};				// digits were found flag
	bool decimal {};			// decimal point was found flag
	bool sign {};				// have negative sign flag
	bool expSign {};			// have exponent sign flag
	std::string number;			// string to hold number

	int pos {m_pos};
	forever
	{
		if (isdigit(m_input.peek()))
		{
			number.push_back(m_input.get());  // get digit
			if (!digits)  // first digit?
			{
				digits = true;
				if (!decimal && number.back() == '0' && m_input.peek() != '.')
				{
					// next character not a digit (or '.')?
					if (isdigit(m_input.peek()))
					{
						// if didn't find a decimal point
						// and first character is a zero
						// and second character is not a decimal point,
						// and second character is a digit
						// then this is in invalid number
						throw Error {Status::ExpNonZeroDigit, pos, 1};
					}
					break;  // single zero, exit loop to process string
				}
			}
		}
		else if (m_input.peek() == '.')
		{
			if (decimal)  // was a decimal point already found?
			{
				if (!digits)  // no digits found?
				{
					throw Error {Status::ExpDigitsOrSngDP, pos, 2};
				}
				break;  // exit loop to process string
			}
			decimal = true;
			number.push_back(m_input.get());  // get '.'
		}
		else if (toupper(m_input.peek()) == 'E')
		{
			if (!digits)
			{
				if (sign && !decimal)
				{
					// if there is a '-E' then not a number
					// (need to interprete '-' as unary operator)
					return TokenUniquePtr{};
				}
				// if there were no digits before 'E' then error
				// (only would happen if mantissa contains only '.')
				throw Error {Status::ExpManDigits, pos, 2};
			}
			number.push_back(m_input.get());  // get 'e' or 'E'
			if (m_input.peek() == '+' || m_input.peek() == '-')
			{
				expSign = true;
				number.push_back(m_input.get());  // get exponent sign
			}
			// now look for exponent digits
			digits = false;
			while (isdigit(m_input.peek()))
			{
				number.push_back(m_input.get());  // get exponent digit
				digits = true;
			}
			if (!digits)  // no exponent digits found?
			{
				pos += number.length();  // move to error
				throw Error {expSign
					? Status::ExpExpDigits : Status::ExpExpSignOrDigits, pos,
					1};
			}
			decimal = true;  // process as double
			break;  // exit loop to process string
		}
		else  // some other non-valid numeric character
		{
			if (!digits && !decimal)  // nothing found?
			{
				// look for negative sign
				if (!sign && m_input.peek() == '-')
				{
					number.push_back(m_input.get());  // get negative sign
					sign = true;
				}
				else
				{
					return TokenUniquePtr{};  // not a numeric constant
				}
			}
			else if (!digits)  // only a decimal point found?
			{
				throw Error {Status::ExpDigits, pos, 1};
			}
			else
			{
				// no more valid number characters, go process what we got
				break;
			}
		}
	}

	int len = number.length();
	if (!decimal)  // no decimal or exponent?
	try
	{
		// try to convert to integer first
		int value {std::stoi(number)};

		m_pos += number.length();
		// save string of number so it later can be reproduced
		return TokenUniquePtr{new Token {pos, len, std::move(number), value}};
	}
	catch(std::out_of_range)
	{
		// overflow or underflow, won't fit into an integer
		// fall through and try as double
	}

	try
	{
		double value {std::stod(number)};

		m_pos += number.length();
		// save string of number so it later can be reproduced
		return TokenUniquePtr{new Token {pos, len, std::move(number), value,
			decimal}};
	}
	catch (std::out_of_range)
	{
		// overflow or underflow, constant is not valid
		throw Error {Status::FPOutOfRange, m_pos, len};
	}
}


// function to get string constant at the current position if there is
// a double quote at current position.
//
//   - strings constants start and end with a double quote
//   - returns default token pointer if no string (position not changed)
//   - returns token if there is a valid string
//   - copy string into token without surrounding quotes

TokenUniquePtr Parser::getString()
{
	// TODO temporary to simulate m_input as input string stream
	std::string tmp {m_input.mid(m_pos).toStdString()};
	std::istringstream m_input {tmp};

	if (m_input.peek() != '"')
	{
		return TokenUniquePtr{};  // not a sting constant
	}

	m_input.get();  // eat first '"'
	int pos {m_pos++};
	std::string string;
	while (m_input.peek() != EOF)
	{
		char c = m_input.get();  // get char from stream
		++m_pos;
		if (c == '"')
		{
			if (m_input.peek() != '"')  // not two in a row?
			{
				// found end of string
				// input stream at character following closing quote
				break;
			}
			// otherwise quote counts as one character
			m_input.get();  // eat second '"'
			++m_pos;
		}
		string.push_back(c);  // append char to string
	}
	return TokenUniquePtr{new Token {pos, m_pos - pos, std::move(string)}};
}


// function to get an operator (of symbol characters) at the current
// position, which may be one or two characters
//
//   - returns default token pointer if no operator (position not changed)
//   - returns token pointer to new token if there is a valid operator

TokenUniquePtr Parser::getOperator()
{
	// search table for current character to see if it is a valid operator
	Code code {m_table.search(Symbol_SearchType, m_input.midRef(m_pos, 1))};
	if (code == Invalid_Code)
	{
		// character(s) at current position not a valid operator
		// (no first of two-character operator is an invalid operator)
		return TokenUniquePtr{};
	}
	if (code == RemOp_Code)
	{
		// remark requires special handling (remark string is to end-of-line)
		int pos {m_pos};
		m_pos = m_input.length();  // set to end-of-line
		return m_table.newToken(pos, 1, RemOp_Code, m_input.mid(pos + 1));
	}

	// current character is at least a valid one-character operator
	int len {1};
	if (m_table.multiple(code) != Multiple::OneChar)
	{
		// operator could be a two-character operator
		Code code2;
		if ((code2 = m_table.search(Symbol_SearchType, m_input.midRef(m_pos,
			2))) != Invalid_Code)
		{
			// two-character operator found
			code = code2;
			len = 2;
		}
	}
	m_pos += len;  // move past operator character(s)
	return m_table.newToken(m_pos - len, len, code);
}


// end: parser.cpp
