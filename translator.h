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
#include "table.h"


// command stack item flag values
//   some flags are used for all commands and some are used only for
//   specific commands, the values of command specific flags may be reused
//   for different commands so each flag will be assigned a value

enum {
	// FLAGS FOR ALL COMMANDS
	None_CmdFlag			= 0x00000000,	// initial value of command flag

	// FLAGS FOR PRINT COMMAND
	PrintStay_CmdFlag		= 0x00010000,	// PRINT stay on line flag
	PrintFunc_CmdFlag		= 0x00020000,	// print func flag (2010-06-08)

	// FLAGS FOR ASSIGNMENT COMMANDS
	AssignList_CmdFlag		= 0x00010000,	// currently an assign list

	// FLAGS FOR INPUT COMMAND
	InputBegin_CmdFlag		= 0x00010000,	// beginning processed
	InputStay_CmdFlag		= 0x00020000	// INPUT stay on line flag
};


// command item
struct CmdItem {
	Token *token;				// pointer to command token
	int flag;					// 2010-06-01: generic flag for command use
	int index;  				// index into output list for command use
};


// structure for holding RPN output list information
struct RpnItem {
	Token *token;							// pointer to token
	int noperands;							// number of operands
	RpnItem **operand;						// array of operand pointers

	RpnItem(Token *_token, int _noperands = 0, RpnItem **_operand = NULL)
	{
		token = _token;
		noperands = _noperands;
		operand = _operand;
	}
	~RpnItem()
	{
		delete token;
		if (noperands > 0)
		{
			delete[] operand;
		}
	}

	// function to set operands without allocating a new array
	void set(int _noperands, RpnItem **_operand)
	{
		noperands = _noperands;
		operand = _operand;
	}
};


class Translator {
	struct HoldStackItem {
		Token *token;				// token pointer on hold stack
		Token *first;				// operator token's first operand pointer
	};
	struct DoneStackItem {
		RpnItem *rpnItem;			// pointer to RPN item
		Token *first;				// operator token's first operand pointer
		Token *last;				// operator token's last operand pointer
	};
	struct CountItem {
		char noperands;				// number of operands seen
		char nexpected;				// number of arguments expected
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
	} state;						// current state of translator

	Table *table;					// pointer to the table object
	QList<RpnItem *> *output;		// pointer to RPN list output
	QStack<HoldStackItem> hold_stack;  // operator/function holding stack
	QStack<DoneStackItem> done_stack;  // items processed stack
	Token *pending_paren;			// closing parentheses token is pending
	int last_precedence;			// precedence of last op added during paren
	QStack<CountItem> count_stack;	// number of operands counter stack
	TokenMode mode;					// current assignment mode
	QStack<CmdItem> cmd_stack;		// stack of commands waiting processing
	bool exprmode;					// expression only mode active flag

public:
	Translator(Table *t): table(t), output(NULL), pending_paren(NULL) {}
	void start(bool _exprmode = false)
	{
		exprmode = _exprmode;  // save flag
		output = new QList<RpnItem *>;
		state = Initial_State;
		// (expression mode for testing)
		mode = exprmode ? Expression_TokenMode : Command_TokenMode;
	}
	// function to access if operand state
	bool get_operand_state(void)
	{
		return state == Operand_State || state == OperandOrEnd_State;
	}
	TokenStatus add_token(Token *&token);
	QList<RpnItem *> *get_result(void)	// only call when add_token returns Done
	{
		QList<RpnItem *> *list = output;
		output = NULL;
		return list;
	}
	void clean_up(void);			// only call when add_token returns an error

private:
	enum Match {
		No_Match,
		Yes_Match,
		Cvt_Match,
		sizeof_Match
	};

	TokenStatus process_operand(Token *&token);
	TokenStatus end_expression_error(void);
	bool process_unary_operator(Token *&token, TokenStatus &status);
    TokenStatus process_binary_operator(Token *&token);
	TokenStatus process_operator(Token *&token);
	TokenStatus operator_error(void);
	// TODO move this function to translator.cpp
	void set_default_datatype(Token *token)
	{
		// only set to double if not an internal function
		if (token->datatype == None_DataType
			&& token->type != IntFuncP_TokenType)
		{
			// TODO for now just set default to double
			token->datatype = Double_DataType;
		}
		// change string DefFuncN/P to TmpStr
		else if ((token->type == DefFuncN_TokenType
			|| token->type == DefFuncP_TokenType)
			&& token->datatype == String_DataType)
		{
			token->datatype = TmpStr_DataType;
		}
	}
	TokenStatus process_first_operand(Token *&token);
	TokenStatus process_final_operand(Token *&token, Token *token2,
		int operand_index, int noperands = 0);
	TokenStatus find_code(Token *&token, int operand_index,
		Token **first = NULL, Token **last = NULL);
	void do_pending_paren(Token *token);
	TokenStatus expression_end(void);
	TokenStatus paren_status(void);
	TokenStatus get_expr_datatype(DataType &datatype);
	void delete_close_paren(Token *last);
	TokenStatus call_command_handler(Token *&token);

	// COMMAND SPECIFIC FUNCTIONS
	TokenStatus add_print_code(void);
	TokenStatus check_assignlist_token(Token *&token);
	TokenStatus set_assign_command(Token *&token, Code assign_code);

	// By DataType Access Functions
	static TokenStatus errStatusExpected(DataType dataType);
	static TokenStatus errStatusActual(DataType dataType);
	static TokenStatus errStatusVariable(DataType dataType);
	static DataType equivalentDataType(DataType dataType);

public:
	// function to delete an open paren token
	// (public to be used to delete token to prevent memory leak)
	void delete_open_paren(Token *first);

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


// token handler function definitions section
extern TokenStatus Operator_Handler(Translator &t, Token *&token);
extern TokenStatus Equal_Handler(Translator &t, Token *&token);
extern TokenStatus Comma_Handler(Translator &t, Token *&token);
extern TokenStatus CloseParen_Handler(Translator &t, Token *&token);
extern TokenStatus EndOfLine_Handler(Translator &t, Token *&token);
extern TokenStatus SemiColon_Handler(Translator &t, Token *&token);


#endif  // TRANSLATOR_H
