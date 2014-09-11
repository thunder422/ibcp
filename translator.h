// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: translator.h - translator class header file
//	Copyright (C) 2012-2013  Thunder422
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
//	2012-11-03	initial version (parts removed from ibcp.h)

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QList>

#include "donestack.h"
#include "rpnlist.h"

class Table;
class Parser;


class Translator
{
public:
	explicit Translator(void);
	~Translator(void);

	enum class TestMode {
		No,				// do normal translation
		Expression,		// translate as expression only
		Yes				// don't set code size after translation
	};

	enum class Reference {
		None,
		Variable,
		VarDefFn,
		All
	};

	// Main Function
	RpnList translate(const QString &input, TestMode testMode = TestMode::No);

	// Get Functions
	Token::Status getCommands(Token *&token);
	Token::Status getExpression(Token *&token, DataType dataType,
		int level = 0);
	Token::Status getOperand(Token *&token, DataType dataType,
		Reference reference = Reference::None);
	Token::Status getToken(Token *&token, DataType dataType = DataType{});

	// Public Processing Functions
	Token::Status processFinalOperand(Token *&token, Token *token2 = NULL,
		int operandIndex = 0);
	Token::Status processDoneStackTop(Token *&token, int operandIndex = 0,
		Token **first = NULL, Token **last = NULL);

	// Public Support Functions
	static Token::Status expectedErrStatus(DataType dataType,
		Reference reference = Reference::None);

	// Table Access Function
	Table &table(void) const
	{
		return m_table;
	}

	// Done Stack Access Functions
	RpnItemPtr doneStackPop(void)
	{
		RpnItemPtr rpnItem = m_doneStack.top().rpnItem;
		m_doneStack.pop();
		return rpnItem;
	}
	void doneStackDrop(void)
	{
		 m_doneStack.pop();
	}
	Token *doneStackTopToken(void) const
	{
		return m_doneStack.top().rpnItem->token();
	}
	bool doneStackEmpty(void)
	{
		return m_doneStack.empty();
	}
	Token *doneStackPopErrorToken(void);

	// Output List Access Functions
	int outputCount(void) const
	{
		return m_output.count();
	}
	Token *outputLastToken(void) const
	{
		return m_output.lastToken();
	}
	RpnItemPtr outputAppend(Token *token)
	{
		return m_output.append(token);
	}
	RpnItemList::iterator outputInsert(RpnItemList::iterator iterator,
		Token *token)
	{
		return m_output.insert(iterator, token);
	}
	RpnItemList::iterator outputAppendIterator(Token *token)
	{
		return m_output.appendIterator(token);
	}

private:
	// Private Processing Functions
	Token::Status processCommand(Token *&commandToken);
	Token::Status processInternalFunction(Token *&token);
	Token::Status processParenToken(Token *&token);
	Token::Status processOperator(Token *&token);
	Token::Status processFirstOperand(Token *&token);

	// Private Support Functions
	void checkPendingParen(Token *token, bool popped);
	void cleanUp(void);		// only called when error occurs

	// Output List Function
	Token::Status outputAssignCodes(Token *&token);

	struct HoldItem
	{
		HoldItem(Token *_token, Token *_first = nullptr) : token{_token},
			first{_first} {}

		Token *token;				// token pointer on hold stack
		Token *first;				// operator token's first operand pointer
	};
	using HoldStack = std::stack<HoldItem>;

	Table &m_table;					// reference to the table instance
	Parser *m_parser;				// pointer to parser instance
	RpnList m_output;				// pointer to RPN list output
	HoldStack m_holdStack;			// operator/function holding stack
	DoneStack m_doneStack;			// items processed stack
	Token *m_pendingParen;			// closing parentheses token is pending
	int m_lastPrecedence;			// precedence of last op added during paren
};


#endif  // TRANSLATOR_H
