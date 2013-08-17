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


// command stack item flag values
//   some flags are used for all commands and some are used only for
//   specific commands, the values of command specific flags may be reused
//   for different commands so each flag will be assigned a value

enum {
	// FLAGS FOR ALL COMMANDS
	None_CmdFlag			= 0x00000000,	// initial value of command flag

	// FLAGS FOR PRINT COMMAND
	PrintStay_CmdFlag		= 0x00010000,	// PRINT stay on line flag
	PrintFunc_CmdFlag		= 0x00020000,	// print function flag

	// FLAGS FOR ASSIGNMENT COMMANDS
	AssignList_CmdFlag		= 0x00010000,	// currently an assign list

	// FLAGS FOR INPUT COMMAND
	InputBegin_CmdFlag		= 0x00010000,	// beginning processed
	InputStay_CmdFlag		= 0x00020000	// INPUT stay on line flag
};


// command item
struct CmdItem
{
	Token *token;				// pointer to command token
	int flag;					// generic flag for command use
	int index;  				// index into output list for command use
};


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

	struct CountItem
	{
		char nOperands;				// number of operands seen
		char nExpected;				// number of arguments expected
		Code code;					// table index of internal function
	};
	enum State
	{
		Initial_State,				// initial state
		BinOp_State,				// expecting binary operator
		Operand_State,				// expecting unary operator or operand
		OperandOrEnd_State,			// expecting operand or end
		EndExpr_State,				// expecting end of expression
		EndStmt_State,				// expecting end of statement
		UnSet_State,				// FIXME used to indicate new translator
		sizeof_State
	} m_state;						// current state of translator

	Table &m_table;					// reference to the table instance
	Parser *m_parser;				// pointer to parser instance
	RpnList *m_output;				// pointer to RPN list output
	HoldStack m_holdStack;			// operator/function holding stack
	DoneStack m_doneStack;			// items processed stack
	Token *m_pendingParen;			// closing parentheses token is pending
	int m_lastPrecedence;			// precedence of last op added during paren
	QStack<CountItem> m_countStack;	// number of operands counter stack
	TokenMode m_mode;				// current assignment mode
	QStack<CmdItem> m_cmdStack;		// stack of commands waiting processing
	bool m_exprMode;				// expression only mode active flag

public:
	explicit Translator(Table &table);
	~Translator(void);

	enum Reference {
		None_Reference,
		Variable_Reference,
		All_Reference,
		sizeof_Reference
	};

	RpnList *translate(const QString &input, bool exprMode = false);
	// New Translator Functions
	RpnList *translate2(const QString &input, bool exprMode);
	TokenStatus getExpression(Token *&token, DataType dataType, int level = 0);
	TokenStatus getOperand(Token *&token, DataType dataType,
		Reference reference = None_Reference);
	TokenStatus getInternalFunction(Token *&token);
	TokenStatus getParenToken(Token *&token);
	TokenStatus getToken(Token *&token, DataType dataType = No_DataType);

	// Main Processing Functions
	TokenStatus processFinalOperand(Token *&token, Token *token2,
		int operandIndex, int nOperands = 0);

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
	Token *doneStackPopErrorToken(void);

	// Determine Error Funtions (By DataType)
	static TokenStatus expectedErrStatus(DataType dataType,
		Reference reference = None_Reference);
	static TokenStatus variableErrStatus(DataType dataType)
	{	// FIXME remove with old translator routines
		return expectedErrStatus(dataType, All_Reference);
	}

private:
	enum Match
	{
		No_Match,
		Yes_Match,
		Cvt_Match,
		sizeof_Match
	};

	// New Translator Functions
	TokenStatus getCommand(Token *&token);
	TokenStatus processOperator2(Token *&token);
	void checkPendingParen(Token *token, bool popped);

	// Main Processing Functions
	TokenStatus addToken(Token *&token);
	TokenStatus processOperand(Token *&token);
	bool processUnaryOperator(Token *&token, TokenStatus &status);
	TokenStatus processBinaryOperator(Token *&token);
	TokenStatus processOperator(Token *&token);
	TokenStatus processFirstOperand(Token *&token);
	TokenStatus expressionEnd(void);

	// Support Functions
	TokenStatus callCommandHandler(Token *&token);
	TokenStatus getExprDataType(DataType &dataType) const;
	TokenStatus parenStatus(void) const;
	void doPendingParen(Token *token);
	void cleanUp(void);		// only call when addToken() returns error

	// Command Specific Functions
	TokenStatus addPrintCode(void);
	TokenStatus checkAssignListToken(Token *&token);
	TokenStatus setAssignCommand(Token *&token, Code assign_code);

	// Determine Error Funtions (By DataType)
	static TokenStatus actualErrStatus(DataType dataType);

	// Determine Error Functions (By Current State)
	TokenStatus operatorError(void) const;
	TokenStatus assignmentError(void) const;
	TokenStatus endExpressionError(void) const;
	TokenStatus unexpectedEndError(void) const;

	// set error token (deleting any previous error token first)
	// FIXME remove this, only used by old translator
	void setErrorToken(Token *errorToken)
	{
		m_output->setError(errorToken);
		delete errorToken;
	}

public:
	// token handler friend function definitions section
	friend TokenStatus Operator_Handler(Translator &t, Token *&token);
	friend TokenStatus Equal_Handler(Translator &t, Token *&token);
	friend TokenStatus Comma_Handler(Translator &t, Token *&token);
	friend TokenStatus CloseParen_Handler(Translator &t, Token *&token);
	friend TokenStatus EndOfLine_Handler(Translator &t, Token *&token);
	friend TokenStatus SemiColon_Handler(Translator &t, Token *&token);
	friend TokenStatus RemOp_Handler(Translator &t, Token *&token);

	// command handler friend function definitions section
	friend TokenStatus Assign_CmdHandler(Translator &t, CmdItem *cmdItem,
		Token *token);
	friend TokenStatus Print_CmdHandler(Translator &t, CmdItem *cmdItem,
		Token *token);
	friend TokenStatus Let_CmdHandler(Translator &t, CmdItem *cmdItem,
		Token *token);
	friend TokenStatus Input_CmdHandler(Translator &t, CmdItem *cmdItem,
		Token *token);
	friend TokenStatus Rem_CmdHandler(Translator &t, CmdItem *cmdItem,
		Token *token);
};


#endif  // TRANSLATOR_H
