// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: commandhandlers.cpp - contains code for translator command handlers
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
#include "table.h"


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                            COMMAND HANDLERS                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


// command handler functions are called to process command tokens on the
// command stack at the end of a statements
//
//   - these functions are friends of the Translator class so that
//     pointers to these functions can be put into table entries
//   - there is a reference to the Translator instance so the functions
//     have access to all the variables of the instance
//   - the other argument is a pointer to the command stack item being handled
//     (in case of an error, the original token will be deleted and the token
//     pointer changed to point to the token with the error)
//   - returns Good if successful, else an error
//   - command handlers must handle the following tokens:
//     Comma, SemiColon, and codes with EndStmt_Flag


//**************************************
//**    ASSIGNMENT COMMAND HANDLER    **
//**************************************

TokenStatus Assign_CmdHandler(Translator &t, CmdItem *cmd_item, Token *token)
{
	TokenStatus status;

	switch (token->code())
	{
	case EOL_Code:
		// check done stack is empty before calling process_final_operand()
		// to avoid bug error check for empty done stack in find_code()
		if (t.done_stack.empty())
		{
			// save expected type
			DataType datatype = cmd_item->token->dataType();

			cmd_item->token = token;  // point error to end-of-statement token
			return Translator::errStatusExpected(datatype);
		}

		// turnoff reference flag of assign token, no longer needed
		cmd_item->token->setReference(false);

		return t.process_final_operand(cmd_item->token, NULL, 1);

	default:  // token was not expected command
		switch (t.mode)
		{
		case AssignmentList_TokenMode:
			// point error to unexpected token
			cmd_item->token = token;
			return ExpEqualOrComma_TokenStatus;

		case Expression_TokenMode:
			if (t.state == Translator::BinOp_State)
			{
				// point error to unexpected token
				cmd_item->token = token;
				return ExpOpOrEnd_TokenStatus;
			}
			status = Translator::errStatusExpected(t.cmd_stack.top().token
				->dataType());
			// point error to unexpected token after setting error
			cmd_item->token = token;
			return status;

		default:
			return BUG_InvalidMode;
		}
	}
}


//*********************************
//**    PRINT COMMAND HANDLER    **
//*********************************

TokenStatus Print_CmdHandler(Translator &t, CmdItem *cmd_item, Token *token)
{
    TokenStatus status;

	switch (token->code())
	{
	case Comma_Code:
		// make sure the expression before comma is complete
		status = t.expression_end();
		if (status != Good_TokenStatus)
		{
			return status;
		}

		status = t.add_print_code();
		if (status > Good_TokenStatus)
		{
			return status;
		}

		// append comma (advance to next column) token to output
		t.output->append(new RpnItem(token));

		// set PRINT command item flag in case last item in statement
		// (resets PrintFunc_CmdFlag if set)
		t.cmd_stack.top().flag = PrintStay_CmdFlag;

		// switch back to operand state (expecting operand next)
		t.state = Translator::OperandOrEnd_State;
		return Good_TokenStatus;

	case SemiColon_Code:
		status = t.add_print_code();
		if (status == Good_TokenStatus)
		{
			// print code added, delete semicolon token
			delete token;
		}
		else if (status == Null_TokenStatus)
		{
			// check if last token added was a print function
			if (t.cmd_stack.top().flag & PrintFunc_CmdFlag)
			{
				// set semicolon subcode flag on print function
				t.output->last()->token->setSubCodeMask(SemiColon_SubCode);
				// sub-code set, delete semicolon token
				delete token;
			}
			else  // no expression, add dummy semicolon token
			{
				t.output->append(new RpnItem(token));
			}
		}
		else  // an error occurred
		{
			return status;
		}

		// set PRINT command item flag in case last item in statement
		// (resets PrintFunc_CmdFlag if set)
		t.cmd_stack.top().flag = PrintStay_CmdFlag;

		// switch back to operand state (expecting operand next)
		t.state = Translator::OperandOrEnd_State;

		return Good_TokenStatus;

	case EOL_Code:
		status = t.add_print_code();
		if (status == Good_TokenStatus  // data type specific code added?
			|| status == Null_TokenStatus  // or not stay on line?
			&& !(cmd_item->flag & PrintStay_CmdFlag))
		{
			// append the print token to go to newline at runtime
			t.output->append(new RpnItem(cmd_item->token));
			// set good status; could be set to null
			status = Good_TokenStatus;
		}
		// check if print token was not used
		else if (status == Null_TokenStatus)
		{
			delete cmd_item->token;  // print token not used
			status = Good_TokenStatus;
		}
		return status;

	default:  // token was not expected command
		// TODO may need a proper error here
		return BUG_UnexpToken;
	}
}


//*******************************
//**    LET COMMAND HANDLER    **
//*******************************

TokenStatus Let_CmdHandler(Translator &t, CmdItem *cmd_item, Token *token)
{
	// this function should not be called for correct statements
	// if it is then it means that the LET command was not completed
	cmd_item->token = token;  // point to end-of-statement token
	// make sure done stack is not empty
	if (t.done_stack.empty())
	{
		return ExpAssignItem_TokenStatus;
	}
	return ExpEqualOrComma_TokenStatus;
}


//*********************************
//**    INPUT COMMAND HANDLER    **
//*********************************

TokenStatus Input_CmdHandler(Translator &t, CmdItem *cmd_item, Token *token)
{
    TokenStatus status;

	Code code = token->code();
	// TODO check for invalid token, report correct error

	// PROCESS BEGIN/PROMPT
	// used input begin command flag instead of element pointer
	if (!(t.cmd_stack.top().flag & InputBegin_CmdFlag))  // no begin code yet?
	{
		t.cmd_stack.top().flag |= InputBegin_CmdFlag;  // begin processed
		if (cmd_item->token->isCode(InputPrompt_Code))
		{
			if (t.done_stack.empty())
			{
				// report error against token
				cmd_item->token = token;  // set error token
				return ExpStrExpr_TokenStatus;
			}
			if (Translator::equivalentDataType(t.done_stack.top().rpnItem
				->token->dataType()) != String_DataType)
			{
				// don't delete token, caller will delete it
				// report error against first token of expression on done stack
				token = t.done_stack.top().first
					->setThrough(t.done_stack.top().last);
				// delete last token if close paren, remove from done stack
				t.delete_close_paren(t.done_stack.pop().rpnItem->token);
				cmd_item->token = token;  // set error token
				return ExpStrExpr_TokenStatus;
			}
			if (t.table.flags(code) & EndStmt_Flag)
			{
				cmd_item->token = token;  // set error token
				return ExpOpSemiOrComma_TokenStatus;
			}
			// change token to InputBeginStr and set sub-code
			t.table.setToken(token, InputBeginStr_Code);
			switch (code)
			{
			case Comma_Code:
				token->setSubCode(Question_SubCode);
				break;
			case SemiColon_Code:
				token->setSubCode(None_SubCode);
				break;
			default:  // unexpected token
				cmd_item->token = token;  // set error token
				return ExpSemiCommaOrEnd_TokenStatus;
			}
			status = t.process_final_operand(token, NULL, 0);
			if (status == Good_TokenStatus)
			{
				// set pointer to new end of output
				t.cmd_stack.top().index = t.output->size();
			}
			else
			{
				;//cmd_item->token = token;  // set error token (2011-03-24)
			}
			t.mode = Reference_TokenMode;  // change mode
			// set state for first variable
			t.state = Translator::Operand_State;
			return status;
		}

		// Input_Code
		if (!t.done_stack.empty())
		{
			// create new token for InputBegin
			// insert at begin of command before first variable
			t.output->insert(t.cmd_stack.top().index++,
				new RpnItem(t.table.newToken(InputBegin_Code)));
		}
		// if no variable on done stack, error will be reported below
		// now continue with input variable
	}

	// PROCESS INPUT VARIABLE
	if (t.state != Translator::EndStmt_State)
	{
		if (t.done_stack.empty())
		{
			cmd_item->token = token;  // set error token
			return ExpVar_TokenStatus;
		}

		// set token for data type specific input assign token
		t.table.setToken(token, InputAssign_Code);
		// set reference flag to be handled correctly in find_code
		token->setReference();
		// find appropriate input assign code and append to output
		status = t.process_final_operand(token, NULL, 0);
		if (status != Good_TokenStatus)
		{
			cmd_item->token = token;  // set error token
			return status;
		}
		token->setReference(false);  // now reset reference flag

		// create new token for input parse code
		// (which is first associated 2 code of input assign code)
		// insert input parse token into list at current index (update index)

		t.output->insert(t.cmd_stack.top().index++,
			new RpnItem(t.table.newToken(
			t.table.assoc2Code(token->code()))));

		// now process code
		if (code == Comma_Code)
		{
			t.state = Translator::Operand_State;  // for next variable
			return Good_TokenStatus;  // done now, but expecting more variables
		}
		if (code == SemiColon_Code)
		{
			t.cmd_stack.top().flag |= InputStay_CmdFlag;  // stay on line
			t.state = Translator::EndStmt_State;  // expecting end-of-statment
			return Good_TokenStatus;   // done now, wait for end-of-statment
		}
		if (!(t.table.flags(code) & EndStmt_Flag))
		{
			// unexpected token
			cmd_item->token = token;  // set error token
			return ExpSemiCommaOrEnd_TokenStatus;
		}
		// else fall thru to end processing
	}
	// else leave end stmt token for caller to deal with (delete if not needed)

	// PROCESS END OF INPUT
	// mark last parse code with end subcode
	(*t.output)[t.cmd_stack.top().index - 1]->token->setSubCode(End_SubCode);
	// append the input token to go to newline at runtime
	if (t.cmd_stack.top().flag & InputStay_CmdFlag)
	{
		cmd_item->token->setSubCode(Keep_SubCode);
	}
	t.output->append(new RpnItem(cmd_item->token));
	return Good_TokenStatus;
}


// end: commandhandlers.cpp
