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

#include <stack>

#include <QList>

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
	Token::Status getCommands(TokenPtr &token);
	Token::Status getExpression(TokenPtr &token, DataType dataType,
		int level = 0);
	Token::Status getOperand(TokenPtr &token, DataType dataType,
		Reference reference = Reference::None);
	Token::Status getToken(TokenPtr &token, DataType dataType = DataType{});

	// Public Processing Functions
	Token::Status processFinalOperand(TokenPtr &token,
		TokenPtr token2 = TokenPtr{}, int operandIndex = 0);
	Token::Status processDoneStackTop(TokenPtr &token, int operandIndex = 0,
		TokenPtr *first = nullptr, TokenPtr *last = nullptr);

	// Public Support Functions
	static Token::Status expectedErrStatus(DataType dataType,
		Reference reference = Reference::None);

	// Table Access Function
	Table &table(void) const
	{
		return m_table;
	}

	// Done Stack Access Functions
	void doneStackPop(void)
	{
		m_doneStack.pop();
	}
	TokenPtr doneStackTopToken(void) const
	{
		return m_doneStack.top().rpnItem->token();
	}
	bool doneStackEmpty(void)
	{
		return m_doneStack.empty();
	}
	TokenPtr doneStackPopErrorToken(void);

	// Output List Access Functions
	int outputCount(void) const
	{
		return m_output.count();
	}
	TokenPtr outputLastToken(void) const
	{
		return m_output.lastToken();
	}
	void outputAppend(TokenPtr token)
	{
		m_output.append(std::move(token));
	}
	RpnItemList::iterator outputInsert(RpnItemList::iterator iterator,
		TokenPtr token)
	{
		return m_output.insert(iterator, token);
	}
	RpnItemList::iterator outputAppendIterator(TokenPtr token)
	{
		return m_output.appendIterator(std::move(token));
	}

private:
	// Private Processing Functions
	Token::Status processCommand(TokenPtr &commandToken);
	Token::Status processInternalFunction(TokenPtr &token);
	Token::Status processParenToken(TokenPtr &token);
	Token::Status processOperator(TokenPtr &token);
	Token::Status processFirstOperand(TokenPtr &token);

	// Private Support Functions
	enum class Popped {No, Yes};
	void checkPendingParen(const TokenPtr &token, Popped popped);
	void cleanUp(void);		// only called when error occurs

	struct HoldItem
	{
		HoldItem(TokenPtr _token) : token{_token}, first{} {}
		HoldItem(TokenPtr _token, TokenPtr _first) : token{_token},
			first{_first} {}

		TokenPtr token;				// token pointer on hold stack
		TokenPtr first;				// operator token's first operand pointer
	};
	using HoldStack = std::stack<HoldItem>;

	struct DoneItem
	{
		DoneItem(RpnItemPtr _rpnItem) : rpnItem{_rpnItem}, first{}, last{} {}
		DoneItem(RpnItemPtr _rpnItem, TokenPtr _last) : rpnItem{_rpnItem},
			first{}, last{_last} {}
		DoneItem(RpnItemPtr _rpnItem, TokenPtr _first, TokenPtr _last) :
			rpnItem{_rpnItem}, first{_first}, last{_last} {}

		// replace the item's first and last operand token
		void replaceFirstLast(TokenPtr _first, TokenPtr _last)
		{
			first = _first;
			last = _last;
		}

		RpnItemPtr rpnItem;			// pointer to RPN item
		TokenPtr first;				// operator token's first operand pointer
		TokenPtr last;				// operator token's last operand pointer
	};
	using DoneStack = std::stack<DoneItem>;

	Table &m_table;					// reference to the table instance
	Parser *m_parser;				// pointer to parser instance
	RpnList m_output;				// pointer to RPN list output
	HoldStack m_holdStack;			// operator/function holding stack
	DoneStack m_doneStack;			// items processed stack
	TokenPtr m_pendingParen;		// closing parentheses token is pending
	int m_lastPrecedence;			// precedence of last op added during paren
};


#endif  // TRANSLATOR_H
