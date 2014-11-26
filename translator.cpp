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
	TokenPtr token;
	Status status;

	m_holdStack.emplace(m_table.newToken(Null_Code));

	if (testMode == TestMode::Expression)
	{
		status = getExpression(token, DataType::Any);

		if (status == Status::UnknownToken)
		{
			status = Status::ExpOpOrEnd;
		}
		else if (status == Status::Done)
		{
			// pop final result off of done stack
			if (m_doneStack.empty())
			{
				status = Status::BUG_DoneStackEmpty;
			}
			else
			{
				// drop result
				m_doneStack.pop();
			}
		}
	}
	else
	{
		getCommands(token);
		status = Status::Done;	// TODO temporary
	}

	if (status == Status::Done)
	{
		if (token->isCode(EOL_Code))
		{
			// pop and delete null token from top of stack
			m_holdStack.pop();

			if (!m_holdStack.empty())
			{
				throw TokenError {Status::BUG_HoldStackNotEmpty, token};
			}

			if (!m_doneStack.empty())
			{
				throw TokenError {Status::BUG_DoneStackNotEmpty, token};
			}

			if (testMode == TestMode::No
				&& !m_output.setCodeSize(m_table, token))
			{
				throw TokenError {Status::BUG_NotYetImplemented, token};
			}
		}
		else
		{
			throw TokenError {Status::ExpOpOrEnd, token};
		}
	}
	else
	{
		throw TokenError {status, token};
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
//   - returns terminating token (end-of-line or unknown token)
//   - caller determines validity of unknown token if returned
//   - throws token error upon detection of error

void Translator::getCommands(TokenPtr &token)
{
	forever
	{
		// get any reference token and ignore status
		// if not a command token then let translate will handle token
		if (getToken(token, DataType::Any, Reference::All) != Status::Good)
		{
			throw TokenError {Status::ExpCmdOrAssignItem, token};
		}

		if (token->isCode(EOL_Code) && m_output.empty())
		{
			return;  // blank line allowed
		}

		if (token->isCode(Rem_Code) || token->isCode(RemOp_Code))
		{
			break;
		}

		processCommand(token);
		if (token->isCode(RemOp_Code))
		{
			break;
		}
		else if (token->isCode(Colon_Code))
		{
			// set colon sub-code on last token
			outputLastToken()->addSubCode(Colon_SubCode);
		}
		else  // unknown end statement token, return to caller
		{
			return;
		}
	}
	m_output.append(std::move(token));  // Rem or RemOp token
	if (getToken(token) != Status::Good)
	{
		// parser problem, should be EOL
		throw TokenError {Status::BUG_UnexpToken, token};
	}
}


// function to get an expression from the input line
//
//   - takes an already obtained token (gets a token if none)
//   - takes a data type argument for the desired data type of the expression
//   - returns Done_TokenStatus upon success
//   - returns an error status if an error was detected
//   - returns the token that terminated the expression

Status Translator::getExpression(TokenPtr &token, DataType dataType, int level)
{
	Status status;
	DataType expectedDataType {dataType};

	forever
	{
		if (!token && (status = getToken(token, expectedDataType))
			!= Status::Good)
		{
			break;
		}

		if (token->isCode(OpenParen_Code))
		{
			// push open parentheses onto hold stack to block waiting tokens
			// during the processing of the expression inside the parentheses
			m_holdStack.emplace(std::move(token));

			// get an expression and terminating token
			if (expectedDataType == DataType::None)
			{
				expectedDataType = DataType::Any;
			}
			if ((status = getExpression(token, expectedDataType, level + 1))
				!= Status::Done)
			{
				if (m_table.isUnaryOperator(token))
				{
					status = Status::ExpBinOpOrParen;
				}
				if (status == Status::UnknownToken)
				{
					status = Status::ExpOpOrParen;
				}
				break;  // exit on error
			}

			// check terminating token
			if (!token->isCode(CloseParen_Code))
			{
				status = Status::ExpOpOrParen;
				break;
			}

			// make sure holding stack contains the open parentheses
			TokenPtr topToken {std::move(m_holdStack.top().token)};
			m_holdStack.pop();
			if (!topToken->code() == OpenParen_Code)
			{
				// oops, no open parentheses (this should not happen)
				return Status::BUG_UnexpectedCloseParen;
			}

			// replace first and last operands of token on done stack
			m_doneStack.top().replaceFirstLast(topToken, token);

			// set highest precedence if not an operator on done stack top
			// (no operators in the parentheses)
			topToken = m_doneStack.top().rpnItem->token();
			m_lastPrecedence = topToken->isType(Token::Type::Operator)
				? m_table.precedence(topToken) : HighestPrecedence;

			// set pending parentheses token pointer
			m_pendingParen = std::move(token);
		}
		else
		try
		{
			Code unaryCode;
			if ((unaryCode = m_table.unaryCode(token)) != Null_Code)
			{
				token->setCode(unaryCode);  // change token to unary operator
			}
			// get operand
			else if (!getOperand(token, expectedDataType))
			{
				status = Status::Done;  // TODO temporary (terminating token)
				break;  // terminating token, let caller determine action
			}
			else if (doneStackTopToken()->isDataType(DataType::None)
				&& dataType != DataType::None)
			{
				// print functions are not allowed
				token = doneStackPopErrorToken();
				status = expectedErrStatus(dataType);
				break;
			}
			else
			{
				token.reset();  // get another token
			}
		}
		catch (TokenError &error)  // TODO temporary, from getOperand()
		{
			status = error();
			break;
		}
		if (!token)
		{
			// get binary operator or end-of-expression token
			if ((status = getToken(token)) != Status::Good)
			{
				// if parser error then caller needs to handle it
				break;
			}
			if (doneStackTopToken()->isDataType(DataType::None)
				&& m_holdStack.top().token->isNull()
				&& dataType == DataType::None)
			{
				// for print functions return now with token as terminator
				status = Status::Done;
				break;
			}
			// check for unary operator (token should be a binary operator)
			if (m_table.isUnaryOperator(token))
			{
				status = Status::ExpBinOpOrEnd;
				break;
			}
		}

		// check for and process operator (unary or binary)
		try
		{
			if (!processOperator(token))
			{
				if (level == 0)
				{
					// add convert code if needed or report error
					TokenPtr doneToken {m_doneStack.top().rpnItem->token()};
					Code cvtCode {doneToken->convertCode(dataType)};
					if (cvtCode == Invalid_Code)
					{
						token = doneStackPopErrorToken();
						status = expectedErrStatus(dataType);
						break;
					}
					else if (cvtCode != Null_Code)
					{
						if (doneToken->isType(Token::Type::Constant))
						{
							// constants don't need conversion
							if (dataType == DataType::Integer
								&& doneToken->isDataType(DataType::Double))
							{
								token = doneStackPopErrorToken();
								status = Status::ExpIntConst;
								break;
							}
						}
						else  // append hidden conversion code
						{
							m_output.append(m_table.newToken(cvtCode));
						}
					}
				}
				status = Status::Done;
				break;
			}
		}
		catch (TokenError &error)
		{
			status = error();
			token = std::make_shared<Token>(error.m_column, error.m_length);
			break;
		}

		// get operator's expected data type, reset token and loop back
		expectedDataType = m_table.expectedDataType(token);
		token = nullptr;
	}
	return status;
}


// function to get an operand
//
//   - a token may be passed in, otherwise a token is obtained
//   - the data type argument is used for reporting the correct error when
//     the token is not a valid operand token
//   - return true upon success or false if token not an operand
//   - throws token error upon detection of error
//   - does not return false if reference requested (throws error)

bool Translator::getOperand(TokenPtr &token, DataType dataType,
	Reference reference)
{
	Status status;

	// get token if none was passed (no numbers for a reference)
	if (!token && (status = getToken(token, dataType, reference))
		!= Status::Good)
	{
		throw TokenError {status, token};
	}

	// set default data type for token if it has none
	token->setDataType();

	bool doneAppend {true};
	switch (token->type())
	{
	case Token::Type::Command:
	case Token::Type::Operator:
		if (dataType == DataType::None)
		{
			// nothing is acceptable, this is terminating token
			return false;
		}
		throw TokenError {expectedErrStatus(dataType, reference), token};

	case Token::Type::Constant:
		// check if specific numeric data type requested
		if ((dataType == DataType::Double || dataType == DataType::Integer)
			&& token->isDataType(DataType::Integer))
		{
			// for integer constants, force to desired data type
			token->setDataType(dataType);
			token->removeSubCode(Double_SubCode);
		}
		if (dataType == DataType::Double || dataType == DataType::Integer
			|| dataType == DataType::String)
		{
			m_table.setTokenCode(token, Const_Code);
		}
		if (reference != Reference::None)
		{
			throw TokenError {expectedErrStatus(dataType, reference), token};
		}
		break;  // go add token to output and push to done stack

	case Token::Type::IntFuncN:
		if (reference != Reference::None)
		{
			throw TokenError {expectedErrStatus(dataType, reference), token};
		}
		break;  // go add token to output and push to done stack

	case Token::Type::DefFuncN:
		if (reference == Reference::Variable)
		{
			throw TokenError {expectedErrStatus(dataType, reference), token};
		}
		if (reference != Reference::None)
		{
			token->setReference();
		}
		// TODO temporary until define functions are fully implemented
		dataType = token->dataType();  // preserve data type
		m_table.setToken(token, DefFuncN_Code);
		token->setDataType(dataType);
		break;  // go add token to output and push to done stack

	case Token::Type::NoParen:
		// REMOVE for now assume a variable
		// TODO first check if identifier is in function dictionary
		// TODO only a function reference if name of current function
		m_table.setTokenCode(token, reference == Reference::None
			? Var_Code : VarRef_Code);
		break;  // go add token to output and push to done stack

	case Token::Type::IntFuncP:
		if (reference != Reference::None)
		{
			if (reference == Reference::All
				&& m_table.hasFlag(token, SubStr_Flag))
			{
				token->setReference();
			}
			else
			{
				throw TokenError {expectedErrStatus(dataType, reference),
					token};
			}
		}
		else if (token->isDataType(DataType::None)
			&& dataType != DataType::None)
		{
			throw TokenError {expectedErrStatus(dataType), token};
		}
		processInternalFunction(token);
		// reset reference if it was set above, no longer needed
		token->setReference(false);
		doneAppend = false;  // already appended
		break;

	case Token::Type::DefFuncP:
		if (reference == Reference::Variable)
		{
			throw TokenError {expectedErrStatus(dataType, reference), token};
		}
		else if (reference != Reference::None)
		{
			// NOTE these are allowed in the DEF command
			// just point to the open parentheses of the token
			throw TokenError {Status::ExpEqualOrComma, token->column()
				+ token->length(), 1};
		}
		processParenToken(token);

		// TODO temporary until define functions are fully implemented
		dataType = token->dataType();  // preserve data type
		m_table.setToken(token, DefFuncP_Code);
		token->setDataType(dataType);
		doneAppend = false;  // already appended
		break;

	case Token::Type::Paren:
		if (reference != Reference::None)
		{
			token->setReference();
		}
		processParenToken(token);
		doneAppend = false;  // already appended
		break;

	default:
		throw TokenError {Status::BUG_NotYetImplemented, token};
	}

	if (doneAppend)
	{
		// add token directly to output list
		// and push element pointer on done stack
		m_output.append(token);
		m_doneStack.emplace(m_output.back());
	}
	// for reference, check data type
	if (reference != Reference::None
		&& token->convertCode(dataType) != Null_Code)
	{
		throw TokenError {expectedErrStatus(dataType, reference),
			doneStackPopErrorToken()};
	}
	return true;
}


// function to get a token from the parser
//
//   - data type argument determines if number tokens are allowed
//   - returns parser error if the parser returned an error exception

Status Translator::getToken(TokenPtr &token, DataType dataType,
	Reference reference)
try
{
	// if data type is not blank and not string, then allow a number token
	token = (*m_parse)(dataType != DataType{} && dataType != DataType::String
		&& reference == Reference::None
		? Parser::Number::Yes : Parser::Number::No);
	return Status::Good;
}
catch (TokenError &error)
{
	// TODO for now, create an error token to return
	token = std::make_shared<Token>(error.m_column, error.m_length);
	if (dataType != DataType{} && dataType != DataType::None
		&& error(Status::UnknownToken))
	{
		// non-number constant error, return expected expression error
		return expectedErrStatus(dataType, reference);
	}
	else
	{
		// caller may need to convert this error to appropriate error
		return error();
	}
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                             PROCESS FUNCTIONS                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


// function to process a command in the input line
//
//   - returns the token that terminated the command through argument
//   - throws token error upon detection of error

void Translator::processCommand(TokenPtr &commandToken)
{
	TranslateFunction translate;
	TokenPtr token;

	if (commandToken->isType(Token::Type::Command))
	{
		translate = m_table.translateFunction(commandToken->code());
	}
	else  // assume an assignment statement
	{
		translate = m_table.translateFunction(Let_Code);
		token = std::move(commandToken);
		// pass token onto let translate function
	}
	if (!translate)
	{
		throw TokenError {Status::BUG_NotYetImplemented, commandToken};
	}
	(*translate)(*this, std::move(commandToken), token);
	commandToken = std::move(token);
}


// function to get and process an internal function's arguments
//
//   - the token argument contains the internal function token
//   - throws token error upon detection of error

void Translator::processInternalFunction(TokenPtr &token)
{
	// push internal function token onto hold stack to block waiting tokens
	// during the processing of the expressions of each argument
	m_holdStack.emplace(token);
	TokenPtr topToken {std::move(token)};

	Code code {topToken->code()};
	int lastOperand {m_table.operandCount(code) - 1};
	for (int i {}; ; i++)
	{
		DataType expectedDataType;
		if (i == 0 && topToken->reference())
		{
			// sub-string assignment, look for reference operand
			expectedDataType = DataType::String;
			try
			{
				// will not return false (returns error if not reference)
				getOperand(token, expectedDataType, Reference::VarDefFn);
				// get next token (should be a comma)
				if (getToken(token) != Status::Good
					|| !token->isCode(Comma_Code))
				{
					throw TokenError {Status::UnknownToken, token};
				}
			}
			catch (TokenError &error)
			{
				if (error(Status::UnknownToken))  // from getToken() only
				{
					error = Status::ExpComma;
				}
				throw;  // throws getOperand() errors as is
			}
		}
		else
		{
			expectedDataType = i == 0 ? m_table.expectedDataType(topToken)
				: m_table.operandDataType(code, i);
			try
			{
				Status status = getExpression(token, expectedDataType);
				if (status != Status::Done)
				{
					throw TokenError {status, token};
				}
			}
			catch (TokenError &error)
			{
				if (m_table.isUnaryOperator(token))
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

		// check if associated code for function is needed
		if (expectedDataType == DataType::Number)
		{
			TokenPtr doneToken {m_doneStack.top().rpnItem->token()};
			if (doneToken->dataType()
				!= m_table.operandDataType(topToken->code(), 0))
			{
				// change token's code and data type to associated code
				m_table.setToken(topToken,
					m_table.associatedCode(topToken->code()));
			}
			else if (doneToken->hasSubCode(Double_SubCode))
			{
				// change token (constant) from integer to double
				doneToken->setDataType(DataType::Double);
				doneToken->removeSubCode(Double_SubCode);
			}
			if (doneToken->isType(Token::Type::Constant))
			{
				doneToken->setCode(Const_Code);
			}
		}

		// check terminating token
		if (token->isCode(Comma_Code))
		{
			if (i == lastOperand)
			{
				if (!m_table.hasFlag(code, Multiple_Flag))
				{
					// function doesn't have multiple entries
					throw TokenError {Status::ExpOpOrParen, token};
				}
				// move to next code; update code and last operand index
				code = topToken->nextCode();
				lastOperand = m_table.operandCount(code) - 1;
			}
			token.reset();  // delete comma token, it's not needed
			m_doneStack.pop();
		}
		else if (token->isCode(CloseParen_Code))
		{
			if (i < lastOperand)
			{
				throw TokenError {Status::ExpOpOrComma, token};
			}

			m_doneStack.pop();  // remove from done stack

			// add token to output list if not sub-string assignment
			RpnItemPtr rpnItem;
			if (topToken->reference())
			{
				rpnItem = std::make_shared<RpnItem>(topToken);
			}
			else
			{
				m_output.append(topToken);
				rpnItem = m_output.back();
			}

			// push internal function to done stack
			m_doneStack.emplace(rpnItem, std::move(token));

			m_holdStack.pop();
			token = std::move(topToken);  // return original token
			break;  // done
		}
		else
		{
			// determine error for unknown tokens
			throw TokenError {expressionErrorStatus(i == lastOperand, false,
				code), token};
		}
	}
}


// function to get and process a parentheses token's arguments
//
//   - the token argument contains the token with parentheses
//   - throws token error upon detection of error

void Translator::processParenToken(TokenPtr &token)
{
	// push parentheses token onto hold stack to block waiting tokens
	// during the processing of the expressions of each operand
	m_holdStack.emplace(token);
	// determine data type (number for subscripts, any for arguments)
	DataType dataType;
	// TODO need to check test mode once dictionaries are implemented
	// REMOVE for now assume functions start with an 'F'
	// TODO temporary until array and functions are fully implemented
	bool isArray;
	if (token->isType(Token::Type::Paren))
	{
		isArray = toupper(token->string().front()) != 'F';
		token->setCode(isArray ? Array_Code : Function_Code);
	}
	if (token->isType(Token::Type::Paren) && (token->reference() || isArray))
	{
		dataType = DataType::Integer;  // array subscripts
	}
	else
	{
		// TODO with function dictionaries, get data type for argument
		// TODO (move into loop below)
		// TODO and check number of arguments
		dataType = DataType::Any;  // function arguments
	}
	TokenPtr topToken {std::move(token)};

	for (int count {1}; ; count++)
	{
		try
		{
			Status status = getExpression(token, dataType);
			if (status != Status::Done)
			{
				throw TokenError {status, token};
			}
		}
		catch (TokenError &error)
		{
			if (error(Status::UnknownToken))
			{
				error = Status::ExpOpCommaOrParen;
			}
			else if (m_table.isUnaryOperator(token))
			{
				error = Status::ExpBinOpCommaOrParen;
			}
			throw;
		}

		// set reference for appropriate token types
		if (topToken->isType(Token::Type::Paren))
		{
			if (dataType == DataType::Integer)  // array subscript?
			{
				m_doneStack.pop();  // don't need item
			}
			else  // function argument
			{
				TokenPtr operandToken {m_doneStack.top().rpnItem->token()};
				// TODO may also need to check for DefFuncN type here
				if ((operandToken->isType(Token::Type::NoParen)
					|| operandToken->isType(Token::Type::Paren))
					&& !operandToken->hasSubCode(Paren_SubCode))
				{
					operandToken->setReference();
				}
			}
		}

		// check terminating token
		if (token->isCode(Comma_Code))
		{
			token.reset();  // delete comma token, it's not needed
		}
		else if (token->isCode(CloseParen_Code))
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
			m_doneStack.emplace(m_output.back(), token);

			m_holdStack.pop();
			token = std::move(topToken);  // return original token
			break;  // done
		}
		else  // unexpected token
		{
			throw TokenError {Status::ExpOpCommaOrParen, token};
		}
	}
}


// function to process an operator token received, all operators with higher
// precedence on the hold stack are added to output list first
//
// for each operator added to the output list, any pending parentheses is
// processed first, the final operand of the operator is processed
//
//    - sets last_precedence for operator being added to output list
//    - after higher precedence operators are processed from the hold stack,
//      a check for pending parethesis is made, and if the present token is
//      not a unary or binary operator, the end of the expression has been
//      reached and false is returned
//    - otherwise, the first operand of the operator is processed,
//      which also handles pushing the operator to the hold stack
//    - true is returned indicating operator successfully processed

bool Translator::processOperator(TokenPtr &token)
{
	// determine precedence of incoming token
	// (set highest precedence for unary operators,
	// which don't force other operators from hold stack)
	int tokenPrecedence {m_table.isUnaryOperator(token)
		? HighestPrecedence : m_table.precedence(token)};

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

	checkPendingParen(token, Popped::No);

	if (!m_table.isUnaryOrBinaryOperator(token))
	{
		return false;  // not unary or binary operator, end of expression
	}
	// otherwise process first operand and return
	TokenPtr first;			// first operand token (empty for unary operator)

	// check first operand of binary operators
	if (!m_table.isUnaryOperator(token))
	{
		// changed token operator code or insert conversion codes as needed
		processDoneStackTop(token, 0, &first);
	}

	// push it onto the holding stack and attach first operand
	m_holdStack.emplace(token, first);
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
//   - for command, token2 is not set

void Translator::processFinalOperand(TokenPtr &token, TokenPtr token2,
	int operandIndex)
{
	TokenPtr first;			// first operand token
	TokenPtr last;			// last operand token

	processDoneStackTop(token, operandIndex, &first, &last);

	if (token->isType(Token::Type::Operator))
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
	if (token->isType(Token::Type::Operator))
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
			m_output.append(m_table.newToken(cvtCode));
		}
	}
	else  // no match found, throw error
	{
		// use main code's expected data type for operand
		// (if no data type, then double constant can't be converted to integer)
		// report entire expression from first token through last
		throw TokenError {topToken->isDataType(DataType{})
			? Status::ExpIntConst : expectedErrStatus(topToken->dataType()),
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


// function to pop the top item from the done stack and return the token
// from the first to the last operand appropriate for an error token

TokenPtr Translator::doneStackPopErrorToken()
{
	TokenPtr token {std::move(m_doneStack.top().first)};
	if (!token)
	{
		// if no first operand token, set to token itself
		token = m_doneStack.top().rpnItem->token();
	}
	// if last operand token set, set error token through last token
	if (m_doneStack.top().last)
	{
		token->setThrough(m_doneStack.top().last);
	}
	m_doneStack.pop();
	return token;  // return error token
}


// function to return the token error status for an expected data type
// and reference type
Status Translator::expectedErrStatus(DataType dataType, Reference reference)
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
	Code code)
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
