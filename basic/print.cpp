// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: print.cpp - print command functions source file
//	Copyright (C) 2013  Thunder422
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
//	2013-08-03	initial version

#include "basic/basic.h"
#include "recreator.h"
#include "table.h"
#include "token.h"
#include "translator.h"


// PRINT command translate function
Token::Status printTranslate(Translator &translator, Token *commandToken,
	Token *&token)
{
	Token::Status status;
	Token *lastSemiColon = NULL;
	bool separator = false;
	bool printFunction = false;

	forever
	{
		if ((status = translator.getExpression(token, DataType::None))
			!= Token::Status::Done)
		{
			if (status == Token::Status::Parser
				&& token->isDataType(DataType::None))
			{
				if (translator.doneStackEmpty())
				{
					status = Token::Status::ExpExprCommaPfnOrEnd;
				}
				// change parser error if not inside paren
				else if (translator.doneStackTopToken()
					->isDataType(DataType::None))
				{
					status = Token::Status::ExpSemiCommaOrEnd;
				}
				else  // not a print function
				{
					status = Token::Status::ExpOpSemiCommaOrEnd;
				}
			}
			break;
		}

		if (!translator.doneStackEmpty())
		{
			if (translator.doneStackTopToken()->isDataType(DataType::None))
			{
				translator.doneStackDrop();  // print function
				printFunction = true;
			}
			else  // append appropriate print code for done stack top item
			{
				Token *printToken = translator.table().newToken(PrintDbl_Code);
				translator.processFinalOperand(printToken);
				printFunction = false;
			}
			separator = true;
			delete lastSemiColon;
			lastSemiColon = NULL;
		}

		if (token->isCode(Comma_Code))
		{
			if (lastSemiColon != NULL)
			{
				status = Token::Status::ExpExprPfnOrEnd;
				break;
			}
			translator.outputAppend(token);
			delete lastSemiColon;
			lastSemiColon = NULL;
		}
		else if (token->isCode(SemiColon_Code))
		{
			if (!separator)
			{
				status = lastSemiColon == NULL
					? Token::Status::ExpExprCommaPfnOrEnd
					: Token::Status::ExpExprPfnOrEnd;
				break;
			}
			delete lastSemiColon;
			lastSemiColon = token;
		}
		else  // some other token, maybe end-of-statement
		{
			break;  // exit loop
		}
		separator = false;
		token = NULL;
	}

	if (status != Token::Status::Done)
	{
		delete lastSemiColon;
		delete commandToken;
		return status;
	}

	if (lastSemiColon != NULL)
	{
		// append last semicolon token as command token
		delete commandToken;
		commandToken = lastSemiColon;
	}
	translator.outputAppend(commandToken);

	if (!translator.table().hasFlag(token, EndStmt_Flag))
	{
		return printFunction ? Token::Status::ExpSemiCommaOrEnd
			: Token::Status::ExpOpSemiCommaOrEnd;
	}
	return Token::Status::Done;
}


// function to recreate the print item code
void printItemRecreate(Recreator &recreator, RpnItem *)
{
	QString string;

	if (recreator.separatorIsSet())
	{
		// append the previous separator
		string = recreator.separator();

		// FLAG option: space between print commas (default=no)
		if (!recreator.separatorIsSet(' '))
		{
			// if it is not a space (comma) then append a space
			// FLAG option: space after print semicolons (default=yes)
			string.append(' ');
		}
	}
	// pop the string on top of the stack and append it to the string
	string.append(recreator.pop());

	if (recreator.stackIsEmpty())
	{
		// if nothing else on the stack then push the string
		recreator.push(string);
	}
	else  // append the string to the string on top of the stack
	{
		recreator.topAppend(string);
	}

	recreator.setSeparator(';');  // set separator for next item
}


// function to recreate the print comma code
void printCommaRecreate(Recreator &recreator, RpnItem *)
{
	QString string;

	// get string on top of the stack if there is one
	if (!recreator.stackIsEmpty())
	{
		string = recreator.pop();
	}

	// append comma to string and push it back to the stack
	// FLAG option: space after print commas (default=no)
	string.append(',');
	recreator.push(string);

	// set separator to space (used to not add spaces 'between' commas)
	recreator.setSeparator(' ');
}


// function to recreate a print function code
void printFunctionRecreate(Recreator &recreator, RpnItem *rpnItem)
{
	// process as internal function first then as an item
	internalFunctionRecreate(recreator, rpnItem);
	printItemRecreate(recreator, rpnItem);
}


// function to recreate the print semicolon code
void printSemicolonRecreate(Recreator &recreator, RpnItem *rpnItem)
{
	// push string on top of stack with final semicolon then recreate command
	recreator.push(recreator.pop() + ';');
	printRecreate(recreator, rpnItem);
}


// function to recreate the print code
void printRecreate(Recreator &recreator, RpnItem *rpnItem)
{
	Q_UNUSED(rpnItem)

	// append PRINT keyword
	recreator.append(recreator.table().name(Print_Code));

	// if stack is not empty then append space with string on top of stack
	if (!recreator.stackIsEmpty())
	{
		// FLAG option: all spaces after commands (default=yes)
		recreator.append(" ");
		recreator.append(recreator.pop());
	}

	recreator.clearSeparator();  // clear separator for next command
}


// end: print.cpp
