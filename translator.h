// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: translator.h - translator class header file
//	Copyright (C) 2012-2015  Thunder422
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

#include "parser.h"
#include "rpnlist.h"

class Table;
class TableEntry;
class TokenError;


class Translator
{
public:
	explicit Translator(const std::string &input);

	enum class TestMode {
		No,				// do normal translation
		Expression,		// translate as expression only
		Yes				// don't set code size after translation
	};
	using Operands = std::pair<TokenPtr, TokenPtr>;

	// Main Function
	RpnList operator()(TestMode testMode = {});

	// Get Functions
	void getCommands();
	void getExpression(DataType dataType, int level = 0);
	bool getOperand(DataType dataType, Reference reference = Reference::None);
	void getToken(Status errorStatus, DataType dataType = DataType{},
		Reference reference = Reference::None);

	// Public Processing Functions
	void processFinalOperand(TokenPtr &token, TokenPtr &&first = {});
	Operands processDoneStackTop(TokenPtr &token, int operandIndex = 0);

	// Public Support Functions
	static Status expectedErrorStatus(DataType dataType,
		Reference reference = Reference::None) noexcept;

	// Current Token Access Functions
	const TokenPtr &token() const
	{
		return m_token;
	}
	void resetToken()
	{
		m_token.reset();
	}
	TokenPtr moveToken()
	{
		return std::move(m_token);
	}

	// Done Stack Access Functions
	void doneStackPop()
	{
		m_doneStack.pop();
	}
	TokenPtr doneStackTopToken() const
	{
		return m_doneStack.top().rpnItem->token();
	}
	bool doneStackEmpty()
	{
		return m_doneStack.empty();
	}
	TokenError doneStackTopTokenError(Status errorStatus) noexcept;

	// Output List Access Functions
	int outputCount() const
	{
		return m_output.count();
	}
	TokenPtr outputLastToken() const
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
	void processInternalFunction(Reference reference);
	void processParenToken();
	bool processOperator();

	// Private Support Functions
	enum class Popped {No, Yes};
	void checkPendingParen(const TokenPtr &token, Popped popped) noexcept;
	Status expressionErrorStatus(bool lastOperand, bool unaryOperator,
		TableEntry *entry) noexcept;

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
		DoneItem(RpnItemPtr _rpnItem) : rpnItem{_rpnItem}, operands{} {}
		DoneItem(RpnItemPtr _rpnItem, TokenPtr second) : rpnItem{_rpnItem},
			operands{{}, second} {}
		DoneItem(RpnItemPtr _rpnItem, TokenPtr first, TokenPtr second) :
			rpnItem{_rpnItem}, operands{first, second} {}
		DoneItem(RpnItemPtr _rpnItem, Operands &&_operands) : rpnItem{_rpnItem},
			operands{_operands} {}

		void replaceOperands(TokenPtr first, TokenPtr second)
		{
			operands.first = first;
			operands.second = second;
		}

		RpnItemPtr rpnItem;			// pointer to RPN item
		Operands operands;			// first/second operand of RPN item
	};
	using DoneStack = std::stack<DoneItem>;

	std::unique_ptr<Parser> m_parse;	// pointer to parser instance
	RpnList m_output;				// pointer to RPN list output
	HoldStack m_holdStack;			// operator/function holding stack
	DoneStack m_doneStack;			// items processed stack
	TokenPtr m_token;				// current token being processed
	TokenPtr m_pendingParen;		// closing parentheses token is pending
	int m_lastPrecedence;			// precedence of last op added during paren
};


#endif  // TRANSLATOR_H
