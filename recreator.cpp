// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: recreator.cpp - recreator class source file
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
//	2013-11-02	initial version

#include "recreator.h"
#include "rpnlist.h"
#include "table.h"
#include "basic/basic.h"


Recreator::Recreator(void) :
	m_table(Table::instance())
{

}


// function to recreate original program text from an rpn list
QString Recreator::recreate(RpnList *rpnList, bool exprMode)
{
	m_output.clear();
	for (int i = 0; i < rpnList->count(); i++)
	{
		RpnItem *rpnItem = rpnList->at(i);
		RecreateFunction recreate;
		if (!rpnItem->token()->hasValidCode()
			|| (recreate = m_table.recreateFunction(rpnItem->token()->code()))
			== NULL)
		{
			// if no recreate function, then it is missing from table
			push('?' + rpnItem->token()->string() + '?');
		}
		else  // call recreate function for code
		{
			recreate(*this, rpnItem);
		}
		if (rpnItem->token()->hasSubCode(Paren_SubCode))
		{
			parenRecreate(*this, rpnItem);
		}
	}
	if (exprMode)
	{
		append(pop());
	}
	while (!m_stack.isEmpty())  // stack empty error check
	{
		append(QString(" <NotEmpty:%1>").arg(pop()));
	}
	return m_output;
}


// function to push a string with optional precedence to holding stack
void Recreator::push(QString string, int precedence, bool unaryOperator)
{
	m_stack.resize(m_stack.size() + 1);
	m_stack.top().string = string;
	m_stack.top().precedence = precedence;
	m_stack.top().unaryOperator = unaryOperator;
}


// function to pop the string on holding stack top, optional return precedence
QString Recreator::pop(void)
{
    return m_stack.isEmpty() ? "<Empty>" : m_stack.pop().string;
}


// function to return the reference to the top item of the stack
const Recreator::StackItem &Recreator::top(void) const
{
	return m_stack.top();
}


// function to append a string to the top and optionally set precedence
void Recreator::topAppend(QString string)
{
	m_stack.top().string.append(string);
}


// function to append a string to the output string
void Recreator::append(QString string)
{
	m_output.append(string);
}


// function to get an operand from the top of the stack
// (surround operand with parentheses if requested
QString Recreator::popWithParens(bool addParens, int *precedence,
	bool *unaryOperator)
{
	QString string;
	if (addParens)
	{
		string.append('(');
	}
	if (precedence)
	{
		*precedence = top().precedence;
	}
	if (unaryOperator)
	{
		*unaryOperator = top().unaryOperator;
	}
	string.append(pop());
	if (addParens)
	{
		string.append(')');
	}
	return string;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                      GENERAL TABLE RECREATE FUNCTIONS                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


// function to recreate an operand
void operandRecreate(Recreator &recreator, RpnItem *rpnItem)
{
	// just push the string of the token
	recreator.push(rpnItem->token()->string());
}

// function to recreate a unary operator
void unaryOperatorRecreate(Recreator &recreator, RpnItem *rpnItem)
{
	int precedence = recreator.table().precedence(rpnItem->token()->code());

	// get string for operator
	QString string = recreator.table().name(rpnItem->token());
	// if operator is a plain word operator, then need to add a space
	if (rpnItem->token()->code() < EndPlainWord_Code)
	{
		string.append(' ');
	}
	// append operand and push to holding stack
	// (add parens if item on top of the stack is not a unary operator
	// and operator precendence is higher than the operand)
	string.append(recreator.popWithParens(!recreator.top().unaryOperator
		&& precedence > recreator.top().precedence));

	// push operator expression back to stack with precedence of operator
	recreator.push(string, precedence, true);
}


// function to recreate a binary operator
void binaryOperatorRecreate(Recreator &recreator, RpnItem *rpnItem)
{
	QString string;
	int precedence = recreator.table().precedence(rpnItem->token()->code());

	// get string of second operand
	// (add parens if operator precedence is higher than or same as operand)
	string = recreator.popWithParens(precedence >= recreator.top().precedence);

	// get string of operator with spaces, append to first operand
	// (add parens if operator precendence is higher than the operand)
	string = recreator.popWithParens(precedence > recreator.top().precedence)
		+ ' ' + recreator.table().name(rpnItem->token()) + ' ' + string;

	// push operator expression back to stack with precedence of operator
	recreator.push(string, precedence);
}


// function to surround item on top of the holding stack with parentheses
void parenRecreate(Recreator &recreator, RpnItem *rpnItem)
{
	int precedence;
	bool unaryOperator;

	QString string = recreator.popWithParens(true, &precedence, &unaryOperator);
	recreator.push(string, precedence, unaryOperator);
}


// end: recreator.cpp
