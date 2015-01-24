// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: recreator.cpp - recreator class source file
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
//	2013-11-02	initial version

#include "recreator.h"
#include "rpnlist.h"
#include "table.h"
#include "token.h"
#include "basic/basic.h"


Recreator::Recreator() :
	m_separator {}
{

}


// function to recreate original program text from an rpn list
std::string Recreator::operator()(const RpnList &rpnList, bool exprMode)
{
	for (RpnItemPtr rpnItem : rpnList)
	{
		RecreateFunction recreate;
		if (!(recreate = rpnItem->token()->tableEntry()->recreateFunction()))
		{
			// if no recreate function, then it is missing from table
			emplace('?' + rpnItem->token()->string() + '?');
		}
		else  // call recreate function for code
		{
			recreate(*this, rpnItem);
		}
		if (!rpnItem->token()->hasFlag(Command_Flag)
			&& rpnItem->token()->hasSubCode(Paren_SubCode))
		{
			parenRecreate(*this, rpnItem);
		}
		if (rpnItem->token()->hasFlag(Command_Flag)
			&& rpnItem->token()->hasSubCode(Colon_SubCode))
		{
			// FLAG option: spaces before colons (default=no)
			m_output += ':';
			// FLAG option: spaces after colons (default=yes)
			m_output += ' ';
		}
	}
	if (exprMode)
	{
		m_output += popString();
	}
	while (!m_stack.empty())  // stack empty error check
	{
		m_output += " NotEmpty:" + popString();
	}
	return std::move(m_output);
}


// function to get an operand from the top of the stack
// (surround operand with parentheses if requested)
std::string Recreator::popWithParens(bool addParens)
{
	std::string string;

	if (addParens)
	{
		string += '(';
	}
	string += popString();
	if (addParens)
	{
		string += ')';
	}
	return string;
}


// function to push an item with all of operands popped from the stack
// (used for functions and arrays; also work with no operand functions)
void Recreator::pushWithOperands(std::string &&name, int count)
{
	std::stack<std::string> stack;		// local stack of operands

	std::string separator {")"};
	while (--count >= 0)
	{
		std::string string {popString()};
		stack.emplace(string + separator);
		separator = ", ";
	}
	while (!stack.empty())
	{
		name += stack.top();
		stack.pop();
	}
	emplace(name);
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                      GENERAL TABLE RECREATE FUNCTIONS                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


// function to recreate an operand
void operandRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	// just push the string of the token
	recreator.emplace(rpnItem->token()->stringWithDataType());
}

// function to recreate a unary operator
void unaryOperatorRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	int precedence {rpnItem->token()->precedence()};

	// get string of operand from stack
	// (add parens if item on top of the stack is not a unary operator
	// and operator precendence is higher than the operand)
	std::string operand {recreator.popWithParens(!recreator.topUnaryOperator()
		&& precedence > recreator.topPrecedence())};

	// get string for operator
	std::string string {rpnItem->token()->name()};
	// if operator is a plain word operator or operand is a number,
	//  then need to add a space
	if (isalpha(string.back())
		|| isdigit(operand.front()) || operand.front() == '.')
	{
		string += ' ';
	}
	// append operand and push to holding stack
	string += operand;

	// push operator expression back to stack with precedence of operator
	recreator.emplace(string, precedence, true);
}


// function to recreate a binary operator
void binaryOperatorRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	std::string string;
	int precedence {rpnItem->token()->precedence()};

	// get string of second operand
	// (add parens if operator precedence is higher than or same as operand
	// and operand is not a unary operator)
	string = recreator.popWithParens(precedence >= recreator.topPrecedence()
		&& !recreator.topUnaryOperator());

	// get string of operator with spaces, append to first operand
	// (add parens if operator precendence is higher than the operand)
	string = recreator.popWithParens(precedence > recreator.topPrecedence())
		+ ' ' + rpnItem->token()->name() + ' ' + string;

	// push operator expression back to stack with precedence of operator
	recreator.emplace(string, precedence);
}


// function to surround item on top of the holding stack with parentheses
void parenRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	(void)rpnItem;

	recreator.topAddParens();
}


// function to recreate an internal function
void internalFunctionRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	Table *entry {rpnItem->token()->tableEntry()};
	recreator.pushWithOperands(entry->name(), entry->operandCount());
}


// function to recreate an array
void arrayRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	// add data type and close paren since it is not stored with name
	std::string string = rpnItem->token()->stringWithDataType();
	string.push_back('(');
	recreator.pushWithOperands(std::move(string), rpnItem->attachedCount());
}


// function to recreate an array
void functionRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	// add data type and close paren since it is not stored with name
	std::string string = rpnItem->token()->stringWithDataType();
	string.push_back('(');
	recreator.pushWithOperands(std::move(string), rpnItem->attachedCount());
}


// function to recreate an array
void defineFunctionRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	std::string name {rpnItem->token()->stringWithDataType()};
	int count {rpnItem->attachedCount()};
	if (count > 0)
	{
		name += '(';  // add paren since it is not stored with name
	}
	recreator.pushWithOperands(std::move(name), count);
}


// function to do nothing (for hidden codes)
void blankRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	(void)recreator;
	(void)rpnItem;
}


// function to do nothing (for hidden codes)
void remRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	std::string string {rpnItem->token()->name()};
	std::string remark {rpnItem->token()->string()};
	if (islower(remark.front()))
	{
		std::transform(string.begin(), string.end(), string.begin(), tolower);
	}
	if (rpnItem->token()->isCode(Code::RemOp) && recreator.backIsNotSpace())
	{
		// FLAG option: space before rem operator (default=yes)
		recreator.append(' ');
	}
	recreator.append(std::move(string));
	recreator.append(std::move(remark));
}


// end: recreator.cpp
