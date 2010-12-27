// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: translator.cpp - contains code for the translator class
//	Copyright (C) 2010  Thunder422
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
//  Change History:
//
//  2010-03-01  initial release
//
//  2010-03-20  continued development - functions now implemented for
//              handling simple expressions (basic operands and operators)
//
//  2010-03-21  needed to pop final result off of done stack, added checks
//              added check for EOL at begin of line
//              changed unary operator check, corrected in add_operator()
//
//  2010-03-25  added parentheses support
//              added switch for special operator token processing
//
//  2010-03-26  corrected issue with setting last_precedence by moving it from
//              being set with '(' to being set with ')' added
//              do_pending_paren() to replace code in three locations, and call
//              from two new locations
//
//  2010-04-02  added array and function support
//              added count stack to keep track of (count) commas within
//                arrays (subscripts) and functions (arguments), and to make
//                sure commas are not found within regular parentheses
//              modified close parentheses processing to handle the operands
//                (subscripts and arguments) within arrays and functions
//
//  2010-04-04  added internal function number of arguments checking
//              changed argument of add_token() to reference so that it can
//                modified to point to the token with the error
//
//  2010-04-11  added assignment operator handling including multiple
//              assignments (additional comma handling)
//  2010-04-12  added reference support
//  2010-04-13  changed add_operator() argument to reference so that different
//              token can be returned
//              added code to add_operator() to check for references for
//              assignment operator
//  2010-04-14  added code to add_operator() to check for references for
//              list assignment operator
//  2010-04-16  added mode checking for the open parentheses token processing
//              clear reference flag to top item in done stack for close paren
//              added count stack is empty check before checking mode for open
//              parentheses, equal operator and no special operator
//
//  2010-04-25  set default data type for operands
//              corrected memory leak of comma and close parentheses tokens
//              added data type handling for internal functions and operators
//              implemented new find_code() and match_code() functions for data
//                type handling
//  2010-04-26  changed bug error names
//
//  2010-05-08  implemented data type handling for assignment operators
//              modified find_code() to handle references on first operand
//              modified find_code() to get number of associated codes
//              modified match_code() to handle references on first operand
//              added last_operand argument to find_code()
//
//  2010-05-15  added temporary string support:
//              changed output list and done stack from Token* to RpnItem*
//              for arrays and functions, save pointers to operands
//              for internal functions with string operands, saved operands
//              for operators with string operands, saved operands
//              moved find_code() operand[] array to class for all to access
//              removed operandp[] argument from match_code(), now uses member
//              added entries to match_code() conversion code table for TmpStr
//
//  2010-05-16  corrected problem where saved operand was not pointing to a
//              conversion code that was inserted after the operand - the
//              operand was set to the return value of the append call when the
//              conversion code is inserted
//
//  2010-05-19  added sub-string support:
//              in find_code() rearranged first operand code and added check for
//                a sub-string function to get the string operands reference
//                flag
//              added SubStr entries to the convert code table in match_code()
//  2010-05-21  when an error occurs with a sub-string function where its string
//              operand cannot be assigned, the change the token with the error
//              to the string operand that can't be assigned instead of pointing
//              to the sub-string function
//
//  2010-05-22  corrected issue where string list assignments were not saving
//                all of the operands instead of just the last two
//              updated add_operator() to handle mix string list assignments
//
//  2010-05-28  changed hold_stack and done_stack from List to SimpleStack,
//                which required some small changes of push and pop calls
//              replaced switch and case code to token handler functions
//              moved special token processing in switch statement to individual
//                token handler functions, switch statement was removed
//  2010-05-29  moved no special operator code was also moved to a token handler
//                function
//              updated for moving/renaming Translator::Status and
//                Translator::Mode to TokenStatus and TokenMode
//              changed do_pending_paren() from appending dummy parentheses
//                token to output list to setting parentheses sub-code of the
//                last token appended to list
//              changed argument of do_pending_paren() from table entry index
//                to token pointer (which was only used to get the precedence);
//                this corrects a problem where the token on top of the hold
//                stack was an identifier with parentheses (array or function)
//                because these tokens don't have an index value, therefore the
//                precedence access function needs to be used, which takes these
//                token types into account
//              added setting of comma sub-code flag of assignment list token
//                when receiving an equal in comma assignment mode
//              added section of code at beginning of add_token for doing
//                initial processing for Command tokens when received,
//                if command has token_mode table entry value then command is
//                pushed on new command stack, otherwise unexpected command
//                occurs
//              added check to add_operator() that if operator is an assignment
//                (Reference_Flag is set) then check if there is a LET command
//                on the command stack, pop it, and set LET sub-code flag on
//                assignment operator token
//
//  2010-06-01  corrected a bug when an operand is processed, if the mode is
//                currently Command mode, the mode needs to be changed to
//                Assignment mode to prevent commands from being accepted
//              additional support for commands and command stack
//              added support for print-only functions
//  2010-06-02  added token handler for semicolon (for PRINT command)
//  2010-06-03  added PRINT support for comma
//              created expression_end() from parts of EndOfLine_Handler for
//                end of expression checks (also used by comma and semicolon)
//  2010-06-04  implemented new add_print_code() function to be used by
//              Comma_Handler(), SemiColon_Handler() and Print_CmdHandler()
//  2010-06-05  implemented Assign_CmdHandler() and Print_CmdHandler()
//              push assignment token to command stack instead of done stack so
//                the end of statement is handled properly and simply
//  2010-06-06  correctly check for expression only mode at end-of-line
//              end-of-line no longer pops a result off from done stack, which
//                should now be empty (commands deal with done stack) except for
//                special expression only mode
//              added support for end expression flag - codes that can end an
//                expression (currently comma, semicolon and EOL)
//              switch back to operand state after comma and semicolon
//  2010-06-08  set a print function flag in print command stack item to
//                indicate a print function flag was just added to the output,
//                which semicolon checks for and sets the semicolon sub-code
//                flag on the print function token in case the print function is
//                at the end of the print statement
//              moved end expression flag check to operand section before unary
//                operator check, if in the middle of a parentheses expression
//                or array/function, then return expected closing parentheses
//                error
//  2010-06-09  changed count stack from holding just a counter to holding the
//                counter and the expected number of arguments for internal
//                functions, so that the number of arguments for internal
//                functions can be checked as each comma token is received
//  2010-06-10  added new state FirstOperand set at the beginning of an
//                expression
//              renamed errors for clarity
//  2010-06-11  added check to detect invalid print-only functions sooner
//  2010-06-13  changed to push AssignList token to hold stack when first comma
//                token is received instead of when equal token is received
//                (necessary for proper error condition tests)
//              Let_CmdHandler implemented, necessary to catch errors when a
//                LET command statement was not completed correctly
//  2010-06-14  added check for immediate command token (append to output list
//                and return Done)
//              added paren_status() from duplicated code in add_token() and
//                expression_end()
//  2010-06-10/14  various changes to return appropriate easy to understand
//                 error messages
//
//  2010-06-24  corrected errors for internal functions not in expressions
//                except for sub-string functions that can be used for
//                assignments
//              at end of expression, added check if in assignment due to an
//                an array or sub-string function, to report correct errors
//  2010-06-26  added end of statement checking in the binary operator section
//              made more error reporting corrections related to checking if the
//                done stack is empty to determine the correct error to report
//  2010-06-29  added support for expression type
//              added data type to expression type conversion array
//  2010-06-30  continues to add support for expression type
//              added expected variable messages for assign list errors
//  2010-07-01  moved cvtcode_have_need[], equivalent_datatype[] to begin of
//                source file so all can access
//              implemented new assign and assign list code to Comma_Handler(),
//                Semicolon_Handler() and Assign_CmdHandler() with support
//                functions set_assign_command() and check_assignlist_token()
//              removed last_operand argument from find_code(), which was for
//                assign list support
//  2010-07-02  added more expr_type checking code
//  2010-07-03  added errstatus_datatype[]
//  2010-07-04  temporarily removed all expr_type code (may be removed)
//  2010-07-05  many changes for error reporting
//  2010-07-06  many changes for error reporting
//
//  2010-07-29  set reference of sub-string tokens within assignments so that
//                first operand (string being assigned) can be checked
//  2010-08-01  removed EqualAssignment, replaced CommaAssignment with
//                AssignmentList (multiple equal assignment statements no longer
//                being supported)
//  2010-08-07  begin modification of find_code() to only process one operand
//                at a time
//              removed match_code() since it's not necessary to match all
//                operands of operators or internal functions
//  2010-08-14  changed the way multiple argument codes are handled (due to
//                processing one operand at a time)
//  2010-08-30  begin implementation of process_final_operand() to handle
//                processing of last operand of operand or internal function
//  2010-09-11  begin implementation of leaving only string operands on stack
//                to be attached to an operand or internal function token
//  2010-10-01  corrected code to return expected type errors
//  2010-10-10  check first operand of sub-string assignment is not a string
//                variable
//  2010-12-24  corrected process_final_operand() to pop the correct number of
//                string operands to attach
//  2010-12-25  modified to leave string operand on stack for sub-string
//                functions and not push the sub-string token to the done stack
//

#include "ibcp.h"

// array of conversion codes [have data type] [need data type]
// 2010-05-15: add entries for TmpStr_DataType
// 2010-05-19: add entries for SubStr_DataType
// 2010-07-01: moved to begin of file
static Code cvtcode_have_need[numberof_DataType][numberof_DataType] = {
	{	// have Double,    need:
		Null_Code,		// Double
		CvtInt_Code,	// Integer
		Invalid_Code,	// String
		Invalid_Code,	// TmpStr
		Invalid_Code	// SubStr
	},
	{	// have Integer,   need:
		CvtDbl_Code,	// Double
		Null_Code,		// Integer
		Invalid_Code,	// String
		Invalid_Code,	// TmpStr
		Invalid_Code	// SubStr
	},
	{	// have String,    need:
		Invalid_Code,	// Double
		Invalid_Code,	// Integer
		Null_Code,		// String
		Null_Code,		// TmpStr
		Null_Code		// SubStr
	},
	{	// have TmpStr,    need:
		Invalid_Code,	// Double
		Invalid_Code,	// Integer
		Null_Code,		// String
		Null_Code,		// TmpStr
		Null_Code		// SubStr
	},
	{	// have SubStr,    need:
		Invalid_Code,	// Double
		Invalid_Code,	// Integer
		Null_Code,		// String
		Null_Code,		// TmpStr
		Null_Code		// SubStr
	}
};

// 2010-05-22: equivalent data type array (moved 2010-07-01)
static DataType equivalent_datatype[numberof_DataType] = {
	Double_DataType,	// Double
	Integer_DataType,	// Integer
	String_DataType,	// String
	String_DataType,	// TmpStr
	String_DataType		// SubStr
};

// 2010-06-29: equivalent data type array
static ExprType exprtype_datatype[sizeof_DataType] = {
	Num_ExprType,	// Double
	Num_ExprType,	// Integer
	Str_ExprType,	// String
	Str_ExprType,	// TmpStr
	Str_ExprType	// SubStr
//	Any_ExprType	// None
};

// 2010-07-03: error status data type array
static struct {
	TokenStatus expected;
	TokenStatus actual;
	TokenStatus variable;
} errstatus_datatype[sizeof_DataType] = {
	{	// Double
		ExpNumExpr_TokenStatus,
		ExpStrExpr_TokenStatus,
		ExpDblVar_TokenStatus
	},
	{	// Integer
		ExpNumExpr_TokenStatus,
		ExpStrExpr_TokenStatus,
		ExpIntVar_TokenStatus
	},
	{	// String
		ExpStrExpr_TokenStatus,
		ExpNumExpr_TokenStatus,
		ExpStrItem_TokenStatus
	},
	{	// TmpStr
		ExpStrExpr_TokenStatus,
		ExpNumExpr_TokenStatus,
		BUG_InvalidDataType
	},
	{	// SubStr
		ExpStrExpr_TokenStatus,
		ExpNumExpr_TokenStatus,
		BUG_InvalidDataType
	},
	{	// numberof
		BUG_InvalidDataType,
		BUG_InvalidDataType,
		BUG_InvalidDataType
	},
	{	// None
		BUG_InvalidDataType,
		BUG_InvalidDataType,
		ExpAssignItem_TokenStatus
	},
	{	// CmdArgs
		BUG_InvalidDataType,
		BUG_InvalidDataType,
		BUG_InvalidDataType
	}
};


// function to add a token to the output list, but token may be placed
// on hold stack pending adding it to the output list so that higher
// precedence tokens may be added to the list first
//
//     - Done status returned when last token is processed
//     - Good status returned when token successfully processed
//     - error status returned when an error is detected
//     - token argument may be changed when an error is detected

// 2010-04-04: made argument a reference so different token can be returned
TokenStatus Translator::add_token(Token *&token)
{
	if (state == Initial)
	{
		// 2010-03-21: check for end of line at begin of input
		if (token->type == Operator_TokenType
			&& table->code(token->index) == EOL_Code)
		{
			delete token;  // 2010-04-04: delete EOL token
			return Done_TokenStatus;
		}

		// push null token to be last operator on stack
		// to prevent from popping past bottom of stack
		// 2010-06-02: replaced new and set_token() with new_token()
		hold_stack.push(table->new_token(Null_Code));

		// 2010-06-10: initialize to FirstOperand instead of Operand
		state = FirstOperand;
	}

	// 2010-06-14: check for immediate command
	if (token->type == ImmCmd_TokenType)
	{
		// append to output, and done
		RpnItem *rpn_item = new RpnItem(token);
		output->append(&rpn_item);
		return Done_TokenStatus;
	}

	// 2010-05-29: added check for command token
	if (token->type == Command_TokenType)
	{
		if (mode == Command_TokenMode)
		{
			if (table->token_mode(token->index) != Null_TokenMode)
			{
				mode = table->token_mode(token->index);
//+				// 2010-06-29: if token mode is expression, set expression type
//+				if (mode == Expression_TokenMode)
//+				{
//+					expr_type = table->expr_type(token->index);
//+				}
				cmd_stack.push();
				cmd_stack.top().token = token;
				cmd_stack.top().code = table->code(token->index);
				cmd_stack.top().flag = None_CmdFlag;  // 2010-06-01: reset flag
				return Good_TokenStatus;  // nothing more to do
			}
			else
			{
				return BUG_NotYetImplemented;
			}
		}
		else  // command not permitted if not in command mode
		{
			return UnExpCommand_TokenStatus;
		}
	}

	// check for both operand states (2010-06-10)
	if (state == Operand || state == FirstOperand)
	{
		if (!token->is_operator())
		{
			// 2010-03-25: added parentheses support
			// 2010-03-26: check for and add dummy token if necessary
			// 2010-05-29: changed argument to token pointer
			do_pending_paren(hold_stack.top());

			// 2010-04-25: set default data type for token if it has none
			set_default_datatype(token);

			// 2010-07-02: moved expr_type checking
			// 2010-04-02: moved to after pending parentheses check
			if (token->has_paren())
			{
				// token is an array or a function
				// 2010-04-02: implemented array/function support
				// 2010-06-08: added number of operands for internal functions
				if (token->type == IntFuncP_TokenType)
				{
					// 2010-06-11: detect invalid print-only function early
					if ((table->flags(token->index) & Print_Flag)
						&& (cmd_stack.empty()
						|| cmd_stack.top().code != Print_Code))
					{
						return PrintOnlyIntFunc_TokenStatus;
					}
					// 2010-06-24: check if not in expression mode
					switch (mode)
					{
					case Command_TokenMode:
						if (count_stack.empty())
						{
							if (table->datatype(token->index)
								!= SubStr_DataType)
							{
								return ExpStatement_TokenStatus;
							}

							// 2010-07-29: set reference of sub-string function
							token->reference = true;
						}
						// check if first operand of sub-string (2010-10-10)
						else if (hold_stack.top()->reference
							&& count_stack.top().noperands == 1)
						{
							return ExpStrVar_TokenStatus;
						}
						break;

					case Assignment_TokenMode:
					case AssignmentList_TokenMode:
						if (table->datatype(token->index) != SubStr_DataType)
						{
							// in a comma separated list
							return errstatus_datatype[cmd_stack.top().token
								->datatype].variable;
						}
						// 2010-07-29: set reference flag of sub-string function
						token->reference = true;
						break;
					}
				}

				// 2010-06-08: changed count stack to hold count items
				count_stack.push();  // add an operand counter
				count_stack.top().noperands = 1;  // assume at least one
				if (token->type == IntFuncP_TokenType)
				{
					count_stack.top().nexpected
						= table->noperands(token->index);

					// 2010-06-29: save index of internal function's table entry
					count_stack.top().index = token->index;
//+
//+					// 2010-06-29: get expression type for first argument
//+					expr_type = exprtype_datatype[table->operand_datatype(token
//+						->index, 0)];
				}
				else  // token->type != IntFuncP_TokenType
				{
					count_stack.top().nexpected = 0;
//+					// 2010-06-29: array or function, set any expression type
//+					expr_type = Any_ExprType;
				}

				hold_stack.push(token);
				// leave state == Operand
				state = Operand;  // make sure not FirstOperand (2010-06-10)
			}
			else  // !token->has_paren()
			{
				// token is a variable or a function with no arguments
				// 2010-04-12: set reference flag for variable or function
				if (token->type == NoParen_TokenType
					|| token->type == DefFuncN_TokenType)
				{
					token->reference = true;
				}

				// add token directly output list
				// and push element pointer on done stack
				// 2010-05-15: create rpn item to add to output list
				RpnItem *rpn_item = new RpnItem(token);
				done_stack.push(output->append(&rpn_item));
				state = BinOp;  // next token must be a binary operator
			}
#if 0  // Comma and Equal will take care of this FIXME
			// 2010-06-01: if command mode then need to change to assignment
			if (mode == Command_TokenMode)
			{
				mode = Assignment_TokenMode;
			}
#endif
			return Good_TokenStatus;
		}
		// 2010-06-06: end-of-statement code acceptable instead of operand
		else if (table->flags(token) & EndExpr_Flag)
		{
			if (state != FirstOperand)
			{
				switch (mode)
				{
				case Command_TokenMode:
				case Assignment_TokenMode:
				case AssignmentList_TokenMode:
					// 2010-06-26: make sure done stack is not empty
					if (count_stack.empty())
					{
						return errstatus_datatype[cmd_stack.top().token
							->datatype].variable;
					}
					else if (count_stack.top().nexpected == 0)
					{
						// in array
						return ExpNumExpr_TokenStatus;
					}
					else if (count_stack.top().noperands == 1)
					{
						// in function at first argument (sub-string function)
						return ExpStrVar_TokenStatus;
					}
					else  // in function not at first argument
					{
						return errstatus_datatype[table
							->operand_datatype(count_stack.top().index,
							count_stack.top().noperands - 1)].expected;
					}

				case Expression_TokenMode:
					if (count_stack.empty())
					{
						if (hold_stack.empty())
						{
							return BUG_HoldStackEmpty;
						}
						// TODO operator on hold stack needs correct data type
						return BUG_Debug1;
						return errstatus_datatype[cmd_stack.top().token
							->datatype].variable;
					}
					else if (count_stack.top().nexpected == 0)
					{
						// in array or non-internal function
						// (cannot determine type of expression)
						return ExpExpr_TokenStatus;
					}
					else  // in internal function
					{
						// 2010-10-01: corrected to return expected type error
						return errstatus_datatype
							[table->operand_datatype(count_stack.top().index,
							count_stack.top().noperands - 1)].expected;
					}

				default:
					return BUG_InvalidMode;
				}
			}
			else  // at first operand (no operands received yet)
			{
				switch (mode)
				{
				case Expression_TokenMode:
					// FIXME this won't happen
					if (table->code(hold_stack.top()->index) == AssignList_Code)
					{
						return BUG_Debug;
					}
				}
			}
			// otherwise fall thru to where token handler will be called
		}
		else  // operator when expecting operand, must be a unary operator
		{
			// 2010-06-13: if command mode, then this is not the way it starts
			// 2010-07-01: check if count stack is empty before checking mode
			if (count_stack.empty() && mode == Command_TokenMode)
			{
				return ExpStatement_TokenStatus;
			}
			Code unary_code = table->unary_code(token->index);
			if (unary_code == Null_Code)
			{
				// oops, not a valid unary operator
				return BUG_Debug8;
				return ExpExpr_TokenStatus;
			}
			// change token to unary operator
			token->index = table->index(unary_code);
			// 2010-03-25: added check for opening parentheses
			if (unary_code == OpenParen_Code)
			{
				// 2010-03-26: don't initialize last_precedence here
				// 2010-03-26: check for and add dummy token if necessary
				// 2010-05-29: changed argument to token pointer
				do_pending_paren(hold_stack.top());

				// 2010-04-16: implemented assignment handling
				if (count_stack.empty())
				{
					// only check mode if not within parentheses
					// (otherwise in an expression)
					switch (mode)
					{
					case Assignment_TokenMode:
						// oops, not expecting parentheses
						return UnexpParenInCmd_TokenStatus;

					case AssignmentList_TokenMode:
						// in a comma separated list
						return errstatus_datatype[cmd_stack.top().token
							->datatype].variable;

					case Expression_TokenMode:
						// inside an expression, nothing extra to do
						break;

					default:
						return BUG_InvalidMode;
					}
				}
				// push open parentheses right on stack and return
				hold_stack.push(token);
				state = Operand;

				// 2010-04-02: add a null counter to prevent commas
				// 2010-06-09: changed count stack to hold count items
				count_stack.push();
				count_stack.top().noperands = 0;
				count_stack.top().nexpected = 0;
				return Good_TokenStatus;
			}
			// fall thru to operator code
		}
	}
	else  // a binary operator is expected
	{
		if (!token->is_operator())
		{
			// state == BinOp, but token is not an operator
			TokenStatus status;
			if ((status = paren_status()) != Good_TokenStatus)
			{
				return status;
			}
			else  // error is dependent on command
			{
				switch (mode)
				{
				case Command_TokenMode:
				case Assignment_TokenMode:
				case AssignmentList_TokenMode:
					return ExpEqualOrComma_TokenStatus;

				case Expression_TokenMode:
					return ExpOpOrEnd_TokenStatus;

				default:
					return BUG_InvalidMode;
				}
			}
		}
		// 2010-03-21: changed unary operator check
		if (table->is_unary_operator(token->index))
		{
			return ExpBinOpOrEnd_TokenStatus;
		}
		// 2010-10-10: check if after first operand of sub-string assignment
		if (hold_stack.top()->reference && count_stack.top().noperands == 1
			&& table->code(token->index) != Comma_Code)
		{
			// only a comma is allowed here
			return ExpComma_TokenStatus;
		}
		// 2010-03-26: initialize last precedence before emptying stack for ')'
		if (table->code(token->index) == CloseParen_Code)
		{
			// 2010-03-26: check for and add dummy token if necessary
			//             before emptying stack
			// 2010-05-29: changed argument to token pointer
			do_pending_paren(hold_stack.top());

			// now set last precedence to highest in case no operators in ( )
			last_precedence = Highest_Precedence;
		}
		// 2010-06-26: added checking at end of statment
		if (table->flags(token) & EndStatement_Flag)
		{
			if (mode == AssignmentList_TokenMode)
			{
				// no equal token received yet
				return ExpEqualOrComma_TokenStatus;
			}
		}
	}

	// process all operators
	// 2010-04-02: changed stack top precedence to work with paren tokens
	Token *top_token;
	while (table->precedence(hold_stack.top())
		>= table->precedence(token->index))
	{
		// TODO need top_token to pass to add_operator so that token can be
		// TODO changed in the event of an error being returned
		// pop operator on top of stack and add it to the output
		// 2010-04-13: set top_token so reference can be passed
		top_token = hold_stack.pop();
		TokenStatus status = add_operator(top_token);
		if (status != Good_TokenStatus)
		{
			token = top_token;  // 2010-04-13: return token with error
			return status;
		}
	}

	// check for special token processing
	// 2010-03-25: change code to switch and added closing parentheses support
	// 2010-05-28: change code from switch to token handler functions
	TokenHandler token_handler = table->token_handler(token->index);
	if (token_handler == NULL)  // no special handler?
	{
		// use default operator token handler
		token_handler = Operator_Handler;
	}
	return (*token_handler)(*this, token);
}


// function to add operator token to output list and to process data types
// for operator with the done stack
//
//    - for now just pops operands of operator from done stack
//    - for now just pushes operator onto done stack
//    - before added operator to done stack, checks for pending paren token
//    - sets last_precedence for operator being added to output list

// 2010-04-13: made argument a reference so different value can be returned
TokenStatus Translator::add_operator(Token *&token)
{
	// 2010-04-25: implemented data type handling for non-assignment operators
	//             removed popping of operands, find_code() does this
	//             removed unary operator check
	// 2010-05-08: removed temporary assignment code
	//             find_code() now handles assignment operators
	//             but only last assignment list operand
	// 2010-07-01: removed assignment and assignment list code, now handled in
	//             Equal_Handler(), Comma_Handler() and Assign_CmdHandler()
	// 2010-08-07: check second operator or binary operator or only operand of
	//             unary operator

	// 2010-03-25: added parentheses support BEGIN
	// 2010-03-26: replaced code with function call
	// 2010-05-29: changed argument to token pointer
	// 2010-06-05: do before assign check, which will append to output
	// 2010-08-22: moved to before final operand processing
	do_pending_paren(token);

	// changed token operator code or insert conversion codes as needed
	TokenStatus status = process_final_operand(token,
		table->is_unary_operator(token->index) ? 0 : 1);
	if (status != Good_TokenStatus)
	{
		return status;
	}

	// save precedence of operator being added
	// (doesn't matter if not currently within parentheses,
	// it will be reset upon next open parentheses)
	last_precedence = table->precedence(token->index);
	// 2010-03-25: added parentheses support END

	return Good_TokenStatus;
}


// function to check if there is a pending parentheses token and if there is,
// check to see if it should be added to the output as a dummy token so that
// the Recreator can added the unnecessary, but entered by the user, set of
// parentheses
//
//   - index argument is table index of operator to check against

// 2010-05-29: changed argument to token pointer
void Translator::do_pending_paren(Token *token)
{
	if (pending_paren != NULL)  // is a closing parentheses token pending?
	{
		// may need to add a dummy token
		// if the precedence of the last operator added within the
		// last parentheses sub-expression is higher than or
		// same as (operand state only) the operator
		// 2010-05-29: changed index argument to token
		int precedence = table->precedence(token);
		if (last_precedence > precedence
			|| state == Operand && last_precedence == precedence)
		{
			// 2010-05-29: replaced dummy token with parentheses sub-code flag
			if (output->last()->value->token->subcode & Paren_SubCode)
			{
				// already has parentheses sub-code set, so
				// add dummy token
				// 2010-05-15: create rpn item to add to output list
				RpnItem *rpn_item = new RpnItem(pending_paren);
				output->append(&rpn_item);
			}
			else
			{
				List<RpnItem *>::Element *last = output->last();
				if (table->flags(last->value->token) & Hidden_Flag)
				{
					// last token added is a hidden code
					last = last->prev;  // get token before this
				}
				last->value->token->subcode |= Paren_SubCode;
			}
			// TODO something needed on done stack?
			// TODO (reference flag already cleared by close parentheses)
		}
		else  // don't need pending token
		{
			delete pending_paren;  // release it's memory
		}
		pending_paren = NULL;  // reset pending token
	}
}


// function to process the final operand for an operator, function, array or
// command
//
//   - called with token final operand is for
//   - operand_index only used when number of operands is zero
//   - for arrays and non-internal functions, number of operands is non-zero
//     (all operands are attached to RPN output item)
//   - for operands/internal functions/commands: find_code() is called find
//     correct code and insert any necessary hidden conversion codes;
//     string operands are counted (only string operands are attached to RPN
//     output item); and print-only functions are checked for
//   - if there are operands to save: an operand array is allocated and
//     operands are popped from the done stack and put into the array
//   - for sub-string functions, string operand is left on stack for next token
//   - an RPN output item is created and any operands are attached
//	 - the RPN output item is appended to the output list
//	 - item is pushed to the done stack if not a print-only function

// 2010-08-30: new function created
TokenStatus Translator::process_final_operand(Token *&token, int operand_index,
	int noperands)
{
	bool done_push = true;	// 2010-06-01: for print-only functions

	if (noperands == 0)  // internal function or operator
	{
		TokenStatus status = find_code(token, operand_index);
		if (status != Good_TokenStatus)
		{
			return status;
		}

		// save string operands only (2010-08-07)
		// get number of strings for non-sub-string codes (2010-12-24)
		if (token->datatype != SubStr_DataType)
		{
			noperands = table->nstrings(token->index);
		}
		// don't push non-reference sub-string function (2010-12-25)
		else if (!token->reference)
		{
			done_push = false;  // don't push sub-string function on done stack
		}

		// 2010-06-01: process print-only internal functions
		if (table->flags(token->index) & Print_Flag)
		{
			if (cmd_stack.top().code != Print_Code)
			{
				return PrintOnlyIntFunc_TokenStatus;
			}
			// tell PRINT to stay on same line
			// also set print function flag (2010-06-08)
			cmd_stack.top().flag = PrintStay_CmdFlag | PrintFunc_CmdFlag;
			done_push = false;  // don't push on done stack
		}
	}

	List<RpnItem *>::Element **operand;
	if (noperands == 0)  // no string operands to save?
	{
		operand = NULL;
	}
	else  // pop string operands off of stack into new array
	{
		operand = new List<RpnItem *>::Element *[noperands];
		// 2010-05-15: save operands for storage in output list
		// TODO reverse loop, add second index variable
		for (int i = noperands; --i >= 0;)
		{
			if (done_stack.empty())
			{
				return BUG_Debug3;
				return BUG_DoneStackEmptyOperands;
			}
			operand[i] = done_stack.pop();
		}
	}

	// add token to output list and push element pointer on done stack
	// 2010-05-15: create rpn item to add to output list
	RpnItem *rpn_item = new RpnItem(token, noperands, operand);
	// 2010-06-01: check if output item is to be pushed on done stack
	List<RpnItem *>::Element *output_item = output->append(&rpn_item);
	if (done_push)
	{
		// add token to output list and push element pointer on done stack
		// 2010-05-22: moved setting of noperands to above with operand_array
		done_stack.push(output_item);
	}

	return Good_TokenStatus;
}


// function to find a code where the expected data types of the operands match
// the actual operands present; the code in the token is checked first followed
// by any associated codes that the main code has; the token is updated to the
// code found
//
//   - the token is unchanged is there is an exact match of data types
//   - conversion codes are inserted into the output after operands that need
//     to be converted
//   - if there is no match, then an appropriate error is returned and the
//     token argument is changed to pointer to the token with the error
//
//   - operand index must be valid to token code

// 2010-04-25: implemented new function
// 2010-05-08: added last_operand argument for assignment list processing
// 2010-07-01: removed last_operand argument, no longer needed
// 2010-08-30: only process one operand at a time, added operand_index arg
TokenStatus Translator::find_code(Token *&token, int operand_index)
{
	if (done_stack.empty())
	{
		// oops, there should have been operands on done stack
		return BUG_DoneStackEmptyFindCode;
	}
	Token *top_token = done_stack.top()->value->token;

	// 2010-05-08: check if reference is required for first operand
	if (operand_index == 0 && token->reference)
	{
		if (!top_token->reference)
		{
			// need a reference, so return error

			// only delete token if it's not an internal function (2010-10-09)
			if (token->type != IntFuncP_TokenType)
			{
				// (internal function is on hold stack, will be deleted later)
				delete token;  // delete the token
			}
			// return non-reference operand
			token = done_stack.pop()->value->token;

			// XXX check command on top of command stack XXX
			return ExpAssignRef_TokenStatus;
		}
	}
	else
	{
		// reset reference flag of operand
		top_token->reference = false;
	}

	// see if main code's data type matches
	DataType datatype = top_token->datatype;
	DataType operand_datatype = table->operand_datatype(token->index,
		operand_index);
	// 2010-10-04: actually check for exact match, not cvt_code is Null_Code
	if (datatype == operand_datatype)  // exact match?
	{
		// 2010-10-03: pop all references (for assignments) from stack
		// TODO TmpStr
		if (operand_datatype != String_DataType || token->reference)
		{
			done_stack.pop();  // pop non-string from done stack
		}
		return Good_TokenStatus;
	}
	Code cvt_code = cvtcode_have_need[datatype][operand_datatype];
	int cvt_index = cvt_code == Invalid_Code ? -1 : token->index;

	// see if any associated code's data types match
	// 2010-05-08: get actual number of associated codes
	// 2010-08-07: get start/end indexes to support second assoc codes group
	int start = operand_index != 1 ? 0 : table->assoc2_index(token->index);
	int end = table->nassoc_codes(token->index);
	if (operand_index == 0 && table->assoc2_index(token->index) != 0)
	{
		// for first operand, end at begin of second group of associated codes
		end = table->assoc2_index(token->index);
	}
	for (int i = start; i < end; i++)
	{
		Code assoc_code = table->assoc_code(token->index, i);
		int assoc_index = table->index(assoc_code);
		DataType operand_datatype2 = table->operand_datatype(assoc_index,
			operand_index);
		// 2010-10-04: actually check for exact match, not cvt_code is Null_Code
		if (datatype == operand_datatype2)  // exact match?
		{
			// change token's code and data type to associated code
			table->set_token(token, assoc_index);

			// 2010-10-03: pop all references (for assignments) from stack
			// TODO TmpStr
			if (operand_datatype2 != String_DataType || token->reference)
			{
				done_stack.pop();  // pop non-string from done stack
			}
			return Good_TokenStatus;
		}
		Code cvt_code2 = cvtcode_have_need[datatype][operand_datatype2];
		if (cvt_code2 != Invalid_Code && cvt_index == -1)
		{
			cvt_index = assoc_index;
			cvt_code = cvt_code2;
		}
	}

	if (cvt_index != -1)  // found a convertible code?
	{
		if (cvt_index != token->index)  // not the main code?
		{
			// change token's code and data type to associated code
			table->set_token(token, cvt_index);
			token->datatype = table->datatype(token->index);
		}

		// is there an actual conversion code to insert? (2010-10-04)
		if (cvt_code != Null_Code)
		{
			// a conversion code implies a non-string on done stack (2010-09-11)
			done_stack.pop();  // pop non-string from done stack

			// INSERT CONVERSION CODE
			// create convert token with convert code
			// append token to end of output list (after operand)
			// 2010-05-15: create rpn item to add to output list
			// 2010-06-02: replaced code with call to new_token()
			RpnItem *rpn_item = new RpnItem(table->new_token(cvt_code));
			output->append(&rpn_item);
		}

		return Good_TokenStatus;
	}

	// no match found, report error
	// change token to token with invalid data type and return error
	// use main code's expected data type for operand
	// 2010-10-10: return expected variable error for references and sub-strings
	// 2010-12-25: breakup single statement into if-else structure
	TokenStatus status;
	if (!token->reference)
	{
		status = errstatus_datatype[operand_datatype].expected;
		// check if operand on stack is argument of sub-string (2010-12-25)
		Token *last_token = output->last()->value->token;
		if (last_token != top_token)
		{
			if (last_token->datatype == SubStr_DataType)
			{
				// point error to sub-string function
				top_token = last_token;
			}
			else  // possible problem (last token should match top token)
			{
				status = BUG_Debug8;
			}
		}
	}
	else if (token->datatype == SubStr_DataType)
	{
		status = ExpStrVar_TokenStatus;
	}
	else
	{
		status = errstatus_datatype[operand_datatype].variable;
	}
	token = top_token;
	return status;
}
//-
//-
//-// function to check if the data types in the array of operands matches the
//-// data types for the code specified
//-//
//-//   - returns No_Match if the data types do not match (can't be converted)
//-//   - returns Yes_Match if the data types are an exact match
//-//   - returns Cvt_Match if the data types match or can be converted
//-//   - for Cvt_Match, the cvt_code array is filled with conversion codes
//-//     for each operarand
//-//   - for No_Match, the cvt_code array is only filled up to the first operand
//-//     that can be converted (conversion code set to Invalid)
//-
//-// 2010-04-25: implemented new function
//-// 2010-05-15: removed operand argument, operand now a class member
//-Translator::Match Translator::match_code(Code *cvt_code, Code code)
//-{
//-	int index = table->index(code);
//-	Match match = Yes_Match;  // assume match to start
//-	for (int i = 0; i < table->noperands(index); i++)
//-	{
//-		// 2010-05-08: check if first operand is a reference
//-		if (i == 0 && operand[0]->value->token->reference)
//-		{
//-			// for reference, the data type must be an exact match
//-			if (operand[0]->value->token->datatype
//-				== table->operand_datatype(index, 0))
//-			{
//-				cvt_code[0] = Null_Code;
//-			}
//-			else  // not an exact match, so no match
//-			{
//-				cvt_code[0] = Invalid_Code;
//-				return No_Match;
//-			}
//-		}
//-		else  // non-reference operand
//-		{
//-			cvt_code[i] = cvtcode_have_need[operand[i]->value->token->datatype]
//-				[table->operand_datatype(index, i)];
//-		}
//-		if (cvt_code[i] == Invalid_Code)
//-		{
//-			return No_Match;  // no match here, exit
//-		}
//-		else if (cvt_code[i] != Null_Code)  // have a conversion code?
//-		{
//-			match = Cvt_Match;  // then this is a convertible match
//-		}
//-		// else Null_Code, leave match as is
//-	}
//-	return match;
//-}


// function to do end of expression hold stack check to make sure
// nothing is on the hold stack (the initial null entry should be on
// top)
//
//   - checks if there is an open parentheses, parentheses token or
//     internal function on hold stack (missing closing parentheses)
//   - checks the null token is on top of stack
//   - or some other unexpected token (BUG)
//   - performs pending parentheses check before returning
//   - returns good status or error status

// 2010-06-03: created new function from code in EOL token handler
TokenStatus Translator::expression_end(void)
{
	TokenStatus status;
	Token *token;

	// do end of statement processing
	if (hold_stack.empty())
	{
		// oops, stack is empty
		return BUG_HoldStackEmpty;  // 2010-04-26: changed bug named
	}

	if ((status = paren_status()) != Good_TokenStatus)
	{
		return status;
	}
	else
	{
		token = hold_stack.top();
		if (!token->table_entry())
		{
			return BUG_Debug;
		}
		if (table->code(token->index) != Null_Code)
		{
			// check if there is some unexpected token on hold stack
			// could be assignment on hold stack
			switch (mode)
			{
			case Command_TokenMode:
				// no command was received yet
				return ExpStatement_TokenStatus;

			case Assignment_TokenMode:
				// expression hasn't started yet
				return ExpEqualOrComma_TokenStatus;

			case AssignmentList_TokenMode:
				// in a comma separated list
				return ExpEqualOrComma_TokenStatus;

			case Expression_TokenMode:
				// something is on the stack that's not suppose to be there
				// this is a diagnostic error, should not occur
				return ExpOpOrEnd_TokenStatus;

			default:
				return BUG_InvalidMode;
			}
		}
	}

	// 2010-03-25: check if there is a pending closing parentheses
	// 2010-03-26: replaced code with function call
	// 2010-05-29: changed argument to token pointer
	// pass Null_Code token (will always add dummy)
	do_pending_paren(token);
	return Good_TokenStatus;
}


// function to determine the current parentheses status by looking if there
// is an outstanding opening parentheses, token with parentheses (array or
// define/user function) or internal function token
//
//   - an outstanding token is determined by looking at the count stack
//   - returns Good status if there is no outstanding parentheses, array or
//     function token
//   - returns the appropriate error for the outstanding token
//   - for open parentheses, an operator or parentheses is expected
//   - for parentheses token, an operator, comma or parentheses is expected
//   - for internal function, looks at number of arguments processed so far
//     to determine if an operator or parentheses is expected, or an operator,
//     comma or parentheses is expected

// 2010-06-14: implement new function from other code
TokenStatus Translator::paren_status(void)
{
	if (count_stack.empty())
	{
		// no parentheses
		return Good_TokenStatus;
	}
	// parentheses, array or function without closing parentheses

	if (count_stack.top().noperands == 0)
	{
		// open parentheses
		return ExpOpOrParen_TokenStatus;
	}

	if (count_stack.top().nexpected == 0)
	{
		// array, defined function or user function
		// (more subscripts/arguments possible)
		return ExpOpCommaOrParen_TokenStatus;
	}

	if (count_stack.top().noperands == count_stack.top().nexpected)
	{
		// internal function - at last argument (no more expected)
		return ExpOpOrParen_TokenStatus;
	}

	// internal function - more arguments expected
	// (number of arguments doesn't match function's entry)
	int index;
	Token *top_token = hold_stack.top();
	if ((table->flags(top_token->index) & Multiple_Flag) != 0
		&& (index = table->search(top_token->index,
		count_stack.top().noperands)) > 0)
	{
		// found alternate code matching current number of operands
		// (could be at last argument, could be more arguments)
		return ExpOpCommaOrParen_TokenStatus;
	}
	else if (mode == Command_TokenMode || mode == AssignmentList_TokenMode)
	{
		// sub-string function
		return ExpComma_TokenStatus;
	}
	else  // more arguments are expected
	{
		return ExpOpOrComma_TokenStatus;
	}
}


// function to clean up the Translator variables after an error is detected
//
//   - must be called after add_token() returns an error

void Translator::clean_up(void)
{
	// clean up from error
	while (!output->empty())
	{
		// 2010-04-04: added delete to free the token that was in the list
		delete output->pop();  // using pop doesn't require pointer variable
	}
	delete output;
	output = NULL;
	while (!hold_stack.empty())
	{
		// 2010-04-04: added delete to free the token that was on the stack
		delete hold_stack.pop();
	}
	while (!done_stack.empty())
	{
		done_stack.pop();
	}
	// 2010-04-02: comma support - need to empty count_stack
	while (!count_stack.empty())
	{
		count_stack.pop();
	}
	// 2010-03-25: parentheses support - need to delete pending parentheses
	if (pending_paren != NULL)
	{
		delete pending_paren;
		pending_paren = NULL;
	}
	// 2010-06-02: command support - need to empty command_stack
	while (!cmd_stack.empty())
	{
		cmd_stack.pop();
	}
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                           COMMAND SPECIFIC FUNCTIONS                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


// PRINT: function to process the result of an expression on top of the done
// stack, if not empty, add a data type specific print code to the output
//
//   - print string code gets operands for later determination of string type
//   - returns Good if print code added successfully
//   - returns Null if stack was empty

// 2010-06-04: implemented new function
TokenStatus Translator::add_print_code(void)
{
	if (!done_stack.empty())
	{
		// create token for data type specific print token
		Token *token = table->new_token(PrintDbl_Code);
		TokenStatus status;
		process_final_operand(token, 0, status);
		return status;
//-		TokenStatus status = find_code(token);
//-		if (status != Good_TokenStatus)
//-		{
//-			return status;
//-		}
//-
//-		// save operand if string operand
//-		int noperands = table->flags(token->index) & String_Flag ? 1 : 0;
//-
//-		// append token to output
//-		RpnItem *rpn_item = new RpnItem(token, noperands, operand);
//-		output->append(&rpn_item);
//-		return Good_TokenStatus;
	}
	return Null_TokenStatus;  // nothing done
}


// ASSIGNMENT: function to set the top of the command stack to either an
// Assign_Code (equal) or an AssignList_Code (comma)
//
//   - for comma (AssignList_Code), set comma sub-code of command token
//   - checks if top of command stack has Let token (deleted, let sub-code set)
//   - returns Good if successful
//   - returns error if one is detected

TokenStatus Translator::set_assign_command(Token *&token, Code assign_code)
{
	// start of assign list statement
	table->set_token(token, assign_code);
	// 2010-08-01: removed setting of Comma_SubCode to AssignList
	// 2010-07-29: set reference flag of assignment
	token->reference = true;

	// 2010-07-01: find appropriate assign or assign list code
	TokenStatus status = find_code(token, 0);
	if (status != Good_TokenStatus)
	{
		return status;
	}

	if (cmd_stack.empty())  // still command mode?
	{
		cmd_stack.push();  // push new command on stack
	}
	else  // was preceded by let keyword
	{
		token->subcode |= Let_SubCode;  // set sub-code to indicate let
		delete cmd_stack.top().token;   // delete the let token
	}
	cmd_stack.top().token = token;
	cmd_stack.top().code = assign_code;
	cmd_stack.top().flag = None_CmdFlag;
	return Good_TokenStatus;
}


// ASSIGN LIST: function to check the assignment list item token on top of the
// done stack to make sure if has its reference flag set and it is the correct
// data type for the assignment list (matches the previous list items)
//
//   - for string types, strings and sub-strings are allowed in the same list
//     assignment, and if the assignment contains both strings and sub-strings,
//     then the AssignListMixStr code is used
//   - returns Good if successful
//   - returns error if one is detected

TokenStatus Translator::check_assignlist_token(Token *&token)
{
	if (!done_stack.empty())
	{
		token = done_stack.pop()->value->token;
		if (!token->reference
			|| equivalent_datatype[cmd_stack.top().token->datatype]
			!= equivalent_datatype[token->datatype])
		{
			// 2010-06-30: replaced with expected variable errors
			return errstatus_datatype[cmd_stack.top().token->datatype].variable;
		}
		if (equivalent_datatype[token->datatype] == String_DataType
			&& token->datatype != cmd_stack.top().token->datatype)
		{
			cmd_stack.top().token->index
				= table->index(AssignListMixStr_Code);
		}
	}
	return Good_TokenStatus;
}


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

// 2010-05-29: created function from add_token() no special operator section
TokenStatus Operator_Handler(Translator &t, Token *&token)
{
	// 2010-04-16: only check mode if not in parentheses (expression)
	if (t.count_stack.empty())
	{
		// 2010-04-11: implemented assignment handling
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

	// check first operand of binary operators (2010-08-07)
	if (!t.table->is_unary_operator(token->index))
	{
		// changed token operator code or insert conversion codes as needed
		TokenStatus status = t.find_code(token, 0);
		if (status != Good_TokenStatus)
		{
			return status;
		}
	}

	// push it onto the holding stack
	t.hold_stack.push(token);
	t.state = Translator::Operand;
	return Good_TokenStatus;
}


//********************************
//**    EQUAL TOKEN HANDLER    **
//********************************

// 2010-05-28: created function from add_token() case Eq_Code
TokenStatus Equal_Handler(Translator &t, Token *&token)
{
	TokenStatus status;
	Token *org_token;

	// 2010-04-11: implemented assignment handling
	// 2010-04-16: only check mode if not in parentheses (expression)
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

			// 2010-08-01: switch straight to expression mode
			t.mode = Expression_TokenMode;

//+			// set expression type (2010-07-01)
//+			t.expr_type = exprtype_datatype[t.cmd_stack.top().token->datatype];
//+
			// expecting first operand next (2010-06-10)
			t.state = Translator::FirstOperand;
			return Good_TokenStatus;

		case AssignmentList_TokenMode:
			// 2010-06-13: comma puts AssignList on hold stack
			delete token;  // assign list operator already on hold stack

			status = t.check_assignlist_token(token);
			if (status != Good_TokenStatus)
			{
				return status;
			}

			t.mode = Expression_TokenMode;  // end of list, expression follows

//+			// set expression type (2010-07-01)
//+			t.expr_type = exprtype_datatype[t.cmd_stack.top().token->datatype];
//+
			// expecting first operand next (2010-06-10)
			t.state = Translator::FirstOperand;
			return Good_TokenStatus;

		// 2010-08-01: removed EqualAssignment case

		case Expression_TokenMode:
			break;  // handle as equal operator below

		default:
			return BUG_InvalidMode;
		}
	}

	// inside an expression, keep Eq_Code

	// changed token code or insert conversion as needed (2010-09-10)
	status = t.find_code(token, 0);  // first operand
	if (status != Good_TokenStatus)
	{
		return status;
	}

	// push on hold stack
	t.hold_stack.push(token);

	// expecting another operand next (2010-06-10)
	t.state = Translator::Operand;

	return Good_TokenStatus;
}


//********************************
//**    COMMA TOKEN HANDLER    **
//********************************

// 2010-05-28: created function from add_token() case Comma_Code
// 2010-06-11: reorganized function for better error reporting
TokenStatus Comma_Handler(Translator &t, Token *&token)
{
	int noperands;
	RpnItem *rpn_item;
	TokenStatus status;

	// 2010-04-02: implemented comma operator code handling
	// 2010-04-11: implemented multiple assignment handling
	// 2010-04-17: only check mode if not in parentheses (expression)
	if (t.count_stack.empty())
	{
		switch (t.mode)
		{
		case Command_TokenMode:
		case Assignment_TokenMode:
			// this is an assignment list
			// 2010-06-13: comma puts AssignList on hold stack
			// assignment for a comma separated list, change token
			// 2010-06-26: make sure done stack is not empty
			if (t.done_stack.empty())
			{
				// if nothing before comma, comma unexpected (2010-06-13)
				return t.cmd_stack.empty()
					? ExpStatement_TokenStatus : ExpAssignItem_TokenStatus;
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

			// 2010-06-03: check if command allows comma
			switch (t.cmd_stack.empty() ? Null_Code : t.cmd_stack.top().code)
			{
			case Print_Code:
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

				// append comma token to output
				rpn_item = new RpnItem(token);
				t.output->append(&rpn_item);

				// set PRINT command item flag in case last item in statement
				// (resets PrintFunc_CmdFlag if set)
				t.cmd_stack.top().flag = PrintStay_CmdFlag;

				// switch back to operand state (2010-06-06)
				// expecting first operand next (2010-06-10)
				t.state = Translator::FirstOperand;
//+				// switch back to any expression type (2010-06-30)
//+				t.expr_type = Any_ExprType;
				return Good_TokenStatus;

			default:
				return ExpOpOrEnd_TokenStatus;
			}

		default:
			return BUG_InvalidMode;
		}
	}
	else
	{
		// inside an expression, check if in array or function
		// 2010-06-30: added support for expression type
		if (t.count_stack.top().noperands == 0)
		{
			// inside parentheses
			return ExpOpOrParen_TokenStatus;
		}
		else if (t.count_stack.top().nexpected > 0)  // internal function?
		{
			Token *top_token = t.hold_stack.top();
			if (t.count_stack.top().noperands == t.count_stack.top().nexpected)
			{
				// number of arguments doesn't match current function's entry
				// see if function has multiple entries (2010-08-14)
				if ((t.table->flags(top_token->index) & Multiple_Flag) != 0)
				{
					// change token to next code (index)
					// (table entries have been validated during initialization)
					t.count_stack.top().index = top_token->index++;
					// update number of expected operands
					t.count_stack.top().nexpected
						= t.table->noperands(top_token->index);
				}
				else
				{
					return ExpOpOrParen_TokenStatus;
				}
			}

//-			switch (t.mode)
//-			{
//-			case Command_TokenMode:
//-			case AssignmentList_TokenMode:
//-				if (t.count_stack.top().noperands == 1)
//-				{
//-					// in function at first argument (sub-string function)
//-					if (t.done_stack.empty())
//-					{
//-						return BUG_DoneStackEmpty;
//-					}
//-					if (!t.done_stack.top()->value->token->reference)
//-					{
//-						return BUG_Debug9;
//-					}
//-					if (t.done_stack.top()->value->token->datatype
//-						!= String_DataType)
//-					{
//-						// point to token with wrong data type
//-						token = t.done_stack.top()->value->token;
//-						return ExpStrVar_TokenStatus;
//-					}
//-				}
//-			}
			// check argument, change code and insert conversion (2010-08-10)
			status = t.find_code(top_token, t.count_stack.top().noperands - 1);
			if (status != Good_TokenStatus)
			{
				token = top_token;  // return token with error (2010-10-01)
				return status;
			}
//+
//+			// 2010-06-29: get expression type for next argument
//+			t.expr_type = exprtype_datatype[t.table->operand_datatype(
//+				t.count_stack.top().index,
//+				t.count_stack.top().noperands)];
//+		}
//+		else  // array or non-internal function
//+		{
//+			// 2010-06-30: expression type could be any
//+			t.expr_type = Any_ExprType;
		}
		// increment the number of operands
		t.count_stack.top().noperands++;
		// delete comma token, it's not needed (2010-04-25)
		delete token;
	}

	t.state = Translator::Operand;
	return Good_TokenStatus;
}


//***********************************
//**    SEMICOLON TOKEN HANDLER    **
//***********************************

// 2010-06-02 implement new function for Semicolon_Code
TokenStatus SemiColon_Handler(Translator &t, Token *&token)
{
	TokenStatus status;
	int noperands;
	RpnItem *rpn_item;

	// make sure the expression before semicolon is complete
	status = t.expression_end();
	if (status != Good_TokenStatus)
	{
		return status;
	}

	switch (t.cmd_stack.empty()
		|| !t.count_stack.empty() && t.count_stack.top().noperands > 0
		? Null_Code : t.cmd_stack.top().code)
	{
	case Print_Code:
		status = t.add_print_code();
		if (status == Good_TokenStatus)
		{
			// print code added, delete semicolon token
			delete token;
		}
		else if (status == Null_TokenStatus)
		{
			// check if last token added was a print function (2010-06-08)
			if (t.cmd_stack.top().flag & PrintFunc_CmdFlag)
			{
				// set semicolon subcode flag on print function
				t.output->last()->value->token->subcode |= SemiColon_SubCode;
			}
			else  // no expression, add dummy semicolon token
			{
				rpn_item = new RpnItem(token);
				t.output->append(&rpn_item);
			}
		}
		else  // an error occurred
		{
			return status;
		}

		// set PRINT command item flag in case last item in statement
		// (resets PrintFunc_CmdFlag if set)
		t.cmd_stack.top().flag = PrintStay_CmdFlag;

		// switch back to operand state (2010-06-06)
		// expecting first operand next (2010-06-10)
		t.state = Translator::FirstOperand;
//+		// switch back to any expression type (2010-06-30)
//+		t.expr_type = Any_ExprType;

		return Good_TokenStatus;

	case Assign_Code:
	case AssignList_Code:
		switch (t.mode)
		{
		case AssignmentList_TokenMode:
			return ExpEqualOrComma_TokenStatus;

		case Expression_TokenMode:
			if (t.state == Translator::BinOp)
			{
				return ExpOpOrEnd_TokenStatus;
			}
			return errstatus_datatype[t.cmd_stack.top().token->datatype]
				.expected;

		default:
			return BUG_InvalidMode;
		}

	default:
		switch (t.mode)
		{
		case Command_TokenMode:
			if (t.done_stack.empty())
			{
				return ExpStatement_TokenStatus;
			}
			return ExpEqualOrComma_TokenStatus;

		case Assignment_TokenMode:
			if (t.done_stack.empty())
			{
				return ExpAssignItem_TokenStatus;
			}
			return ExpEqualOrComma_TokenStatus;

		default:
			return BUG_InvalidMode;
		}
	}
}


//*******************************************
//**    CLOSE PARENTHESES TOKEN HANDLER    **
//*******************************************

// 2010-05-28: created function from add_token() case CloseParen_Code
TokenStatus CloseParen_Handler(Translator &t, Token *&token)
{
	Token *top_token;
	int noperands;  // 2010-04-02: for array/function support
	int done_push = true;  // 2010-06-01: for print-only functions

	// do closing parentheses processing
	if (t.hold_stack.empty())
	{
		// oops, stack is empty
		return BUG_DoneStackEmptyParen;
	}
	top_token = t.hold_stack.pop();

	// 2010-04-02: implemented array/function support BEGIN
	if (t.count_stack.empty())
	{
		return NoOpenParen_TokenStatus;
	}
	// 2010-06-09: changed count stack to hold count items
	noperands = t.count_stack.top().noperands;
	t.count_stack.pop();
	if (noperands == 0)
	{
		// just a parentheses expression
		if (t.table->code(top_token->index) != OpenParen_Code)
		{
			// oops, no open parentheses
			return BUG_UnexpectedCloseParen;  // this should not happen
		}
		delete top_token;  // delete open parentheses token

		// 2010-04-16: clear reference for item on top of done stack
		t.done_stack.top()->value->token->reference = false;

		// 2010-03-30: set pending parentheses token pointer
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

		// 2010-04-12: set reference flag for array or function
		// FIXME not sure DefFuncP should have reference set, set it for now
		if (top_token->type != IntFuncP_TokenType)
		{
			top_token->reference = true;
			operand_index = 0;  // not applicable

//-			// data types for array subscripts and define/user functions
//-			// cannot be checked here
//-			// 2010-05-15: save operands for storage in output list
//-			for (int i = noperands; --i >= 0;)
//-			{
//-				if (t.done_stack.empty())
//-				{
//-					return BUG_DoneStackEmptyArrFunc;
//-				}
//-				t.operand[i] = t.done_stack.pop();
//-			}
//+
//+			// 2010-06-29: reset expression type to type of array or function
//+			//             (if type was not right for the array of function,
//+			//             it would have been rejected when first received)
//+			t.expr_type = exprtype_datatype[top_token->datatype];
		}
		else  // INTERNAL FUNCTION
		{
			// 2010-04-04: check for number of arguments for internal functions
			// changed to pointer to close parentheses for error (2010-04-10)
			if (noperands != t.table->noperands(top_token->index))
			{
				return ExpOpOrComma_TokenStatus;
			}

//-			if (noperands != t.table->noperands(top_token->index))
//-			{
//-				// number of arguments doesn't match function's entry
//-				int index;
//-
//-				if ((t.table->flags(top_token->index) & Multiple_Flag) != 0
//-					&& (index = t.table->search(top_token->index, noperands))
//-					> 0)
//-				{
//-					// change token to new code (index)
//-					top_token->index = index;
//-				}
//-				else
//-				{
//-					// 2010-06-10: renamed error
//-					return ExpOpOrComma_TokenStatus;
//-				}
//-			}
//-
			// moved deleting of token and setting to top_token (2010-04-10)
			operand_index = t.table->noperands(top_token->index) - 1;
			// tell process_final_operand() to use operand_index (2010-09-10)
			noperands = 0;

//-			// check argument, change code and insert conversion (2010-08-21)
//-			status = t.find_code(token, noperands - 1);
//-			if (status != Good_TokenStatus)
//-			{
//-				return status;
//-			}
//-
//-			// 2010-04-25: implemented data type handling
//-			// process data types of arguments (find proper code)
//-			TokenStatus status = t.find_code(token);
//-			if (status != Good_TokenStatus)
//-			{
//-				return status;
//-			}
//-
//-			// 2010-05-15: don't save operands if none are strings
//-			if (!(t.table->flags(token->index) & String_Flag))
//-			{
//-				noperands = 0;  // no string operands, don't save operands
//-			}
//-
//-			// 2010-06-01: process print-only internal functions
//-			if (t.table->flags(token->index) & Print_Flag)
//-			{
//-				if (t.cmd_stack.top().code != Print_Code)
//-				{
//-					return PrintOnlyIntFunc_TokenStatus;
//-				}
//-				// tell PRINT to stay on same line
//-				// also set print function flag (2010-06-08)
//-				t.cmd_stack.top().flag = PrintStay_CmdFlag | PrintFunc_CmdFlag;
//-				done_push = false;  // don't push on done stack
//-			}
		}

//+		// 2010-07-02: check it parentheses token has correct data type
//+		if (t.expr_type != Any_ExprType
//+			&& t.expr_type != exprtype_datatype[top_token->datatype])
//+		{
//+			switch (t.expr_type)
//+			{
//+			case Num_ExprType:
//+				return BUG_Debug;
//+				return ExpNumExpr_TokenStatus;
//+
//+			case Str_ExprType:
//+				return ExpStrExpr_TokenStatus;
//+
//+			default:
//+				return BUG_InvalidExprType;
//+			}
//+		}
//+
//-		// add token to output list and push element pointer on done stack
//-		// 2010-05-15: create rpn item to add to output list
//-		RpnItem *rpn_item = new RpnItem(top_token, noperands, t.operand);
//-		// 2010-06-01: check if output item is to be pushed on done stack
//-		List<RpnItem *>::Element *output_item = t.output->append(&rpn_item);
//-		if (done_push)
//-		{
//-			t.done_stack.push(output_item);
//-		}
		// delete close paren token, it's not needed (2010-04-25)
		// 2010-10-02: moved to after error checking
		delete token;

		// changed token operator code or insert conversion codes as needed
		token = top_token;
		TokenStatus status = t.process_final_operand(token, operand_index,
			noperands);
		if (status != Good_TokenStatus)
		{
			return status;
		}
	}
	// 2010-04-02: implemented array/function support END

	// 2010-03-26: moved pending check to before stack emptying while

	// 2010-04-02: moved set pending paren, not needed for array/function
	return Good_TokenStatus;
}


//*************************************
//**    END-OF-LINE TOKEN HANDLER    **
//*************************************

// 2010-05-28: created function from add_token() case EOL_Code
TokenStatus EndOfLine_Handler(Translator &t, Token *&token)
{
	// TODO this is end of statement processing

	// 2010-06-10: check for proper end of expression
	TokenStatus status = t.expression_end();
	if (status != Good_TokenStatus)
	{
		return status;
	}

	// 2010-06-06: check for expression only mode
	if (!t.exprmode)
	{
		// process command on top of command stack

		// 2010-06-05: implemented command handling
		if (t.cmd_stack.empty())
		{
			switch (t.mode)
			{
			case Assignment_TokenMode:
				if (t.state != Translator::BinOp)
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
		CmdItem cmd_item = t.cmd_stack.pop();
		CmdHandler cmd_handler = t.table->cmd_handler(cmd_item.token->index);
		if (cmd_handler == NULL)  // missing command handler?
		{
			return BUG_NotYetImplemented;
		}
		status = (*cmd_handler)(t, &cmd_item, token);
		if (status != Good_TokenStatus)
		{
			token = cmd_item.token;  // command decides where error is
			return status;
		}
		// upon return from the command handler,
		// the hold stack should have the null token on top
		// and done stack should be empty

		// 2010-06-03: expression end check is handled by command

		// pop and delete null token from top of stack
		delete t.hold_stack.pop();

		// TODO do end of line processing here, but for now...

		// 2010-06-05: removed popping/checking of result from done stack
		if (!t.done_stack.empty())
		{
			return BUG_DoneStackNotEmpty;
		}
	}
	// 2010-05-29: make sure command stack is empty (temporary TODO)
	if (!t.cmd_stack.empty())
	{
		return BUG_CmdStackNotEmpty;
	}
	delete token;  // 2010-04-04: delete EOL token
	return Done_TokenStatus;
}


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


//**************************************
//**    ASSIGNMENT COMMAND HANDLER    **
//**************************************

// 2010-05-28: created function from add_token() case EOL_Code
TokenStatus Assign_CmdHandler(Translator &t, CmdItem *cmd_item, Token *token)
{
	// assignment code removed from add_operator(), now handled in
	// Equal_Handler() and Comma_Handler() for each item being assigned,
	// here the last value needs to be processed (2010-07-01)

	// pop operand element pointer off of done stack
	if (t.done_stack.empty())
	{
		DataType datatype = cmd_item->token->datatype;  // save expected type
		cmd_item->token = token;  // point error to end-of-statement token
		return errstatus_datatype[datatype].expected;
	}
	List<RpnItem *>::Element *assign_operand = t.done_stack.pop();

	// reset reference flag of operand
	assign_operand->value->token->reference = false;

	Code cvt_code = cvtcode_have_need[assign_operand->value->token->datatype]
		[cmd_item->token->datatype];

	if (cvt_code == Invalid_Code)
	{
		cmd_item->token = assign_operand->value->token;
		return errstatus_datatype[cmd_item->token->datatype].actual;
	}

	RpnItem *rpn_item;

	if (cvt_code != Null_Code)  // assignment value needs conversion?
	{
		// create hidden convert token with convert code
		rpn_item = new RpnItem(t.table->new_token(cvt_code));
		t.output->append(&rpn_item);  // 2010-10-02
	}

	// save operand if string
	int noperands;
	List<RpnItem *>::Element **operand;
	// don't attach constant strings (2010-12-24)
	if (assign_operand->value->token->datatype == String_DataType
		&& assign_operand->value->token->type != Constant_TokenType)
	{
		noperands = 1;
		operand = new List<RpnItem *>::Element *[1];
		operand[0] = assign_operand;
	}
	else  // don't save value
	{
		noperands = 0;
		operand = NULL;
	}

	// turn of reference flag of assign token, no longer needed (2010-10-02)
	cmd_item->token->reference = false;

	// append token to output
	rpn_item = new RpnItem(cmd_item->token, noperands, operand);
	t.output->append(&rpn_item);

	return Good_TokenStatus;
}


//*********************************
//**    PRINT COMMAND HANDLER    **
//*********************************

// 2010-05-28: created function from add_token() case EOL_Code
TokenStatus Print_CmdHandler(Translator &t, CmdItem *cmd_item, Token *token)
{
	// 2010-06-10: moved end of expression check to end of statement processing

	TokenStatus status = t.add_print_code();
	if (status == Good_TokenStatus  // data type specific code added?
		|| status == Null_TokenStatus  // or not stay on line?
		&& !(cmd_item->flag & PrintStay_CmdFlag))
	{
		// append the print token to go to newline at runtime
		RpnItem *rpn_item = new RpnItem(cmd_item->token);
		t.output->append(&rpn_item);
	}
	else if (status > Good_TokenStatus)
	{
		return status;
	}

	return Good_TokenStatus;
}


//*******************************
//**    LET COMMAND HANDLER    **
//*******************************

// 2010-06-13: implemented function to catch errors
TokenStatus Let_CmdHandler(Translator &t, CmdItem *cmd_item, Token *token)
{
	// this function should not be called for correct statements
	// if it is then it means that the LET command was not completed
	cmd_item->token = token;  // point to end-of-statement token
	// 2010-06-26: make sure done stack is not empty
	if (t.done_stack.empty())
	{
		return ExpAssignItem_TokenStatus;
	}
	return ExpEqualOrComma_TokenStatus;
}


// end: translator.cpp
