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

#include <stack>
#include <string>

#include "ibcp.h"

class RpnList;
class Table;


class Recreator
{
	struct StackItem
	{
		StackItem(std::string string, int precedence = HighestPrecedence,
			bool unaryOperator = false) : m_string {string},
			m_precedence {precedence}, m_unaryOperator {unaryOperator} {}

		std::string m_string;			// string of stack item
		int m_precedence;				// precedence of stack item
		bool m_unaryOperator;			// stack item is a unary operator
	};

public:
	explicit Recreator(void);

	std::string recreate(const RpnList &rpnList, bool exprMode = false);

	// holding stack access functions
	template <typename... Args>
	void emplace(Args&&... args)
	{
		m_stack.emplace(std::forward<Args>(args)...);
	}
	std::string popString()
	{
		std::string string = std::move(m_stack.top().m_string);
		m_stack.pop();
		return string;
	}
	int topPrecedence(void) const
	{
		return m_stack.top().m_precedence;
	}
	bool topUnaryOperator(void) const
	{
		return m_stack.top().m_unaryOperator;
	}
	void topAppend(std::string &&string)
	{
		m_stack.top().m_string += string;
	}
	void topAppend(char c)
	{
		m_stack.top().m_string += c;
	}
	void topAddParens()
	{
		m_stack.top().m_string = '(' + std::move(m_stack.top().m_string) + ')';
	}
	std::string popWithParens(bool addParens);
	void pushWithOperands(std::string &&name, int count);
	bool empty() const
	{
		return m_stack.empty();
	}

	// output string access functions
	void append(std::string &&string)
	{
		m_output += string;
	}
	void append(char c)
	{
		m_output += c;
	}
	bool backIsNotSpace()
	{
		return !m_output.empty() && m_output.back() != ' ';
	}

	// separator access functions
	char separator() const
	{
		return m_separator;
	}
	bool separatorIsSet() const
	{
		return m_separator != '\0';
	}
	bool separatorIsSet(char separator) const
	{
		return m_separator == separator;
	}
	void clearSeparator()
	{
		m_separator = '\0';
	}
	void setSeparator(char separator)
	{
		m_separator = separator;
	}

	// table instance access function
	const Table &table(void) const
	{
		return m_table;
	}

private:
	Table &m_table;					// reference to table instance
	std::stack<StackItem> m_stack;	// holding string stack
	char m_separator;				// current separator character
	std::string m_output;			// output string
};


#endif // RECREATOR_H
