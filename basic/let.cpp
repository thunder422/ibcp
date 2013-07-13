// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: let.cpp - let command functions source file
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
//	2013-07-06	initial version

#include "table.h"
#include "token.h"
#include "translator.h"


// LET command translate function
TokenStatus letTranslate(Translator &translator, Token *commandToken,
	Token *&token)
{
	TokenStatus status;
	bool hidden;
	DataType dataType;
	bool done;
	TokenStack letStack;

	if (commandToken == NULL)
	{
		hidden = true;
	}
	else  // delete unneeded command token and get another token
	{
		delete commandToken;
		hidden = false;
	}
	dataType = Any_DataType;
	do
	{
		if ((status = translator.getOperand(token, dataType,
			Translator::Variable_Reference)) != Good_TokenStatus)
		{
			token->setSubCodeMask(UnUsed_SubCode);
			if (token->column() > 0)
			{
				return status;
			}
			// next token determines error
			Token *nextToken;
			if ((status = translator.getToken(nextToken)) != Good_TokenStatus)
			{
				status = ExpCmd_TokenStatus;
			}
			if (nextToken->isCode(Comma_Code) || nextToken->isCode(Eq_Code))
			{
				status = ExpAssignItem_TokenStatus;
			}
			else
			{
				status = ExpCmd_TokenStatus;
			}
			delete nextToken;
			return status;
		}

		// get and check next token for comma or equal
		if ((status = translator.getToken(token)) != Good_TokenStatus)
		{
			token->setSubCodeMask(UnUsed_SubCode);
			return ExpEqualOrComma_TokenStatus;
		}
		if (token->isCode(Comma_Code))
		{
			done = false;
		}
		else if (token->isCode(Eq_Code))
		{
			done = true;
		}
		else
		{
			token->setSubCodeMask(UnUsed_SubCode);
			return ExpEqualOrComma_TokenStatus;
		}

		// change token to appropriate LET code and add to list
		translator.table().setToken(token, Assign_Code);
		token->setReference();
		if ((status = translator.findCode(token, 0)) != Good_TokenStatus)
		{
			return status;
		}
		// reset reference, no longer needed, and save token
		token->setReference(false);
		letStack.push(token);

		// get data type for multiple assignments
		if (dataType == Any_DataType)
		{
			dataType = token->dataType();
		}
		else if (token->dataType() != dataType)
		{
			token = translator.outputLastToken();
			return translator.variableErrStatus(dataType);
		}

		token = NULL;
	}
	while (!done);

	// get expression for value to assign
	if ((status = translator.getExpression(token,
		translator.equivalentDataType(dataType))) != Done_TokenStatus)
	{
		return status;
	}

	Token *letToken = letStack.pop();
	if (!letStack.isEmpty())
	{
		// have a multiple assigment, change to list code
		translator.table().setToken(letToken,
			translator.table().assoc2Code(letToken->code()));
	}

	// check end process expression, inserting conversion codes as needed
	Token *saveToken = letToken;
	if ((status = translator.processFinalOperand(letToken, NULL, 1))
		!= Good_TokenStatus)
	{
		delete token;  // expression terminating token
		delete saveToken;  // wasn't added to output
		token = letToken;
		return status;
	}

	// reset reference, no longer needed, and set hidden LET flag if needed
	letToken->setReference(false);
	if (!hidden)
	{
		letToken->setSubCodeMask(Let_SubCode);
	}

	// check terminating token for end-of-statement
	if ((translator.table().flags(token) & EndStmt_Flag) == 0)
	{
		token->setSubCodeMask(UnUsed_SubCode);
		return ExpOpOrEnd_TokenStatus;
	}

	return Done_TokenStatus;
}


// end: let.cpp
