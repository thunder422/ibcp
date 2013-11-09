// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: recreator.h - recreator class header file
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

#ifndef RECREATOR_H
#define RECREATOR_H

#include <QStack>
#include <QString>

#include "ibcp.h"

class RpnList;
class Table;


class Recreator
{
	struct StackItem
	{
		QString string;				// string of stack item
		int precedence;				// precedence of stack item
		bool unaryOperator;			// stack item is a unary operator
	};

public:
	explicit Recreator(void);

	QString recreate(RpnList *rpnList, bool exprMode = false);

	void push(QString string, int precedence = HighestPrecedence,
		bool unaryOperator = false);
	QString pop(void);
	const StackItem &top(void) const;
	void topAppend(QString string);
	void append(QString string);
	QString popWithParens(bool addParens, int *precedence = NULL,
		bool *unaryOperator = NULL);
	void pushWithOperands(QString &name, int count);

	const Table &table(void) const
	{
		return m_table;
	}

private:
	Table &m_table;					// reference to table instance
	QStack<StackItem> m_stack;		// holding string stack
	QString m_output;				// output string
};

#endif // RECREATOR_H
