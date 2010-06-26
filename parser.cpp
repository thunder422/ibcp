// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: parser.cpp - contains code for the parser class
//	Copyright (C) 2010  Thunder422
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
//  2010-03-01  initial release
//
//  2010-03-06  split DefFunc_TokenType into DefFuncN_TokenType and
//              DefFuncP_TokenType so that it can be determined if define
//              function name has a parentheses or not
//
//  2010-03-07  in get_number() changed situation of 0 leading another digit
//                from terminating the number at the first 0 to a leading zero
//                error (no legal BASIC would have this situation;
//              changed several set_error() calls to have a length
//                argument for the new length of token error field in Token
//
//  2010-03-08  removed no closing quote check from get_string() since this
//              condition was made acceptable in scan_string()
//
//  2010-03-10  rewrote get_operator() to properly handle two character
//              operators
//
//  2010-03-11  allow tabs to be white space in skip_whitespace()
//
//  2010-03-17  modified get_token() to return special end-of-line token
//                instead of a NULL pointer
//              changed all token->code to token->index
//
//  2010-03-18  replaced code with call to set_token()
//
//  2010-03-20  set length of token field for words, two words, string
//              constants, one and two character operators
//
//  2010-03-21  FIXME removed an error check for translator simple expression
//              testing
//
//  2010-04-16  removed the unexpected character errors in scan_command() that
//              was occurring in a statements like "A,B=3" because these were
//              not valid immediate commands, so these errors were removed to
//              let the Translator process it (and report an error as necessary)
//
//  2010-05-29  corrected a problem in the get_number() function where a
//              constant of a single '0' caused "invalid leading zero in numeric
//              constant" error, the solution was to check if the next character
//              is a digit, the number is complete, otherwise an error occurs
//
//  2010-06-25  TableSearch replaced with SearchType
//

#include <stdio.h>

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "ibcp.h"


// function to get a token at the current position
//
//     - a pointer to the token if returned
//     - the token must be dellocated when it is no longer needed
//     - the token may contain an error message if an error was found

Token *Parser::get_token(void)
{
	bool first = pos == input;
	skip_whitespace();
	token = new Token(pos - input);  // allocate new token to return
	if (*pos == '\0')
	{
		// 2010-03-17: changed to return special end-of-line (last) token
		// 2010-03-18: replaced code with function call
		table->set_token(token, EOL_Code);
		return token;
	}

	if (first)
	{
		// upon first call, see if line contains an immediate command
		if (get_command())
		{
			return token;  // return immediate command or error
		}
		pos = input;  // reset position and parse line normally
		skip_whitespace();
	}

	if (!get_identifier() && !get_number() && !get_string() && !get_operator())
	{
		// not a valid token, create error token
		token->set_error("unrecognizable character");
	}
	return token;  // token may contain an error
}


// function to get an immediate command one the entire line, which
// consists of a single letter for the command followed by possible
// arguments
//
//     - returns false if no valid command is found
//     - returns true if command is found with token set to command

bool Parser::get_command(void)
{
	char letter;
	int flag;
	int num;
	CmdArgs args;

	letter = *pos;
	if (table->search(letter, Null_Flag) == -1)
	{
		return false;  // first character not a valid immediate command
	}
	pos++;  // advance past command letter
	skip_whitespace();
	int col = pos - input;  // save column in case arguments are invalid
	flag = scan_command(args);
	if (flag == Null_Flag)
	{
		// not a command
		return false;
	}
	if (flag == Error_Flag)
	{
		// an error with the command
		return true;
	}
	// search table again for the proper form
	if ((token->index = table->search(letter, flag)) == -1)
	{
		token->set_error(col, "invalid arguments for command");
	}
	else
	{
		token->type = ImmCmd_TokenType;
		// if string argument, then leave datatype = String (string already set)
		if (flag != String_Flag)
		{
			// store command arguments in string of token
			token->datatype = CmdArgs_DataType;
			token->string = new String(&args, sizeof(args));
		}
	}
	return true;
}


// function to scan for the arguments of a command, which can take one
// of the following forms:
//
//                              nothing (Blank_Flag)
//     "sss"                    string constant (String_Flag)
//     xxx                      single line number (Line_Flag)
//     -  xxx-  -yyy  xxx-yyy   line number range (Range_Flag)
//     ,  xxx,  ,zz  xxx,zz     line number/increment (LineIncr_Flag)
//     xxx-yyy,                 line number range/start/increment
//     xxx-yyy,zz               plus all forms a range listed above
//     xxx-yyy,nnn,             for each
//     xxx-zzz,nnn,zz           (RangeIncr_Flag)
//
// white space is allowed between any of the parts; an error is returned
// when the line could not be a valid BASIC immediate command 
//
//     - returns Null_Flag if no valid command is found
//     - returns Error_Flag with token set to error if error found
//     - returns xxx_Flag when a valid command is found

int Parser::scan_command(CmdArgs &args)
{
	const char *UnexpChr  = "unexpected character";
	const char *BadLineNo = "invalid line number";
	const char *BadIncr   = "invalid increment";
	const char *ZeroIncr  = "zero increment invalid";
	int flag;
	int num;
	int col;
	char *end;

	if (*pos == '\0')  // end of line?
	{
		return Blank_Flag;
	}

	if (get_string())  // is there a string constant next?
	{
		// token is filled with string constant
		skip_whitespace();
		if (*pos != '\0')  // oops, something else on line
		{
			delete token->string;  // release the string constant
			token->set_error(pos - input, UnexpChr);
			return Error_Flag;
		}
		return String_Flag;
	}

	// look for line number or begin line number of range
	flag = Null_Flag;
	if (isdigit(*pos))
	{
		num = strtol(pos, &end, 10);  // (base 10)
		if (errno == ERANGE)
		{
			errno = 0;
			token->set_error(pos - input, BadLineNo, end - pos);
			return Error_Flag;  // let rest of parser process it
		}
		flag = Line_Flag;
		args.begin = num;
		pos = end;  // advance past number
		skip_whitespace();
		if (*pos == '\0')
		{
			return flag;
		}
	}

	// look for range
	if (*pos == '-')
	{
		flag = Range_Flag;
		pos++;  // advance past dash
		args.end = INT_MAX;  // default end if no end line number
		skip_whitespace();
		if (*pos == '\0')
		{
			return flag;
		}
		if (isdigit(*pos))
		{
			num = strtol(pos, &end, 10);  // (base 10)
			if (errno == ERANGE)
			{
				errno = 0;
				token->set_error(pos - input, BadLineNo, end - pos);
				return Error_Flag;
			}
			args.end = num;
			pos = end;  // advance past number
			skip_whitespace();
			if (*pos == '\0')
			{
				return flag;
			}
		}
	}

	// flag is Null (nothing found so far), Line (number) or Range ('-')
	if (*pos != ',')
	{
		if (flag != Range_Flag)
		{
			// not a range (just line number could be valid, e.g. L100=5)
			// - it's probably not a command
			return Null_Flag;  // let rest of parser process it
		}
		// otherwise, it's probably a malformed command, so return an error
		// (no valid BASIC immediate command starts this way)
		return Null_Flag;  // 2010-03-21: TODO temporarily ignore this
		//TODO token->set_error(pos - input, UnexpChr);
		//TODO return Error_Flag;
	}
	pos++;  // advance past comma
	flag = flag != Range_Flag ? LineIncr_Flag : RangeIncr_Flag;
	skip_whitespace();
	if (*pos == '\0')
	{
		args.incr = 0;  // no increment specified, command will use a default
		return flag;
	}

	// number (increment or start line) must be next
	if (!isdigit(*pos))  // make sure it's a digit
	{
		// 2010-04-16: this could be a valid multiple assignment, error removed
		return Null_Flag;
		//token->set_error(pos - input, UnexpChr);
		//return Error_Flag;
	}
	num = strtol(pos, &end, 10);  // (base 10)
	col = pos - input;  // save column in case number is invalid
	if (errno == ERANGE)  // bad number?
	{
		errno = 0;
		token->set_error(col, *end == '\0' ? BadIncr : BadLineNo, end - pos);
		return Error_Flag;
	}
	if (end == pos)  // no number?
	{
		if (flag == Range_Flag || flag == RangeIncr_Flag)
		{
			// already have range, so this would probably be an invalid command
			token->set_error(col, UnexpChr);
			return Error_Flag;
		}
		// could be a multiple assignment, let rest of parser process it
		return Null_Flag;
	}
	pos = end;  // advance past number

	// number could be increment or start line number
	skip_whitespace();
	if (*pos == '\0')
	{
		// value is increment
		if (num == 0)
		{
			token->set_error(col, ZeroIncr);
			return Error_Flag;
		}
		args.incr = num;
		return flag;
	}

	// for LineIncr, there should be no more characters
	if (flag == LineIncr_Flag || *pos != ',')
	{
		// 2010-04-16: let translator report error, so no error here
		return Null_Flag;
		//token->set_error(pos - input, UnexpChr);
		//return Error_Flag;
	}
	pos++;  // advance past second comma
	// current number is start line, look for increment
	args.start = num;
	skip_whitespace();
	if (*pos == '\0')
	{
		args.incr = 0;  // no increment specified, command will use a default
		return flag;
	}
	// number (increment) must be next
	if (!isdigit(*pos))  // make sure it's a digit
	{
		token->set_error(pos - input, UnexpChr);
		return Error_Flag;
	}
	num = strtol(pos, &end, 10);  // (base 10)
	col = pos - input;  // save column in case number is invalid
	if (errno == ERANGE || end == pos)  // bad or no number?
	{
		errno = 0;
		token->set_error(col, BadIncr, end - pos);
		return Error_Flag;
	}
	if (num == 0)
	{
		token->set_error(col, ZeroIncr);
		return Error_Flag;
	}
	args.incr = num;
	pos = end;  // advance past number
	skip_whitespace();
	// make sure there's nothing else on line
	if (*pos != '\0')
	{
		token->set_error(pos - input, UnexpChr);
		return Error_Flag;
	}
	return flag;
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

bool Parser::get_identifier(void)
{
	DataType datatype;		// data type of word
	bool paren;				// word has opening parenthesis flag
	SearchType search;		// table search type

	char *p = scan_word(pos, datatype, paren);
	if (p == NULL)
	{
		return false;  // not an identifier
	}

	int len = p - pos;
	if (strncasecmp(pos, "FN", 2) == 0)  // defined function?
	{
		// 2010-03-06: split DefFunc_TokenType
		token->type = paren ? DefFuncP_TokenType : DefFuncN_TokenType;
		token->datatype = datatype;
		token->string = new String(pos, p);
		pos = p;  // move position past defined function identifier
		return true;
	}
	if (paren)
	{
		search = ParenWord_SearchType;
	}
	else if (datatype != None_DataType)
	{
		search = DataTypeWord_SearchType;
	}
	else
	{
		search = PlainWord_SearchType;
	}
	int index = table->search(search, pos, len);
	if (index < 0)
	{
		// word not found in table, therefore
		// must be variable, array, generic function, or subroutine
		// but that can't be determined here, so just return data type,
		// whether opening parenthesis is present, and string of identifier
		token->type = paren ? Paren_TokenType : NoParen_TokenType;
		token->datatype = datatype;
		token->string = new String(pos, len);
		pos = p;  // move position past word
		return true;
	}
	// found word in table (command, internal function, or operator)
	char *word1 = pos;  // save position of first word
	pos = p;  // move position past first word

	// setup token in case this is only one word
	token->type = table->type(index);
	token->datatype = table->datatype(index);
	token->string = NULL;  // string not needed
	token->index = index;
	token->length = len;  // 2010-03-20: set length of token

	if (table->multiple(index) == OneWord_Multiple)
	{
		// identifier can only be a single word
		if (table->code(index) == Rem_Code)
		{
			// remark requires special handling
			// remark string is to end of line
			len = strlen(pos);
			token->string = new String(pos, len);
			pos += len;  // move position to end of line
		}
		return true;
	}

	// command could be a two word command
	skip_whitespace();
	p = scan_word(pos, datatype, paren);
	if (datatype != None_DataType || paren
		|| (index = table->search(word1, len, pos, p - pos)) < 0)
	{
		if (token->type == Error_TokenType)
		{
			// first word by itself is not valid
			token->string = new String("Invalid Two Word Command");
		}
		// otherwise single word is valid command,
		// token already setup, position already set past word
		return true;
	}
	// get information from two word command
	token->type = table->type(index);
	token->datatype = table->datatype(index);
	token->index = index;
	token->length += p - pos + 1;  // 2010-03-20: set length of token

	// 2010-03-20: moved to here so pos can be used to set length
	pos = p;  // move position past second word
	return true;
}


// function to get a word at the position specified
//
//     - returns NULL if there is not an identifier at point
//     - returns pointer to character after identifier
//     - returns data type found or None if none was found
//     - returns flag if opening parenthesis at end of identifier

char *Parser::scan_word(char *p, DataType &datatype, bool &paren)
{
	if (!isalpha(*p))
	{
		return NULL;  // not an identifier
	}
	do
	{
		p++;
	}
	while (isalnum(*p) || *p == '_');
	// p now points to non-alnum or '_'
	
	// see if there is a data type symbol next
	switch (*p)
	{
	case '%':
		datatype = Integer_DataType;
		p++;
		break;
	case '$':
		datatype = String_DataType;
		p++;
		break;
	case '#':
		datatype = Double_DataType;
		p++;
		break;
	default:
		datatype = None_DataType;
	}

	// see if there is an opening parenthesis
	if (*p == '(')
	{
		paren = true;
		p++;
	}
	else
	{
		paren = false;
	}

	// p now points to next character after identifier
	return p;
}


// function to skip white space at the specified position
//
//     - for now the only white space is a space character
//     - returns pointer to next non-white space character
//     - if no white space found then argument is returned

void Parser::skip_whitespace(void)
{
	// 2010-03-11: replaced space check with isblank() to allow tabs
	while (isblank(*pos))
	{
		pos++;
	}
}


// function to get numeric constant at the current position, which may
// be an integer or double constant.  If the value does not contain a
// decimal point or an exponent, and it fits into a 32-bit integer, then
// it will be returned as an integer, otherwise is will be returned as a 
// double.
//
//     - numbers starting with zero must be followed by a decimal point
//     - 
//     - returns false if no number (position not changed)
//     - returns true if there is and token is filled
//     - returns true for errors and special error token is set
//     - string of the number is converted to a value
//     - string of the number is saved so it can be later reproduced

bool Parser::get_number(void)
{
	bool digits = false;		// digits were found flag
	bool decimal = false;		// decimal point was found flag

	char *p = pos;
	for (;;)
	{
		if (isdigit(*p))
		{
			p++;  // move past digit
			if (!digits)  // first digit?
			{
				digits = true;
				if (!decimal && *(p - 1) == '0' && *p != '.')
				{
					// 2010-05-29: added check for single zero digit
					if (!isdigit(*p))  // next char not a digit (or '.')?
					{
						break;  // single zero, exit loop to process string
					}
					else
					{
						// if didn't find a decimal point
						// and first character is a zero
						// and second character is not a decimal point,
						// and second character is a digit (2010-05-29)
						// this is in invalid number
						// 2010-03-07: changed to error
						token->set_error("invalid leading zero in numeric "
							"constant");
						return true;
					}
				}
			}
		}
		else if (*p == '.')
		{
			if (decimal)  // was a decimal point already found?
			{
				if (!digits)  // no digits found?
				{
					token->set_error("constant only contains a decimal point "
						"or has two decimal points", 2);
					return true;
				}
				break;  // exit loop to process string
			}
			decimal = true;
			p++;  // move past '.'
		}
		else if (*p == 'e' || *p == 'E')
		{
			if (!digits)
			{
				// if there were no digits before 'E' then error
				// (only would happen if mantissa contains only '.')
				token->set_error("mantissa of constant only contains a "
					"decimal point");
				return true;
			}
			p++;  // move past 'e' or 'E'
			if (*p == '+' || *p == '-')
			{
				p++;  // move past exponent sign
			}
			// now look for exponent digits
			digits = false;
			while (isdigit(*p))
			{
				p++;  //move past exponent digit
				digits = true;
			}
			if (!digits)  // no exponent digits found?
			{
				token->set_error(p - input, "exponent contains no digits");
				return true;
			}
			decimal = true;  // process as double
			break;  // exit loop to process string
		}
		else  // some other non-valid numeric character
		{
			if (!digits && !decimal)  // nothing found?
			{
				return false;  // not a numeric constant
			}
			if (!digits)  // only a decimal point found?
			{
				token->set_error("constant only contains a decimal point");
				return true;
			}
			break;  // no more valid number characters, go process what we got
		}
	}
	// p pointing to first character that is not part of constant
	char *end;
	errno = 0;
	if (!decimal)  // no decimal or exponent?
	{
		// try to convert to integer first
		token->int_value = strtol(pos, &end, 10);  // (base 10)
		if (errno == ERANGE)
		{
			// overflow or underflow, won't fit into an integer
			errno = 0;  // reset error number
			decimal = true;  // try as double
		}
		else
		{
			token->type = Constant_TokenType;
			token->datatype = Integer_DataType;
		}
	}
	if (decimal)
	{
		token->dbl_value = strtod(pos, &end);
		if (errno == ERANGE)
		{
			// overflow or underflow, constant is not valid
			errno = 0;  // reset error number
			token->set_error("constant is out of range", end - pos);
			return true;
		}
		token->type = Constant_TokenType;
		token->datatype = Double_DataType;
	}
	// check if end of constant is where it should be
	if (end != p)
	{
		// point to where error actual is
		token->set_error(end - input, "BUG: error in constant");
		return true;
	}
	// create string of number so it later can be reproduced
	token->string = new String(pos, p);
	pos = p;  // move to next character after constant
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

bool Parser::get_string(void)
{
	if (*pos != '"')
	{
		return false;  // not a sting constant
	}

	char *p = pos + 1;
	int len = scan_string(p, NULL);  // get length of string
	// 2010-03-08: removed no closing quote error check - now valid
	token->type = Constant_TokenType;
	token->datatype = String_DataType;
	token->string = new String(len);  // allocate space only
	token->length = p - pos;  // 2010-03-19: set length of token
	// advance pos past quote, pos will point after string upon return
	scan_string(++pos, token->string);  // copy string to token
	return true;
}


// support function to scan for a string constant, without a string
// argument, the string characters are counted only, otherwise the
// characters of the string constant are copied into the string
//
//     - if string pointer argument is NULL then characters counted only
//     - two double quotes together places a double quote within string
//     - the character pointer argument is left pointing after string
//     - returns the number of characters in string constant
//     - the end of the line also terminates the string

int Parser::scan_string(char *&p, String *s)
{
	int count = 0;
	while (*p != '\0')
	{
		if (*p == '"')
		{
			if (*++p != '"')  // not two in a row?
			{
				// found end of string
				// p at character following closing quote
				break;
			}
			// otherwise quote counts as one character
		}
		if (s != NULL)  // copy to string?
		{
			s->set(count, *p);  // copy character into string
		}
		count++;
		p++;
	}
	// if here then reached end of line with no closing quote
	return count;
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
//
// 2010-03-10: rewrote to properly handle two character operators

bool Parser::get_operator(void)
{
	// search table for current character to see if it is a valid operator
	int index = table->search(Symbol_SearchType, pos, 1);
	if (index > 0)
	{
		// current character is a valid single character operator

		// setup token in case this is only one character
		token->type = table->type(index);
		token->datatype = table->datatype(index);
		token->index = index;
		token->length = 1;  // 2010-03-20: set length of token

		if (table->multiple(index) == OneChar_Multiple)
		{
			// operator can only be a single character
			pos++;  // move past operator
			if (table->code(index) == RemOp_Code)
			{
				// remark requires special handling
				// remark string is to end of line
				int len = strlen(pos);
				token->string = new String(pos, len);
				pos += len;  // move position to end of line
			}
			return true;
		}
	}
		
	// operator could be a two character operator
	// search table again for two characters at current position
	int index2 = table->search(Symbol_SearchType, pos, 2);
	if (index2 < 0)
	{
		if (index > 0)  // was first character a valid operator?
		{
			pos++;  // move past first character
			// token already setup
			return true;
		}
		return false;  // character(s) as current position not a valid operator
	}

	// valid two character operator
	pos += 2;  // move past two characters
	// get information from two character operator
	token->type = table->type(index2);
	token->datatype = table->datatype(index2);
	token->index = index2;
	token->length = 2;  // 2010-03-20: set length of token
	return true;
}


// end: parser.cpp
