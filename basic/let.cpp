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
void letTranslate(Translator &translator)
{
	bool done;
	std::stack<TokenPtr> letStack;

	int column {translator.token()->column()};
	bool hidden;
	if (!translator.token()->isCode(Let_Code))
	{
		hidden = true;
	}
	else  // delete unneeded command token and get another token
	{
		translator.resetToken();
		hidden = false;
	}
	DataType dataType {DataType::Any};
	bool haveSubStr {};
	do
	{
		try
		{
			// does not return false (throws error for reference)
			translator.getOperand(dataType, Reference::All);
		}
		catch (TokenError &error)
		{
			if (error.m_column == column)  // at begin command?
			{
				try
				{
					// next token determines error
					translator.resetToken();
					translator.getToken(Status{});
					error = translator.token()->isCode(Comma_Code)
						|| translator.token()->isCode(Eq_Code)
						? Status::ExpAssignItem : Status::ExpCmdOrAssignItem;
				}
				catch (TokenError)
				{
					error = Status::ExpCmdOrAssignItem;
				}
			}
			throw;
		}

		// get and check next token for comma or equal
		translator.getToken(Status::ExpEqualOrComma);
		if (translator.token()->isCode(Comma_Code))
		{
			done = false;
		}
		else if (translator.token()->isCode(Eq_Code))
		{
			done = true;
		}
		else  // invalid token
		{
			throw TokenError {Status::ExpEqualOrComma, translator.token()};
		}

		// check if this is a sub-string assignment
		TokenPtr token;
		if (translator.table().hasFlag(translator.doneStackTopToken(),
			SubStr_Flag))
		{
			// get sub-string function token from rpn item on top of stack
			// (delete rpn item since it was not appended to output)
			token = translator.doneStackTopToken();
			translator.doneStackPop();

			token->setFirstAlternate(1);  // change to sub-string assignment

			translator.resetToken();  // don't need comma/equal token
			haveSubStr = true;
		}
		else  // reuse comma/equal token
		{
			token = translator.moveToken();
			// change token to appropriate assign code
			token->setCode(Table::entry(Let_Code)->alternate(0)->code());
			translator.processDoneStackTop(token);
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
	try
	{
		translator.getExpression(dataType);
	}
	catch (TokenError &error)
	{
		if (error(Status::UnknownToken))
		{
			error = Status::ExpOpOrEnd;
		}
		throw;
	}

	// check terminating token for end-of-statement
	if (!translator.table().hasFlag(translator.token(), EndStmt_Flag))
	{
		throw TokenError {Status::ExpOpOrEnd, translator.token()};
	}

	TokenPtr token {std::move(letStack.top())};
	letStack.pop();
	if (!letStack.empty())
	{
		if (haveSubStr)
		{
			// add each token saved in let stack except the last
			do
			{
				// change to keep code (first alternate code)
				token->setCode(token->table()->alternate()->code());

				// append to output and pop next token from let stack
				translator.outputAppend(std::move(token));
				token = std::move(letStack.top());
				letStack.pop();
			}
			while (!letStack.empty());  // continue until last token
		}
		else  // have a multiple assignment
		{
			token->setFirstAlternate(1);  // change to list assignment
		}
	}

	// set hidden LET flag if needed
	if (!hidden)
	{
		token->addSubCode(Option_SubCode);
	}

	// drop expresion result from done stack, append last assignment token
	translator.doneStackPop();
	translator.outputAppend(std::move(token));
}


// function to append LET keyword if the option sub-code is set
void letRecreate(Recreator &recreator, TokenPtr token)
{
	if (token->hasSubCode(Option_SubCode))
	{
		recreator.append(recreator.table().optionName(token->code()) + ' ');
	}
}


// function to recreate assignment and list assignment statements
void assignRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	std::stack<std::string> stack;

	stack.emplace(recreator.popString());  // push value
	std::string separator = ' ' + recreator.table().name(rpnItem->token())
		+ ' ';
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
		string = ' ' + recreator.table().name(Assign_Code) + ' ';
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
		// for sub-string assignments, treat as function to recreate
		recreator.pushWithOperands(std::string{recreator.table().name(code)},
			recreator.table().operandCount(code));
	}

	// deterine if assignment is an sub-string assignment keep code
	if (recreator.table().hasFlag(code, Keep_Flag))
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
