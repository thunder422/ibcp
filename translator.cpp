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

#include "ibcp.h"


// function to get a token at the current position
//
//     - a pointer to the token is returned
//     - the token must be deallocated when it is no longer needed
//     - the token may contain an error message if an error was found

Translator::Status Translator::add_token(Token *token)
{
	if (state == Initial)
	{
		// 2010-03-21: check for end of line at begin of input
		if (token->type == Operator_TokenType
			&& table->code(token->index) == EOL_Code)
		{
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
			if (token->has_paren())
			{
				// token is an array or a function
				// TODO
				return NotYetImplemented;
			}
			// token is a variable or a function with no arguments
			//
			// 2010-03-25: added parentheses support
			// 2010-03-26: check for and add dummy token if necessary
			do_pending_paren(hold_stack.top()->index);

			// add token directly output list
			// and push element pointer on done stack
			done_stack.push(output->append(&token));
			state = BinOp;  // next token must be a binary operator
			return Good;
		}
		else  // operator when expecting operand, must be a unary operator
		{
			Code unary_code = table->unary_code(token->index);
			if (unary_code == Null_Code)
			{
				// oops, not a valid unary operator
				return ExpectedOperand;
			}
			// change token to unary operator
			token->index = table->index(unary_code);
			// 2010-03-25: added check for opening parentheses
			if (unary_code == OpenParen_Code)
			{
				// 2010-03-26: don't initialize last_precedence here
				// 2010-03-26: check for and add dummy token if necessary
				do_pending_paren(hold_stack.top()->index);

				// push open parentheses right on stack and return
				hold_stack.push(&token);
				state = Operand;
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
			return ExpectedOperator;
		}
		// 2010-03-21: changed unary operator check
		if (table->is_unary_operator(token->index))
		{
			return ExpectedBinOp;
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
	while (table->precedence(hold_stack.top()->index)
		>= table->precedence(token->index))
	{
		// pop operator on top of stack and add it to the output
		Status status = add_operator(hold_stack.pop());
		if (status != Good)
		{
			return status;
		}
	}

	// check for special token processing
	// 2010-03-25: change code to switch and added closing parentheses support
	Translator::Status status;
	Token *top_token;
	int index;
	Code code;
	switch (table->code(token->index))
	{
	case CloseParen_Code:
		// do closing parentheses processing
		if (hold_stack.empty())
		{
			// oops, stack is empty
			return StackEmpty4;
		}
		top_token = hold_stack.pop();
		if (table->code(top_token->index) != OpenParen_Code)
		{
			// oops, no open parentheses
			return MissingOpenParen;
		}
		delete top_token;  // delete open parentheses token

		// 2010-03-26: moved pending check to before stack emptying while

		// set pending parentheses token pointer
		pending_paren = token;
		break;

	case EOL_Code:
		// do end of line processing
		if (hold_stack.empty())
		{
			// oops, stack is empty
			return StackEmpty;
		}
		token = hold_stack.pop();
		// 2010-03-26: get index/code and delete token before error checking
		index = token->index;
		code = table->code(index);
		delete token;  // delete EOL token

		if (code == Null_Code)
		{
			// TODO do end of line processing here, but for now...
			// nothing is on the stack that's not suppose to be there
			status = Done;
		}
		// 2010-03-25: added missing opening parentheses check
		else if (code == OpenParen_Code)
		{
			// oops, open paren without a close paren
			return MissingCloseParen;
		}
		else
		{
			// this is a diagnostic error, should not occur
			return StackNotEmpty;
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
			return StackEmpty3;
		}
		if (!done_stack.empty())
		{
			return StackNotEmpty2;
		}
		return Done;

	default:  // no special operator, push it onto the holding stack
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

Translator::Status Translator::add_operator(Token *token)
{
	List<Token *>::Element *operand1;
	List<Token *>::Element *operand2;

	// TODO process data types of operands
	// for now just pop the operands off of the stack
	if (!done_stack.pop(&operand1))
	{
		return StackEmpty1;
	}
	// 2010-03-21: corrected unary operator check
	if (!table->is_unary_operator(token->index))
	{
		if (!done_stack.pop(&operand2))
		{
			return StackEmpty2;
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
		output->pop();  // using pop doesn't require pointer variable
	}
	delete output;
	output = NULL;
	while (!hold_stack.empty())
	{
		hold_stack.pop();
	}
	while (!done_stack.empty())
	{
		done_stack.pop();
	}
	// 2010-03-25: parentheses support - need to delete pending parentheses
	if (pending_paren != NULL)
	{
		delete pending_paren;
		pending_paren = NULL;
	}
}


// end: translator.cpp
