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
		Token *null_token = new Token();
		table->set_token(null_token, Null_Code);
		hold_stack.push(null_token);

		state = Operand;
	}
	// 2010-05-29: added check for command token
	if (token->type == Command_TokenType)
	{
		if (mode == Command_TokenMode)
		{
			if (table->token_mode(token->index) != Null_TokenMode)
			{
				mode = table->token_mode(token->index);
				cmd_stack.push();
				cmd_stack.top().token = token;
				cmd_stack.top().code = table->code(token->index);
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

	if (state == Operand)
	{
		if (!token->is_operator())
		{
			// 2010-03-25: added parentheses support
			// 2010-03-26: check for and add dummy token if necessary
			// 2010-05-29: changed argument to token pointer
			do_pending_paren(hold_stack.top());

			// 2010-04-25: set default data type for token if it has none
			set_default_datatype(token);

			// 2010-04-02: moved to after pending parentheses check
			if (token->has_paren())
			{
				// token is an array or a function
				// 2010-04-02: implemented array/function support
				count_stack.push(1);  // add an operand counter
				hold_stack.push(token);
				// leave state == Operand
			}
			else
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
			return Good_TokenStatus;
		}
		else  // operator when expecting operand, must be a unary operator
		{
			Code unary_code = table->unary_code(token->index);
			if (unary_code == Null_Code)
			{
				// oops, not a valid unary operator
				return ExpOperand_TokenStatus;
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
					case Command_TokenMode:
					case Assignment_TokenMode:
						// oops, not expecting parentheses
						return UnexpParenInCmd_TokenStatus;

					case EqualAssignment_TokenMode:
						// continue of a multiple equal assignment
						mode = Expression_TokenMode;  // start of expression
						break;

					case CommaAssignment_TokenMode:
						// in a comma separated list
						return UnexpParenInComma_TokenStatus;

					case Expression_TokenMode:
						// inside an expression, nothing extra to do
						break;
					}
				}
				// push open parentheses right on stack and return
				hold_stack.push(token);
				state = Operand;

				// 2010-04-02: add a null counter to prevent commas
				count_stack.push(0);
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
			return ExpOperator_TokenStatus;
		}
		// 2010-03-21: changed unary operator check
		if (table->is_unary_operator(token->index))
		{
			return ExpBinOp_TokenStatus;
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
	List<RpnItem *>::Element *last_operand;
	// 2010-05-15: renamed operand to list_operand to avoid conflict with member
	List<RpnItem *>::Element *list_operand;
	// 2010-05-22: moved variables used to create rpn item
	int noperands = 0;
	List<RpnItem *>::Element **operand_array;
	// 2010-05-22: moved rpn item point to here
	RpnItem *rpn_item;

	// 2010-04-25: implemented data type handling for non-assignment operators
	//             removed popping of operands, find_code() does this
	//             removed unary operator check
	// 2010-05-08: removed temporary assignment code
	//             find_code() now handles assignment operators
	//             but only last assignment list operand

	// save if token is an assignment list operator
	int assignlist = table->flags(token->index) & AssignList_Flag;

	// process data types of operands (find proper code)
	TokenStatus status = find_code(token, &last_operand);

	// 2010-05-08: process rest of operands for assignment list
	if (assignlist)
	{
		// 2010-05-22: equivalent data type array
		static DataType equivalent_datatype[numberof_DataType] =
		{
			Double_DataType,	// Double
			Integer_DataType,	// Integer
			String_DataType,	// String
			String_DataType,	// TmpStr
			String_DataType		// SubStr
		};

		// 2010-05-22: stack to hold list operands
		SimpleStack<List<RpnItem *>::Element *> list_operands;
		Token *org_token;
		Token *bad_token;
		// 2010-05-22: variables to detect mix string list assignment
		bool mix_string = false;
		DataType datatype;

		// 2010-05-22: push already processed operands onto stack
		list_operands.push(operand[1]);
		list_operands.push(operand[0]);

		if (status == Good_TokenStatus)
		{
			bad_token = NULL;
			org_token = token;  // save original token
		}
		else
		{
			// even though an error already occurred, continue checking rest
			// of assignment list to point to the first error in the statement
			bad_token = token;
			org_token = NULL;  // original token already deleted
			if (status == ExpAssignRef_TokenStatus)
			{
				status = ExpAssignListRef_TokenStatus;
			}
		}
		// 2010-05-22: save data type of last operand
		datatype = last_operand->value->token->datatype;
		while (!done_stack.empty())
		{
			list_operand = done_stack.pop();

			// 2010-05-22: save operand to stack
			list_operands.push(list_operand);

			// 2010-05-22: check for equivalent data type
			if (last_operand->value->token->reference 
				&& equivalent_datatype[list_operand->value->token->datatype]
				!= equivalent_datatype[last_operand->value->token->datatype])
			{
				// data type does not match, set error at last token
				bad_token = last_operand->value->token;
				switch (list_operand->value->token->datatype)
				{
				case Double_DataType:
					status = ExpDouble_TokenStatus;
					break;
				case Integer_DataType:
					status = ExpInteger_TokenStatus;
					break;
				case String_DataType:
				case SubStr_DataType:  // 2010-05-22: added data type
					status = ExpString_TokenStatus;
					break;
				}
			}

			if (!list_operand->value->token->reference)
			{
				// found a non-reference, set bad token to return
				bad_token = list_operand->value->token;
				status = ExpAssignListRef_TokenStatus;
			}

			// 2010-05-22: see if mix strings in list
			if (equivalent_datatype[datatype] == String_DataType
				&& datatype != list_operand->value->token->datatype)
			{
				mix_string = true;
			}

			// make this operand the last operand
			last_operand = list_operand;
		}

		if (bad_token != NULL)
		{
			if (org_token != NULL)
			{
				delete org_token;  // delete the operator token
			}
			token = bad_token;  // return operand with error
		}
		if (status != Good_TokenStatus)
		{
			return status;
		}

		// 2010-05-22: see if there is a mix string assignment list
		if (mix_string)
		{
				token->index = table->index(AssignListMixStr_Code);
		}
		// 2010-05-15: if string, create an array to hold all operands
		if (table->flags(token->index) & String_Flag)
		{
			noperands = list_operands.nitems();
			operand_array = new List<RpnItem *>::Element *[noperands];
			// fill array from stack in reverse order
			for (int i = 0; i < noperands; i++)
			{
				operand_array[i] = list_operands.pop();
			}
		}
		rpn_item = new RpnItem(token);
		rpn_item->set(noperands, operand_array);
	}
	// 2010-05-22: added check for string here if not a list operator
	else
	{
		if (status != Good_TokenStatus)
		{
			return status;
		}
		if (table->flags(token->index) & String_Flag)
		{
			// for string tokens, save operands
			noperands = table->noperands(token->index);
			operand_array = operand;
		}
		rpn_item = new RpnItem(token, noperands, operand_array);
	}

	// 2010-05-29: if assignment operator, check for LET command
	if (table->flags(token->index) & Reference_Flag)
	{
		if (!cmd_stack.empty() && cmd_stack.top().code == Let_Code)
		{
			// if let command on stack, pop it off and set let sub-code
			cmd_stack.pop();
			token->subcode |= Let_SubCode;
		}
	}

	// 2010-03-25: added parentheses support BEGIN
	// 2010-03-26: replaced code with function call
	// 2010-05-29: changed argument to token pointer
	do_pending_paren(token);

	// save precedence of operator being added
	// (doesn't matter if not currently within parentheses,
	// it will be reset upon next open parentheses)
	last_precedence = table->precedence(token->index);
	// 2010-03-25: added parentheses support END

	// add token to output list and push element pointer on done stack
	// 2010-05-15: for operators that have string operand, save the operands
	// 2010-05-22: moved setting of noperands to above with operand_array
	// 2010-05-15: create rpn item to add to output list
	done_stack.push(output->append(&rpn_item));

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

// 2010-04-25: implemented new function
// 2010-05-08: added last_operand argument for assignment list processing
TokenStatus Translator::find_code(Token *&token,
	List<RpnItem *>::Element **last_operand)
{
	struct {
		Code code;						// code match attempted for
		Code cvt_code[Max_Operands];	// resulting conversion codes
	} info[Max_Assoc_Codes + 1];		// matching information
	static const int MAIN = Max_Assoc_Codes;	// index of main code in info[]
	int convert;						// index of convertible match
	int partial;						// index of partial match
	// 2010-05-15: moved operand[] to class
	int i;								// loop index variable

	// get number of operands for token and allocate arrays
	int noperands = table->noperands(token->index);

	// pop operand element pointers off of done stack
	for (int i = noperands; --i >= 0;)
	{
		if (done_stack.empty())
		{
			// oops, there should have been operands on done stack
			return BUG_DoneStackEmpty;
		}
		operand[i] = done_stack.pop();

		if (i == 0)  // first operand?
		{
			// 2010-05-08: for assignment list, return first operand
			if (last_operand != NULL)
			{
				*last_operand = operand[0];
			}

			// 2010-05-08: check if reference is required for first operand
			if (table->flags(token->index) & Reference_Flag)
			{
				if (!operand[0]->value->token->reference)
				{
					// need a reference, so return error
					delete token;  // delete the operator token
					token = operand[0]->value->token;  // return operand w/error

					// 2010-05-21: check if error is from a sub-string function
					if (token->datatype == SubStr_DataType)
					{
						// get string argument of sub-string function
						token = operand[0]->value->operand[0]->value->token;
					}
					return ExpAssignRef_TokenStatus;
				}
				continue;  // skip clearing of reference flag (will exit loop)
			}

			// 2010-05-19: check for a sub-string function and string operand
			if (token->datatype == SubStr_DataType
				&& operand[0]->value->token->datatype == String_DataType)
			{
				// set sub-string functions reference flag to operand's
				// (this will make sub-string function a reference if the
				// string operand was a reference)
				token->reference = operand[0]->value->token->reference;
			}
		}

		// reset reference flag of operand
		operand[i]->value->token->reference = false;
	}

	// see if main code's data types match
	Match match = match_code(info[MAIN].cvt_code,
		info[MAIN].code = table->code(token->index));
	if (match == Yes_Match)
	{
		return Good_TokenStatus;  // an exact match, we're done here
	}
	convert = match == Cvt_Match ? MAIN : -1;
	partial = MAIN;

	// see if any associated code's data types match
	// 2010-05-08: get actual number of associated codes
	for (int i = 0; i < table->nassoc_codes(token->index); i++)
	{
		info[i].code = table->assoc_code(token->index, i);
		if (info[i].code == Null_Code)
		{
			break;  // no more codes to check
		}
		match = match_code(info[i].cvt_code, info[i].code);
		if (match == Yes_Match)
		{
			// change token's code and data type to associated code
			token->index = table->index(info[i].code);
			token->datatype = table->datatype(token->index);

			return Good_TokenStatus;  // an exact match, we're done here
		}
		else if (match == Cvt_Match && convert == -1)
		{
			convert = i;  // remember this as possible code
		}
		else if (info[i].cvt_code[0] == Null_Code)
		{
			partial = i;  // remember this partial match for error reporting
		}
	}

	// no exact matches, check for a convertible match
	if (convert != -1)
	{
		// change token's code and data type to convertible code
		// (convertible code may be main code)
		token->index = table->index(info[convert].code);
		token->datatype = table->datatype(token->index);

		// insert conversion codes
		for (i = 0; i < noperands; i++)
		{
			Code code = info[convert].cvt_code[i];
			if (code != Null_Code)
			{
				// create convert token with convert code
				Token *cvt_token = new Token;
				cvt_token->index = table->index(code);
				cvt_token->type = table->type(cvt_token->index);
				cvt_token->datatype = table->datatype(cvt_token->index);

				// add token to output list after operand
				// 2010-05-15: create rpn item to add to output list
				RpnItem *rpn_item = new RpnItem(cvt_token);
				// 2010-05-16: set operand to new conversion token
				operand[i] = output->append(operand[i], &rpn_item);
			}
		}
		return Good_TokenStatus;
	}
	
	// no match found, find error and report it
	for (i = 0; i < noperands; i++)
	{
		if (info[partial].cvt_code[i] == Invalid_Code)
		{
			break;  // found first operand with bad data type
		}
	}

	// change token to token with invalid data type and return error
	token = operand[i]->value->token;
	switch (table->operand_datatype(table->index(info[partial].code), i))
	{
	case Double_DataType:
		return ExpDouble_TokenStatus;
	case Integer_DataType:
		return ExpInteger_TokenStatus;
	case String_DataType:
		return ExpString_TokenStatus;
	}
}


// function to check if the data types in the array of operands matches the
// data types for the code specified
//
//   - returns No_Match if the data types do not match (can't be converted)
//   - returns Yes_Match if the data types are an exact match
//   - returns Cvt_Match if the data types match or can be converted
//   - for Cvt_Match, the cvt_code array is filled with conversion codes
//     for each operarand
//   - for No_Match, the cvt_code array is only filled up to the first operand
//     that can be converted (conversion code set to Invalid)

// 2010-04-25: implemented new function
// 2010-05-15: removed operand argument, operand now a class member
Translator::Match Translator::match_code(Code *cvt_code, Code code)
{
	// array of conversion codes [have data type] [need data type]
	// 2010-05-15: add entries for TmpStr_DataType
	// 2010-05-19: add entries for SubStr_DataType
	static Code cvtcode_have_need[numberof_DataType][numberof_DataType] =
	{
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

	int index = table->index(code);
	Match match = Yes_Match;  // assume match to start
	for (int i = 0; i < table->noperands(index); i++)
	{
		// 2010-05-08: check if first operand is a reference
		if (i == 0 && operand[0]->value->token->reference)
		{
			// for reference, the data type must be an exact match
			if (operand[0]->value->token->datatype
				== table->operand_datatype(index, 0))
			{
				cvt_code[0] = Null_Code;
			}
			else  // not an exact match, so no match
			{
				cvt_code[0] = Invalid_Code;
				return No_Match;
			}
		}
		else  // non-reference operand
		{
			cvt_code[i] = cvtcode_have_need[operand[i]->value->token->datatype]
				[table->operand_datatype(index, i)];
		}
		if (cvt_code[i] == Invalid_Code)
		{
			return No_Match;  // no match here, exit
		}
		else if (cvt_code[i] != Null_Code)  // have a conversion code?
		{
			match = Cvt_Match;  // then this is a convertible match
		}
		// else Null_Code, leave match as is
	}
	return match;
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


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                               TOKEN HANDLERS                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


//**********************************
//**    OPERATOR TOKEN HANDLER    **
//**********************************

// 2010-05-29: created function from add_token() no special operator section
TokenStatus Operator_Handler(Translator &p, Token *&token)
{
	// 2010-04-16: only check mode if not in parentheses (expression)
	if (p.count_stack.empty())
	{
		// 2010-04-11: implemented assignment handling
		switch (p.mode)
		{
		case Command_TokenMode:
		case Assignment_TokenMode:
			// oops, expected an equal or comma
			return UnexpOperator_TokenStatus;

		case EqualAssignment_TokenMode:
			// continue of a multiple equal assignment
			p.mode = Expression_TokenMode;  // start of expression
			break;

		case CommaAssignment_TokenMode:
			// in a comma separated list
			return ExpEqualOrComma_TokenStatus;

		case Expression_TokenMode:
			// inside an expression, nothing extra to do
			break;
		}
	}
	// push it onto the holding stack
	p.hold_stack.push(token);
	p.state = Translator::Operand;
	return Good_TokenStatus;
}


//********************************
//**    EQUAL TOKEN HANDLER    **
//********************************

// 2010-05-28: created function from add_token() case Eq_Code
TokenStatus Equal_Handler(Translator &p, Token *&token)
{
	// 2010-04-11: implemented assignment handling
	// 2010-04-16: only check mode if not in parentheses (expression)
	if (p.count_stack.empty())
	{
		switch (p.mode)
		{
		case Command_TokenMode:
		case Assignment_TokenMode:
			// this is an assignment operator, change token
			token->index = p.table->index(Assign_Code);

			p.mode = EqualAssignment_TokenMode;  // allow more equals

			// push assignment operator on hold stack
			p.hold_stack.push(token);
			break;

		case EqualAssignment_TokenMode:
			// continue of a multiple equal assignment
			delete token;  // don't need another assignment token on stack

			// change assignment token on hold stack to list assignment
			p.hold_stack.top()->index = p.table->index(AssignList_Code);
			break;

		case CommaAssignment_TokenMode:
			// assignment for a comma separated list, change token
			// TODO user sub-code; needs to be a flag
			token->index = p.table->index(AssignList_Code);
			// 2010-05-29: set flag in token to indicate comma
			token->subcode |= Comma_SubCode;

			p.mode = Expression_TokenMode;  // end of list, expression follows

			// push assignment operator on hold stack
			p.hold_stack.push(token);
			break;

		case Expression_TokenMode:
			// inside an expression, keep Eq_Code, push on hold stack
			p.hold_stack.push(token);
			break;
		}
	}
	else  // inside  an expression, keep Eq_Code, push on hold stack
	{
		p.hold_stack.push(token);
	}
	p.state = Translator::Operand;
	return Good_TokenStatus;
}


//********************************
//**    COMMA TOKEN HANDLER    **
//********************************

// 2010-05-28: created function from add_token() case Comma_Code
TokenStatus Comma_Handler(Translator &p, Token *&token)
{
	// 2010-04-02: implemented comma operator code handling
	// 2010-04-17: only check mode if not in parentheses (expression)
	if (p.count_stack.empty())
	{
		// 2010-04-11: implemented multiple assignment handling
		switch (p.mode)
		{
		case Command_TokenMode:
		case Assignment_TokenMode:
			// this is an assignment list
			delete token;  // don't need comma token on stack
			// comma separated assignment list
			p.mode = CommaAssignment_TokenMode;
			break;

		case EqualAssignment_TokenMode:
			// oops, comma in a multiple equal assignment - not allowed
			return UnexpAssignComma_TokenStatus;

		case CommaAssignment_TokenMode:
			// continuation a comma separated list
			delete token;  // don't need comma token on stack
			break;

		case Expression_TokenMode:
			// inside an expression, but not in array or function
			return UnexpExprComma_TokenStatus;
		}
	}
	else
	{
		// inside an expression, check if in array or function
		if (p.count_stack.top() == 0)
		{
			return UnexpParenComma_TokenStatus;
		}
		// increment the number of operands
		p.count_stack.top()++;
		// delete comma token, it's not needed (2010-04-25)
		delete token;
	}
	p.state = Translator::Operand;
	return Good_TokenStatus;
}


//*******************************************
//**    CLOSE PARENTHESES TOKEN HANDLER    **
//*******************************************

// 2010-05-28: created function from add_token() case CloseParen_Code
TokenStatus CloseParen_Handler(Translator &p, Token *&token)
{
	Token *top_token;
	int noperands;  // 2010-04-02: for array/function support

	// do closing parentheses processing
	if (p.hold_stack.empty())
	{
		// oops, stack is empty
		return BUG_StackEmpty4;
	}
	top_token = p.hold_stack.pop();

	// 2010-04-02: implemented array/function support BEGIN
	if (p.count_stack.empty())
	{
		return NoOpenParen_TokenStatus;
	}
	noperands = p.count_stack.pop();
	if (noperands == 0)
	{
		// just a parentheses expression 
		if (p.table->code(top_token->index) != OpenParen_Code)
		{
			// oops, no open parentheses
			return BUG_UnexpectedCloseParen;  // this should not happen
		}
		delete top_token;  // delete open parentheses token

		// 2010-04-16: clear reference for item on top of done stack
		p.done_stack.top()->value->token->reference = false;

		// 2010-03-30: set pending parentheses token pointer
		p.pending_paren = token;
	}
	else  // array or function
	{
		// 2010-04-12: set reference flag for array or function
		// FIXME not sure DefFuncP should have reference set, set it for now
		if (top_token->type != IntFuncP_TokenType)
		{
			top_token->reference = true;

			// data types for array subscripts and define/user functions
			// cannot be checked here
			// 2010-05-15: save operands for storage in output list
			for (int i = noperands; --i >= 0;)
			{
				if (p.done_stack.empty())
				{
					return BUG_StackEmpty5;
				}
				p.operand[i] = p.done_stack.pop();
			}
			// delete close paren token, it's not needed (2010-04-25)
			delete token;
		}
		// 2010-04-04: check for number of arguments for internal functions
		else
		{
			// delete close paren token, it's not needed (2010-04-25)
			delete token;
			token = top_token;

			if (noperands != p.table->noperands(token->index))
			{
				// number of arguments doesn't match function's entry
				int index;

				if ((p.table->flags(token->index) & Multiple_Flag) != 0
					&& (index = p.table->search(token->index, noperands)) > 0)
				{
					// change token to new code (index)
					token->index = index;
				}
				else
				{
					return WrongNumOfArgs_TokenStatus;
				}				
			}
			// 2010-04-25: implemented data type handling
			// process data types of arguments (find proper code)
			TokenStatus status = p.find_code(token);
			if (status != Good_TokenStatus)
			{
				return status;
			}

			// 2010-05-15: don't save operands if none are strings
			if (!(p.table->flags(token->index) & String_Flag))
			{
				noperands = 0;  // no string operands, don't save operands
			}
		}

		// make sure token is an array or a function
		if (!top_token->has_paren())
		{
			// unexpected token on stack
			return BUG_UnexpectedToken;
		}

		// add token to output list and push element pointer on done stack
		// 2010-05-15: create rpn item to add to output list
		RpnItem *rpn_item = new RpnItem(top_token, noperands, p.operand);
		p.done_stack.push(p.output->append(&rpn_item));
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
TokenStatus EndOfLine_Handler(Translator &p, Token *&token)
{
	TokenStatus status;
	Token *top_token;
	int index;
	Code code;
	bool has_paren;

	// do end of line processing
	if (p.hold_stack.empty())
	{
		// oops, stack is empty
		return BUG_HoldStackEmpty;  // 2010-04-26: changed bug named
	}
	top_token = p.hold_stack.top();
	code = p.table->code(top_token->index);

	if (code == Null_Code)
	{
		// TODO do end of line processing here, but for now...
		// nothing is on the stack that's not suppose to be there
		status = Done_TokenStatus;
	}
	// 2010-03-25: added missing opening parentheses check
	// 2010-04-02: include array/function tokens in check
	else if (code == OpenParen_Code || top_token->has_paren())
	{
		// oops, open paren without a close paren
		// (leave EOL allocated so error can be reported against it)
		return NoCloseParen_TokenStatus;
	}
	else
	{
		// this is a diagnostic error, should not occur
		return BUG_StackNotEmpty;
	}

	// 2010-03-25: check if there is a pending closing parentheses
	// 2010-03-26: replaced code with function call
	// 2010-05-29: changed argument to token pointer
	// pass Null_Code token (will always add dummy)
	p.do_pending_paren(top_token);
	p.hold_stack.pop();
	delete top_token;  // delete token from top of stack

	if (status != Done_TokenStatus)
	{
		return status;
	}
	// 2010-03-21: check if there is a result on the done_stack
	// there should be one value on the done stack, need to pop it off
	List<RpnItem *>::Element *result;
	if (p.done_stack.empty())
	{
		return BUG_StackEmpty3;
	}
	result = p.done_stack.pop();
	if (!p.done_stack.empty())
	{
		return BUG_StackNotEmpty2;
	}
	if (!p.cmd_stack.empty())
	{
		return BUG_CmdStackNotEmpty;
	}
	delete token;  // 2010-04-04: delete EOL token
	return Done_TokenStatus;
}


// end: translator.cpp
