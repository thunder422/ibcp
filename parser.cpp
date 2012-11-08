// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: parser.cpp - contains code for the parser class
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
//	2010-03-01	initial release
//
//	2010-03-06	split DefFunc_TokenType into DefFuncN_TokenType and
//				DefFuncP_TokenType so that it can be determined if define
//				function name has a parentheses or not
//
//	2010-03-07	in get_number() changed situation of 0 leading another digit
//				  from terminating the number at the first 0 to a leading zero
//				  error (no legal BASIC would have this situation;
//				changed several set_error() calls to have a length
//				  argument for the new length of token error field in Token
//
//	2010-03-08	removed no closing quote check from get_string() since this
//				condition was made acceptable in scan_string()
//
//	2010-03-10	rewrote get_operator() to properly handle two character
//				operators
//
//	2010-03-11	allow tabs to be white space in skip_whitespace()
//
//	2010-03-17	modified get_token() to return special end-of-line token
//				  instead of a NULL pointer
//				changed all token->code to token->index
//
//	2010-03-18	replaced code with call to set_token()
//
//	2010-03-20	set length of token field for words, two words, string
//				constants, one and two character operators
//
//	2010-03-21	FIXME removed an error check for translator simple expression
//				testing
//
//	2010-04-16	removed the unexpected character errors in scan_command() that
//				was occurring in a statements like "A,B=3" because these were
//				not valid immediate commands, so these errors were removed to
//				let the Translator process it (and report an error as necessary)
//
//	2010-05-29	corrected a problem in the get_number() function where a
//				constant of a single '0' caused "invalid leading zero in numeric
//				constant" error, the solution was to check if the next character
//				is a digit, the number is complete, otherwise an error occurs
//
//	2010-06-25	TableSearch replaced with SearchType
//
//	2011-01-11	set token length for identifiers and number constants
//
//	2011-02-26	changed all table index to code, search() now returns code
//
//	2011-03-14	also set token length for FN identifiers
//
//	2011-03-26	modified get_identifier() to not store the open parentheses of
//				  DefFuncP and Paren token types
//	2011-03-27	modified get_number() to look for a negative sign in front of
//				  constants if the new operand_state flag is set
//	2011-06-07	added checks for maximum integer in scan_command() and
//				get_number(), necessary when compiling on with 64-bit
//	2012-10-31	added 'const' to 'char *' variables (input not modified)
//	2012-11-01	removed immediate command parsing
//				replaced String, char processing with QString and QChar
//				renamed variables and functions to Qt style naming

#include "parser.h"
#include "table.h"


// function to get a token at the current position
//
//     - a pointer to the token is returned
//     - the token must be deallocated when it is no longer needed
//     - the token may contain an error message if an error was found

Token *Parser::getToken(void)
{
	skipWhitespace();
	m_token = new Token(m_pos);  // allocate new token to return
	if (m_input[m_pos].isNull())
	{
		m_table.setToken(m_token, EOL_Code);
		return m_token;
	}

	if (!getIdentifier() && !getNumber() && !getString() && !getOperator())
	{
		// not a valid token, create error token
		m_token->setError("unrecognizable character");
	}
	return m_token;  // token may contain an error
}


// function to get an identifier at the current position, which may
// be a command (one or two words), internal function, operator, defined
// function, an identifier with a parenthesis, or an identifier with no
// parenthesis.  The internal function, defined function or identifier
// will have a data type
//
//     - returns false if no identifier (position not changed)
//     - returns true if there is and token is filled
//     - returns true for errors setting special error token

bool Parser::getIdentifier(void)
{
	DataType dataType;		// data type of word
	bool paren;				// word has opening parenthesis flag
	SearchType search;		// table search type

	int pos = scanWord(m_pos, dataType, paren);
	if (pos == -1)
	{
		return false;  // not an identifier
	}

	int len = pos - m_pos;
	// defined function?
	if (m_input.midRef(m_pos).startsWith("FN", Qt::CaseInsensitive))
	{
		if (paren)
		{
			m_token->setType(DefFuncP_TokenType);
			m_token->setString(m_input.mid(m_pos, len - 1));
			m_token->setLength(len - 1);
		}
		else  // no parentheses
		{
			m_token->setType(DefFuncN_TokenType);
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
	else if (dataType != None_DataType)
	{
		search = DataTypeWord_SearchType;
	}
	else
	{
		search = PlainWord_SearchType;
	}
	Code code = m_table.search(search, m_input.midRef(m_pos, len));
	if (code == Invalid_Code)
	{
		// word not found in table, therefore
		// must be variable, array, generic function, or subroutine
		// but that can't be determined here, so just return data type,
		// whether opening parenthesis is present, and string of identifier
		if (paren)
		{
			m_token->setType(Paren_TokenType);
			m_token->setString(m_input.mid(m_pos, len - 1));
			m_token->setLength(len - 1);
		}
		else
		{
			m_token->setType(NoParen_TokenType);
			m_token->setString(m_input.mid(m_pos, len));
			m_token->setLength(len);
		}
		m_token->setDataType(dataType);
		m_pos = pos;  // move position past word
		return true;
	}
	// found word in table (command, internal function, or operator)
	int word1 = m_pos;  // save position of first word
	m_pos = pos;  // move position past first word

	// setup token in case this is only one word
	m_token->setType(m_table.type(code));
	m_token->setDataType(m_table.dataType(code));
	m_token->setCode(code);
	m_token->setLength(len);

	if (m_table.multiple(code) == OneWord_Multiple)
	{
		// identifier can only be a single word
		if (code == Rem_Code)
		{
			// remark requires special handling
			// remark string is to end of line
			m_token->setString(m_input.mid(m_pos));
			m_pos += m_token->stringLength();
		}
		return true;
	}

	// command could be a two word command
	skipWhitespace();
	pos = scanWord(m_pos, dataType, paren);
	int len2 = pos - m_pos;
	if (dataType != None_DataType || paren
		|| (code = m_table.search(m_input.midRef(word1, len),
		m_input.midRef(m_pos, len2))) == Invalid_Code)
	{
		if (m_token->type() == Error_TokenType)
		{
			// first word by itself is not valid
			m_token->setString("Invalid Two Word Command");
		}
		// otherwise single word is valid command,
		// token already setup, position already set past word
		return true;
	}
	// get information from two word command
	m_token->setType(m_table.type(code));
	m_token->setDataType(m_table.dataType(code));
	m_token->setCode(code);
	m_token->setLength(len2 + 1);

	m_pos = pos;  // move position past second word
	return true;
}


// function to get a word at the position specified
//
//     - returns -1 if there is not an identifier at point
//     - returns index to character after identifier
//     - returns data type found or None if none was found
//     - returns flag if opening parenthesis at end of identifier

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
		dataType = Integer_DataType;
		pos++;
		break;
	case '$':
		dataType = String_DataType;
		pos++;
		break;
	case '#':
		dataType = Double_DataType;
		pos++;
		break;
	default:
		dataType = None_DataType;
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
//     - for now the only white space is a space character
//     - returns pointer to next non-white space character
//     - if no white space found then argument is returned

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
//     - numbers starting with zero must be followed by a decimal point
//     - returns false if no number (position not changed)
//     - returns true if there is and token is filled
//     - returns true for errors and special error token is set
//     - string of the number is converted to a value
//     - string of the number is saved so it can be later reproduced

bool Parser::getNumber(void)
{
	bool digits = false;		// digits were found flag
	bool decimal = false;		// decimal point was found flag
	bool sign = false;			// have negative sign flag (2011-03-27)

	int pos = m_pos;
	for (;;)
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
						m_token->setError("invalid leading zero in numeric "
							"constant");
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
					m_token->setError("constant only contains a decimal point "
						"or has two decimal points", 2);
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
				// if there were no digits before 'E' then error
				// (only would happen if mantissa contains only '.')
				m_token->setError("mantissa of constant only contains a "
					"decimal point");
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
				m_token->setError(pos, "exponent contains no digits");
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
				if (m_operandState && !sign && m_input[pos] == '-')
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
				m_token->setError("constant only contains a decimal point");
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
    int len = pos - m_pos;
	QString numStr = m_input.mid(m_pos, len);
	// FIXME hack for memory issue reported against QString::toInt()/toDouble()
	QByteArray numBytes;
	numBytes.append(numStr);
	if (!decimal)  // no decimal or exponent?
	{
		// try to convert to integer first
		m_token->setValue(numBytes.toInt(&ok));
		if (!ok)
		{
			// overflow or underflow, won't fit into an integer
			decimal = true;  // try as double
		}
		else
		{
			m_token->setType(Constant_TokenType);
			m_token->setDataType(Integer_DataType);
		}
	}
	if (decimal)
	{
		m_token->setValue(numBytes.toDouble(&ok));
		if (!ok)
		{
			// overflow or underflow, constant is not valid
			m_token->setError("constant is out of range", len);
			return true;
		}
		m_token->setType(Constant_TokenType);
		m_token->setDataType(Double_DataType);
	}
	// save string of number so it later can be reproduced
	m_token->setString(numStr);
	m_token->setLength(len);
	m_pos = pos;  // move to next character after constant
	return true;
}


// function to get string constant at the current position if there is
// a double quote at current position.
//
//     - strings constants start and end with a double quote
//     - returns false if no string (position not changed)
//     - returns true if there is and token is filled
//     - copy string into token without surrounding quotes
//     - returns true for errors and special error token is set

bool Parser::getString(void)
{
	if (m_input[m_pos] != '"')
	{
		return false;  // not a sting constant
	}

	int pos = m_pos + 1;
	int len = 0;
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
	m_token->setType(Constant_TokenType);
	m_token->setDataType(String_DataType);
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
//     - returns false if no operator (position not changed)
//     - returns true if there is and token is filled
//     - returns true for errors setting special error token

bool Parser::getOperator(void)
{
	// search table for current character to see if it is a valid operator
	Code code = m_table.search(Symbol_SearchType, m_input.midRef(m_pos, 1));
	if (code != Invalid_Code)
	{
		// current character is a valid single character operator

		// setup token in case this is only one character
		m_token->setType(m_table.type(code));
		m_token->setDataType(m_table.dataType(code));
		m_token->setCode(code);
		m_token->setLength(1);

		if (m_table.multiple(code) == OneChar_Multiple)
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
	Code code2 = m_table.search(Symbol_SearchType, m_input.midRef(m_pos, 2));
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
