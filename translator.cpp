// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: translator.cpp - translator class source file
//	Copyright (C) 2010-2013  Thunder422
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


Translator::Translator(const std::string &input) :
	m_table(Table::instance()),
	m_parse {new Parser {input}}
{

}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                               MAIN FUNCTION                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


// function to parse and translate an input line to an RPN output list
//
//   - allows for a special expression mode for testing
//   - returns the RPN output list upon successful translation
//   - throws token error upon detection of error

RpnList Translator::operator()(TestMode testMode)
{
	m_holdStack.emplace(std::make_shared<Token>(Null_Code));

	if (testMode == TestMode::Expression)
	{
		try
		{
			getExpression(DataType::Any);
		}
		catch (TokenError &error)
		{
			if (error(Status::UnknownToken))
			{
				error = Status::ExpOpOrEnd;
			}
			throw;
		}
		// pop final result off of done stack
		if (m_doneStack.empty())
		{
			throw TokenError {Status::BUG_DoneStackEmpty, m_token};
		}
		// drop result
		m_doneStack.pop();
	}
	else
	{
		getCommands();
	}

	if (!m_token->isCode(EOL_Code))
	{
		throw TokenError {Status::ExpOpOrEnd, m_token};
	}
	else
	{
		// pop and delete null token from top of stack
		m_holdStack.pop();

		if (!m_holdStack.empty())
		{
			throw TokenError {Status::BUG_HoldStackNotEmpty, m_token};
		}

		if (!m_doneStack.empty())
		{
			throw TokenError {Status::BUG_DoneStackNotEmpty, m_token};
		}

		if (testMode == TestMode::No
			&& !m_output.setCodeSize(m_table, m_token))
		{
			throw TokenError {Status::BUG_NotYetImplemented, m_token};
		}
	}
	return std::move(m_output);
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                               GET FUNCTIONS                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


// function to get colon separated statements
//
//   - stops at rem, end-of-line or unknown token
//   - if end is rem (command or operator), end-of-line is returned
//   - current token is terminating token (end-of-line or unknown token)
//   - caller determines validity of unknown token if returned
//   - throws token error upon detection of error

void Translator::getCommands()
{
	for (;;)
	{
		// get any reference token and ignore status
		// if not a command token then let translate will handle token
		try
		{
			getToken(Status{}, DataType::Any, Reference::All);
		}
		catch (TokenError &error)
		{
			error = Status::ExpCmdOrAssignItem;
			throw;
		}

		if (m_token->isCode(EOL_Code) && m_output.empty())
		{
			return;  // blank line allowed
		}

		if (m_token->isCode(Rem_Code) || m_token->isCode(RemOp_Code))
		{
			break;
		}

		// process command (assume assignment statement if not command token)
		if (TranslateFunction translate =
			m_table.translateFunction(m_token->isType(Type::Command)
			? m_token->code() : Let_Code))
		{
			(*translate)(*this);
		}
		else
		{
			throw TokenError {Status::BUG_NotYetImplemented, m_token};
		}

		if (m_token->isCode(RemOp_Code))
		{
			break;
		}
		else if (m_token->isCode(Colon_Code))
		{
			// set colon sub-code on last token
			outputLastToken()->addSubCode(Colon_SubCode);
			m_token.reset();
		}
		else  // unknown end statement token, return to caller
		{
			return;
		}
	}
	m_output.append(std::move(m_token));  // Rem or RemOp token
	getToken(Status{});
}


// function to get an expression from the input line
//
//   - takes a data type argument for the desired data type of the expression
//   - current token is token that terminated the expression
//   - throws an error if an error was detected

void Translator::getExpression(DataType dataType, int level)
{
	DataType expectedDataType {dataType};

	for (;;)
	{
		getToken(Status{}, expectedDataType);

		if (m_token->isCode(OpenParen_Code))
		{
			// push open parentheses onto hold stack to block waiting tokens
			// during the processing of the expression inside the parentheses
			m_holdStack.emplace(std::move(m_token));

			// get an expression and terminating token
			if (expectedDataType == DataType::None)
			{
				expectedDataType = DataType::Any;
			}
			try
			{
				getExpression(expectedDataType, level + 1);
			}
			catch (TokenError &error)
			{
				if (error(Status::ExpBinOpOrEnd))  // (aka unary operator?)
				{
					error = Status::ExpBinOpOrParen;
				}
				else if (error(Status::UnknownToken))
				{
					error = Status::ExpOpOrParen;
				}
				throw;  // exit on error
			}

			// check terminating token
			if (!m_token->isCode(CloseParen_Code))
			{
				throw TokenError {Status::ExpOpOrParen, m_token};
			}

			// make sure holding stack contains the open parentheses
			TokenPtr topToken {std::move(m_holdStack.top().token)};
			m_holdStack.pop();
			if (!topToken->code() == OpenParen_Code)
			{
				// oops, no open parentheses (this should not happen)
				throw TokenError {Status::BUG_UnexpectedCloseParen, m_token};
			}

			// replace first and last operands of token on done stack
			m_doneStack.top().replaceFirstLast(topToken, m_token);

			// set highest precedence if not an operator on done stack top
			// (no operators in the parentheses)
			topToken = m_doneStack.top().rpnItem->token();
			m_lastPrecedence = topToken->isType(Type::Operator)
				? m_table.precedence(topToken) : HighestPrecedence;

			// set pending parentheses token pointer
			m_pendingParen = std::move(m_token);
		}
		else if (!m_table.isUnaryOperator(m_token))
		{
			if (!getOperand(expectedDataType))
			{
				break;  // terminating token, let caller determine action
			}
			else if (doneStackTopToken()->isDataType(DataType::None)
				&& dataType != DataType::None)
			{
				// print functions are not allowed
				throw doneStackTopTokenError(expectedErrorStatus(dataType));
			}
		}
		if (!m_token)
		{
			// get binary operator or end-of-expression token
			// if parser error then caller needs to handle it
			getToken(Status{});
			if (doneStackTopToken()->isDataType(DataType::None)
				&& m_holdStack.top().token->isNull()
				&& dataType == DataType::None)
			{
				// for print functions return now with token as terminator
				break;
			}
			// check for unary operator (token should be a binary operator)
			if (m_table.isUnaryOperator(m_token))
			{
				// check if code has a binary operator
				if (m_table.alternateCodeCount(m_token->code(), 1) > 0)
				{
					// change token to binary operator
					m_token->setCode(m_table.alternateCode(m_token->code(), 1));
				}
				else
				{
					// caller may need to change this error
					throw TokenError {Status::ExpBinOpOrEnd, m_token};
				}
			}
		}

		// check for and process operator (unary or binary)
		if (!processOperator())
		{
			if (level == 0)
			{
				// add convert code if needed or report error
				TokenPtr doneToken {m_doneStack.top().rpnItem->token()};
				Code cvtCode {doneToken->convertCode(dataType)};
				if (cvtCode == Invalid_Code)
				{
					if (doneToken->isType(Type::Constant)
						&& dataType == DataType::Integer
						&& doneToken->isDataType(DataType::Double))
					{
						// constant could not be converted
						throw doneStackTopTokenError(Status::ExpIntConst);
					}
					throw doneStackTopTokenError(expectedErrorStatus(dataType));
				}
				else if (cvtCode != Null_Code)
				{
					m_output.append(std::make_shared<Token>(cvtCode));
				}
			}
			break;
		}

		// get operator's expected data type (was pushed on hold stack)
		expectedDataType = m_table.expectedDataType(m_holdStack.top().token);
	}
}


// function to get an operand
//
//   - the data type argument is used for reporting the correct error when
//     the token is not a valid operand token
//   - return true upon success or false if token not an operand
//   - upon true, current token is left empty
//   - upon false, current token is not changed
//   - throws token error upon detection of error
//   - does not return false if reference requested (throws error)

bool Translator::getOperand(DataType dataType, Reference reference)
{
	// get token (no numbers for a reference)
	getToken(Status{}, dataType, reference);

	bool doneAppend {true};
	switch (m_token->type())
	{
	case Type::Command:
	case Type::Operator:
		if (dataType == DataType::None)
		{
			// nothing is acceptable, this is terminating token
			return false;
		}
		throw TokenError {expectedErrorStatus(dataType, reference),
			std::move(m_token)};

	case Type::Constant:
		break;  // go add token to output and push to done stack

	case Type::NoParen:
		break;  // go add token to output and push to done stack

	case Type::IntFunc:
		if (reference != Reference::None)
		{
			if (reference != Reference::All
				|| !m_table.hasFlag(m_token, SubStr_Flag))
			{
				throw TokenError {expectedErrorStatus(dataType, reference),
					std::move(m_token)};
			}
		}
		else if (m_token->isDataType(DataType::None)
			&& dataType != DataType::None)
		{
			throw TokenError {expectedErrorStatus(dataType),
				std::move(m_token)};
		}
		if (m_table.operandCount(m_token) > 0)
		{
			processInternalFunction(reference);
			doneAppend = false;  // already appended
		}
		break;

	case Type::DefFunc:
		if (reference == Reference::Variable)
		{
			throw TokenError {expectedErrorStatus(dataType, reference),
				std::move(m_token)};
		}
		if (m_parse->getParen())
		{
			if (reference != Reference::None)
			{
				// NOTE these are allowed in the DEF command
				// just point to the open parentheses of the token
				TokenPtr token = std::move(m_token);
				throw TokenError {Status::ExpEqualOrComma, token->column()
					+ token->length(), 1};
			}
			processParenToken();
			doneAppend = false;  // already appended
		}
		break;  // go add token to output and push to done stack

	case Type::Paren:
		m_parse->getParen();
		processParenToken();
		doneAppend = false;  // already appended
		break;

	default:
		throw TokenError {Status::BUG_NotYetImplemented, std::move(m_token)};
	}

	if (doneAppend)
	{
		// add token directly to output list
		// and push element pointer on done stack
		m_output.append(std::move(m_token));
		m_doneStack.emplace(m_output.back());
	}
	// for reference, check data type
	if (reference != Reference::None
		&& m_doneStack.top().rpnItem->token()->convertCode(dataType)
		!= Null_Code)
	{
		throw doneStackTopTokenError(expectedErrorStatus(dataType, reference));
	}
	return true;
}


// function to get current token from the parser if current token not set
//
//   - data type and references arguments determine if numbers are allowed
//   - throws parser error if the parser threw an error exception
//   - if caller requests specific error, then that error is thrown

void Translator::getToken(Status errorStatus, DataType dataType,
	Reference reference)
try
{
	if (!m_token)
	{
		// if data type is not blank and not string, then allow a number token
		m_token.reset((*m_parse)(dataType, reference));
	}
}
catch (TokenError &error)
{
	if (dataType != DataType{} && dataType != DataType::None
		&& error(Status::UnknownToken))
	{
		// non-number constant error, change to expected expression error
		error = expectedErrorStatus(dataType, reference);
	}
	else if (errorStatus != Status{})
	{
		error = errorStatus;
	}
	// else caller may need to convert this error to appropriate error
	throw;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                             PROCESS FUNCTIONS                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


// function to get and process an internal function's arguments
//
//   - current token contains the internal function token
//   - reference argument not none for sub-string assignments
//   - throws token error upon detection of error

void Translator::processInternalFunction(Reference reference)
{
	// push internal function token onto hold stack to block waiting tokens
	// during the processing of the expressions of each argument
	TokenPtr topToken {std::move(m_token)};
	m_holdStack.emplace(topToken);

	Code code {topToken->code()};
	int lastOperand {m_table.operandCount(code) - 1};
	for (int i {}; ; i++)
	{
		DataType expectedDataType;
		if (i == 0 && reference != Reference::None)
		{
			// sub-string assignment, look for reference operand
			expectedDataType = DataType::String;
			// will not return false (returns error if not reference)
			getOperand(expectedDataType, Reference::VarDefFn);
			// get next token (should be a comma)
			getToken(Status::ExpComma);
			if (!m_token->isCode(Comma_Code))
			{
				throw TokenError {Status::ExpComma, m_token};
			}
		}
		else
		{
			expectedDataType = i == 0 ? m_table.expectedDataType(topToken)
				: m_table.operandDataType(code, i);
			try
			{
				getExpression(expectedDataType);
			}
			catch (TokenError &error)
			{
				if (error(Status::ExpBinOpOrEnd))  // (aka unary operator?)
				{
					// determine error for unary operator token
					error = expressionErrorStatus(i == lastOperand, true, code);
				}
				else if (error(Status::UnknownToken))
				{
					// determine error for unknown tokens
					error = expressionErrorStatus(i == lastOperand, false,
						code);
				}
				throw;
			}
		}

		// check if alternate code for function is needed
		if (expectedDataType == DataType::Number)
		{
			TokenPtr &token = m_doneStack.top().rpnItem->token();
			if (token->dataType()
				!= m_table.operandDataType(topToken->code(), 0))
			{
				// change token's code and data type to associated code
				m_table.setToken(topToken.get(),
					m_table.alternateCode(topToken->code()));
			}
			token->removeSubCode(IntConst_SubCode);  // safe for all tokens
		}

		// check terminating token
		if (m_token->isCode(Comma_Code))
		{
			if (i == lastOperand)
			{
				if (!m_table.hasFlag(code, Multiple_Flag))
				{
					// function doesn't have multiple entries
					throw TokenError {Status::ExpOpOrParen, m_token};
				}
				// get second associated code; update code and last operand
				code = m_table.alternateCode(topToken->code(), ++lastOperand);
				topToken->setCode(code);
			}
			m_token.reset();  // delete comma token, it's not needed
			m_doneStack.pop();
		}
		else if (m_token->isCode(CloseParen_Code))
		{
			if (i < lastOperand)
			{
				throw TokenError {Status::ExpOpOrComma, m_token};
			}

			m_doneStack.pop();  // remove from done stack

			// add token to output list if not sub-string assignment
			RpnItemPtr rpnItem;
			if (reference != Reference::None)
			{
				rpnItem = std::make_shared<RpnItem>(topToken);
			}
			else
			{
				m_output.append(std::move(topToken));
				rpnItem = m_output.back();
			}

			// push internal function to done stack
			m_doneStack.emplace(rpnItem, std::move(m_token));

			m_holdStack.pop();
			break;  // done
		}
		else
		{
			// determine error for unknown tokens
			throw TokenError {expressionErrorStatus(i == lastOperand, false,
				code), m_token};
		}
	}
}


// function to get and process a parentheses token's arguments
//
//   - current token contains the token with parentheses
//   - throws token error upon detection of error

void Translator::processParenToken()
{
	// push parentheses token onto hold stack to block waiting tokens
	// during the processing of the expressions of each operand
	m_holdStack.emplace(m_token);
	// determine data type (number for subscripts, any for arguments)
	// TODO with function dictionaries, get data type for each argument
	// TODO (move into loop below) and check number of arguments
	DataType dataType {DataType::Any};
	// TODO temporary until array and functions are fully implemented
	// TODO this check needs updated for data typed array codes
	if (m_token->isCode(Array_Code))
	{
		dataType = DataType::Integer;  // array subscripts
	}
	TokenPtr topToken {std::move(m_token)};

	for (int count {1}; ; count++)
	{
		try
		{
			getExpression(dataType);
		}
		catch (TokenError &error)
		{
			if (error(Status::ExpBinOpOrEnd))  // (aka unary operator?)
			{
				error = Status::ExpBinOpCommaOrParen;
			}
			else if (error(Status::UnknownToken))
			{
				error = Status::ExpOpCommaOrParen;
			}
			throw;
		}

		// set reference for appropriate token types
		if (topToken->isType(Type::Paren))
		{
			if (dataType == DataType::Integer)  // array subscript?
			{
				m_doneStack.pop();  // don't need item
			}
			else  // function argument
			{
				TokenPtr &token = m_doneStack.top().rpnItem->token();
				// TODO may also need to check for DefFuncN type here
				if ((token->isType(Type::NoParen) || token->isType(Type::Paren))
					&& !token->hasSubCode(Paren_SubCode))
				{
					token->setReference();
				}
			}
		}

		// check terminating token
		if (m_token->isCode(Comma_Code))
		{
			m_token.reset();  // delete comma token, it's not needed
		}
		else if (m_token->isCode(CloseParen_Code))
		{
			RpnItemVector attached;
			// save operands for storage in output list
			while (--count >= 0)
			{
				// if array subscript then apppend empty pointer
				// else pop and append operands
				if (dataType == DataType::Integer)
				{
					attached.emplace_back();
				}
				else
				{
					attached.emplace_back(m_doneStack.top().rpnItem);
					m_doneStack.pop();
					// TODO will need to keep first/last operands for each
					// TODO (in case expression needs to be reported as error)
					// TODO (RpnItem should have DoneItem operands, not RpnItem)
				}
			}

			// add token to output list and push element pointer on done stack
			m_output.append(topToken, attached);
			m_doneStack.emplace(m_output.back(), std::move(m_token));

			m_holdStack.pop();
			break;  // done
		}
		else  // unexpected token
		{
			throw TokenError {Status::ExpOpCommaOrParen, m_token};
		}
	}
}


// function to process an operator token, all operators with higher
// precedence on the hold stack are added to output list first
//
// for each operator added to the output list, any pending parentheses is
// processed first, the final operand of the operator is processed
//
//    - current token is the operator token
//    - sets last_precedence for operator being added to output list
//    - after higher precedence operators are processed from the hold stack,
//      a check for pending parenthesis is made, and if the present token is
//      not a unary or binary operator, the end of the expression has been
//      reached and false is returned
//    - otherwise, the first operand of the operator is processed,
//      which also handles pushing the operator to the hold stack
//    - true is returned indicating operator successfully processed
//    - upon true, current token is left empty
//    - upon false, current token is not changed

bool Translator::processOperator()
{
	// determine precedence of incoming token
	// (set highest precedence for unary operators,
	// which don't force other operators from hold stack)
	int tokenPrecedence {m_table.isUnaryOperator(m_token)
		? HighestPrecedence : m_table.precedence(m_token)};

	// process and drop unary or binary operators on hold stack
	// while they have higher or same precedence as incoming token
	TokenPtr topToken;
	while (m_table.precedence(topToken = m_holdStack.top().token)
		>= tokenPrecedence && m_table.isUnaryOrBinaryOperator(topToken))
	{
		checkPendingParen(topToken, Popped::Yes);

		// change token operator code or insert conversion codes as needed
		processFinalOperand(topToken, std::move(m_holdStack.top().first),
			m_table.isUnaryOperator(topToken) ? 0 : 1);

		// save precedence of operator being added
		// (doesn't matter if not currently within parentheses,
		// it will be reset upon next open parentheses)
		m_lastPrecedence = m_table.precedence(topToken->code());

		m_holdStack.pop();
	}

	checkPendingParen(m_token, Popped::No);

	if (!m_table.isUnaryOrBinaryOperator(m_token))
	{
		return false;  // not unary or binary operator, end of expression
	}
	// otherwise process first operand and return
	TokenPtr first;			// first operand token (empty for unary operator)

	// check first operand of binary operators
	if (!m_table.isUnaryOperator(m_token))
	{
		// changed token operator code or insert conversion codes as needed
		processDoneStackTop(m_token, 0, &first);
	}

	// push it onto the holding stack and attach first operand
	m_holdStack.emplace(std::move(m_token), std::move(first));
	return true;
}


// function to process the final operand for an operator or internal code
//
//   - called with token final operand is for (operator or internal code)
//   - operand_index is zero except for binary operator second operand
//   - processDoneStackTop() is called to set correct code for data type of
//     done stack top item and insert any necessary hidden conversion code
//   - first/last of done stack top item transferred to non-reference tokens
//   - operator or internal code token appended to RPN output list
//	 - RPN item is pushed to done stack if operator
//
//   - for operator, token2 is first operand of operator on hold stack
//   - for internal code, token2 is not set

void Translator::processFinalOperand(TokenPtr &token, TokenPtr token2,
	int operandIndex)
{
	TokenPtr first;			// first operand token
	TokenPtr last;			// last operand token

	processDoneStackTop(token, operandIndex, &first, &last);

	if (token->isType(Type::Operator))
	{
		// set first token
		// unary operator: unary operator token
		// binary operator: first operator of operator (token2)
		first = operandIndex == 0 ? token : token2;
		// last operand set from token that was on done stack
	}

	// add token to output list
	m_output.append(token);

	// push operator token to the done stack
	if (token->isType(Type::Operator))
	{
		m_doneStack.emplace(m_output.back(), first, last);
	}
}


// function to process the item on top of the done stack for a given operand
// of an operator or only operand of an internal code token
//
//   - if requested, the first and last operands of the item are returned
//   - calls Table::findCode() to check the data type of item:
//     changes token code to a matching associated code if available
//     else returns a conversion code if conversion is possible
//   - if no match found or conversion not possible, an error is thrown
//   - if conversion possible, a conversion code token is appended to output

void Translator::processDoneStackTop(TokenPtr &token, int operandIndex,
	TokenPtr *first, TokenPtr *last)
{
	if (m_doneStack.empty())
	{
		// oops, there should have been operands on done stack
		throw TokenError {Status::BUG_DoneStackEmptyFindCode, token};
	}
	TokenPtr topToken {m_doneStack.top().rpnItem->token()};
	// get first and last operands for top token
	TokenPtr localFirst {m_doneStack.top().first};
	if (!localFirst)
	{
		localFirst = topToken;  // first operand not set, set to operand token
	}
	if (first)  // requested by caller?
	{
		*first = localFirst;
	}
	TokenPtr localLast {m_doneStack.top().last};
	if (!localLast)
	{
		localLast = topToken;  // last operand not set, set to operand token
	}
	if (last)  // requested by caller?
	{
		*last = localLast;
	}

	// see if main code's data type matches
	Code cvtCode {m_table.findCode(token, topToken, operandIndex)};

	if (cvtCode != Invalid_Code)
	{
		m_doneStack.pop();  // remove from done stack

		// is there an actual conversion code to insert?
		if (cvtCode != Null_Code)
		{
			// INSERT CONVERSION CODE
			// create convert token with convert code
			// append token to end of output list (after operand)
			m_output.append(std::make_shared<Token>(cvtCode));
		}
	}
	else  // no match found, throw error
	{
		// use main code's expected data type for operand
		// (if no data type, then double constant can't be converted to integer)
		// report entire expression from first token through last
		throw TokenError {topToken->isDataType(DataType{})
			? Status::ExpIntConst : expectedErrorStatus(topToken->dataType()),
			localFirst->column(), localLast->column() + localLast->length()
			- localFirst->column()};
	}
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                             SUPPORT FUNCTIONS                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


// function to check if there is a pending parentheses token and if there is,
// check to see if it should be added to the output as a dummy token so that
// the unnecessary set of parentheses, but entered by the user, will be
// recreated
//
//   - token argument is token of operator to check against
//   - popped argument indicates if token will be popped from the hold stack

void Translator::checkPendingParen(const TokenPtr &token, Popped popped)
	noexcept
{
	if (m_pendingParen)  // is a closing parentheses token pending?
	{
		// may need to add a dummy token if the precedence of the last
		// operator added within the last parentheses sub-expression
		// is higher than or same as (popped tokens only) the operator
		int precedence {m_table.precedence(token)};
		if (m_lastPrecedence > precedence
			|| (popped == Popped::No && m_lastPrecedence == precedence))
		{
			TokenPtr lastToken {m_output.lastToken()};
			if (!lastToken->hasSubCode(Paren_SubCode))
			{
				// mark last code for unnecessary parentheses
				lastToken->addSubCode(Paren_SubCode);
			}
			else   // already has parentheses sub-code set
			{
				// add dummy token, and reset pending token
				m_output.append(std::move(m_pendingParen));
				return;
			}
		}
		m_pendingParen.reset();  // reset pending token
	}
}


// function to create token error from done stack top
// (error is from first operand to the last operand)

TokenError Translator::doneStackTopTokenError(Status errorStatus) noexcept
{
	TokenPtr token {std::move(m_doneStack.top().first)};
	if (!token)
	{
		// if no first operand token, start at token itself
		token = m_doneStack.top().rpnItem->token();
	}
	int column = token->column();
	int length = token->length();

	// if last operand token set, set error length through last token
	if (TokenPtr token = std::move(m_doneStack.top().last))
	{
		length = token->column() - column + token->length();
	}
	return TokenError {errorStatus, column, length};
}


// function to return the token error status for an expected data type
// and reference type
Status Translator::expectedErrorStatus(DataType dataType, Reference reference)
	noexcept
{
	switch (dataType)
	{
	case DataType::Double:
		return reference == Reference::None
			? Status::ExpNumExpr : Status::ExpDblVar;
	case DataType::Integer:
		return reference == Reference::None
			? Status::ExpNumExpr : Status::ExpIntVar;
	case DataType::String:
		switch (reference)
		{
		case Reference::None:
			return Status::ExpStrExpr;
		case Reference::Variable:
		case Reference::VarDefFn:
			return Status::ExpStrVar;
		case Reference::All:
			return Status::ExpStrItem;
		}
	case DataType::None:
		return reference == Reference::None
			? Status::ExpExpr : Status::ExpAssignItem;
	case DataType::Number:
		return reference == Reference::None
			? Status::ExpNumExpr : Status::BUG_InvalidDataType;
	case DataType::Any:
		switch (reference)
		{
		case Reference::None:
			return Status::ExpExpr;
		case Reference::Variable:
			return Status::ExpVar;
		case Reference::VarDefFn:
		case Reference::All:
			return Status::ExpAssignItem;
		}
	}
	return Status::BUG_InvalidDataType;  // won't get here
}


// function to determine error status for an expression error
Status Translator::expressionErrorStatus(bool lastOperand, bool unaryOperator,
	Code code) noexcept
{
	if (!lastOperand)
	{
		return unaryOperator ? Status::ExpBinOpOrComma : Status::ExpOpOrComma;
	}
	else if (!m_table.hasFlag(code, Multiple_Flag))
	{
		// function doesn't have multiple entries
		return unaryOperator ? Status::ExpBinOpOrParen : Status::ExpOpOrParen;
	}
	else  // could be another operand or at last operand
	{
		return unaryOperator ? Status::ExpBinOpCommaOrParen
			: Status::ExpOpCommaOrParen;
	}
}


// end: translator.cpp
