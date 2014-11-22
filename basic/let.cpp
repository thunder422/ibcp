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
void letTranslate(Translator &translator, TokenPtr commandToken,
	TokenPtr &token)
{
	Status status;
	int column;
	bool hidden;
	bool done;
	std::stack<TokenPtr> letStack;
	bool haveSubStr {};

	if (!commandToken)
	{
		column = token->column();
		hidden = true;
	}
	else  // delete unneeded command token and get another token
	{
		column = commandToken->column();
		hidden = false;
	}
	DataType dataType {DataType::Any};
	do
	{
		if ((status = translator.getOperand(token, dataType,
			Translator::Reference::All)) != Status::Good)
		{
			if (token->column() == column)  // at begin command?
			{
				// next token determines error
				TokenPtr nextToken;
				if ((status = translator.getToken(nextToken)) == Status::Good)
				{
					status = nextToken->isCode(Comma_Code)
						|| nextToken->isCode(Eq_Code)
						? Status::ExpAssignItem : Status::ExpCmdOrAssignItem;
				}
				else
				{
					status = Status::ExpCmdOrAssignItem;
				}
			}
			throw TokenError {status, token};
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
			throw TokenError {Status::ExpEqualOrComma, token};
		}

		// check if this is a sub-string assignment
		if (translator.table().hasFlag(translator.doneStackTopToken(),
			SubStr_Flag))
		{
			// get sub-string function token from rpn item on top of stack
			// (delete rpn item since it was not appended to output)
			token = translator.doneStackTopToken();
			translator.doneStackPop();

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
			if (status != Status::Good)
			{
				throw TokenError {status, token};
			}
		}

		// get data type for assignment
		if (dataType == DataType::Any)
		{
			dataType = token->dataType();
		}

		letStack.emplace(std::move(token));  // save token
	}
	while (!done);

	// get expression for value to assign
	if ((status = translator.getExpression(token, dataType)) != Status::Done)
	{
		if (status == Status::UnknownToken)
		{
			throw TokenError {Status::ExpOpOrEnd, token};
		}
		throw TokenError {status, token};
	}

	// check terminating token for end-of-statement
	if (!translator.table().hasFlag(token, EndStmt_Flag))
	{
		throw TokenError {Status::ExpOpOrEnd, token};
	}

	TokenPtr letToken {letStack.top()};
	letStack.pop();
	if (!letStack.empty())
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
				translator.outputAppend(std::move(letToken));
				letToken = letStack.top();
				letStack.pop();
			}
			while (!letStack.empty());  // continue until last token
		}
		else  // have a multiple assignment, change to list code
		{
			translator.table().setToken(letToken,
				translator.table().secondAssociatedCode(letToken->code()));
		}
	}

	// set hidden LET flag if needed
	if (!hidden)
	{
		letToken->addSubCode(Option_SubCode);
	}

	// drop expresion result from done stack, append last assignment token
	translator.doneStackPop();
	translator.outputAppend(std::move(letToken));
}


// function to append LET keyword if the option sub-code is set
void letRecreate(Recreator &recreator, TokenPtr token)
{
	if (token->hasSubCode(Option_SubCode))
	{
		recreator.append(recreator.table().optionName(token->code())
			.toStdString() + ' ');
	}
}


// function to recreate assignment and list assignment statements
void assignRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	std::stack<std::string> stack;

	stack.emplace(recreator.popString());  // push value
	std::string separator = ' ' + recreator.table().name(rpnItem->token())
		.toStdString() + ' ';
	while (!recreator.empty())
	{
		stack.emplace(recreator.popString() + separator);
		separator = ", ";
	}
	letRecreate(recreator, rpnItem->token());
	while (!stack.empty())
	{
		recreator.append(std::move(stack.top()));
		stack.pop();
	}
}


// function to recreate string and sub-string assignment statements
void assignStrRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	std::string string;

	// check if this first assign code
	if (!recreator.separatorIsSet())
	{
		string = ' ' + recreator.table().name(Assign_Code).toStdString() + ' ';
		recreator.setSeparator(',');
	}
	else  // continuation of assignment list
	{
		string = recreator.separator();
		string += ' ';
	}
	string.append(recreator.popString());

	Code code {rpnItem->token()->code()};
	if (recreator.table().hasFlag(code, SubStr_Flag))
	{
		// for sub-string assignments, get original sub-string function code
		Code subStrCode {recreator.table().secondAssociatedCode(code)};
		recreator.pushWithOperands(recreator.table().name(subStrCode)
			.toStdString(), recreator.table().operandCount(subStrCode));
	}

	// deterine if assignment is an assignment keep code
	// (only assignment keep codes have second associated index of zero)
	if (recreator.table().secondAssociatedIndex(code) == 0)
	{
		// for keep codes, append string so far to reference string on stack
		recreator.topAppend(std::move(string));
	}
	else
	{
		// end of statement, append reference and string so far
		letRecreate(recreator, rpnItem->token());
		recreator.append(recreator.popString());
		recreator.append(std::move(string));
		recreator.clearSeparator();  // for next command
	}
}


// end: let.cpp
