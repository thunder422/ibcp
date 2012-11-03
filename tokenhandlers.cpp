// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: tokenhandlers.cpp - contains code for translator token handlers
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
//	2012-11-02	initial version (parts removed from translator.cpp)

#include "translator.h"


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                               TOKEN HANDLERS                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// token handler functions are called to process tokens when received after
// the hold stack has been emptied of lower precedence operators
//
//   - these functions are friends of the Translator class so that
//     pointers to these functions can be put into table entries
//   - there is a reference to the Translator instance so the functions
//     have access to all the variables of the instance
//   - the other argument is a reference to the token being handled
//     (a reference so that the pointer can be change in case of an error)
//   - returns Good if successful, else an error


//**********************************
//**    OPERATOR TOKEN HANDLER    **
//**********************************

TokenStatus Operator_Handler(Translator &t, Token *&token)
{
	// only check mode if not in parentheses (expression)
	if (t.count_stack.empty())
	{
		switch (t.mode)
		{
		case Command_TokenMode:
		case Assignment_TokenMode:
			// oops, expected an equal or comma
			return ExpEqualOrComma_TokenStatus;

		case AssignmentList_TokenMode:
			// in a comma separated list
			return ExpEqualOrComma_TokenStatus;

		case Expression_TokenMode:
			// inside an expression, nothing extra to do
			break;

		default:
			return BUG_InvalidMode;
		}
	}

	return t.process_first_operand(token);
}


//********************************
//**    EQUAL TOKEN HANDLER    **
//********************************

TokenStatus Equal_Handler(Translator &t, Token *&token)
{
	TokenStatus status;
	Token *org_token;

	// only check mode if not in parentheses (expression)
	if (t.count_stack.empty())
	{
		switch (t.mode)
		{
		case Command_TokenMode:
		case Assignment_TokenMode:
			// start of assign statement
			status = t.set_assign_command(token, Assign_Code);
			if (status != Good_TokenStatus)
			{
				return status;
			}

			// switch straight to expression mode
			t.mode = Expression_TokenMode;

			// expecting operand next
			t.state = Translator::Operand_State;
			return Good_TokenStatus;

		case AssignmentList_TokenMode:
			// comma puts AssignList on hold stack
			delete token;  // assign list operator already on hold stack

			status = t.check_assignlist_token(token);
			if (status != Good_TokenStatus)
			{
				return status;
			}

			t.mode = Expression_TokenMode;  // end of list, expression follows

			// expecting operand next
			t.state = Translator::Operand_State;
			return Good_TokenStatus;

		case Expression_TokenMode:
			break;  // handle as equal operator below

		default:
			return BUG_InvalidMode;
		}
	}

	// inside an expression, keep Eq_Code
	return t.process_first_operand(token);
}


//********************************
//**    COMMA TOKEN HANDLER    **
//********************************

TokenStatus Comma_Handler(Translator &t, Token *&token)
{
	int noperands;
	TokenStatus status;

	// only check mode if not in parentheses (expression)
	if (t.count_stack.empty())
	{
		switch (t.mode)
		{
		case Command_TokenMode:
		case Assignment_TokenMode:
			// this is an assignment list
			// (comma puts AssignList on hold stack)
			// assignment for a comma separated list, change token
			if (t.done_stack.empty())  // make sure stack not empty
			{
				// if nothing before comma, comma unexpected
				// return appropriate error for mode
				return t.mode == Command_TokenMode
					? ExpCmd_TokenStatus : ExpAssignItem_TokenStatus;
			}

			status = t.set_assign_command(token, AssignList_Code);
			if (status != Good_TokenStatus)
			{
				return status;
			}

			// comma separated assignment list
			t.mode = AssignmentList_TokenMode;
			break;

		case AssignmentList_TokenMode:
			// continuation a comma separated list
			delete token;  // don't need comma token on stack

			status = t.check_assignlist_token(token);
			if (status != Good_TokenStatus)
			{
				return status;
			}

			break;

		case Expression_TokenMode:
			// inside an expression, but not in array or function

			// check if command allows comma
			status = t.call_command_handler(token);
			if (status == Null_TokenStatus)  // command didn't expect comma
			{
				// this can only occur in expression only test mode
				return ExpOpOrEnd_TokenStatus;
			}
			return status;

		case Reference_TokenMode:  // 2011-03-20: added
			return t.call_command_handler(token);
			// it is up to command handler to set state appropriately
			// XXX assume command used comma token (otherwise need to check)
			// XXX assume command set state according to its needs

		default:
			return BUG_InvalidMode;
		}
	}
	else
	{
		// inside an expression, check if in array or function
		if (t.count_stack.top().noperands == 0)
		{
			// inside parentheses
			return ExpOpOrParen_TokenStatus;
		}
		else if (t.count_stack.top().nexpected > 0)  // internal function?
		{
			Token *top_token = t.hold_stack.top().token;
			if (t.count_stack.top().noperands == t.count_stack.top().nexpected)
			{
				// number of arguments doesn't match current function's entry
				// see if function has multiple entries
				if ((t.table->flags(top_token->code) & Multiple_Flag) != 0)
				{
					// change token to next code (index)
					// (table entries have been validated during initialization)
					// (need to increment index before assignment)
					t.count_stack.top().code = ++top_token->code;
					// update number of expected operands
					t.count_stack.top().nexpected
						= t.table->noperands(top_token->code);
				}
				else
				{
					return ExpOpOrParen_TokenStatus;
				}
			}

			// check argument, change code and insert conversion
			status = t.find_code(top_token, t.count_stack.top().noperands - 1);
			if (status != Good_TokenStatus)
			{
				delete token;       // delete comma token
				token = top_token;  // return token with error
				return status;
			}
		}
		// increment the number of operands
		t.count_stack.top().noperands++;
		// delete comma token, it's not needed
		delete token;
	}

	t.state = Translator::Operand_State;
	return Good_TokenStatus;
}


//***********************************
//**    SEMICOLON TOKEN HANDLER    **
//***********************************

TokenStatus SemiColon_Handler(Translator &t, Token *&token)
{
	TokenStatus status;
	int noperands;

	// make sure the expression before semicolon is complete
	status = t.expression_end();
	if (status != Good_TokenStatus)
	{
		return status;
	}

	status = t.call_command_handler(token);
	// it is up to command handler to set state appropriately
	if (status == Null_TokenStatus)  // command stack was empty
	{
		if (t.done_stack.empty())
		{
			// no tokens received yet
			return ExpCmd_TokenStatus;
		}
		// an operands was received (start of an assignment)
		return ExpEqualOrComma_TokenStatus;
	}
	return status;
}


//*******************************************
//**    CLOSE PARENTHESES TOKEN HANDLER    **
//*******************************************

TokenStatus CloseParen_Handler(Translator &t, Token *&token)
{
	Token *top_token;
	int noperands;			// for array/function support
	int done_push = true;	// for print-only functions

	// do closing parentheses processing
	if (t.hold_stack.empty())
	{
		// oops, stack is empty
		return BUG_DoneStackEmptyParen;
	}
	// don't pop top token yet in case error occurs
	top_token = t.hold_stack.top().token;

	if (t.count_stack.empty())
	{
		return NoOpenParen_TokenStatus;
	}
	noperands = t.count_stack.pop().noperands;
	if (noperands == 0)
	{
		// just a parentheses expression
		if (top_token->code != OpenParen_Code)
		{
			// oops, no open parentheses
			return BUG_UnexpectedCloseParen;  // this should not happen
		}

		// clear reference for item on top of done stack
		t.done_stack.top().rpnItem->token->reference = false;
		// replace first and last operands of item on done stack
		t.delete_open_paren(t.done_stack.top().first);
		t.done_stack.top().first = top_token;
		t.delete_close_paren(t.done_stack.top().last);
		t.done_stack.top().last = token;
		// mark close paren token as used for last operand and pending paren
		// (so that it doesn't get deleted until its not used anymore)
		token->subcode |= Last_SubCode + Used_SubCode;

		// set pending parentheses token pointer
		t.pending_paren = token;
	}
	else  // array or function
	{
		int operand_index;

		// make sure token is an array or a function
		if (!top_token->has_paren())
		{
			// unexpected token on stack
			return BUG_UnexpectedToken;
		}

		// set reference flag for array or function
		// (DefFuncP should not have reference set)
		if (top_token->type == Paren_TokenType)
		{
			top_token->reference = true;
			operand_index = 0;  // not applicable
		}
		else if (top_token->type == DefFuncP_TokenType)
		{
			operand_index = 0;  // not applicable
		}
		else  // INTERNAL FUNCTION
		{
			// check for number of arguments for internal functions
			if (noperands != t.table->noperands(top_token->code))
			{
				return ExpOpOrComma_TokenStatus;
			}

			operand_index = t.table->noperands(top_token->code) - 1;
			// tell process_final_operand() to use operand_index
			noperands = 0;
		}

		// change token operator code or insert conversion codes as needed
		TokenStatus status = t.process_final_operand(top_token, token,
			operand_index, noperands);
		if (status != Good_TokenStatus)
		{
			// if top_token was not changed, pop it now
			if (top_token == t.hold_stack.top().token)
			{
				t.hold_stack.resize(t.hold_stack.size() - 1);
			}
			delete token;		// delete close paren token
			token = top_token;  // set token with error
			return status;
		}
	}

	// now pop the top token
	t.hold_stack.resize(t.hold_stack.size() - 1);

	return Good_TokenStatus;
}


//*************************************
//**    END-OF-LINE TOKEN HANDLER    **
//*************************************

TokenStatus EndOfLine_Handler(Translator &t, Token *&token)
{
	// TODO this is end of statement processing

	// check for proper end of expression
	TokenStatus status = t.expression_end();
	if (status != Good_TokenStatus)
	{
		return status;
	}

	// check for expression only mode
	if (!t.exprmode)
	{
		// process command on top of command stack
		if (t.cmd_stack.empty())
		{
			switch (t.mode)
			{
			case Assignment_TokenMode:
				if (t.state != Translator::BinOp_State)
				{
					return ExpAssignItem_TokenStatus;
				}
				// fall thru
			case Command_TokenMode:
				return ExpEqualOrComma_TokenStatus;

			case Expression_TokenMode:
			case AssignmentList_TokenMode:
				return BUG_CmdStackEmpty;

			default:
				return BUG_InvalidMode;
			}
		}
		status = t.call_command_handler(token);
		if (status != Good_TokenStatus)
		{
			return status;
		}
		t.cmd_stack.resize(t.cmd_stack.size() - 1);
		// upon return from the command handler,
		// the hold stack should have the null token on top
		// and done stack should be empty

		// TODO do end of line processing here, but for now...
	}
	else  // handle expression only test mode
	{
		// check if find result is only thing on done stack
		if (t.done_stack.empty())
		{
			return BUG_DoneStackEmpty;
		}
		// pop result and delete any paren tokens in first/last operands
		t.delete_open_paren(t.done_stack.top().first);
		t.delete_close_paren(t.done_stack.pop().last);
	}

	// pop and delete null token from top of stack
	delete t.hold_stack.pop().token;

	if (!t.done_stack.empty())
	{
		return BUG_DoneStackNotEmpty;
	}

	// make sure command stack is empty (temporary TODO)
	if (!t.cmd_stack.empty())
	{
		return BUG_CmdStackNotEmpty;
	}
	// check if token has been used by command handler
	if (token->code == EOL_Code)
	{
		delete token;  // delete EOL token
	}
	return Done_TokenStatus;
}


// end: tokenhandlers.cpp
