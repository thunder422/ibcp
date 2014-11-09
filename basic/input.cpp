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
Status inputTranslate(Translator &translator, TokenPtr commandToken,
	TokenPtr &token)
{
	Status status;
	bool done;
	TokenPtr inputToken;

	if (commandToken->isCode(Input_Code))
	{
		token = translator.table().newToken(InputBegin_Code);
	}
	else  // InputPrompt_Code
	{
		status = translator.getExpression(token, DataType::String);
		if (status != Status::Done)
		{
			if (status == Status::UnknownToken)
			{
				status = Status::ExpSemiOrComma;
			}
			return status;
		}
		translator.doneStackPop();
		if (token->isCode(Comma_Code))
		{
			token->addSubCode(Option_SubCode);
		}
		else if (!token->isCode(SemiColon_Code))
		{
			return Status::ExpOpSemiOrComma;
		}
		token->setCode(InputBeginStr_Code);
	}

	// append input begin and save iterator where to insert input parse codes
	auto insertPoint = translator.outputAppendIterator(std::move(token));

	// loop to read input variables
	do
	{
		// get variable reference
		if ((status = translator.getOperand(token, DataType::Any,
			Translator::Reference::Variable)) != Status::Good)
		{
			break;
		}

		// get and check next token
		if ((status = translator.getToken(token)) != Status::Good)
		{
			status = Status::ExpCommaSemiOrEnd;
			break;
		}
		if (token->isCode(Comma_Code))
		{
			done = false;
			inputToken = std::move(token);
		}
		else if (token->isCode(SemiColon_Code))
		{
			commandToken->addSubCode(Option_SubCode);
			done = true;
			inputToken = std::move(token);

			// get and check next token
			if ((status = translator.getToken(token)) != Status::Good)
			{
				status = Status::ExpEndStmt;
				break;
			}
		}
		else  // possible end-of-statement (checked below)
		{
			done = true;
			inputToken = std::make_shared<Token>();
		}

		// change token to appropriate assign code and append to output
		translator.table().setToken(inputToken, InputAssign_Code);
		status = translator.processFinalOperand(inputToken);
		if (status != Status::Good)
		{
			break;
		}

		// create and insert input parse code at insert point
		// (inserted in reverse order for each input variable)
		insertPoint = translator.outputInsert(insertPoint, translator.table()
			.newToken(translator.table()
			.secondAssociatedCode(inputToken->code())));
	}
	while (!done);

	if (status != Status::Good)
	{
		return status;
	}
	translator.outputAppend(commandToken);

	// check terminating token for end-of-statement
	if (!translator.table().hasFlag(token, EndStmt_Flag))
	{
		return Status::ExpCommaSemiOrEnd;
	}

	return Status::Done;
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
	Q_UNUSED(rpnItem)

	if (recreator.separatorIsSet())
	{
		// if there is a separator,
		// then append top string to previous string with separator between
		QString string {recreator.popString()};
		// FLAG option: space after comma (default=yes)
		recreator.topAppend(QChar(recreator.separator()) + ' ' + string);
	}
	recreator.setSeparator(',');  // set separator for next reference
}


// function to recreate the input command code
void inputRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	recreator.append(recreator.table().name(rpnItem->token()));
	// FLAG option: all spaces after commands (default=yes)
	recreator.append(" ");
	recreator.append(recreator.popString());
	if (rpnItem->token()->hasSubCode(Option_SubCode))
	{
		recreator.append(";");
	}
	recreator.clearSeparator();
}


// end: input.cpp
