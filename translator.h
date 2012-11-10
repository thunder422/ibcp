// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: translator.h - translator class definitions file
//	Copyright (C) 2012  Thunder422
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

#include "token.h"

class Table;


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
struct CmdItem {
	Token *token;				// pointer to command token
	int flag;					// generic flag for command use
	int index;  				// index into output list for command use
};


// structure for holding RPN output list information
class RpnItem {
	Token *m_token;				// pointer to token
	int m_nOperands;			// number of operands
	RpnItem **m_operand;		// array of operand pointers

public:
	RpnItem(Token *token, int nOperands = 0, RpnItem **operand = NULL)
	{
		m_token = token;
		m_nOperands = nOperands;
		m_operand = operand;
	}
	~RpnItem()
	{
		delete m_token;
		if (m_nOperands > 0)
		{
			delete[] m_operand;
		}
	}

	// access functions
	Token *token(void)
	{
		return m_token;
	}
	void setToken(Token *token)
	{
		m_token = token;
	}

	int nOperands(void)
	{
		return m_nOperands;
	}
	void setNOperands(int nOperands)
	{
		m_nOperands = nOperands;
	}

	RpnItem **operand(void)
	{
		return m_operand;
	}
	void setOperand(RpnItem **operand)
	{
		m_operand = operand;
	}
	RpnItem *operand(int index)
	{
		return m_operand[index];
	}
	void setOperand(int index, RpnItem *operand)
	{
		m_operand[index] = operand;
	}

	// function to set operands without allocating a new array
	void set(int nOperands, RpnItem **operand)
	{
		m_nOperands = nOperands;
		m_operand = operand;
	}
};


class Translator {
	struct HoldItem {
		Token *token;				// token pointer on hold stack
		Token *first;				// operator token's first operand pointer
	};
	struct DoneItem {
		RpnItem *rpnItem;			// pointer to RPN item
		Token *first;				// operator token's first operand pointer
		Token *last;				// operator token's last operand pointer
	};
	struct CountItem {
		char nOperands;				// number of operands seen
		char nExpected;				// number of arguments expected
		Code code;					// table index of internal function
	};
	enum State {
		Initial_State,				// initial state
		BinOp_State,				// expecting binary operator
		Operand_State,				// expecting unary operator or operand
		OperandOrEnd_State,			// expecting operand or end (2010-06-10)
		EndExpr_State,				// expecting end of expression (2011-03-05)
        EndStmt_State,				// expecting end of statement (2011-03-19)
		sizeof_State
	} m_state;						// current state of translator

	Table &m_table;					// reference to the table instance
	QList<RpnItem *> *m_output;		// pointer to RPN list output
	QStack<HoldItem> m_holdStack;	// operator/function holding stack
	QStack<DoneItem> m_doneStack;	// items processed stack
	Token *m_pendingParen;			// closing parentheses token is pending
	int m_lastPrecedence;			// precedence of last op added during paren
	QStack<CountItem> m_countStack;	// number of operands counter stack
	TokenMode m_mode;				// current assignment mode
	QStack<CmdItem> m_cmdStack;		// stack of commands waiting processing
	bool m_exprMode;				// expression only mode active flag
	Token *m_errorToken;			// token when error occurred
	QString m_errorMessage;			// message of error that occurred

public:
	Translator(Table &table): m_table(table), m_output(NULL),
	    m_pendingParen(NULL), m_errorToken(NULL) {}
	~Translator(void)
	{
		if (m_errorToken != NULL)
		{
			delete m_errorToken;
		}
	}

	bool setInput(const QString &input, bool exprMode = false);
	QList<RpnItem *> *output(void)	// only call when setInput() returns true
	{
		QList<RpnItem *> *list = m_output;
		m_output = NULL;
		return list;
	}
	Token *errorToken(void)     // only call when setInput() returns false
	{
		return m_errorToken;
	}
	QString errorMessage(void)	// only call when setInput() returns false
	{
		return m_errorMessage;
	}

private:
	enum Match {
		No_Match,
		Yes_Match,
		Cvt_Match,
		sizeof_Match
	};

	TokenStatus addToken(Token *&token);
	TokenStatus processOperand(Token *&token);
	TokenStatus endExpressionError(void);
	bool processUnaryOperator(Token *&token, TokenStatus &status);
    TokenStatus processBinaryOperator(Token *&token);
	TokenStatus processOperator(Token *&token);
	TokenStatus operatorError(void);
	TokenStatus processFirstOperand(Token *&token);
	TokenStatus processFinalOperand(Token *&token, Token *token2,
		int operandIndex, int nOperands = 0);
	TokenStatus findCode(Token *&token, int operandIndex,
		Token **first = NULL, Token **last = NULL);
	void doPendingParen(Token *token);
	TokenStatus expressionEnd(void);
	TokenStatus parenStatus(void);
	TokenStatus getExprDataType(DataType &dataType);
	void deleteOpenParen(Token *token);
	void deleteCloseParen(Token *token);
	TokenStatus callCommandHandler(Token *&token);
	void cleanUp(void);		// only call when addToken() returns error

	// COMMAND SPECIFIC FUNCTIONS
	TokenStatus addPrintCode(void);
	TokenStatus checkAssignListToken(Token *&token);
	TokenStatus setAssignCommand(Token *&token, Code assign_code);

	// By DataType Access Functions
	static TokenStatus expectedErrStatus(DataType dataType);
	static TokenStatus actualErrStatus(DataType dataType);
	static TokenStatus variableErrStatus(DataType dataType);
	static DataType equivalentDataType(DataType dataType);

	// set error token (deleting any previous error token first)
	void setErrorToken(Token *errorToken)
	{
		if (m_errorToken != NULL)
		{
			delete m_errorToken;
		}
		m_errorToken = errorToken;
    }

public:
	// token handler friend function definitions section
	friend TokenStatus Operator_Handler(Translator &t, Token *&token);
	friend TokenStatus Equal_Handler(Translator &t, Token *&token);
	friend TokenStatus Comma_Handler(Translator &t, Token *&token);
	friend TokenStatus CloseParen_Handler(Translator &t, Token *&token);
	friend TokenStatus EndOfLine_Handler(Translator &t, Token *&token);
	friend TokenStatus SemiColon_Handler(Translator &t, Token *&token);

	// command handler friend function definitions section
	friend TokenStatus Assign_CmdHandler(Translator &t, CmdItem *cmd_item,
		Token *token);
	friend TokenStatus Print_CmdHandler(Translator &t, CmdItem *cmd_item,
		Token *token);
	friend TokenStatus Let_CmdHandler(Translator &t, CmdItem *cmd_item,
		Token *token);
	friend TokenStatus Input_CmdHandler(Translator &t, CmdItem *cmd_item,
		Token *token);
};


#endif  // TRANSLATOR_H
