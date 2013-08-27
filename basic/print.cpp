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

#include "table.h"
#include "token.h"
#include "translator.h"


// PRINT command translate function
TokenStatus printTranslate(Translator &translator, Token *commandToken,
	Token *&token)
{
	TokenStatus status;
	Token *lastSemiColon = NULL;
	bool separator = false;
	bool printFunction = false;

	forever
	{
		if ((status = translator.getExpression(token, None_DataType))
			!= Done_TokenStatus)
		{
			if (status == Parser_TokenStatus
				&& token->isDataType(None_DataType))
			{
				if (translator.doneStackEmpty())
				{
					status = ExpExprCommaPfnOrEnd_TokenStatus;
				}
				// change parser error if not inside paren
				else if (translator.doneStackTopToken()
					->isDataType(None_DataType))
				{
					status = ExpSemiCommaOrEnd_TokenStatus;
				}
				else  // not a print function
				{
					status = ExpOpSemiCommaOrEnd_TokenStatus;
				}
			}
			break;
		}

		if (!translator.doneStackEmpty())
		{
			if (translator.doneStackTopToken()->isDataType(None_DataType))
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
				status = ExpExprPfnOrEnd_TokenStatus;
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
					? ExpExprCommaPfnOrEnd_TokenStatus
					: ExpExprPfnOrEnd_TokenStatus;
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

	if (status != Done_TokenStatus)
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
		return printFunction
			? ExpSemiCommaOrEnd_TokenStatus : ExpOpSemiCommaOrEnd_TokenStatus;
	}
	return Done_TokenStatus;
}


// end: print.cpp
