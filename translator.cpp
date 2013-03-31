// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: translator.cpp - contains code for the translator class
//	Copyright (C) 2010-2012  Thunder422
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
//	2010-03-01	initial version

#include "translator.h"
#include "token.h"
#include "table.h"
#include "parser.h"
#include "tokenhandlers.h"


// highest precedence value
enum {
	HighestPrecedence = 127
	// this value was selected as the highest value because it is the highest
	// one-byte signed value (in case the precedence member is changed to an
	// char); all precedences in the table must be below this value
};


// array of conversion codes [have data type] [need data type]
static Code cvtCodeHaveNeed[numberof_DataType][numberof_DataType] = {
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


// function to parse and translate an input line to an RPN output list
//
//   - returns true if successful, use output() to get RPN output list
//   - returns false if failed, use errorToken() and errorMessage()
//     to get token error occurred at and the error message

bool Translator::setInput(const QString &input, bool exprMode)
{
	Token *token;
	Token *parsedToken;
	TokenStatus status;

	Parser parser(m_table);
	parser.setInput(input);

	m_exprMode = exprMode;  // save flag
	// (expression mode for testing)
	m_mode = m_exprMode ? Expression_TokenMode : Command_TokenMode;

	m_output = new RpnList;
	m_state = Initial_State;

	do {
		// set parser operand state from translator
		parser.setOperandState(m_state == Operand_State
			|| m_state == OperandOrEnd_State);
		token = parsedToken = parser.token();
		if (token->isType(Error_TokenType))
		{
			setErrorToken(token);
			if (m_mode == Command_TokenMode)
			{
				token->setLength(1);  // just point to first character
				status = ExpCmd_TokenStatus;
			}
			else if (m_state == EndExpr_State)
			{
				token->setLength(1);  // just point to first character
				status = endExpressionError();
			}
			else if (m_state != Operand_State && m_state != OperandOrEnd_State)
			{
				token->setLength(1);  // just point to first character
				status = operatorError();
			}
			else  // Operand_State
			{
				DataType dataType;

				switch (m_mode)
				{
				case Assignment_TokenMode:
				case AssignmentList_TokenMode:
					// in a comma separated list
					status = assignmentError();
					if (status != ExpNumExpr_TokenStatus)
					{
						token->setLength(1);  // just point to first character
					}
					else if (token->dataType() == Double_DataType)
					{
						// token has number error, return parser error
						status = Null_TokenStatus;
					}
					break;

				case Expression_TokenMode:
					// something is on the stack that's not suppose to be there
					// this is a diagnostic error, should not occur
					if ((status = getExprDataType(dataType))
						== Good_TokenStatus)
					{
						status = expectedErrStatus(dataType);
					}
					if ((status == ExpNumExpr_TokenStatus
						|| status == ExpExpr_TokenStatus)
						&& token->dataType() == Double_DataType)
					{
						// for numeric parser errors in numeric expressions
						status = Null_TokenStatus;  // return parser error
					}
					else
					{
						token->setLength(1);  // just point to first character
					}
					break;

				case Reference_TokenMode:
					if (m_countStack.isEmpty())
					{
						token->setLength(1);  // just point to first character
						status = ExpVar_TokenStatus;
					}
					else if (token->dataType() != Double_DataType)
					{
						token->setLength(1);  // just point to first character
						status = ExpNumExpr_TokenStatus;
					}
					else
					{
						// for numeric parser errors in numeric expressions
						status = Null_TokenStatus;  // return parser error
					}
					break;

				default:
					status = BUG_InvalidMode;
					break;
				}
			}
			m_errorMessage = status == Null_TokenStatus
				?  token->string() : token->message(status);
			cleanUp();
			return false;
		}
		status = addToken(token);
	}
	while (status == Good_TokenStatus);

	if (status != Done_TokenStatus)
	{
		// token pointer is set to cause of error
		// check if token is the parsed token or is an open paren
		if (token == parsedToken || token->hasTableEntry()
			&& token->isCode(OpenParen_Code))
		{
			// token is not already in the output list
			setErrorToken(token);
		}
		else  // token is in the rpn output list
		{
			// make a copy of the token to save
			setErrorToken(new Token(*token));
		}
		m_errorMessage = token->message(status);
		cleanUp();
		return false;
	}
	// TODO check if stacks are empty
	return true;
}


// function to add a token to the output list, but token may be placed
// on hold stack pending adding it to the output list so that higher
// precedence tokens may be added to the list first
//
//     - Done status returned when last token is processed
//     - Good status returned when token successfully processed
//     - error status returned when an error is detected
//     - token argument may be changed when an error is detected

TokenStatus Translator::addToken(Token *&token)
{
	TokenStatus status;

	if (m_state == Initial_State)
	{
		// check for end of line at begin of input
		if (token->isType(Operator_TokenType) && token->isCode(EOL_Code))
		{
			delete token;  // delete EOL token
			return Done_TokenStatus;
		}

		// push null token to be last operator on stack
		// to prevent from popping past bottom of stack
		m_holdStack.resize(m_holdStack.size() + 1);
		m_holdStack.top().token = m_table.newToken(Null_Code);
		m_holdStack.top().first = NULL;

		m_state = OperandOrEnd_State;
	}

	// added check for command token
	if (token->isType(Command_TokenType))
	{
		if (m_mode == Command_TokenMode)
		{
			if (m_table.tokenMode(token->code()) != Null_TokenMode)
			{
				m_mode = m_table.tokenMode(token->code());
				m_cmdStack.resize(m_cmdStack.size() + 1);
				m_cmdStack.top().token = token;
				m_cmdStack.top().flag = None_CmdFlag;
				// initialize index to current last index of output
				m_cmdStack.top().index = m_output->size();
				return Good_TokenStatus;  // nothing more to do
			}
			else
			{
				return BUG_NotYetImplemented;
			}
		}
		// fall through so proper error is reported
	}

	// check for end statement
	if (m_state == EndStmt_State && !(m_table.flags(token) & EndStmt_Flag))
	{
		return ExpEndStmt_TokenStatus;
	}

	// check for both operand states
	if (m_state == Operand_State || m_state == OperandOrEnd_State)
	{
		if (!token->isOperator())
		{
			return processOperand(token);
		}
		// end-of-statement code acceptable instead of operand
		else if (m_table.flags(token) & EndExpr_Flag)
		{
			if (m_state != OperandOrEnd_State)
			{
				// an operand is expected, get the appropriate error
				return unexpectedEndError();
			}
			// fall thru to process end of expression operator token
		}
		else  // operator when expecting operand, must be a unary operator
		{
			if (processUnaryOperator(token, status) == false)
			{
				return status;
			}
			// fall thru to process unary operator token
		}
	}
	else  // a binary operator is expected
	{
		status = processBinaryOperator(token);
		if (status != Good_TokenStatus)
		{
			return status;
		}
	}

	// process all operators
	return processOperator(token);
}


// function to process and operand token
//
//   - the default data type of the operand is set
//   - if the token has a parentheses and is an internal function, then
//     - an error occurs if a print function is not in a print command
//     - only sub-string functions are permitted in command mode
//     - only sub-string functions are permitted in assignment modes
//   - function or array is pushed onto count stack
//   - number of expected operands put on count stack for internal functions 
//   - state is set to operand for function or array
//
//   - reference flag is set for non-parentheses tokens
//   - non-parentheses token is pushed onto done stack, appended to output
//   - state is set to binary operator

TokenStatus Translator::processOperand(Token *&token)
{
	// check for and add dummy token if necessary
	doPendingParen(m_holdStack.top().token);

	// set default data type for token if it has none
	token->setDataType();

	if (token->hasParen())
	{
		// token is an array or a function
		if (token->isType(IntFuncP_TokenType))
		{
			// detect invalid print-only function
			if ((m_table.flags(token->code()) & Print_Flag)
				&& (!m_cmdStack.isEmpty()
				&& !m_cmdStack.top().token->isCode(Print_Code)
				|| !m_holdStack.top().token->isNull()))
			{
				TokenStatus status;
				DataType dataType;
				if ((status = getExprDataType(dataType)) == Good_TokenStatus)
				{
					status = expectedErrStatus(dataType);
				}
				return status;
			}
			// check if not in expression mode
			switch (m_mode)
			{
			case Command_TokenMode:
			case Assignment_TokenMode:
				if (m_countStack.isEmpty())
				{
					if (m_table.dataType(token->code()) != SubStr_DataType)
					{
						// return appropriate error for mode
						return m_mode == Command_TokenMode
							? ExpCmd_TokenStatus : ExpAssignItem_TokenStatus;
					}

					// set reference of sub-string function
					token->setReference();
				}
				// check if first operand of sub-string
				else if (m_holdStack.top().token->reference()
					&& m_countStack.top().nOperands == 1)
				{
					return ExpStrVar_TokenStatus;
				}
				break;

			case AssignmentList_TokenMode:
				if (m_table.dataType(token->code()) != SubStr_DataType)
				{
					// in a comma separated list
					return variableErrStatus(m_cmdStack.top().token
						->dataType());
				}
				// set reference flag of sub-string function
				token->setReference();
				break;

			case Reference_TokenMode:
				if (m_countStack.isEmpty())
				{
					return ExpVar_TokenStatus;
				}
				break;
			}
		}
		// for reference mode, only no parentheses tokens allowed
		else if (m_countStack.isEmpty() && !token->isType(Paren_TokenType))
		{
			// return appropriate error for mode
			if (m_mode == Command_TokenMode || m_mode == Assignment_TokenMode)
			{
				if (token->isType(DefFuncP_TokenType))
				{
					// TODO these are allowed in the DEF command
					// just point to open parentheses on token
					token->addLengthToColumn();
					token->setLength(1);
				}
				return ExpEqualOrComma_TokenStatus;
			}
			else if (m_mode == Reference_TokenMode)
			{
				return ExpVar_TokenStatus;
			}
		}

		// 2010-06-08: changed count stack to hold count items
		m_countStack.resize(m_countStack.size() + 1);
		m_countStack.top().nOperands = 1;  // assume at least one
		if (token->isType(IntFuncP_TokenType))
		{
			m_countStack.top().nExpected = m_table.nOperands(token->code());
			m_countStack.top().code = token->code();
		}
		else  // !token->isType(IntFuncP_TokenType)
		{
			m_countStack.top().nExpected = 0;
		}

		m_holdStack.resize(m_holdStack.size() + 1);
		m_holdStack.top().token = token;
		m_holdStack.top().first = NULL;
		// leave state == Operand
		m_state = Operand_State;  // make sure not OperandOrEnd
	}
	else  // !token->hasParen()
	{
		// for reference mode, only no parentheses tokens allowed
		if (m_mode == Reference_TokenMode && m_countStack.isEmpty()
			&& !token->isType(NoParen_TokenType))
		{
			return ExpVar_TokenStatus;
		}

		// token is a variable or a function with no arguments
		// set reference flag for variable or function
		if (token->isType(NoParen_TokenType)
			|| token->isType(DefFuncN_TokenType))
		{
			token->setReference();
		}

		// add token directly output list
		// and push element pointer on done stack
		RpnItem *rpnItem = new RpnItem(token);
        m_output->append(rpnItem);
		m_doneStack.resize(m_doneStack.size() + 1);
		m_doneStack.top().rpnItem = rpnItem;
		m_doneStack.top().first = m_doneStack.top().last = NULL;
		// in reference mode, if have variable, set end expression state
		// otherwise next token must be a binary operator
		m_state = m_mode == Reference_TokenMode && m_countStack.isEmpty()
			? EndExpr_State : BinOp_State;
	}

	// if command mode then change to assignment mode
	if (m_mode == Command_TokenMode)
	{
		m_mode = Assignment_TokenMode;
	}

	return Good_TokenStatus;
}



bool Translator::processUnaryOperator(Token *&token, TokenStatus &status)
{
	// check if count stack is empty before checking mode
	if (m_countStack.isEmpty())
	{
        switch (m_mode)
        {
        case Command_TokenMode:
			// if command mode, then this is not the way it starts
            status = ExpCmd_TokenStatus;
            return false;

		case Assignment_TokenMode:
			status = ExpAssignItem_TokenStatus;
			return false;

		case AssignmentList_TokenMode:
			status = variableErrStatus(m_cmdStack.top().token->dataType());
			return false;

        case Reference_TokenMode:
			status = ExpVar_TokenStatus;
			return false;
		}
	}
	else if (m_holdStack.top().token->isDataType(SubStr_DataType)
		&& m_holdStack.top().token->reference()
		&& m_countStack.top().nOperands == 1)
	{
		status = ExpStrVar_TokenStatus;
		return false;
	}

	Code unary_code = m_table.unaryCode(token->code());
	if (unary_code == Null_Code)
	{
		DataType dataType;

		// oops, not a valid unary operator
		if ((status = getExprDataType(dataType)) == Good_TokenStatus)
		{
			status = expectedErrStatus(dataType);
		}
		return false;
	}
	// change token to unary operator
	token->setCode(unary_code);
	if (unary_code == OpenParen_Code)
	{
		// check for and add dummy token if necessary
		doPendingParen(m_holdStack.top().token);

		// assign current expression data type to paren token
		DataType dataType = token->dataType();
		if ((status = getExprDataType(dataType)) != Good_TokenStatus)
		{
			return false;
		}
		token->setDataType(dataType);
		// push open parentheses right on stack and return
		m_holdStack.resize(m_holdStack.size() + 1);
		m_holdStack.top().token = token;
		m_holdStack.top().first = NULL;
		m_state = Operand_State;

		// add a null counter to prevent commas
		m_countStack.resize(m_countStack.size() + 1);
		m_countStack.top().nOperands = 0;
		m_countStack.top().nExpected = 0;
		status = Good_TokenStatus;
		return false;
	}
	status = Good_TokenStatus;
	return true;
}


// function to process a binary operator token
//
//   - error occurs if token is not an operator
//   - error occurs if token is a unary operator
//   - error occurs comma expected after variable in sub-string assignment
//   - if closing parentheses then process any pending parentheses
//     and set last precedence to higher value
//   - error occurs if end statement operator in uncompleted multiple
//     assignment statement

TokenStatus Translator::processBinaryOperator(Token *&token)
{
	TokenStatus status = Good_TokenStatus;

	// check if after first operand of sub-string assignment
	// make sure token has table entry before checking code
	if (m_holdStack.top().token->reference()
		&& m_countStack.top().nOperands == 1
		&& (!token->hasTableEntry() || !token->isCode(Comma_Code)))
	{
		// only a comma is allowed here
		return ExpComma_TokenStatus;
	}
	// end of expression token expected check
	if (m_state == EndExpr_State && !(m_table.flags(token) & EndExpr_Flag))
	{
		// TODO currently only occurs after print function
		// TODO add correct error based on current command
		// TODO call command handler to get appropriate error here
		return endExpressionError();
	}

	if (!token->isOperator())
	{
		// state == BinOp_State, but token is not an operator
		return operatorError();
	}
	else if (m_table.isUnaryOperator(token->code()))
	{
		status = ExpBinOpOrEnd_TokenStatus;
	}
	// initialize last precedence before emptying stack for ')'
	else if (token->isCode(CloseParen_Code))
	{
		// check for and add dummy token if necessary before emptying stack
		doPendingParen(m_holdStack.top().token);

		// now set last precedence to highest in case no operators in ( )
		m_lastPrecedence = HighestPrecedence;
	}
	// checking at end of statement
	else if (m_table.flags(token) & EndStmt_Flag
		&& m_mode == AssignmentList_TokenMode)
	{
		// no equal token received yet
		status = ExpEqualOrComma_TokenStatus;
	}
	return status;
}


// function to process an operator token received, all operators with higher
// precedence on the hold stack are added to output list first
//
// for each operator added to the output list, any pending parentheses is
// processed first, the final operand of the operator is processed
//
//    - if error occurs on last operand, the operator token passed in is
//      deleted and the token with the error is returned
//    - sets last_precedence for operator being added to output list
//    - after higher precedence operators are processed from the hold stack,
//      the token handler for the operator is called

TokenStatus Translator::processOperator(Token *&token)
{
	// unary operators don't force other tokens from hold stack
	while (m_table.precedence(m_holdStack.top().token)
		>= m_table.precedence(token->code())
		&& !m_table.isUnaryOperator(token->code()))
	{
		// pop operator on top of stack and add it to the output
		Token *topToken = m_holdStack.top().token;
		// (don't pop token here in case error occurs)

		doPendingParen(topToken);

		// change token operator code or insert conversion codes as needed
		TokenStatus status = processFinalOperand(topToken,
			m_holdStack.top().first,
			m_table.isUnaryOperator(topToken->code()) ? 0 : 1);

		if (status != Good_TokenStatus)
		{
			delete token;
			token = topToken;  // return token with error
			return status;
		}

		// save precedence of operator being added
		// (doesn't matter if not currently within parentheses,
		// it will be reset upon next open parentheses)
		m_lastPrecedence = m_table.precedence(topToken->code());

		m_holdStack.resize(m_holdStack.size() - 1);
	}

	// check for special token processing
	TokenHandler tokenHandler = m_table.tokenHandler(token->code());
	if (tokenHandler == NULL)  // no special handler?
	{
		// check for command token with no token handler
		if (token->isType(Command_TokenType))
		{
			// valid commands in BinOp state must have a token handler
			return operatorError();
		}

		// use default operator token handler
		tokenHandler = Operator_Handler;
	}
	return (*tokenHandler)(*this, token);
}


// function to process the first operand for an operator
//
//  - called from the Operator_Handler or Equal_Handler (for equal operator)
//  - attaches first operand token from operand on top of done stack
//  - no first operand token for unary operators

TokenStatus Translator::processFirstOperand(Token *&token)
{
	Token *first = NULL;		// first operand token

	// check first operand of binary operators
	if (!m_table.isUnaryOperator(token->code()))
	{
		// changed token operator code or insert conversion codes as needed
		Token *orgToken = token;
		TokenStatus status = findCode(token, 0, &first);
		if (status != Good_TokenStatus)
		{
			// check if different token has error
			if (token != orgToken)
			{
				delete orgToken;  // delete operator token
			}
			return status;
		}
	}

	// push it onto the holding stack
	m_holdStack.resize(m_holdStack.size() + 1);
	m_holdStack.top().token = token;
	m_holdStack.top().first = first;  // attach first operand

	// expecting another operand next
	m_state = Translator::Operand_State;

	return Good_TokenStatus;
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
//   - from add_operator, token2 if first operand attached on hold stack
//   - from add_print_code, token2 is NULL
//   - from CloseParen_Handler, token2 is CloseParen token
//   - from Assign_CmdHandler, token2 will be NULL (not used)

TokenStatus Translator::processFinalOperand(Token *&token, Token *token2,
	int operandIndex, int nOperands)
{
	bool donePush = true;	// for print-only functions
	Token *first;			// first operand token
	Token *last;			// last operand token

	if (nOperands == 0)  // internal function or operator
	{
		TokenStatus status = findCode(token, operandIndex, &first, &last);
		if (status != Good_TokenStatus)
		{
			return status;
		}

		// check for non-assignment operator
		if ((m_table.flags(token->code()) & Reference_Flag))
		{
			// for assignment operators, nothing gets pushed to the done stack
			donePush = false;

			// get number of strings for this assignment operator
			nOperands = m_table.nStrings(token->code());

			// no longer need the first and last operands
			deleteOpenParen(first);
			deleteCloseParen(last);
		}
		// save string operands only
		// get number of strings for non-sub-string codes
		// include sub-string reference, which are put on done stack
		else if ((!token->isDataType(SubStr_DataType) || token->reference()))
		{
			// no operands for sub-string references
			nOperands = token->reference()
				? 0 : m_table.nStrings(token->code());

			// set first and last operands
			deleteOpenParen(first);
			if (token->isOperator())
			{
				// if unary operator, then operator, else first from hold stack
				// (set first token to unary op)
				first = operandIndex == 0 ? token : token2;
				// last operand from token that was on done stack
			}
			else  // non-sub-string internal function
			{
				first = NULL;   // token itself is first operand
				deleteCloseParen(last);
				last = token2;  // last operand is CloseParen token
			}
		}
		// don't push non-reference sub-string function
		else
		{
			donePush = false;  // don't push sub-string function on done stack
			// set first/last operands of operand on done stack
			deleteOpenParen(m_doneStack.top().first);
			m_doneStack.top().first = token;  // set to sub-str function token
			deleteCloseParen(m_doneStack.top().last);
			m_doneStack.top().last = token2;  // set to CloseParen token
		}

		// process print-only internal functions
		// (check for function with no return value)
		if (token->isDataType(None_DataType))
		{
			if (m_table.flags(token->code()) & Print_Flag)
			{
				// tell PRINT to stay on same line
				m_cmdStack.top().flag = PrintStay_CmdFlag | PrintFunc_CmdFlag;
			}
			donePush = false;  // don't push on done stack
			// delete closing parentheses if print function
			// (token2 could be null, e.g. print code)
			if (token2 != NULL)
			{
				// if print function, make sure expression ends
				if (m_table.flags(token->code()) & Print_Flag)
				{
					m_state = EndExpr_State;
				}
				delete token2;
			}
		}
	}
	else  // array or user function
	{
		first = NULL;   // token itself is first operand
		last = token2;  // token's CloseParen is last
		// check for end of array in reference mode
		if (m_mode == Reference_TokenMode && m_countStack.isEmpty())
		{
			// have array element, set end expression state
			m_state = EndExpr_State;
		}
	}

	RpnItem **operand;
	if (nOperands == 0)  // no string operands to save?
	{
		operand = NULL;
	}
	else  // pop string operands off of stack into new array
	{
		operand = new RpnItem *[nOperands];
		// save operands for storage in output list
		for (int i = nOperands; --i >= 0;)
		{
			if (m_doneStack.isEmpty())
			{
				return BUG_DoneStackEmptyOperands;
			}
			// TODO need to keep first/last operands for array/function args
			deleteOpenParen(m_doneStack.top().first);
			// check if operand's last token was CloseParen
			deleteCloseParen(m_doneStack.top().last);
			operand[i] = m_doneStack.pop().rpnItem;
		}
	}

	// add token to output list and push element pointer on done stack
	RpnItem *rpnItem = new RpnItem(token, nOperands, operand);
	m_output->append(rpnItem);
	// check if output item is to be pushed on done stack
	if (donePush)
	{
		// push index of rpm item about to be added to output list
		m_doneStack.resize(m_doneStack.size() + 1);
		m_doneStack.top().rpnItem = rpnItem;
		m_doneStack.top().first = first;
		m_doneStack.top().last = last;
	}

	return Good_TokenStatus;
}


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

TokenStatus Translator::expressionEnd(void)
{
	TokenStatus status;
	Token *token;

	// do end of statement processing
	if (m_holdStack.isEmpty())
	{
		// oops, stack is empty
		return BUG_HoldStackEmpty;
	}

	if ((status = parenStatus()) != Good_TokenStatus)
	{
		return status;
	}
	else
	{
		token = m_holdStack.top().token;
		if (!token->hasTableEntry())
		{
			return BUG_Debug2;
		}
		if (!token->isCode(Null_Code))
		{
			// check if there is some unexpected token on hold stack
			// could be assignment on hold stack
			switch (m_mode)
			{
			// note: Command_TokenMode can't happen here
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

	// check if there is a pending closing parentheses
	// pass Null_Code token (will always add dummy)
	doPendingParen(token);
	return Good_TokenStatus;
}


// function to call command handler of command on top of command stack
//
//   - if command stack empty and expression only mode, return null
//     status indicating token was not supported by command (caller will
//     report appropriate error)
//   - otherwise if command stack, then bug
//   - if command stack top does not have a command handler, then bug
//   - if command handler returns an error, then token passed in is
//      deletedunless it is the token reported as the error
//   - for errors, the token returned by the command handler is returned

TokenStatus Translator::callCommandHandler(Token *&token)
{
	if (m_cmdStack.isEmpty())
	{
		// token was not expected
		// let caller report the appropriate error
		return Null_TokenStatus;
	}
	CmdItem cmdItem = m_cmdStack.top();
	CommandHandler cmdHandler = m_table.commandHandler(cmdItem.token->code());
	if (cmdHandler == NULL)  // missing command handler?
	{
		return BUG_NotYetImplemented;
	}
	TokenStatus status = (*cmdHandler)(*this, &cmdItem, token);
	if (status != Good_TokenStatus)
	{
		// delete token if error at another token
		if (cmdItem.token != token)
		{
			delete token;
		}
		token = cmdItem.token;  // command decided where error is
	}
	return status;
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

TokenStatus Translator::findCode(Token *&token, int operandIndex, Token **first,
	Token **last)
{
	if (m_doneStack.isEmpty())
	{
		// oops, there should have been operands on done stack
		return BUG_DoneStackEmptyFindCode;
	}
	Token *topToken = m_doneStack.top().rpnItem->token();
	// get first and last operands for top token
	Token *workFirst = m_doneStack.top().first;
	if (workFirst == NULL)
	{
		workFirst = topToken;  // first operand not set, set to operand token
	}
	if (first != NULL)  // requested by caller?
	{
		*first = workFirst;
		m_doneStack.top().first = NULL;  // prevent deletion below
	}
	Token *workLast = m_doneStack.top().last;
	if (workLast == NULL)
	{
		workLast = topToken;  // last operand not set, set to operand token
	}
	if (last != NULL)  // requested by caller?
	{
		*last = workLast;
		m_doneStack.top().last = NULL;  // prevent deletion below
	}

	// check if reference is required for first operand
	if (operandIndex == 0 && token->reference())
	{
		if (!topToken->reference())
		{
			// need a reference, so return error

			// only delete token if it's not an internal function
			if (!token->isType(IntFuncP_TokenType))
			{
				// (internal function is on hold stack, will be deleted later)
				delete token;  // delete the token
			}
			// return non-reference operand
			// (report entire expression)
			token = workFirst->setThrough(workLast);

			// delete last token if close paren
			deleteCloseParen(workLast);

			// XXX check command on top of command stack XXX
			return ExpAssignItem_TokenStatus;
		}
	}
	else
	{
		// reset reference flag of operand
		topToken->setReference(false);
	}

	// see if main code's data type matches
	DataType dataType = topToken->dataType();
	DataType operandDataType = m_table.operandDataType(token->code(),
		operandIndex);
	if (dataType == operandDataType)  // exact match?
	{
		// pop all references (for assignments) from stack
		if (operandDataType != String_DataType || token->reference())
		{
			// pop non-string from done stack
			deleteOpenParen(m_doneStack.top().first);
			// check if operand's last token was CloseParen
			deleteCloseParen(m_doneStack.pop().last);
		}
		return Good_TokenStatus;
	}
	Code cvt_code = cvtCodeHaveNeed[dataType][operandDataType];
	Code new_code = cvt_code == Invalid_Code ? Null_Code : token->code();

	// see if any associated code's data types match
	int start = operandIndex != 1 ? 0 : m_table.assoc2Index(token->code());
	int end = m_table.nAssocCodes(token->code());
	if (operandIndex == 0 && m_table.assoc2Index(token->code()) != 0)
	{
		// for first operand, end at begin of second group of associated codes
		end = m_table.assoc2Index(token->code());
	}
	for (int i = start; i < end; i++)
	{
		Code assoc_code = m_table.assocCode(token->code(), i);
		DataType operandDatatype2 = m_table.operandDataType(assoc_code,
			operandIndex);
		if (dataType == operandDatatype2)  // exact match?
		{
			// change token's code and data type to associated code
			m_table.setToken(token, assoc_code);

			// pop all references (for assignments) from stack
			if (operandDatatype2 != String_DataType || token->reference())
			{
				// pop non-string from done stack
				deleteOpenParen(m_doneStack.top().first);
				// check if operand's last token was CloseParen
				deleteCloseParen(m_doneStack.pop().last);
			}
			return Good_TokenStatus;
		}
		Code cvt_code2 = cvtCodeHaveNeed[dataType][operandDatatype2];
		if (cvt_code2 != Invalid_Code && new_code == Null_Code)
		{
			new_code = assoc_code;
			cvt_code = cvt_code2;
		}
	}

	if (new_code != Null_Code)  // found a convertible code?
	{
		if (!token->isCode(new_code))  // not the main code?
		{
			// change token's code and data type to associated code
			m_table.setToken(token, new_code);
			token->setDataType(m_table.dataType(token->code()));
		}

		// is there an actual conversion code to insert?
		if (cvt_code != Null_Code)
		{
			// a conversion code implies a non-string on done stack
			// pop non-string from done stack
			deleteOpenParen(m_doneStack.top().first);
			// check if operand's last token was CloseParen
			deleteCloseParen(m_doneStack.pop().last);

			// INSERT CONVERSION CODE
			// create convert token with convert code
			// append token to end of output list (after operand)
			m_output->append(new RpnItem(m_table.newToken(cvt_code)));
		}

		return Good_TokenStatus;
	}

	// no match found, report error
	// change token to token with invalid data type and return error
	// use main code's expected data type for operand
	// (return expected variable error for references and sub-strings)
	TokenStatus status;
	if (!token->reference())
	{
		status = expectedErrStatus(operandDataType);
		// sub-string no longer needed with first/last operands
	}
	else if (token->isDataType(SubStr_DataType))
	{
		status = ExpStrVar_TokenStatus;
	}
	else
	{
		status = variableErrStatus(operandDataType);
	}
	// report entire expression
	token = workFirst->setThrough(workLast);

	// delete last token if close paren
	deleteCloseParen(workLast);

	return status;
}


// function to get current expression data type
//
//   - sets data type from current expression
//   - return Good_TokenStatus is successful, error otherwise
//   - if operator on hold stack, then get it's operand's data type
//   - if Null (empty hold stack), then get command's expression type
//   - if OpenParen, then get it's data type (which could be None)
//   - if internal function, then get it's current operand's data type
//   - if array or non-internal function, then data type is None

TokenStatus Translator::getExprDataType(DataType &dataType) const
{
	TokenStatus status = Good_TokenStatus;
	if (m_holdStack.isEmpty())  // at least NULL token should be on hold stack
	{
		status = BUG_HoldStackEmpty;
	}
	else if (m_holdStack.top().token->isOperator())
	{
		Code code = m_holdStack.top().token->code();
		if (code == Null_Code)
		{
			// nothing on hold stack, get expected type for command
			if (m_cmdStack.isEmpty())
			{
				if (!m_exprMode)
				{
					status = BUG_CmdStackEmptyExpr;  // this shouldn't happen
				}
			}
			else
			{
				dataType = m_cmdStack.top().token->dataType();
			}
		}
		else if (code == OpenParen_Code)
		{
			// no operator, get data type of open parentheses (could be none)
			dataType = m_holdStack.top().token->dataType();
		}
		else  // an regular operator on top of hold stack
		{
			// data type from operand of operator on top of hold stack
			// (first operand for unary and second operand for binary operator)
			dataType = m_table.operandDataType(code,
				m_table.isUnaryOperator(code) ? 0 : 1);
		}
	}
	else if (m_countStack.isEmpty())  // no parentheses, array or function?
	{
		// this situation should have been handled above
		status = BUG_CountStackEmpty;  // this shouldn't happen
	}
	else if (m_countStack.top().nExpected > 0)  // internal function?
	{
		// data type from current operator of internal function
		dataType = m_table.operandDataType(m_countStack.top().code,
			m_countStack.top().nOperands - 1);
	}
	else if (m_countStack.top().nOperands == 0)  // in parentheses?
	{
		// this situation should have been handled above
		status = BUG_UnexpParenExpr;
	}
	else  // in array or non-internal function
	{
		// (cannot determine type of expression)
		dataType = None_DataType;
	}
	return status;
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

TokenStatus Translator::parenStatus(void) const
{
	if (m_countStack.isEmpty())
	{
		// no parentheses
		return Good_TokenStatus;
	}
	// parentheses, array or function without closing parentheses

	if (m_countStack.top().nOperands == 0)
	{
		// open parentheses
		return ExpOpOrParen_TokenStatus;
	}

	if (m_countStack.top().nExpected == 0)
	{
		// array, defined function or user function
		// (more subscripts/arguments possible)
		return ExpOpCommaOrParen_TokenStatus;
	}

	// internal function
	int index;
	Token *topToken = m_holdStack.top().token;
	if (m_countStack.top().nOperands == m_countStack.top().nExpected)
	{
		// check if there could be more arguments
		if ((m_table.flags(topToken->code()) & Multiple_Flag) == 0)
		{
			// internal function - at last argument (no more expected)
			return ExpOpOrParen_TokenStatus;
		}
		else  // multiple flag set, could have more arguments
		{
			return ExpOpCommaOrParen_TokenStatus;
		}
	}

	// internal function - more arguments expected
	// (number of arguments doesn't match function's entry)
	if ((m_table.flags(topToken->code()) & Multiple_Flag) != 0
		&& (index = m_table.search(topToken->code(),
		m_countStack.top().nOperands)) > 0)
	{
		// found alternate code matching current number of operands
		// (could be at last argument, could be more arguments)
		return ExpOpCommaOrParen_TokenStatus;
	}
	// note: Command_TokenMode can't happen here
	else if (m_mode == AssignmentList_TokenMode)
	{
		// sub-string function
		return ExpComma_TokenStatus;
	}
	else  // more arguments are expected
	{
		return ExpOpOrComma_TokenStatus;
	}
}


// function to check if there is a pending parentheses token and if there is,
// check to see if it should be added to the output as a dummy token so that
// the Recreator can added the unnecessary, but entered by the user, set of
// parentheses
//
//   - index argument is table index of operator to check against

void Translator::doPendingParen(Token *token)
{
	if (m_pendingParen != NULL)  // is a closing parentheses token pending?
	{
		// may need to add a dummy token if the precedence of the last
		// operator added within the last parentheses sub-expression
		// is higher than or same as (operand state only) the operator
		int precedence = m_table.precedence(token);
		if (m_lastPrecedence > precedence
			|| m_state == Operand_State && m_lastPrecedence == precedence)
		{
			if (m_output->last()->token()->isSubCode(Paren_SubCode))
			{
				// already has parentheses sub-code set,
				// so add dummy token
				m_output->append(new RpnItem(m_pendingParen));
				// reset pending token and return
				m_pendingParen = NULL;
				return;
			}
			else
			{
				RpnList::iterator last = m_output->end() - 1;
				if (m_table.flags((*last)->token()) & Hidden_Flag)
				{
					// last token added is a hidden code
					last--;  // so get token before hidden code
				}
				(*last)->token()->setSubCodeMask(Paren_SubCode);
			}
			// TODO something needed on done stack?
			// TODO (reference flag already cleared by close parentheses)
		}
		// check if being used as last operand before deleting
		if (m_pendingParen->isSubCode(Last_SubCode))
		{
			// still used as last operand token, just clear used flag
			m_pendingParen->clearSubCodeMask(Used_SubCode);
		}
		else  // don't need pending token
		{
			delete m_pendingParen;  // release it's memory
		}
		m_pendingParen = NULL;  // reset pending token
	}
}


// function to delete a opening parentheses token that is no longer being used
// as the first operand token of another token
//
//   - the first operand token pointer is passed
//   - no action if first operand token pointer is not set
//   - no action if first operand token does not have a table entry
//   - no action if first operand token is not a closing parentheses token

void Translator::deleteOpenParen(Token *token)
{
	if (token != NULL && token->hasTableEntry()
		&& token->isCode(OpenParen_Code))
	{
		delete token;  // delete CloseParen token of operand
	}
}


// function to delete a closing parentheses token that is no longer being used
// as the last operand token of another token
//
//   - the last operand token pointer is passed
//   - no action if last operand token pointer is not set
//   - no action if last operand token does not have a table entry
//   - no action if last operand token is not a closing parentheses token
//   - if closing parentheses is being used by pending parentheses or in output
//     as a dummy parentheses token, then last operand flag is cleared
//   - if closing parentheses token is not being used, then it is deleted

void Translator::deleteCloseParen(Token *token)
{
	if (token != NULL && token->hasTableEntry()
		&& token->isCode(CloseParen_Code))
	{
		// check if parentheses token is still being used
		if (token->isSubCode(Used_SubCode))
		{
			// no longer used as last token
			token->clearSubCodeMask(Last_SubCode);
		}
		else  // not used, close parentheses token can be deleted
		{
			delete token;  // delete CloseParen token of operand
		}
	}
}


// function to clean up the Translator variables after an error is detected
//
//   - must be called after add_token() returns an error

void Translator::cleanUp(void)
{
	// clean up from error
	while (!m_holdStack.isEmpty())
	{
		// delete first token in command stack item
		deleteOpenParen(m_holdStack.top().first);
		// delete to free the token that was on the stack
		delete m_holdStack.pop().token;
	}
	while (!m_doneStack.isEmpty())
	{
		deleteOpenParen(m_doneStack.top().first);
		// check if operand's last token was CloseParen
		deleteCloseParen(m_doneStack.pop().last);
	}
	// comma support - need to empty count_stack
	while (!m_countStack.isEmpty())
	{
		m_countStack.resize(m_countStack.size() - 1);
	}

	// clear the RPN output list of all items
	delete m_output;
	m_output = NULL;

	// need to delete pending parentheses
	if (m_pendingParen != NULL)
	{
		delete m_pendingParen;
		m_pendingParen = NULL;
	}
	while (!m_cmdStack.isEmpty())
	{
		// delete token in command stack item
		delete m_cmdStack.pop().token;
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

TokenStatus Translator::addPrintCode(void)
{
	if (!m_doneStack.isEmpty())
	{
		// create token for data type specific print token
		Token *token = m_table.newToken(PrintDbl_Code);
		return processFinalOperand(token, NULL, 0);
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

TokenStatus Translator::setAssignCommand(Token *&token, Code assign_code)
{
	// start of assign list statement
	m_table.setToken(token, assign_code);
	// set reference flag of assignment
	token->setReference();

	// find appropriate assign or assign list code
	TokenStatus status = findCode(token, 0);
	if (status != Good_TokenStatus)
	{
		return status;
	}

	if (m_cmdStack.isEmpty())  // still command mode?
	{
		m_cmdStack.resize(m_cmdStack.size() + 1);  // push new command on stack
	}
	else  // was preceded by let keyword
	{
		token->setSubCodeMask(Let_SubCode);  // set sub-code to indicate let
		delete m_cmdStack.top().token;   // delete the let token
	}
	m_cmdStack.top().token = token;
	m_cmdStack.top().flag = None_CmdFlag;
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

TokenStatus Translator::checkAssignListToken(Token *&token)
{
	if (!m_doneStack.isEmpty())
	{
		// delete close paren (array or sub-string) on operand
		deleteCloseParen(m_doneStack.top().last);
		token = m_doneStack.pop().rpnItem->token();
		if (!token->reference()
			|| equivalentDataType(m_cmdStack.top().token->dataType())
			!= equivalentDataType(token->dataType()))
		{
			// point to just open parentheses of DefFuncP tokens
			if (token->isType(DefFuncP_TokenType))
			{
				// just point to open parentheses on token
				token->addLengthToColumn();
				token->setLength(1);
				return ExpEqualOrComma_TokenStatus;
			}
			return variableErrStatus(m_cmdStack.top().token->dataType());
		}
		if (equivalentDataType(token->dataType()) == String_DataType
			&& !token->isDataType(m_cmdStack.top().token->dataType()))
		{
			m_cmdStack.top().token->setCode(AssignListMix_Code);
		}
	}
	return Good_TokenStatus;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                           DETERMINE ERROR FUNCTIONS                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


// function to return equivalent data type for data type
// (really to convert the various string data types to String_DataType)
DataType Translator::equivalentDataType(DataType dataType)
{
	static DataType equivalent[numberof_DataType] = {
		Double_DataType,	// Double
		Integer_DataType,	// Integer
		String_DataType,	// String
		String_DataType,	// TmpStr
		String_DataType		// SubStr
	};

	return equivalent[dataType];
}


// function to return the token error status for an expected data type
TokenStatus Translator::expectedErrStatus(DataType dataType)
{
	static TokenStatus tokenStatus[sizeof_DataType] = {
		ExpNumExpr_TokenStatus,		// Double
		ExpNumExpr_TokenStatus,		// Integer
		ExpStrExpr_TokenStatus,		// String
		ExpStrExpr_TokenStatus,		// TmpStr
		ExpStrExpr_TokenStatus,		// SubStr
		BUG_InvalidDataType,		// numberof
		ExpExpr_TokenStatus			// None
	};

	return tokenStatus[dataType];
}


// function to return the token error status for an actual data type
TokenStatus Translator::actualErrStatus(DataType dataType)
{
	static TokenStatus tokenStatus[sizeof_DataType] = {
		ExpStrExpr_TokenStatus,		// Double
		ExpStrExpr_TokenStatus,		// Integer
		ExpNumExpr_TokenStatus,		// String
		ExpNumExpr_TokenStatus,		// TmpStr
		ExpNumExpr_TokenStatus,		// SubStr
		BUG_InvalidDataType,		// numberof
		BUG_InvalidDataType			// None
	};

	return tokenStatus[dataType];
}


// function to return the token error status for a variable data type
TokenStatus Translator::variableErrStatus(DataType dataType)
{
	static TokenStatus tokenStatus[sizeof_DataType] = {
		ExpDblVar_TokenStatus,		// Double
		ExpIntVar_TokenStatus,		// Integer
		ExpStrItem_TokenStatus,		// String
		BUG_InvalidDataType,		// TmpStr
		BUG_InvalidDataType,		// SubStr
		BUG_InvalidDataType,		// numberof
		ExpAssignItem_TokenStatus	// None
	};

	return tokenStatus[dataType];
}


// function to return error when an operator is expected, first paren_status()
// is called to get appropriate error when inside parentheses, internal function
// of array/user function, and if not inside parentheses, then error is
// dependent on current token mode

TokenStatus Translator::operatorError(void) const
{
	TokenStatus status;

	if ((status = parenStatus()) == Good_TokenStatus)
	{
		// error is dependent on mode
		switch (m_mode)
		{
		case Command_TokenMode:
		case Assignment_TokenMode:
		case AssignmentList_TokenMode:
			status = ExpEqualOrComma_TokenStatus;
			break;

		case Expression_TokenMode:
			// FIXME better solution needed once more commands are implemented
			if (m_cmdStack.top().token->isCode(InputPrompt_Code))
			{
				status = ExpSemiCommaOrEnd_TokenStatus;
			}
			else
			{
				status = ExpOpOrEnd_TokenStatus;
			}
			break;

		default:
			status = BUG_InvalidMode;
			break;
		}
	}
	return status;
}


// function to get the proper status when an error occurs in an assignment
//
//   - should only be called when m_mode is Assignment or AssignmentList

TokenStatus Translator::assignmentError(void) const
{
	if (m_countStack.isEmpty())
	{
		// not in a sub-string function of array assignment
		return variableErrStatus(m_cmdStack.top().token->dataType());
	}
	else if (m_countStack.top().nExpected == 0)
	{
		// in array assignement at substript
		return ExpNumExpr_TokenStatus;
	}
	else if (m_countStack.top().nOperands == 1)
	{
		// in sub-string function assignment at first argument
		return ExpStrVar_TokenStatus;
	}
	else  // in sub-string function assignment not at first argument
	{
		return expectedErrStatus(
			m_table.operandDataType(m_countStack.top().code,
			m_countStack.top().nOperands - 1));
	}
}


// function to get the error for when an non-end token occurs when the
// in expression end state
//
//   - current command on command stack determines error

TokenStatus Translator::endExpressionError(void) const
{
	// TODO currently only occurs after print function and input command
	// TODO add correct error based on current command
	// TODO call command handler to get appropriate error here
	return ExpSemiCommaOrEnd_TokenStatus;
}


// function to get the error for a premature end to an expression when
// another operand is expected
//
//   - for command and assignments modes, count stack determines error
//   - for expression mode, current expression type determines error

TokenStatus Translator::unexpectedEndError(void) const
{
	TokenStatus status = Good_TokenStatus;
	DataType dataType;

	// unexpected of end expression - determine error to return
	switch (m_mode)
	{
	case Command_TokenMode:
	case Assignment_TokenMode:
	case AssignmentList_TokenMode:
		status = assignmentError();
		break;

	case Expression_TokenMode:
		if ((status = getExprDataType(dataType)) == Good_TokenStatus)
		{
			status = expectedErrStatus(dataType);
		}
		break;

	case Reference_TokenMode:
		status = ExpVar_TokenStatus;
		break;

	default:
		status = BUG_InvalidMode;
		break;
	}
	return status;
}


// end: translator.cpp
