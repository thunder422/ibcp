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
#include "utility.h"


Parser::Parser(const std::string &input) :
	m_table(Table::instance()),
	m_input {input}
{

}


// function to get a token at the current position
//
//   - a shared pointer to the token is returned
//   - after at time of return, member token is released (set to null)
//   - the token may contain an error message if an error was found

TokenUniquePtr Parser::operator()(Number number, Reference reference)
{
	m_input >> std::ws;
	if (m_input.peek() == EOF)
	{
		int pos = m_input.str().length();
		return m_table.newToken(EOL_Code, pos, 1);
	}
	if (TokenUniquePtr token = getIdentifier(reference))
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
	int pos = m_input.tellg();
	throw TokenError {Status::UnknownToken, pos, 1};
}


// function to get an identifier at the current position, which may
// be a command (one or two words), internal function, operator, defined
// function, an identifier with a parenthesis, or an identifier with no
// parenthesis.  The internal function, defined function or identifier
// will have a data type
//
//   - returns default token pointer if no identifier (position not changed)
//   - returns token pointer to new token if there is

TokenUniquePtr Parser::getIdentifier(Reference reference) noexcept
{
	int pos = m_input.tellg();
	Word word = getWord(WordType::First);
	if (word.string.empty())
	{
		return TokenUniquePtr{};  // not an identifier
	}

	// check to see if this is the start of a remark
	// (need to check separately since a space not required after 'REM')
	std::string name {m_table.name(Rem_Code)};
	if (noCaseStringBeginsWith(word.string, name))
	{
		// clear errors in case peeked past end, which sets error
		m_input.clear();
		// move to first char after 'REM'
		m_input.seekg(pos + name.length());
		// read remark string to end-of-line
		std::getline(m_input, word.string);
		return m_table.newToken(Rem_Code, pos, name.length(),
			std::move(word.string));
	}

	Code code;
	// defined function?  (must also have a letter after "FN")
	if (word.string.length() >= 3 && toupper(word.string[0]) == 'F'
		&& toupper(word.string[1]) == 'N' && isalpha(word.string[2]))
	{
		code = word.paren ? DefFuncP_Code : DefFuncN_Code;
	}
	else
	{
		code = Table::find(word.string);
		if (code == Invalid_Code)
		{
			// word not found in table, therefore
			// must be variable, array, generic function, or subroutine
			if (!word.paren)
			{
				// REMOVE for now assume a variable
				// TODO first check if identifier is in function dictionary
				// TODO only a function reference if name of current function
				code = Var_Code;
				if (reference != Reference::None)
				{
					code = m_table.alternateCode(code, 1);
					reference = Reference::None;  // don't need flag in token
				}
			}
			else
			{
				// TODO first check if identifier is in function dictionary
				//      when searching for function name in dictionary;
				//      need to remove '(' and data type charater; use data type
				//      character to decide which dictionary to search;
				//      reset word.paren to prevent '(' from being removed below
				// REMOVE for now assume functions start with an 'F' for testing
				code = reference == Reference::None
					&& toupper(word.string.front()) == 'F'
					? Function_Code : Array_Code;
			}
		}
	}

	if (m_table.name(code).empty())
	{
		if (word.paren)
		{
			word.string.pop_back();  // don't store parentheses in token string
		}
		int len = word.string.length();
		SubCode subCode {};
		if (word.dataType != DataType::None)
		{
			word.string.pop_back();  // don't store data type character
			if (word.dataType == DataType::Double)
			{
				subCode = Double_SubCode;
			}
		}
		return TokenUniquePtr{new Token {pos, len, m_table.type(code),
			word.dataType, code, std::move(word.string),
			reference != Reference::None, subCode}};
	}

	// found word in table (command, internal function, or operator)
	int len = word.string.length();
	if (m_table.hasFlag(code, Two_Flag))
	{
		// command could be a two word command
		m_input >> std::ws;
		int pos2 = m_input.tellg();  // begin of second word (could by -1)
		Word word2 = getWord(WordType::Second);
		// check for possible second word (no data type and no paren words only)
		if (!word2.string.empty())
		{
			// pos2 was not -1
			Code code2;
			if ((code2 = Table::find(word.string, word2.string))
				!= Invalid_Code)
			{
				// double word command found
				code = code2;
				len = pos2 - pos + word2.string.length();
			}
			else  // reset position back to begin of second word
			{
				// clear errors in case peeked past end, which sets error
				m_input.clear();
				m_input.seekg(pos2);
			}
		}
	}
	return m_table.newToken(code, pos, len);
}


// function to get a word at current position in input stream
//
//   - returns the string of the word along with data type and parentheses
//     flag in a Word structure
//   - returns data type found or None if none was found
//   - returns flag if opening parenthesis at end of identifier
//   - returns empty string if there is not an identifier at position
//   - input position moved to end of word for valid identifier
//   - work type argument identifies first or second word
//   - second word for command only, so no data type or parentheses allowed
//   - the input position is not moved if second word not valid

Parser::Word Parser::getWord(WordType wordType) noexcept
{
	Word word;

	if (!isalpha(m_input.peek()))
	{
		return word;  // not an identifier, return empty word
	}

	int pos = m_input.tellg();
	do
	{
		word.string.push_back(m_input.get());  // get character
	}
	while (isalnum(m_input.peek()) || m_input.peek() == '_');
	// next character is non-alnum or '_'

	// see if there is a data type symbol next
	switch (m_input.peek())
	{
	case '%':
		word.dataType = DataType::Integer;
		word.string.push_back(m_input.get());  // get data type character
		break;
	case '$':
		word.dataType = DataType::String;
		word.string.push_back(m_input.get());  // get data type character
		break;
	case '#':
		word.dataType = DataType::Double;
		word.string.push_back(m_input.get());  // get data type character
		break;
	default:
		word.dataType = DataType::None;
	}

	// see if there is an opening parenthesis
	if (m_input.peek() == '(')
	{
		word.paren = true;
		word.string.push_back(m_input.get());  // get '(' character
	}
	else
	{
		word.paren = false;
	}

	if (wordType == WordType::Second && (word.dataType != DataType::None
		|| word.paren))
	{
		word.string.clear();  // not a valid second command word
		// clear errors in case peeked past end, which sets error
		m_input.clear();
		m_input.seekg(pos);   // move back to begin of second word
	}
	return word;
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
//   - throws an exception for errors
//   - string of the number is converted to a value
//   - string of the number is saved so it can be later reproduced

TokenUniquePtr Parser::getNumber()
{
	bool digits {};				// digits were found flag
	bool decimal {};			// decimal point was found flag
	bool sign {};				// have negative sign flag
	bool expSign {};			// have exponent sign flag
	std::string number;			// string to hold number

	int pos = m_input.tellg();
	for (;;)
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
						throw TokenError {Status::ExpNonZeroDigit, pos, 1};
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
					throw TokenError {Status::ExpDigitsOrSngDP, pos, 2};
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
					m_input.seekg(pos);  // move back to begin
					return TokenUniquePtr{};
				}
				// if there were no digits before 'E' then error
				// (only would happen if mantissa contains only '.')
				throw TokenError {Status::ExpManDigits, pos, 2};
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
				throw TokenError {expSign
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
					// clear errors in case peeked past end, which sets error
					m_input.clear();
					m_input.seekg(pos);       // move back to begin
					return TokenUniquePtr{};  // not a numeric constant
				}
			}
			else if (!digits)  // only a decimal point found?
			{
				throw TokenError {Status::ExpDigits, pos, 1};
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

		// save string of number so it later can be reproduced
		return TokenUniquePtr{new Token {pos, len, std::move(number), value,
			decimal}};
	}
	catch (std::out_of_range)
	{
		// overflow or underflow, constant is not valid
		throw TokenError {Status::FPOutOfRange, pos, len};
	}
}


// function to get string constant at the current position if there is
// a double quote at current position.
//
//   - strings constants start and end with a double quote
//   - returns default token pointer if no string (position not changed)
//   - returns token if there is a valid string
//   - copy string into token without surrounding quotes

TokenUniquePtr Parser::getString() noexcept
{
	if (m_input.peek() != '"')
	{
		return TokenUniquePtr{};  // not a sting constant
	}

	int pos = m_input.tellg();
	m_input.get();  // eat first '"'
	std::string string;
	int len = 1;
	while (m_input.peek() != EOF)
	{
		char c = m_input.get();  // get char from stream
		++len;
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
			++len;
		}
		string.push_back(c);  // append char to string
	}
	return TokenUniquePtr{new Token {pos, len, std::move(string)}};
}


// function to get an operator (of symbol characters) at the current
// position, which may be one or two characters
//
//   - returns default token pointer if no operator (position not changed)
//   - returns token pointer to new token if there is a valid operator

TokenUniquePtr Parser::getOperator() noexcept
{
	std::string string;
	string.push_back(m_input.peek());
	// search table for current character to see if it is a valid operator
	Code code {Table::find(string)};
	if (code == Invalid_Code)
	{
		// character(s) at current position not a valid operator
		// (no first of two-character operator is an invalid operator)
		return TokenUniquePtr{};
	}
	int pos = m_input.tellg();
	m_input.get();  // eat first character (already in string)
	if (code == RemOp_Code)
	{
		// remark requires special handling (remark string is to end-of-line)
		std::getline(m_input, string);  // reads rest of line
		return m_table.newToken(RemOp_Code, pos, 1, std::move(string));
	}

	// current character is at least a valid one-character operator
	int len {1};
	if (m_table.hasFlag(code, Two_Flag))
	{
		// operator could be a two-character operator
		string.push_back(m_input.peek());
		Code code2;
		if ((code2 = Table::find(string)) != Invalid_Code)
		{
			// two-character operator found
			code = code2;
			len = 2;
			m_input.get();  // eat second character
		}
	}
	return m_table.newToken(code, pos, len);
}


// end: parser.cpp
