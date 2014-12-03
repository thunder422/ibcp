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
void printTranslate(Translator &translator)
{
	TokenPtr commandToken = translator.moveToken();  // save command token
	bool printFunction {};
	bool separator {};
	TokenPtr lastSemiColon;
	for (;;)
	{
		try
		{
			translator.getExpression(DataType::None);
		}
		catch (TokenError &error)
		{
			if (error(Status::UnknownToken))
			{
				if (translator.doneStackEmpty())
				{
					error = Status::ExpExprCommaPfnOrEnd;
				}
				// change parser error if not inside paren
				else if (translator.doneStackTopToken()
					->isDataType(DataType::None))
				{
					error = Status::ExpSemiCommaOrEnd;
				}
				else  // not a print function
				{
					error = Status::ExpOpSemiCommaOrEnd;
				}
			}
			throw;
		}

		if (!translator.doneStackEmpty())
		{
			if (translator.doneStackTopToken()->isDataType(DataType::None))
			{
				translator.doneStackPop();  // print function
				printFunction = true;
			}
			else  // append appropriate print code for done stack top item
			{
				TokenPtr printToken
					= translator.table().newToken(PrintDbl_Code);
				translator.processFinalOperand(printToken);
				printFunction = false;
			}
			separator = true;
			lastSemiColon.reset();  // don't need last semicolon token
		}

		if (translator.token()->isCode(Comma_Code))
		{
			if (lastSemiColon)
			{
				throw TokenError {Status::ExpExprPfnOrEnd, translator.token()};
			}
			translator.outputAppend(translator.moveToken());
		}
		else if (translator.token()->isCode(SemiColon_Code))
		{
			if (!separator)
			{
				throw TokenError {lastSemiColon ? Status::ExpExprPfnOrEnd
					: Status::ExpExprCommaPfnOrEnd, translator.token()};
			}
			lastSemiColon = translator.moveToken();  // save semicolon token
		}
		else  // some other token, maybe end-of-statement
		{
			if (!translator.table().hasFlag(translator.token(), EndStmt_Flag))
			{
				throw TokenError {printFunction ? Status::ExpSemiCommaOrEnd
					: Status::ExpOpSemiCommaOrEnd, translator.token()};
			}
			break;  // exit loop
		}
		separator = false;
	}

	// append last semicolon token if set or command token
	// (other will be deleted when function ends)
	translator.outputAppend(lastSemiColon
		? std::move(lastSemiColon) : std::move(commandToken));
}


// function to recreate the print item code
void printItemRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	(void)rpnItem;

	std::string string;

	if (recreator.separatorIsSet())
	{
		// append the previous separator
		string = recreator.separator();

		// FLAG option: space between print commas (default=no)
		if (!recreator.separatorIsSet(' '))
		{
			// if it is not a space (comma) then append a space
			// FLAG option: space after print semicolons (default=yes)
			string += ' ';
		}
	}
	// pop the string on top of the stack and append it to the string
	string.append(recreator.popString());

	if (recreator.empty())
	{
		// if nothing else on the stack then push the string
		recreator.emplace(string);
	}
	else  // append the string to the string on top of the stack
	{
		recreator.topAppend(std::move(string));
	}

	recreator.setSeparator(';');  // set separator for next item
}


// function to recreate the print comma code
void printCommaRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	(void)rpnItem;

	std::string string;

	// get string on top of the stack if there is one
	if (!recreator.empty())
	{
		string = recreator.popString();
	}

	// append comma to string and push it back to the stack
	// FLAG option: space after print commas (default=no)
	string += ',';
	recreator.emplace(string);

	// set separator to space (used to not add spaces 'between' commas)
	recreator.setSeparator(' ');
}


// function to recreate a print function code
void printFunctionRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	// process as internal function first then as an item
	internalFunctionRecreate(recreator, rpnItem);
	printItemRecreate(recreator, rpnItem);
}


// function to recreate the print semicolon code
void printSemicolonRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	// push string on top of stack with final semicolon then recreate command
	recreator.topAppend(';');
	printRecreate(recreator, rpnItem);
}


// function to recreate the print code
void printRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	(void)rpnItem;

	// append PRINT keyword
	recreator.append(recreator.table().name(Print_Code).toStdString());

	// if stack is not empty then append space with string on top of stack
	if (!recreator.empty())
	{
		// FLAG option: all spaces after commands (default=yes)
		recreator.append(' ');
		recreator.append(recreator.popString());
	}

	recreator.clearSeparator();  // clear separator for next command
}


// end: print.cpp
