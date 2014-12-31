// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: input.cpp - input command functions source file
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
	Code code = translator.table().alternateCode(commandToken->code(), 0);
	if (translator.table().name2(commandToken->code()).empty())
	{
		token = translator.table().newToken(code);
	}
	else  // InputPrompt_Code
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
		if (token->isCode(Comma_Code))
		{
			token->addSubCode(Option_SubCode);
		}
		else if (!token->isCode(SemiColon_Code))
		{
			throw TokenError {Status::ExpOpSemiOrComma, token};
		}
		token->setCode(code);  // reuse token
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

		if (translator.token()->isCode(Comma_Code))
		{
			done = false;
			token = translator.moveToken();
		}
		else if (translator.token()->isCode(SemiColon_Code))
		{
			commandToken->addSubCode(Option_SubCode);
			done = true;
			token = translator.moveToken();
			translator.getToken(Status::ExpEndStmt);
		}
		else  // possible end-of-statement
		{
			// check terminating token for end-of-statement
			if (!translator.table().hasFlag(translator.token(), EndStmt_Flag))
			{
				throw TokenError {Status::ExpCommaSemiOrEnd,
					translator.token()};
			}
			done = true;
			token = translator.table().newToken(Null_Code);
		}

		// change token to appropriate assign code and append to output
		code = translator.table().alternateCode(commandToken->code(), 1);
		translator.table().setToken(token.get(), code);
		translator.processFinalOperand(token);

		// create and insert input parse code at insert point
		// (inserted in reverse order for each input variable)
		insertPoint = translator.outputInsert(insertPoint, translator.table()
			.newToken(translator.table().alternateCode(token->code(), 1)));
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
	recreator.append(recreator.table().name(rpnItem->token()));
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
