// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: let.cpp - let command functions source file
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

#include "command.h"
#include "recreator.h"
#include "table.h"
#include "token.h"
#include "translator.h"


//=======================
//  TABLE ENTRY CLASSES
//=======================

class Let : public Command
{
public:
	Let() : Command {"LET", Code::Let} {}

	// TODO virtual run() override function for Let (will do nothing)
	void translate(Translator &translator) override;
	void recreate(Recreator &, RpnItemPtr &) override {}
};


constexpr BaseInfo assignBase = {Code{}, "Assign", TableFlag{}};
constexpr BaseInfo assignListBase = {Code{}, "AssignList", TableFlag{}};
constexpr BaseInfo assignKeepBase = {Code{}, "AssignKeep", Keep_Flag};

extern  ExprInfo Dbl_Dbl_ExprInfo;
extern  ExprInfo Int_Int_ExprInfo;
extern  ExprInfo Str_Str_ExprInfo;

constexpr TypeInfo Dbl = {"", &Dbl_Dbl_ExprInfo};
constexpr TypeInfo Int = {"%", &Int_Int_ExprInfo};
constexpr TypeInfo Str = {"$", &Str_Str_ExprInfo};

class Assign : public Internal
{
public:
	Assign(BaseInfo baseInfo, TypeInfo typeInfo,
			const AlternateItem &alternateItem) :
		Internal {baseInfo, typeInfo, "LET", alternateItem,
			Reference_Flag | Command_Flag} {}

	void recreate(Recreator &recreator, RpnItemPtr &rpnItem) override;

public:
	static void recreateLet(Recreator &recreator, TokenPtr token);
};

class AssignStrBase : public Assign
{
	using Assign::Assign;

	void recreate(Recreator &recreator, RpnItemPtr &rpnItem) override;
};

class AssignDbl : public Assign
{
public:
	AssignDbl(const AlternateItem &alternateItem) :
		Assign {assignBase, Dbl, alternateItem} {}

	// TODO virtual run() override function for AssignDbl
};

class AssignListDbl : public Assign
{
public:
	AssignListDbl(const AlternateItem &alternateItem) :
		Assign {assignListBase, Dbl, alternateItem} {}

	// TODO virtual run() override function for AssignListDbl
};

class AssignInt : public Assign
{
public:
	AssignInt(const AlternateItem &alternateItem) :
		Assign {assignBase, Int, alternateItem} {}

	// TODO virtual run() override function for AssignInt
};

class AssignListInt : public Assign
{
public:
	AssignListInt(const AlternateItem &alternateItem) :
		Assign {assignListBase, Int, alternateItem} {}

	// TODO virtual run() override function for AssignListInt
};

class AssignStr : public AssignStrBase
{
public:
	AssignStr(const AlternateItem &alternateItem) :
		AssignStrBase {assignBase, Str, alternateItem} {}

	// TODO virtual run() override function for AssignStr
};

class AssignListStr : public Assign
{
public:
	AssignListStr(const AlternateItem &alternateItem) :
		Assign {assignListBase, Str, alternateItem} {}

	// TODO virtual run() override function for AssignListStr
};

class AssignKeepStr : public AssignStrBase
{
public:
	AssignKeepStr(const AlternateItem &alternateItem) :
		AssignStrBase {assignKeepBase, Str, alternateItem} {}

	// TODO virtual run() override function for AssignKeepStr
};


//=========================
//  TABLE ENTRY FUNCTIONS
//=========================


void Let::translate(Translator &translator)
{
	bool done;
	std::stack<TokenPtr> letStack;

	int column {translator.token()->column()};
	bool hidden;
	if (!translator.token()->isCode(Code::Let))
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
					error = translator.token()->isCode(Code::Comma)
						|| translator.token()->isCode(Code::Equal)
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
		if (translator.token()->isCode(Code::Comma))
		{
			done = false;
		}
		else if (translator.token()->isCode(Code::Equal))
		{
			done = true;
		}
		else  // invalid token
		{
			throw TokenError {Status::ExpEqualOrComma, translator.token()};
		}

		// check if this is a sub-string assignment
		TokenPtr token;
		if (translator.doneStackTopToken()->hasFlag(SubStr_Flag))
		{
			// get sub-string function token from rpn item on top of stack
			// (delete rpn item since it was not appended to output)
			token = translator.doneStackTopToken();
			translator.doneStackPop();

			token->setToFirstAlternate(SubStringAssignment);

			translator.resetToken();  // don't need comma/equal token
			haveSubStr = true;
		}
		else  // reuse comma/equal token
		{
			token = translator.moveToken();
			// change token to appropriate assign code
			token->setTableEntry(Table::entry(Code::Let)->firstAlternate(0));
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
	if (!translator.token()->hasFlag(EndStmt_Flag))
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
				token->setTableEntry(token->firstAlternate(KeepString));

				// append to output and pop next token from let stack
				translator.outputAppend(std::move(token));
				token = std::move(letStack.top());
				letStack.pop();
			}
			while (!letStack.empty());  // continue until last token
		}
		else  // have a multiple assignment
		{
			token->setToFirstAlternate(ListAssignment);
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


// called from assignRecreate and assignStrRecreate
void Assign::recreateLet(Recreator &recreator, TokenPtr token)
{
	if (token->hasSubCode(Option_SubCode))
	{
		recreator.append(token->optionName() + ' ');
	}
}


void Assign::recreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	std::stack<std::string> stack;

	stack.emplace(recreator.popString());  // push value
	std::string separator = ' ' + Table::entry(Code::Equal)->name() + ' ';
	while (!recreator.empty())
	{
		stack.emplace(recreator.popString() + separator);
		separator = ", ";
	}
	recreateLet(recreator, rpnItem->token());
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
		string = ' ' + Table::entry(Code::Equal)->name() + ' ';
		recreator.setSeparator(',');
	}
	else  // continuation of assignment list
	{
		string = recreator.separator();
		string += ' ';
	}
	string.append(recreator.popString());

	Table *entry {rpnItem->token()->tableEntry()};
	if (entry->hasFlag(SubStr_Flag))
	{
		// for sub-string assignments, treat as function to recreate
		recreator.pushWithOperands(std::string{entry->name()},
			entry->operandCount());
	}

	// deterine if assignment is an sub-string assignment keep code
	if (entry->hasFlag(Keep_Flag))
	{
		// for keep codes, append string so far to reference string on stack
		recreator.topAppend(std::move(string));
	}
	else
	{
		// end of statement, append reference and string so far
		Assign::recreateLet(recreator, rpnItem->token());
		recreator.append(recreator.popString());
		recreator.append(std::move(string));
		recreator.clearSeparator();  // for next command
	}
}

void AssignStrBase::recreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	assignStrRecreate(recreator, rpnItem);
}


//=========================
//  TABLE ENTRY INSTANCES
//=========================

Let let;

AssignDbl assignDbl {&let};
AssignInt assignInt {&assignDbl};
AssignStr assignStr {&assignDbl};

AssignListDbl assignListDbl {{&assignDbl, 1}};
AssignListInt assignListInt {{&assignInt, 1}};
AssignListStr assignListStr {{&assignStr, 1}};

AssignKeepStr assignKeepStr {&assignStr};


// end: let.cpp
