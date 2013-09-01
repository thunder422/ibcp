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

#include "table.h"
#include "token.h"
#include "translator.h"


// INPUT command translate function
TokenStatus inputTranslate(Translator &translator, Token *commandToken,
	Token *&token)
{
	TokenStatus status;
	int indexBegin;
	bool done;
	Token *inputToken;

	if (commandToken->isCode(Input_Code))
	{
		token = translator.table().newToken(InputBegin_Code);
	}
	else  // InputPrompt_Code
	{
		token = NULL;
		status = translator.getExpression(token, String_DataType);
		if (status != Done_TokenStatus)
		{
			if (status == Parser_TokenStatus
				&& token->isDataType(None_DataType))
			{
				status = ExpSemiOrComma_TokenStatus;
			}
			delete commandToken;
			return status;
		}
		translator.doneStackDrop();
		if (token->isCode(Comma_Code))
		{
			token->addSubCode(Question_SubCode);
		}
		else if (!token->isCode(SemiColon_Code))
		{
			delete commandToken;
			return ExpOpSemiOrComma_TokenStatus;
		}
		token->setCode(InputBeginStr_Code);
	}

	// save index where to insert input parse codes and append input begin
	indexBegin = translator.outputCount();
	translator.outputAppend(token);

	// loop to read input variables
	do
	{
		// get variable reference
		token = NULL;
		if ((status = translator.getOperand(token, Any_DataType,
			Translator::Variable_Reference)) != Good_TokenStatus)
		{
			break;
		}

		// get and check next token
		if ((status = translator.getToken(token)) != Good_TokenStatus)
		{
			status = ExpCommaSemiOrEnd_TokenStatus;
			break;
		}
		if (token->isCode(Comma_Code))
		{
			done = false;
			inputToken = token;
		}
		else if (token->isCode(SemiColon_Code))
		{
			commandToken->addSubCode(Keep_SubCode);
			done = true;
			inputToken = token;

			// get and check next token
			if ((status = translator.getToken(token)) != Good_TokenStatus)
			{
				status = ExpEndStmt_TokenStatus;
				break;
			}
		}
		else  // possible end-of-statement (checked below)
		{
			done = true;
			inputToken = new Token;
		}

		// change token to appropriate assign code and append to output
		translator.table().setToken(inputToken, InputAssign_Code);
		status = translator.processFinalOperand(inputToken);
		if (status != Good_TokenStatus)
		{
			break;
		}

		// create and insert input parse code at beginning
		// (inserted in reverse order for each input variable)
		translator.outputInsert(indexBegin, translator.table()
			.newToken(translator.table().assoc2Code(inputToken->code())));
	}
	while (!done);

	if (status != Good_TokenStatus)
	{
		delete commandToken;
		return status;
	}
	translator.outputAppend(commandToken);

	// check terminating token for end-of-statement
	if (!translator.table().hasFlag(token, EndStmt_Flag))
	{
		return ExpCommaSemiOrEnd_TokenStatus;
	}

	return Done_TokenStatus;
}


// end: input.cpp
