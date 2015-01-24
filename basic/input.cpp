// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: input.cpp - input command functions source file
//	Copyright (C) 2013-2015  Thunder422
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
//	2013-08-17	initial version

#include "recreator.h"
#include "table.h"
#include "token.h"
#include "translator.h"


// INPUT command translate function
void inputTranslate(Translator &translator)
{
	TokenPtr commandToken = translator.moveToken();  // save command token
	TokenPtr token;
	Table *entry = commandToken->alternate(0);
	if (commandToken->name2().empty())
	{
		token = std::make_shared<Token>(entry);
	}
	else  // Input Prompt
	{
		try
		{
			translator.getExpression(DataType::String);
		}
		catch (TokenError &error)
		{
			if (error(Status::UnknownToken))
			{
				error = Status::ExpSemiOrComma;
			}
			throw;
		}
		translator.doneStackPop();
		token = translator.moveToken();  // take next token
		if (token->isCode(Code::Comma))
		{
			token->addSubCode(Option_SubCode);
		}
		else if (!token->isCode(Code::Semicolon))
		{
			throw TokenError {Status::ExpOpSemiOrComma, token};
		}
		token->setTableEntry(entry);  // reuse token
	}

	// append input begin and save iterator where to insert input parse codes
	auto insertPoint = translator.outputAppendIterator(std::move(token));

	// loop to read input variables
	bool done;
	do
	{
		// get variable reference
		// (does not return false - returns error for reference)
		translator.getOperand(DataType::Any, Reference::Variable);

		// get and check next token
		translator.getToken(Status::ExpCommaSemiOrEnd);

		if (translator.token()->isCode(Code::Comma))
		{
			done = false;
			token = translator.moveToken();
		}
		else if (translator.token()->isCode(Code::Semicolon))
		{
			commandToken->addSubCode(Option_SubCode);
			done = true;
			token = translator.moveToken();
			translator.getToken(Status::ExpEndStmt);
		}
		else  // possible end-of-statement
		{
			// check terminating token for end-of-statement
			if (!translator.token()->hasFlag(EndStmt_Flag))
			{
				throw TokenError {Status::ExpCommaSemiOrEnd,
					translator.token()};
			}
			done = true;
			token = std::make_shared<Token>(Table::entry(Code::Null));
		}

		// change token to appropriate assign code and append to output
		token->setTableEntry(commandToken->alternate(1));
		translator.processFinalOperand(token);

		// create and insert input parse code at insert point
		// (inserted in reverse order for each input variable)
		insertPoint = translator.outputInsert(insertPoint,
			std::make_shared<Token>(token->alternate(1)));
	}
	while (!done);

	translator.outputAppend(std::move(commandToken));
}


// function to recreate the input begin string code (for INPUT PROMPT)
void inputPromptBeginRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	recreator.setSeparator(rpnItem->token()->hasSubCode(Option_SubCode)
		? ',' : ';');
}


// function to recreate an input assign type code
void inputAssignRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	(void)rpnItem;

	if (recreator.separatorIsSet())
	{
		// if there is a separator,
		// then append top string to previous string with separator between
		std::string string {recreator.popString()};
		recreator.topAppend(recreator.separator());
		// FLAG option: space after comma (default=yes)
		recreator.topAppend(' ');
		recreator.topAppend(std::move(string));
	}
	recreator.setSeparator(',');  // set separator for next reference
}


// function to recreate the input command code
void inputRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	recreator.append(rpnItem->token()->commandName());
	// FLAG option: all spaces after commands (default=yes)
	recreator.append(' ');
	recreator.append(recreator.popString());
	if (rpnItem->token()->hasSubCode(Option_SubCode))
	{
		recreator.append(';');
	}
	recreator.clearSeparator();
}


// end: input.cpp
