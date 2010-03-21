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

#include "ibcp.h"


// function to get a token at the current position
//
//     - a pointer to the token if returned
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

	// check for last token
	if (table->code(token->index) != EOL_Code)
	{
		// now operator can be pushed in the holding stack
		hold_stack.push(&token);
		state = Operand;
	}
	else  // do end of line processing
	{
		Translator::Status status;

		if (hold_stack.empty())
		{
			// oops, stack is empty
			return StackEmpty;
		}
		token = hold_stack.pop();
		if (table->code(token->index) == Null_Code)
		{
			// TODO do end of line processing here, but for now...
			// nothing is on the stack that's not suppose to be there
			status = Done;
		}
		else
		{
			// this is a diagnostic error, should not occur
			status = StackNotEmpty;
		}
		delete token;
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

	}
	return Good;
}


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
	// add token to output list and push element pointer on done stack
	done_stack.push(output->append(&token));

	return Good;
}


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
}


// end: translator.cpp
