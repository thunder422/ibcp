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

#include "recreator.h"
#include "table.h"
#include "token.h"
#include "translator.h"


// LET command translate function
Token::Status letTranslate(Translator &translator, Token *commandToken,
	Token *&token)
{
	Token::Status status;
	int column;
	bool hidden;
	DataType dataType;
	bool done;
	TokenStack letStack;
	bool haveSubStr = false;

	if (commandToken == NULL)
	{
		column = token->column();
		hidden = true;
	}
	else  // delete unneeded command token and get another token
	{
		column = commandToken->column();
		delete commandToken;
		hidden = false;
	}
	dataType = DataType::Any;
	do
	{
		if ((status = translator.getOperand(token, dataType,
			Translator::Reference::All)) != Token::Status::Good)
		{
			if (token->column() > column)
			{
				return status;
			}
			// next token determines error
			Token *nextToken;
			if ((status = translator.getToken(nextToken))
				!= Token::Status::Good)
			{
				status = Token::Status::ExpCmd;
			}
			if (nextToken->isCode(Comma_Code) || nextToken->isCode(Eq_Code))
			{
				status = Token::Status::ExpAssignItem;
			}
			else
			{
				status = Token::Status::ExpCmd;
			}
			delete nextToken;
			return status;
		}

		// get and check next token for comma or equal
		status = translator.getToken(token);
		if (token->isCode(Comma_Code))
		{
			done = false;
		}
		else if (token->isCode(Eq_Code))
		{
			done = true;
		}
		else  // invalid token or parser error
		{
			if (translator.table().hasFlag(translator.doneStackTopToken(),
				SubStr_Flag))
			{
				translator.doneStackPop();
			}
			return Token::Status::ExpEqualOrComma;
		}

		// check if this is a sub-string assignment
		if (translator.table().hasFlag(translator.doneStackTopToken(),
			SubStr_Flag))
		{
			// delete comma/equal token, use sub-string function token
			delete token;

			// get sub-string function token from rpn item on top of stack
			// (delete rpn item since it was not appended to output)
			RpnItemPtr rpnItem = translator.doneStackPop();
			token = rpnItem->token();
			rpnItem->setToken(NULL);  // prevent delete of token

			// change to assign sub-string code (first associated code)
			translator.table().setToken(token,
				translator.table().associatedCode(token->code()));

			haveSubStr = true;
		}
		else  // use comma/equal token
		{
			// change token to appropriate assign code
			translator.table().setToken(token, Assign_Code);
			status = translator.processDoneStackTop(token);
			if (status != Token::Status::Good)
			{
				return status;
			}
		}
		letStack.push(token);  // save token

		// get data type for assignment
		if (dataType == DataType::Any)
		{
			dataType = token->dataType();
		}

		token = NULL;
	}
	while (!done);

	// get expression for value to assign
	if ((status = translator.getExpression(token, dataType))
		!= Token::Status::Done)
	{
		if (status == Token::Status::Parser
			&& token->isDataType(DataType::None))
		{
			status = Token::Status::ExpOpOrEnd;
		}
		return status;
	}

	Token *letToken = letStack.pop();
	if (!letStack.isEmpty())
	{
		if (haveSubStr)
		{
			// add each token saved in let stack except the last
			do
			{
				// change to keep code (second associated code)
				translator.table().setToken(letToken,
					translator.table().associatedCode(letToken->code()));

				// append to output and pop next token from let stack
				translator.outputAppend(letToken);
				letToken = letStack.pop();
			}
			while (!letStack.isEmpty());  // continue until last token
		}
		else  // have a multiple assignment, change to list code
		{
			translator.table().setToken(letToken,
				translator.table().secondAssociatedCode(letToken->code()));
		}
	}

	// drop expresion result from done stack, append last assignment token
	translator.doneStackDrop();
	translator.outputAppend(letToken);

	// set hidden LET flag if needed
	if (!hidden)
	{
		letToken->addSubCode(Option_SubCode);
	}

	// check terminating token for end-of-statement
	if (!translator.table().hasFlag(token, EndStmt_Flag))
	{
		return Token::Status::ExpOpOrEnd;
	}

	return Token::Status::Done;
}


// function to append LET keyword if the option sub-code is set
void letRecreate(Recreator &recreator, Token *token)
{
	if (token->hasSubCode(Option_SubCode))
	{
		recreator.append(recreator.table().optionName(token->code()) + ' ');
	}
}


// function to recreate assignment and list assignment statements
void assignRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	QStack<QString> stack;

	stack.push(recreator.pop());  // push value
	QString separator = ' ' + recreator.table().name(rpnItem->token()) + ' ';
	while (!recreator.stackIsEmpty())
	{
		stack.push(recreator.pop() + separator);
		separator = ", ";
	}
	letRecreate(recreator, rpnItem->token());
	while (!stack.isEmpty())
	{
		recreator.append(stack.pop());
	}
}


// function to recreate string and sub-string assignment statements
void assignStrRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	QString string;

	// check if this first assign code
	if (!recreator.separatorIsSet())
	{
		string = ' ' + recreator.table().name(Assign_Code) + ' ';
		recreator.setSeparator(',');
	}
	else  // continuation of assignment list
	{
		string = recreator.separator() + ' ';
	}
	string.append(recreator.pop());

	Code code = rpnItem->token()->code();
	if (recreator.table().hasFlag(code, SubStr_Flag))
	{
		// for sub-string assignments, get original sub-string function code
		Code subStrCode = recreator.table().secondAssociatedCode(code);
		QString name = recreator.table().name(subStrCode);
		int count = recreator.table().operandCount(subStrCode);
		recreator.pushWithOperands(name, count);
	}

	// deterine if assignment is an assignment keep code
	// (only assignment keep codes have second associated index of zero)
	if (recreator.table().secondAssociatedIndex(code) == 0)
	{
		// for keep codes, append string so far to reference string on stack
		recreator.topAppend(string);
	}
	else
	{
		// end of statement, append reference and string so far
		letRecreate(recreator, rpnItem->token());
		recreator.append(recreator.pop());
		recreator.append(string);
		recreator.clearSeparator();  // for next command
	}
}


// end: let.cpp
