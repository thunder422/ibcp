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
	m_pos {},
	m_errorStatus {}
{

}


// function to get a token at the current position
//
//   - a shared pointer to the token is returned
//   - after at time of return, member token is released (set to null)
//   - the token may contain an error message if an error was found

TokenPtr Parser::operator()(State state)
{
	skipWhitespace();
	m_token = std::make_shared<Token>(m_pos);  // create new token to return
	m_errorStatus = Status{};
	if (m_input[m_pos].isNull())
	{
		m_table.setToken(m_token, EOL_Code);
	}
	else if (!getIdentifier() && (state == State::Operator || !getNumber())
		&& !getString() && !getOperator())
	{
		// not a valid token, create error token
		setError(Status::UnknownToken, DataType::None);
	}
	return std::move(m_token);  // token may contain an error
}


// function to get an identifier at the current position, which may
// be a command (one or two words), internal function, operator, defined
// function, an identifier with a parenthesis, or an identifier with no
// parenthesis.  The internal function, defined function or identifier
// will have a data type
//
//   - returns false if no identifier (position not changed)
//   - returns true if there is and token is filled
//   - returns true for errors setting special error token

bool Parser::getIdentifier(void)
{
	DataType dataType;		// data type of word
	bool paren;				// word has opening parenthesis flag
	SearchType search;		// table search type

	// check to see if this is the start of a remark
	if (m_input.midRef(m_pos).startsWith(m_table.name(Rem_Code),
		Qt::CaseInsensitive))
	{
		m_table.setToken(m_token, Rem_Code);
		m_token->setLength(m_table.name(Rem_Code).length());
		// move position past command and grab rest of line
		m_pos += m_token->length();
		m_token->setString(m_input.mid(m_pos));
		m_pos += m_token->stringLength();
		return true;
	}

	int pos {scanWord(m_pos, dataType, paren)};
	if (pos == -1)
	{
		return false;  // not an identifier
	}

	int len {pos - m_pos};
	// defined function?
	if (m_input.midRef(m_pos).startsWith("FN", Qt::CaseInsensitive))
	{
		if (paren)
		{
			m_token->setType(Token::Type::DefFuncP);
			m_token->setString(m_input.mid(m_pos, len - 1));
			m_token->setLength(len - 1);
		}
		else  // no parentheses
		{
			m_token->setType(Token::Type::DefFuncN);
			m_token->setString(m_input.mid(m_pos, len));
			m_token->setLength(len);
		}
		m_token->setDataType(dataType);
		m_pos = pos;  // move position past defined function identifier
		return true;
	}
	if (paren)
	{
		search = ParenWord_SearchType;
	}
	else if (dataType != DataType::None)
	{
		search = DataTypeWord_SearchType;
	}
	else
	{
		search = PlainWord_SearchType;
	}
	Code code {m_table.search(search, m_input.midRef(m_pos, len))};
	if (code == Invalid_Code)
	{
		// word not found in table, therefore
		// must be variable, array, generic function, or subroutine
		// but that can't be determined here, so just return data type,
		// whether opening parenthesis is present, and string of identifier
		if (paren)
		{
			m_token->setType(Token::Type::Paren);
			m_token->setString(m_input.mid(m_pos, len - 1));
			m_token->setLength(len - 1);
		}
		else
		{
			m_token->setType(Token::Type::NoParen);
			m_token->setString(m_input.mid(m_pos, len));
			m_token->setLength(len);
		}
		m_token->setDataType(dataType);
		m_pos = pos;  // move position past word
		return true;
	}
	// found word in table (command, internal function, or operator)
	int word1 {m_pos};  // save position of first word
	m_pos = pos;  // move position past first word

	// setup token in case this is only one word
	m_table.setToken(m_token, code);
	m_token->setLength(len);

	if (m_table.multiple(code) == Multiple::OneWord)
	{
		// identifier can only be a single word
		return true;
	}

	// command could be a two word command
	skipWhitespace();
	pos = scanWord(m_pos, dataType, paren);
	int len2 {pos - m_pos};
	if (dataType != DataType::None || paren
		|| (code = m_table.search(m_input.midRef(word1, len),
		m_input.midRef(m_pos, len2))) == Invalid_Code)
	{
		if (m_token->type() == Token::Type::Error)
		{
			// first word by itself is not valid
			m_token->setString("Invalid Two Word Command");
		}
		// otherwise single word is valid command,
		// token already setup, position already set past word
		return true;
	}
	// get information from two word command
	m_table.setToken(m_token, code);
	m_token->setLength(pos - m_token->column());

	m_pos = pos;  // move position past second word
	return true;
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
//   - returns false if no number (position not changed)
//   - returns true if there is and token is filled
//   - returns true for errors and special error token is set
//   - string of the number is converted to a value
//   - string of the number is saved so it can be later reproduced

bool Parser::getNumber(void)
{
	bool digits {};				// digits were found flag
	bool decimal {};			// decimal point was found flag
	bool sign {};				// have negative sign flag (2011-03-27)

	int pos {m_pos};
	forever
	{
		if (m_input[pos].isDigit())
		{
			pos++;  // move past digit
			if (!digits)  // first digit?
			{
				digits = true;
				if (!decimal && m_input[pos - 1] == '0' && m_input[pos] != '.')
				{
					// next character not a digit (or '.')?
					if (!m_input[pos].isDigit())
					{
						break;  // single zero, exit loop to process string
					}
					else
					{
						// if didn't find a decimal point
						// and first character is a zero
						// and second character is not a decimal point,
						// and second character is a digit
						// then this is in invalid number
						setError(Status::ExpNonZeroDigit);
						return true;
					}
				}
			}
		}
		else if (m_input[pos] == '.')
		{
			if (decimal)  // was a decimal point already found?
			{
				if (!digits)  // no digits found?
				{
					setErrorLength(Status::ExpDigitsOrSngDP, 2);
					return true;
				}
				break;  // exit loop to process string
			}
			decimal = true;
			pos++;  // move past '.'
		}
		else if (m_input[pos].toUpper() == 'E')
		{
			if (!digits)
			{
				if (sign && !decimal)
				{
					// if there is a '-E' then not a number
					// (need to interprete '-' as unary operator)
					return false;
				}
				// if there were no digits before 'E' then error
				// (only would happen if mantissa contains only '.')
				setError(Status::ExpManDigits);
				return true;
			}
			pos++;  // move past 'e' or 'E'
			if (m_input[pos] == '+' || m_input[pos] == '-')
			{
				pos++;  // move past exponent sign
			}
			// now look for exponent digits
			digits = false;
			while (m_input[pos].isDigit())
			{
				pos++;  // move past exponent digit
				digits = true;
			}
			if (!digits)  // no exponent digits found?
			{
				setErrorColumn(Status::ExpExpDigits, pos);
				return true;
			}
			decimal = true;  // process as double
			break;  // exit loop to process string
		}
		else  // some other non-valid numeric character
		{
			if (!digits && !decimal)  // nothing found?
			{
				// look for negative sign
				if (!sign && m_input[pos] == '-')
				{
					pos++;  // move past negative sign
					sign = true;
				}
				else
				{
					return false;  // not a numeric constant
				}
			}
			else if (!digits)  // only a decimal point found?
			{
				setError(Status::ExpDigits);
				return true;
			}
			else
			{
				// no more valid number characters, go process what we got
				break;
			}
		}
	}
	// pos pointing to first character that is not part of constant
	bool ok;
	int len {pos - m_pos};
	QString numStr {m_input.mid(m_pos, len)};

	// save string of number so it later can be reproduced
	m_token->setString(numStr);
	m_token->setLength(len);
	m_pos = pos;  // move to next character after constant

	m_token->setType(Token::Type::Constant);

	// FIXME hack for memory issue reported against QString::toInt()/toDouble()
	QByteArray numBytes;
	numBytes.append(numStr);
	if (!decimal)  // no decimal or exponent?
	{
		// try to convert to integer first
		m_token->setValue(numBytes.toInt(&ok));
		if (ok)
		{
			m_token->setDataType(DataType::Integer);
			// convert to double in case double is needed
			m_token->setValue((double)m_token->valueInt());
			return true;
		}
		// else overflow or underflow, won't fit into an integer
		// fall thru and try as double
	}
	m_token->setValue(numBytes.toDouble(&ok));
	if (!ok)
	{
		// overflow or underflow, constant is not valid
		setErrorLength(Status::FPOutOfRange, len);
		return true;
	}

	// if double in range of integer, then set as integer
	if (m_token->value() > (double)INT_MIN - 0.5
		&& m_token->value() < (double)INT_MAX + 0.5)
	{
		m_token->setDataType(DataType::Integer);
		// convert to integer in case integer is needed
		m_token->setValue((int)m_token->value());
		if (decimal)  // decimal point or exponent?
		{
			// indicate number is a double value
			m_token->addSubCode(Double_SubCode);
		}
	}
	else  // number can't be converted to integer
	{
		m_token->setDataType(DataType::Double);
	}
	return true;
}


// function to get string constant at the current position if there is
// a double quote at current position.
//
//   - strings constants start and end with a double quote
//   - returns false if no string (position not changed)
//   - returns true if there is and token is filled
//   - copy string into token without surrounding quotes
//   - returns true for errors and special error token is set

bool Parser::getString(void)
{
	if (m_input[m_pos] != '"')
	{
		return false;  // not a sting constant
	}

	int pos {m_pos + 1};
	int len {};
	while (!m_input[pos].isNull())
	{
		if (m_input[pos] == '"')
		{
			if (m_input[++pos] != '"')  // not two in a row?
			{
				// found end of string
				// pos at character following closing quote
				break;
			}
			// otherwise quote counts as one character
		}
		m_token->setString(len++, m_input[pos++]);  // copy char into string
	}
	m_token->setType(Token::Type::Constant);
	m_token->setDataType(DataType::String);
	m_token->setLength(pos - m_pos);
	// advance position past end of string
	m_pos = pos;
	return true;
}


// function to get an operator (of symbol characters) at the current position,
// which may be one or two characters.  For the purpose of the Parser, these
// operators in the table are generally not associated with a data type
// (though the data type from the Table entry is returned, it will usually br
// None).
//
//   - returns false if no operator (position not changed)
//   - returns true if there is and token is filled
//   - returns true for errors setting special error token

bool Parser::getOperator(void)
{
	// search table for current character to see if it is a valid operator
	Code code {m_table.search(Symbol_SearchType, m_input.midRef(m_pos, 1))};
	if (code != Invalid_Code)
	{
		// current character is a valid single character operator

		// setup token in case this is only one character
		m_token->setType(m_table.type(code));
		m_token->setDataType(m_table.dataType(code));
		m_token->setCode(code);
		m_token->setLength(1);

		if (m_table.multiple(code) == Multiple::OneChar)
		{
			// operator can only be a single character
			m_pos++;  // move past operator
			if (code == RemOp_Code)
			{
				// remark requires special handling
				// remark string is to end of line
				m_token->setString(m_input.mid(m_pos));
				// move position to end of line
				m_pos += m_token->stringLength();
			}
			return true;
		}
	}
	// operator could be a two character operator
	// search table again for two characters at current position
	Code code2 {m_table.search(Symbol_SearchType, m_input.midRef(m_pos, 2))};
	if (code2 == Invalid_Code)
	{
		if (code != Invalid_Code)  // was first character a valid operator?
		{
			m_pos++;  // move past first character
			// token already setup
			return true;
		}
		return false;  // character(s) as current position not a valid operator
	}

	// valid two character operator
	m_pos += 2;  // move past two characters
	// get information from two character operator
	m_token->setType(m_table.type(code2));
	m_token->setDataType(m_table.dataType(code2));
	m_token->setCode(code2);
	m_token->setLength(2);
	return true;
}


// end: parser.cpp
