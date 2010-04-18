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

#include "ibcp.h"


// function to add a token to the output list, but token may be placed
// on hold stack pending adding it to the output list so that higher
// precedence tokens may be added to the list first
//
//     - Done status returned when last token is processed
//     - Good status returned when token successfully processed
//     - error status returned when an error is detected
//     - token argument may be changed when an error is detected

// 2010-04-04: made argument a reference so different token can be returned
Translator::Status Translator::add_token(Token *&token)
{
	if (state == Initial)
	{
		// 2010-03-21: check for end of line at begin of input
		if (token->type == Operator_TokenType
			&& table->code(token->index) == EOL_Code)
		{
			delete token;  // 2010-04-04: delete EOL token
			return Done;
		}
		
		// push null token to be last operator on stack
		// to prevent from popping past bottom of stack
		Token *null_token = new Token();
		table->set_token(null_token, Null_Code);
		hold_stack.push(&null_token);

		state = Operand;
	}
	if (state == Operand)
	{
		if (!token->is_operator())
		{
			// token is a variable or a function with no arguments
			//
			// 2010-03-25: added parentheses support
			// 2010-03-26: check for and add dummy token if necessary
			do_pending_paren(hold_stack.top()->index);

			// 2010-04-02: moved to after pending parentheses check
			if (token->has_paren())
			{
				// token is an array or a function
				// 2010-04-02: implemented array/function support
				count_stack.push(1);  // add an operand counter
				hold_stack.push(&token);
				// leave state == Operand
			}
			else
			{
				// 2010-04-12: set reference flag for variable or function
				if (token->type == NoParen_TokenType
					|| token->type == DefFuncN_TokenType)
				{
					token->reference = true;
				}

				// add token directly output list
				// and push element pointer on done stack
				done_stack.push(output->append(&token));
				state = BinOp;  // next token must be a binary operator
			}
			return Good;
		}
		else  // operator when expecting operand, must be a unary operator
		{
			Code unary_code = table->unary_code(token->index);
			if (unary_code == Null_Code)
			{
				// oops, not a valid unary operator
				return Error_ExpectedOperand;
			}
			// change token to unary operator
			token->index = table->index(unary_code);
			// 2010-03-25: added check for opening parentheses
			if (unary_code == OpenParen_Code)
			{
				// 2010-03-26: don't initialize last_precedence here
				// 2010-03-26: check for and add dummy token if necessary
				do_pending_paren(hold_stack.top()->index);

				// 2010-04-16: implemented assignment handling
				if (count_stack.empty())
				{
					// only check mode if not within parentheses
					// (otherwise in an expression)
					switch (mode)
					{
					case Command:
						// oops, not expecting parentheses
						return Error_UnexpParenInCmd;

					case Equal:
						// continue of a multiple equal assignment
						mode = Expression;  // start of expression
						break;

					case Comma:
						// in a comma separated list
						return Error_UnexpParenInComma;

					case Expression:
						// inside an expression, nothing extra to do
						break;
					}
				}
				// push open parentheses right on stack and return
				hold_stack.push(&token);
				state = Operand;

				// 2010-04-02: add a null counter to prevent commas
				count_stack.push(0);
				return Good;
			}
			// fall thru to operator code
		}
	}
	else  // a binary operator is expected
	{
		if (!token->is_operator())
		{
			// state == BinOp, but token is not an operator
			return Error_ExpectedOperator;
		}
		// 2010-03-21: changed unary operator check
		if (table->is_unary_operator(token->index))
		{
			return Error_ExpectedBinOp;
		}
		// 2010-03-26: initialize last precedence before emptying stack for ')'
		if (table->code(token->index) == CloseParen_Code)
		{
			// 2010-03-26: check for and add dummy token if necessary
			//             before emptying stack
			do_pending_paren(hold_stack.top()->index);

			// now set last precedence to highest in case no operators in ( )
			last_precedence = Highest_Precedence;
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
		Status status = add_operator(top_token);
		if (status != Good)
		{
			token = top_token;  // 2010-04-13: return token with error
			return status;
		}
	}

	// check for special token processing
	// 2010-03-25: change code to switch and added closing parentheses support
	Translator::Status status;
	int index;
	Code code;
	bool has_paren;
	int noperands;  // 2010-04-02: for array/function support
	switch (table->code(token->index))
	{
	// 2010-04-11: implemented assignment handling
	case Eq_Code:
		// 2010-04-16: only check mode if not in parentheses (expression)
		if (count_stack.empty())
		{
			switch (mode)
			{
			case Command:
				// this is an assignment operator, change token
				token->index = table->index(Assign_Code);

				mode = Equal;  // allow more equals (multiple assignment)

				// push assignment operator on hold stack
				hold_stack.push(&token);
				break;

			case Equal:
				// continue of a multiple equal assignment
				delete token;  // don't need another assignment token on stack

				// change assignment token on hold stack to list assignment
				hold_stack.top()->index = table->index(AssignList_Code);
				break;

			case Comma:
				// assignment for a comma separated list, change token
				// TODO set flag in token to indicate comma
				token->index = table->index(AssignList_Code);

				mode = Expression;  // end of list, expression follows

				// push assignment operator on hold stack
				hold_stack.push(&token);
				break;

			case Expression:
				// inside an expression, keep Eq_Code, push on hold stack
				hold_stack.push(&token);
				break;
			}
		}
		else  // inside  an expression, keep Eq_Code, push on hold stack
		{
			hold_stack.push(&token);
		}
		state = Operand;
		break;

	// 2010-04-02: implemented comma operator code handling
	case Comma_Code:
		// 2010-04-17: only check mode if not in parentheses (expression)
		if (count_stack.empty())
		{
			// 2010-04-11: implemented multiple assignment handling
			switch (mode)
			{
			case Command:
				// this is an assignment list
				delete token;  // don't need comma token on stack
				mode = Comma;  // comma separated multiple assignment
				break;

			case Equal:
				// oops, comma in a multiple equal assignment - not allowed
				return Error_UnexpAssignComma;

			case Comma:
				// continuation a comma separated list
				// TODO set flag in token to indicate comma
				delete token;  // don't need comma token on stack
				break;

			case Expression:
				// inside an expression, but not in array or function
				return Error_UnexpExprComma;
			}
		}
		else
		{
			// inside an expression, check if in array or function
			if (count_stack.top() == 0)
			{
				return Error_UnexpParenComma;
			}
			// increment the number of operands
			count_stack.top()++;
		}
		state = Operand;
		break;

	case CloseParen_Code:
		// do closing parentheses processing
		if (hold_stack.empty())
		{
			// oops, stack is empty
			return BUG_StackEmpty4;
		}
		top_token = hold_stack.pop();

		// 2010-04-02: implemented array/function support BEGIN
		if (count_stack.empty())
		{
			return Error_MissingOpenParen;
		}
		noperands = count_stack.pop();
		if (noperands == 0)
		{
			// just a parentheses expression 
			if (table->code(top_token->index) != OpenParen_Code)
			{
				// oops, no open parentheses
				return BUG_UnexpectedCloseParen;  // this should not happen
			}
			delete top_token;  // delete open parentheses token

			// 2010-04-16: clear reference for item on top of done stack
			done_stack.top()->value->reference = false;

			// 2010-03-30: set pending parentheses token pointer
			pending_paren = token;
		}
		else  // array or function
		{
			// 2010-04-12: set reference flag for array or function
			// FIXME not sure DefFuncP should have reference set, set it for now
			if (top_token->type != IntFuncP_TokenType)
			{
				top_token->reference = true;
			}
			// 2010-04-04: check for number of arguments for internal functions
			else if (noperands != table->nargs(top_token->index))
			{
				// actual number of arguments doesn't match function's entry
				int index;

				if ((table->flags(top_token->index) & Multiple_Flag) != 0
					&& (index = table->search(top_token->index, noperands)) > 0)
				{
					// change token to new code (index)
					top_token->index = index;
				}
				else
				{
					delete token;  // delete open parentheses token
					token = top_token;
					return Error_WrongNumberOfArgs;
				}				
			}

			// TODO other processing required (checking operands)
			// TODO for now pop operands from done stack and add token to output

			for (int i = 0; i < noperands; i++)
			{
				List<Token *>::Element *operand;
				if (!done_stack.pop(&operand))
				{
					return BUG_StackEmpty5;
				}
				// 2010-04-12: reset reference flag of operands
				if (top_token->type == IntFuncP_TokenType)
				{
					operand->value->reference = false;
				}
			}

			// make sure token is an array or a function
			if (!top_token->has_paren())
			{
				// unexpected token on stack
				return BUG_UnexpectedToken;
			}

			// add token to output list and push element pointer on done stack
			done_stack.push(output->append(&top_token));
		}
		// 2010-04-02: implemented array/function support END

		// 2010-03-26: moved pending check to before stack emptying while

		// 2010-04-02: moved set pending paren, not needed for array/function
		break;

	case EOL_Code:
		// do end of line processing
		if (hold_stack.empty())
		{
			// oops, stack is empty
			return BUG_StackEmpty;
		}
		top_token = hold_stack.pop();
		// 2010-03-26: get index/code and delete token before error checking
		index = top_token->index;
		code = table->code(index);
		has_paren = top_token->has_paren();
		delete top_token;  // delete token from top of stack

		if (code == Null_Code)
		{
			// TODO do end of line processing here, but for now...
			// nothing is on the stack that's not suppose to be there
			status = Done;
		}
		// 2010-03-25: added missing opening parentheses check
		// 2010-04-02: include array/function tokens in check
		else if (code == OpenParen_Code || has_paren)
		{
			// oops, open paren without a close paren
			// (leave EOL allocated so error can be reported against it)
			return Error_MissingCloseParen;
		}
		else
		{
			// this is a diagnostic error, should not occur
			return BUG_StackNotEmpty;
		}

		// 2010-03-25: check if there is a pending closing parentheses
		// 2010-03-26: replaced code with function call
		do_pending_paren(index);  // index of Null_Code (i.e. always add dummy)

		if (status != Done)
		{
			return status;
		}
		// 2010-03-21: check if there is a result on the done_stack
		// there should be one value on the done stack, need to pop it off
		List<Token *>::Element *result;
		if (!done_stack.pop(&result))
		{
			return BUG_StackEmpty3;
		}
		if (!done_stack.empty())
		{
			return BUG_StackNotEmpty2;
		}
		delete token;  // 2010-04-04: delete EOL token
		return Done;

	default:  // no special operator
		// 2010-04-16: only check mode if not in parentheses (expression)
		if (count_stack.empty())
		{
			// 2010-04-11: implemented assignment handling
			switch (mode)
			{
			case Command:
				// oops, expected an equal or comma
				return Error_UnexpectedOperator;

			case Equal:
				// continue of a multiple equal assignment
				mode = Expression;  // start of expression
				break;

			case Comma:
				// in a comma separated list
				return Error_ExpectedEqualOrComma;

			case Expression:
				// inside an expression, nothing extra to do
				break;
			}
		}
		// push it onto the holding stack
		hold_stack.push(&token);
		state = Operand;
	}
	return Good;
}


// function to add operator token to output list and to process data types
// for operator with the done stack
//
//    - for now just pops operands of operator from done stack
//    - for now just pushes operator onto done stack
//    - before added operator to done stack, checks for pending paren token
//    - sets last_precedence for operator being added to output list

// 2010-04-13: made argument a reference so different value can be returned
Translator::Status Translator::add_operator(Token *&token)
{
	List<Token *>::Element *operand1;
	List<Token *>::Element *operand2;

	// TODO process data types of operands
	// for now just pop the operands off of the stack
	if (!done_stack.pop(&operand1))
	{
		return BUG_StackEmpty1;
	}
	// 2010-04-12: reset reference flag of operand (need value at run-time)
	operand1->value->reference = false;

	// 2010-03-21: corrected unary operator check
	if (!table->is_unary_operator(token->index))
	{
		if (!done_stack.pop(&operand2))
		{
			return BUG_StackEmpty2;
		}
		// 2010-04-13: add code to check for references on assignment operator
		Code code = table->code(token->index);
		if (code == Assign_Code)
		{
			if (!operand2->value->reference)
			{
				// need a reference, so return error
				delete token;  // delete the operator token
				token = operand2->value;  // return operand with error
				return Error_ExpAssignReference;
			}
		}
		// 2010-04-14: add code to check for references on assign list operator
		else if (code == AssignList_Code)
		{
			Token *bad_token = NULL;
			do {
				if (!operand2->value->reference)
				{
					// need a reference, so remember last bad token,
					// which is the first one in the list
					bad_token = operand2->value;
				}
			}
			while (done_stack.pop(&operand2));
			if (bad_token != NULL)
			{
				// need a reference, so return error
				delete token;  // delete the operator token
				token = bad_token;  // return operand with error
				return Error_ExpAssignListReference;
			}
		}
		else  // normal operator, reset reference flag (need value at run-time)
		{
			// 2010-04-12: reset reference flag of operand
			operand2->value->reference = false;
		}
	}
	// 2010-03-25: added parentheses support BEGIN
	// 2010-03-26: replaced code with function call
	do_pending_paren(token->index);

	// save precedence of operator being added
	// (doesn't matter if not currently within parentheses,
	// it will be reset upon next open parentheses)
	last_precedence = table->precedence(token->index);
	// 2010-03-25: added parentheses support END

	// add token to output list and push element pointer on done stack
	done_stack.push(output->append(&token));

	return Good;
}


// function to check if there is a pending parentheses token and if there is,
// check to see if it should be added to the output as a dummy token so that
// the Recreator can added the unnecessary, but entered by the user, set of
// parentheses
//
//   - index argument is table index of operator to check against

void Translator::do_pending_paren(int index)
{
	if (pending_paren != NULL)  // is a closing parentheses token pending?
	{
		// may need to add a dummy token
		// if the precedence of the last operator added within the
		// last parentheses sub-expression is higher than or
		// same as (operand state only) the operator
		int precedence = table->precedence(index);
		if (last_precedence > precedence
			|| state == Operand && last_precedence == precedence)
		{
			// add dummy token
			output->append(&pending_paren);
			// TODO something needed on done stack?
		}
		else  // don't need pending token
		{
			delete pending_paren;  // release it's memory
		}
		pending_paren = NULL;  // reset pending token					
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
}


// end: translator.cpp
