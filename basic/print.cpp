// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: print.cpp - print command functions source file
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
//	2013-08-03	initial version

#include "basic/basic.h"
#include "command.h"
#include "operator.h"
#include "recreator.h"
#include "table.h"
#include "token.h"
#include "translator.h"

extern ExprInfo None_Dbl_ExprInfo;
extern ExprInfo None_Int_ExprInfo;
extern ExprInfo None_Str_ExprInfo;


//=======================
//  TABLE ENTRY CLASSES
//=======================

class Print : public Command
{
public:
	Print(const AlternateItem &alternateItem) :
		Command {"PRINT"}
	{
		appendAlternate(alternateItem);
	}

	// TODO virtual run() override function for Print
	void translate(Translator &translator) override;
	void recreate(Recreator &recreator, RpnItemPtr &rpnItem) override;
};


class PrintItem : public Internal
{
public:
	PrintItem(const char *name2, ExprInfo *exprInfo,
			const AlternateItem &alternateItem, unsigned moreFlags = {}) :
		Internal {name2, exprInfo, Print_Flag | moreFlags}
	{
		appendAlternate(alternateItem);
	}

	void recreate(Recreator &recreator, RpnItemPtr &rpnItem) override;
};

class PrintDbl : public PrintItem
{
public:
	PrintDbl(const AlternateItem &alternateItem) :
		PrintItem {"PrintDbl", &None_Dbl_ExprInfo, alternateItem,
			UseConstAsIs_Flag} {}

	// TODO virtual run() override function for PrintDbl
};

class PrintInt : public PrintItem
{
public:
	PrintInt(const AlternateItem &alternateItem) :
		PrintItem {"PrintInt", &None_Int_ExprInfo, alternateItem} {}

	// TODO virtual run() override function for PrintInt
};

class PrintStr : public PrintItem
{
public:
	PrintStr(const AlternateItem &alternateItem) :
		PrintItem {"PrintStr", &None_Str_ExprInfo, alternateItem} {}

	// TODO virtual run() override function for PrintStr
};


class Semicolon : public SpecialOperator
{
public:
	Semicolon() : SpecialOperator {Code::Semicolon, ";", 6, Command_Flag} {}

	// TODO virtual run() override function for Semicolon
	void recreate(Recreator &recreator, RpnItemPtr &rpnItem) override;
};


class Comma : public SpecialOperator
{
public:
	Comma() : SpecialOperator {Code::Comma, ",", 6, Command_Flag} {}

	// TODO virtual run() override function for Comma
	void recreate(Recreator &recreator, RpnItemPtr &rpnItem) override;
};


//=========================
//  TABLE ENTRY FUNCTIONS
//=========================

void Print::translate(Translator &translator)
{
	TokenPtr commandToken = translator.moveToken();  // save command token
	bool printFunction {};
	bool separator {};
	TokenPtr lastSemiColon;
	for (;;)
	{
		try
		{
			translator.getExpression(DataType::None);
		}
		catch (TokenError &error)
		{
			if (error(Status::UnknownToken))
			{
				if (translator.doneStackEmpty())
				{
					error = Status::ExpExprCommaPfnOrEnd;
				}
				// change parser error if not inside paren
				else if (translator.doneStackTopToken()
					->isDataType(DataType::None))
				{
					error = Status::ExpSemiCommaOrEnd;
				}
				else  // not a print function
				{
					error = Status::ExpOpSemiCommaOrEnd;
				}
			}
			throw;
		}

		if (!translator.doneStackEmpty())
		{
			if (translator.doneStackTopToken()->isDataType(DataType::None))
			{
				translator.doneStackPop();  // print function
				printFunction = true;
			}
			else  // append appropriate print code for done stack top item
			{
				Table *entry = commandToken->firstAlternate(FirstOperand);
				TokenPtr printToken {std::make_shared<Token>(entry)};
				translator.processFinalOperand(printToken);
				printFunction = false;
			}
			separator = true;
			lastSemiColon.reset();  // don't need last semicolon token
		}

		if (translator.token()->isCode(Code::Comma))
		{
			if (lastSemiColon)
			{
				throw TokenError {Status::ExpExprPfnOrEnd, translator.token()};
			}
			translator.outputAppend(translator.moveToken());
		}
		else if (translator.token()->isCode(Code::Semicolon))
		{
			if (!separator)
			{
				throw TokenError {lastSemiColon ? Status::ExpExprPfnOrEnd
					: Status::ExpExprCommaPfnOrEnd, translator.token()};
			}
			lastSemiColon = translator.moveToken();  // save semicolon token
		}
		else  // some other token, maybe end-of-statement
		{
			if (!translator.token()->hasFlag(EndStmt_Flag))
			{
				throw TokenError {printFunction ? Status::ExpSemiCommaOrEnd
					: Status::ExpOpSemiCommaOrEnd, translator.token()};
			}
			break;  // exit loop
		}
		separator = false;
	}

	// append last semicolon token if set or command token
	// (other will be deleted when function ends)
	translator.outputAppend(lastSemiColon
		? std::move(lastSemiColon) : std::move(commandToken));
}


void printItemRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	(void)rpnItem;

	std::string string;

	if (recreator.separatorIsSet())
	{
		// append the previous separator
		string = recreator.separator();

		// FLAG option: space between print commas (default=no)
		if (!recreator.separatorIsSet(' '))
		{
			// if it is not a space (comma) then append a space
			// FLAG option: space after print semicolons (default=yes)
			string += ' ';
		}
	}
	// pop the string on top of the stack and append it to the string
	string.append(recreator.popString());

	if (recreator.empty())
	{
		// if nothing else on the stack then push the string
		recreator.emplace(string);
	}
	else  // append the string to the string on top of the stack
	{
		recreator.topAppend(std::move(string));
	}

	recreator.setSeparator(';');  // set separator for next item
}

void PrintItem::recreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	// TODO temporary until print functions put into new table model
	printItemRecreate(recreator, rpnItem);
}


void Comma::recreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	std::string string;

	// get string on top of the stack if there is one
	if (!recreator.empty())
	{
		string = recreator.popString();
	}

	// append comma to string and push it back to the stack
	// FLAG option: space after print commas (default=no)
	string += rpnItem->token()->name();
	recreator.emplace(string);

	// set separator to space (used to not add spaces 'between' commas)
	recreator.setSeparator(' ');
}


// function to recreate a print function code
void printFunctionRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	// process as internal function first then as an item
	internalFunctionRecreate(recreator, rpnItem);
	printItemRecreate(recreator, rpnItem);
}


void Semicolon::recreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	// append final semicolon to string on top of stack then recreate command
	std::string name {rpnItem->token()->name()};
	recreator.topAppend(std::move(name));

	Table *printEntry = rpnItem->token()->firstAlternate();
	TokenPtr token {std::make_shared<Token>(printEntry)};
	RpnItemPtr rpnItemPtr {std::make_shared<RpnItem>(token)};
	printEntry->recreate(recreator, rpnItemPtr);
}


// function to recreate the print code
void Print::recreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	// append PRINT keyword
	std::string name {rpnItem->token()->name()};
	recreator.append(std::move(name));

	// if stack is not empty then append space with string on top of stack
	if (!recreator.empty())
	{
		// FLAG option: all spaces after commands (default=yes)
		recreator.append(' ');
		recreator.append(recreator.popString());
	}

	recreator.clearSeparator();  // clear separator for next command
}


//=========================
//  TABLE ENTRY INSTANCES
//=========================

Semicolon semicolon;
Comma comma;

Print print {&semicolon};

PrintDbl printDbl {&print};
PrintInt printInt {&printDbl};
PrintStr printStr {&printDbl};

// end: print.cpp
