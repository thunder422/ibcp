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

#include "command.h"
#include "recreator.h"
#include "table.h"
#include "token.h"
#include "translator.h"

extern ExprInfo Null_ExprInfo;
extern ExprInfo None_Dbl_ExprInfo;
extern ExprInfo None_Int_ExprInfo;
extern ExprInfo None_Str_ExprInfo;


//=======================
//  TABLE ENTRY CLASSES
//=======================

class InputCommand : public Command
{
public:
	InputCommand(const char *name2 = "") : Command {"INPUT", name2, "Keep"} {}

	void translate(Translator &translator) override;
	void recreate(Recreator &recreator, RpnItemPtr &rpnItem) override;
};

class Input : public InputCommand
{
public:
	Input() : InputCommand {} {}

	// TODO virtual run() override function for Input
};

class InputPrompt : public InputCommand
{
public:
	InputPrompt() : InputCommand {"PROMPT"} {}

	// TODO virtual run() override function for InputPrompt
};


class InputBegin : public Internal
{
public:
	InputBegin(const AlternateItem &alternateItem) :
		Internal {"InputBegin", "", &Null_ExprInfo}
	{
		appendAlternate(alternateItem);
	}

	// TODO virtual run() override function for InputBegin
};

class InputBeginStr : public Internal
{
public:
	InputBeginStr(const AlternateItem &alternateItem) :
		Internal {"InputBegin", "$", &Null_ExprInfo, "Question"}
	{
		appendAlternate(alternateItem);
	}

	// TODO virtual run() override function for InputBegin
	void recreate(Recreator &recreator, RpnItemPtr &rpnItem) override;
};


constexpr BaseInfo inputAssignBase = {Code{}, "InputAssign", Reference_Flag};

constexpr TypeInfo Dbl = {"", &None_Dbl_ExprInfo};
constexpr TypeInfo Int = {"%", &None_Int_ExprInfo};
constexpr TypeInfo Str = {"$", &None_Str_ExprInfo};


class InputAssign : public Internal
{
public:
	InputAssign(TypeInfo typeInfo, const AlternateItem &alternateItem) :
		Internal {inputAssignBase, typeInfo, alternateItem} {}

	void recreate(Recreator &recreator, RpnItemPtr &rpnItem) override;
};

class InputAssignDbl : public InputAssign
{
public:
	InputAssignDbl(const AlternateItem &alternateItem,
			const AlternateItem &alternateItem2) :
		InputAssign {Dbl, alternateItem}
	{
		appendAlternate(alternateItem2);
	}

	// TODO virtual run() override function for InputAssignDbl
};

class InputAssignInt : public InputAssign
{
public:
	InputAssignInt(const AlternateItem &alternateItem) :
		InputAssign {Int, alternateItem} {}

	// TODO virtual run() override function for InputAssignInt
};

class InputAssignStr : public InputAssign
{
public:
	InputAssignStr(const AlternateItem &alternateItem) :
		InputAssign {Str, alternateItem} {}

	// TODO virtual run() override function for InputAssignStr
};


constexpr BaseInfo inputParseBase = {Code{}, "InputParse", TableFlag{}};

class InputParseDbl : public Internal
{
public:
	InputParseDbl(const AlternateItem &alternateItem) :
		Internal {inputParseBase, Dbl, alternateItem} {}

	// TODO virtual run() override function for InputParseDbl
};

class InputParseInt : public Internal
{
public:
	InputParseInt(const AlternateItem &alternateItem) :
		Internal {inputParseBase, Int, alternateItem} {}

	// TODO virtual run() override function for InputParseInt
};

class InputParseStr : public Internal
{
public:
	InputParseStr(const AlternateItem &alternateItem) :
		Internal {inputParseBase, Str, alternateItem} {}

	// TODO virtual run() override function for InputParseStr
};


//=========================
//  TABLE ENTRY FUNCTIONS
//=========================

void InputCommand::translate(Translator &translator)
{
	TokenPtr commandToken = translator.moveToken();  // save command token
	TokenPtr token;
	Table *entry = commandToken->firstAlternate(FirstOperand);
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
		token->setTableEntry(commandToken->firstAlternate(SecondOperand));
		translator.processFinalOperand(token);

		// create and insert input parse code at insert point
		// (inserted in reverse order for each input variable)
		insertPoint = translator.outputInsert(insertPoint,
			std::make_shared<Token>(token->firstAlternate(SecondOperand)));
	}
	while (!done);

	translator.outputAppend(std::move(commandToken));
}


// function to recreate the input begin string code (for INPUT PROMPT)
void InputBeginStr::recreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	recreator.setSeparator(rpnItem->token()->hasSubCode(Option_SubCode)
		? ',' : ';');
}


void InputAssign::recreate(Recreator &recreator, RpnItemPtr &rpnItem)
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


void InputCommand::recreate(Recreator &recreator, RpnItemPtr &rpnItem)
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


//=========================
//  TABLE ENTRY INSTANCES
//=========================

Input input;
InputBegin inputBegin {&input};

InputPrompt inputPrompt;
InputBeginStr inputBeginStr {&inputPrompt};

InputAssignDbl inputAssignDbl {{&input, 1}, {&inputPrompt, 1}};
InputAssignInt inputAssignInt {&inputAssignDbl};
InputAssignStr inputAssignStr {&inputAssignDbl};

InputParseDbl inputParseDbl {{&inputAssignDbl, 1}};
InputParseInt inputParseInt {{&inputAssignInt, 1}};
InputParseStr inputParseStr {{&inputAssignStr, 1}};


// end: input.cpp
