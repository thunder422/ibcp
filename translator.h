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
#include <QStack>

#include "donestack.h"
#include "rpnlist.h"

class Table;
class Parser;


class Translator
{
	struct HoldItem
	{
		Token *token;				// token pointer on hold stack
		Token *first;				// operator token's first operand pointer
	};
	class HoldStack : public QStack<HoldItem>
	{
	public:
		// drop the top item on stask (pop with no return)
		void drop(void)
		{
			resize(size() - 1);
		}
		// push new item with token and first token
		void push(Token *token, Token *first = NULL)
		{
			resize(size() + 1);
			top().token = token;
			top().first = first;
		}
	};

	Table &m_table;					// reference to the table instance
	Parser *m_parser;				// pointer to parser instance
	RpnList *m_output;				// pointer to RPN list output
	HoldStack m_holdStack;			// operator/function holding stack
	DoneStack m_doneStack;			// items processed stack
	Token *m_pendingParen;			// closing parentheses token is pending
	int m_lastPrecedence;			// precedence of last op added during paren
	bool m_exprMode;				// expression only mode active flag

public:
	explicit Translator(Table &table);
	~Translator(void);

	enum Reference {
		None_Reference,
		Variable_Reference,
		VarDefFn_Reference,
		All_Reference,
		sizeof_Reference
	};

	// New Translator Functions
	RpnList *translate2(const QString &input, bool exprMode = false);
	TokenStatus getCommands(Token *&token);
	TokenStatus getExpression(Token *&token, DataType dataType, int level = 0);
	TokenStatus getOperand(Token *&token, DataType dataType,
		Reference reference = None_Reference);
	TokenStatus getInternalFunction(Token *&token);
	TokenStatus getParenToken(Token *&token);
	TokenStatus getToken(Token *&token, DataType dataType = No_DataType);

	// Main Processing Functions
	TokenStatus processFinalOperand(Token *&token, Token *token2 = NULL,
		int operandIndex = 0);

	// Support Functions
	TokenStatus processDoneStackTop(Token *&token, int operandIndex = 0,
		Token **first = NULL, Token **last = NULL);

	// Determine Error Funtions (By DataType)
	static DataType equivalentDataType(DataType dataType);

	// Access Functions
	Table &table(void) const
	{
		return m_table;
	}
	RpnItem *doneStackPop(void)
	{
		return m_doneStack.pop();
	}
	void doneStackDrop(void)
	{
		 m_doneStack.drop();
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

	int outputCount(void) const
	{
		return m_output->count();
	}
	Token *outputLastToken(void) const
	{
		return m_output->last()->token();
	}
	RpnItem *outputAppend(Token *token, int nOperands = 0,
		RpnItem **operand = NULL)
	{
		token->clearSubCodeMask(UnUsed_SubCode);  // mark as used
		RpnItem *rpnItem = new RpnItem(token, nOperands, operand);
		m_output->append(rpnItem);
		return rpnItem;
	}
	void outputInsert(int index, Token *token)
	{
		m_output->insert(index, new RpnItem(token));
	}

	// Determine Error Funtions (By DataType)
	static TokenStatus expectedErrStatus(DataType dataType,
		Reference reference = None_Reference);

private:
	// New Translator Functions
	TokenStatus processCommand(Token *&commandToken);
	TokenStatus processOperator2(Token *&token);
	void checkPendingParen(Token *token, bool popped);

	// Main Processing Functions
	TokenStatus processFirstOperand(Token *&token);

	// Support Functions
	void cleanUp(void);		// only call when addToken() returns error
};


#endif  // TRANSLATOR_H
