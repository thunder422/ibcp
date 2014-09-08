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


Translator::Translator(void) :
	m_table(Table::instance()),
	m_parser(new Parser),
	m_pendingParen(NULL)
{

}


Translator::~Translator(void)
{
	delete m_parser;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                               MAIN FUNCTION                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


// function to parse and translate an input line to an RPN output list
//
//   - returns the RPN output list, which may contain an error instead
//     of translated line
//   - allows for a special expression mode for testing

RpnList Translator::translate(const QString &input, TestMode testMode)
{
	Token *token;
	Token::Status status;

	m_parser->setInput(input);

	m_holdStack.emplace(m_table.newToken(Null_Code));

	if (testMode == TestMode::Expression)
	{
		token = NULL;
		status = getExpression(token, DataType::Any);

		if (status == Token::Status::Parser && token->isDataType(DataType::None))
		{
			status = Token::Status::ExpOpOrEnd;
		}
		else if (status == Token::Status::Done)
		{
			// pop final result off of done stack
			if (m_doneStack.isEmpty())
			{
				status = Token::Status::BUG_DoneStackEmpty;
			}
			else
			{
				// drop result (delete any paren tokens in first/last operands)
				m_doneStack.drop();
			}
		}
	}
	else
	{
		status = getCommands(token);
	}

	if (status == Token::Status::Done)
	{
		if (token->isCode(EOL_Code))
		{
			delete token;  // delete EOL token

			// pop and delete null token from top of stack
			delete m_holdStack.top().token;
			m_holdStack.pop();

			if (!m_holdStack.empty())
			{
				status = Token::Status::BUG_HoldStackNotEmpty;
			}

			if (!m_doneStack.isEmpty())
			{
				status = Token::Status::BUG_DoneStackNotEmpty;
			}

			if (testMode == TestMode::No
				&& !m_output.setCodeSize(m_table, token))
			{
				status = Token::Status::BUG_NotYetImplemented;
			}
		}
		else
		{
			status = Token::Status::ExpOpOrEnd;
		}
	}

	if (status != Token::Status::Done)
	{
		// error token is in the output list - don't delete it
		m_output.setError(token);
		m_output.setErrorMessage(status == Token::Status::Parser
			? token->string() : token->message(status));
		if (token->hasSubCode(UnUsed_SubCode))
		{
			delete token;  // token not in output list, needs to be deleted
		}
		cleanUp();
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

Token::Status Translator::getCommands(Token *&token)
{
	Token::Status status;

	forever
	{
		if ((status = getToken(token)) != Token::Status::Good)
		{
			return Token::Status::ExpCmd;
		}

		if (token->isCode(EOL_Code) && m_output.empty())
		{
			return Token::Status::Done;  // blank line allowed
		}

		if (token->isCode(Rem_Code) || token->isCode(RemOp_Code))
		{
			break;
		}

		if ((status = processCommand(token)) != Token::Status::Done)
		{
			return status;
		}

		if (token->isCode(RemOp_Code))
		{
			break;
		}
		else if (token->isCode(Colon_Code))
		{
			// delete unneeded colon token, set colon sub-code on last token
			delete token;
			outputLastToken()->addSubCode(Colon_SubCode);
		}
		else  // unknown end statement token, return to caller
		{
			return Token::Status::Done;
		}
	}
	m_output.append(token);  // Rem or RemOp token
	if ((status = getToken(token)) != Token::Status::Good)
	{
		return Token::Status::BUG_UnexpToken;  // parser problem, should be EOL
	}
	return Token::Status::Done;
}


// function to get an expression from the input line
//
//   - takes an already obtained token (gets a token if none)
//   - takes a data type argument for the desired data type of the expression
//   - returns Done_TokenStatus upon success
//   - returns an error status if an error was detected
//   - returns the token that terminated the expression

Token::Status Translator::getExpression(Token *&token, DataType dataType,
	int level)
{
	Token::Status status;
	DataType expectedDataType = dataType;

	forever
	{
		if (token == NULL
			&& (status = getToken(token, expectedDataType)) != Token::Status::Good)
		{
			break;
		}

		if (token->isCode(OpenParen_Code))
		{
			// push open parentheses onto hold stack to block waiting tokens
			// during the processing of the expression inside the parentheses
			m_holdStack.emplace(token);

			// get an expression and terminating token
			token = NULL;
			if (expectedDataType == DataType::None)
			{
				expectedDataType = DataType::Any;
			}
			if ((status = getExpression(token, expectedDataType, level + 1))
				!= Token::Status::Done)
			{
				if (m_table.isUnaryOperator(token))
				{
					status = Token::Status::ExpBinOpOrParen;
				}
				if (status == Token::Status::Parser
					&& token->isDataType(DataType::None))
				{
					status = Token::Status::ExpOpOrParen;
				}
				break;  // exit on error
			}

			// check terminating token
			if (!token->isCode(CloseParen_Code))
			{
				status = Token::Status::ExpOpOrParen;
				break;
			}

			// make sure holding stack contains the open parentheses
			Token *topToken = m_holdStack.top().token;
			m_holdStack.pop();
			if (!topToken->code() == OpenParen_Code)
			{
				// oops, no open parentheses (this should not happen)
				return Token::Status::BUG_UnexpectedCloseParen;
			}

			// replace first and last operands of token on done stack
			m_doneStack.replaceTopFirstLast(topToken, token);

			// mark close paren as used for last operand and pending paren
			// (so it doesn't get deleted until it's not used anymore)
			token->addSubCode(Last_SubCode + Used_SubCode);

			// set highest precedence if not an operator on done stack top
			// (no operators in the parentheses)
			topToken = m_doneStack.top().rpnItem->token();
			m_lastPrecedence = topToken->isType(Token::Type::Operator)
				? m_table.precedence(topToken) : HighestPrecedence;

			// set pending parentheses token pointer
			m_pendingParen = token;

			token = NULL;  // get another token
		}
		else
		{
			Code unaryCode;
			if ((unaryCode = m_table.unaryCode(token)) != Null_Code)
			{
				token->setCode(unaryCode);  // change token to unary operator
			}
			// get operand
			else if ((status = getOperand(token, expectedDataType))
				!= Token::Status::Good)
			{
				break;
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
				token = NULL;  // get another token
			}
		}
		if (token == NULL)
		{
			// get binary operator or end-of-expression token
			if ((status = getToken(token)) != Token::Status::Good)
			{
				// if parser error then caller needs to handle it
				break;
			}
			if (doneStackTopToken()->isDataType(DataType::None)
				&& m_holdStack.top().token->isNull()
				&& dataType == DataType::None)
			{
				// for print functions return now with token as terminator
				status = Token::Status::Done;
				break;
			}
			// check for unary operator (token should be a binary operator)
			if (m_table.isUnaryOperator(token))
			{
				status = Token::Status::ExpBinOpOrEnd;
				break;
			}
		}

		// check for and process operator (unary or binary)
		status = processOperator(token);
		if (status == Token::Status::Done)
		{
			if (level == 0)
			{
				// add convert code if needed or report error
				Token *doneToken = m_doneStack.top().rpnItem->token();
				Code cvtCode = doneToken->convertCode(dataType);
				if (cvtCode == Invalid_Code)
				{
					delete token;  // delete terminating token
					token = doneStackPopErrorToken();
					status = expectedErrStatus(dataType);
				}
				else if (cvtCode != Null_Code)
				{
					if (doneToken->isType(Token::Type::Constant))
					{
						// constants don't need conversion
						if (dataType == DataType::Integer
							&& doneToken->isDataType(DataType::Double))
						{
							delete token;  // delete terminating token
							token = doneStackPopErrorToken();
							status = Token::Status::ExpIntConst;
						}
					}
					else  // append hidden conversion code
					{
						m_output.append(m_table.newToken(cvtCode));
					}
				}
			}
			break;
		}
		else if (status != Token::Status::Good)
		{
			break;
		}

		// get operator's expected data type, reset token and loop back
		expectedDataType = m_table.expectedDataType(token);
		token = NULL;
	}
	return status;
}


// function to get an operand
//
//   - a token may be passed in, otherwise a token is obtained
//   - the data type argument is used for reporting the correct error when
//     the token is not a valid operand token

Token::Status Translator::getOperand(Token *&token, DataType dataType,
	Reference reference)
{
	Token::Status status;
	bool doneAppend = true;

	// get token if none was passed
	if (token == NULL
		&& (status = getToken(token, dataType)) != Token::Status::Good)
	{
		if (reference == Reference::None)
		{
			// if parser error then caller needs to handle it
			return status;
		}
		if (status == Token::Status::Parser)
		{
			token->setLength(1);  // only report error at first char of token
		}
		return expectedErrStatus(dataType, reference);
	}

	// set default data type for token if it has none
	token->setDataType();

	switch (token->type())
	{
	case Token::Type::Command:
	case Token::Type::Operator:
		if (dataType == DataType::None)
		{
			// nothing is acceptable, this is terminating token
			return Token::Status::Done;
		}
		return expectedErrStatus(dataType, reference);

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
			return expectedErrStatus(dataType, reference);
		}
		break;  // go add token to output and push to done stack

	case Token::Type::IntFuncN:
		if (reference != Reference::None)
		{
			return expectedErrStatus(dataType, reference);
		}
		break;  // go add token to output and push to done stack

	case Token::Type::DefFuncN:
		if (reference == Reference::Variable)
		{
			return expectedErrStatus(dataType, reference);
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
				return expectedErrStatus(dataType, reference);
			}
		}
		else if (token->isDataType(DataType::None)
			&& dataType != DataType::None)
		{
			return expectedErrStatus(dataType);
		}
		if ((status = processInternalFunction(token)) != Token::Status::Good)
		{
			// drop and delete function token since it was not used
			delete m_holdStack.top().token;
			m_holdStack.pop();
			return status;
		}
		// reset reference if it was set above, no longer needed
		token->setReference(false);
		doneAppend = false;  // already appended
		break;

	case Token::Type::DefFuncP:
		if (reference == Reference::Variable)
		{
			return expectedErrStatus(dataType, reference);
		}
		else if (reference != Reference::None)
		{
			// NOTE these are allowed in the DEF command
			// just point to the open parentheses of the token
			token->addLengthToColumn();
			token->setLength(1);
			return Token::Status::ExpEqualOrComma;
		}
		if ((status = processParenToken(token)) != Token::Status::Good)
		{
			// drop and delete define function token since it was not used
			delete m_holdStack.top().token;
			m_holdStack.pop();
			return status;
		}
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
		if ((status = processParenToken(token)) != Token::Status::Good)
		{
			// drop and delete parentheses token since it was not used
			delete m_holdStack.top().token;
			m_holdStack.pop();
			return status;
		}
		doneAppend = false;  // already appended
		break;

	default:
		return Token::Status::BUG_NotYetImplemented;
	}

	if (doneAppend)
	{
		// add token directly to output list
		// and push element pointer on done stack
		m_doneStack.push(m_output.append(token));
	}
	// for reference, check data type
	if (reference != Reference::None
		&& token->convertCode(dataType) != Null_Code)
	{
		token = doneStackPopErrorToken();
		return expectedErrStatus(dataType, reference);
	}
	return Token::Status::Good;
}


// function to get a token from the parser
//
//   - data type argument determines if token to get is an operand
//   - returns Parser_TokenStatus if the parser returned an error

Token::Status Translator::getToken(Token *&token, DataType dataType)
{
	// if data type is not none, then getting an operand token
	bool operand = dataType != DataType{};
	token = m_parser->token(operand);
	token->addSubCode(UnUsed_SubCode);
	if (token->isType(Token::Type::Error))
	{
		if ((!operand && token->dataType() == DataType::Double)
			|| dataType == DataType::String)
		{
			// only do this for non-operand number constant errors
			token->setLength(1);  // just point to first character
			token->setDataType(DataType::None);  // indicate not a number error
		}
		if (operand && ((token->dataType() != DataType::Double
			&& dataType != DataType::None) || dataType == DataType::String))
		{
			// non-number constant error, return expected expression error
			return expectedErrStatus(dataType);
		}
		else
		{
			// caller needs to convert this error to the appropriate error
			return Token::Status::Parser;
		}
	}
	return Token::Status::Good;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                             PROCESS FUNCTIONS                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


// function to process a command in the input line
//
//   - returns Done_TokenStatus upon success
//   - returns an error status if an error was detected
//   - returns the token that terminated the command

Token::Status Translator::processCommand(Token *&commandToken)
{
	TranslateFunction translate;
	Token *token;

	if (commandToken->isType(Token::Type::Command))
	{
		translate = m_table.translateFunction(commandToken->code());
		token = NULL;  // force translate function to get a token
	}
	else  // assume an assignment statement
	{
		translate = m_table.translateFunction(Let_Code);
		token = commandToken;
		commandToken = NULL;
		// pass token onto let translate function
	}
	if (translate == NULL)
	{
		return Token::Status::BUG_NotYetImplemented;
	}
	Token::Status status = (*translate)(*this, commandToken, token);
	commandToken = token;
	return status;
}


// function to get and process an internal function's arguments
//
//   - the token argument contains the internal function token
//   - the token argument contains the token when an error is detected

Token::Status Translator::processInternalFunction(Token *&token)
{
	Token::Status status;
	DataType expectedDataType;
	bool unaryOperator = false;

	// push internal function token onto hold stack to block waiting tokens
	// during the processing of the expressions of each argument
	m_holdStack.emplace(token);
	Token *topToken = token;

	Code code = token->code();
	int lastOperand = m_table.operandCount(code) - 1;
	for (int i = 0; ; i++)
	{
		token = NULL;
		if (i == 0 && topToken->reference())
		{
			// sub-string assignment, look for reference operand
			expectedDataType = DataType::String;
			status = getOperand(token, expectedDataType, Reference::VarDefFn);
			if (status == Token::Status::Good)
			{
				if ((status = getToken(token)) == Token::Status::Good)
				{
					status = Token::Status::Done;
				}
				else
				{
					status = Token::Status::ExpComma;
				}
			}
		}
		else
		{
			if (i == 0)
			{
				expectedDataType = m_table.expectedDataType(topToken);
			}
			else
			{
				expectedDataType = m_table.operandDataType(code, i);
			}
			status = getExpression(token, expectedDataType);
		}

		if (status == Token::Status::Done)
		{
			// check if associated code for function is needed
			if (expectedDataType == DataType::Number)
			{
				Token *doneToken = m_doneStack.top().rpnItem->token();
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
		}
		else if (status == Token::Status::Parser)
		{
			if (token->isDataType(DataType::Double))
			{
				return status;  // return parser error
			}
		}
		else if (m_table.isUnaryOperator(token))
		{
			unaryOperator = true;
		}
		else
		{
			return status;
		}
		// pass other parser errors and unary operators to error code below

		// check terminating token
		if (token->isCode(Comma_Code))
		{
			if (i == lastOperand)
			{
				if (!m_table.hasFlag(code, Multiple_Flag))
				{
					// function doesn't have multiple entries
					status = Token::Status::ExpOpOrParen;
					break;
				}
				// move to next code; update code and last operand index
				code = topToken->nextCode();
				lastOperand = m_table.operandCount(code) - 1;
			}
			delete token;  // delete comma token, it's not needed
			m_doneStack.drop();
		}
		else if (token->isCode(CloseParen_Code))
		{
			if (i < lastOperand)
			{
				status = Token::Status::ExpOpOrComma;
				break;
			}

			m_doneStack.drop();  // remove from done stack (remove paren tokens)

			// add token to output list if not sub-string assignment
			RpnItemPtr rpnItem = topToken->reference()
				? RpnItemPtr{new RpnItem{topToken}} : m_output.append(topToken);

			// push internal function to done stack
			m_doneStack.push(rpnItem, NULL, token);

			m_holdStack.pop();
			token = topToken;  // return original token
			return Token::Status::Good;
		}
		// unexpected token, determine appropriate error
		else
		{
			if (i == 0 && topToken->reference())
			{
				status = Token::Status::ExpComma;
			}
			else if (i < lastOperand)
			{
				status = unaryOperator ? Token::Status::ExpBinOpOrComma
					: Token::Status::ExpOpOrComma;
			}
			else if (!m_table.hasFlag(code, Multiple_Flag))
			{
				// function doesn't have multiple entries
				status = unaryOperator ? Token::Status::ExpBinOpOrParen
					: Token::Status::ExpOpOrParen;
			}
			else
			{
				status = unaryOperator ? Token::Status::ExpBinOpCommaOrParen
					: Token::Status::ExpOpCommaOrParen;
			}
			break;
		}
	}
	return status;
}


// function to get and process a parentheses token's arguments
//
//   - the token argument contains the token with parentheses
//   - the token argument contains the token when an error is detected

Token::Status Translator::processParenToken(Token *&token)
{
	Token::Status status;

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
		isArray = !token->string().startsWith('F', Qt::CaseInsensitive);
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
	Token *topToken = token;

	for (int count = 1; ; count++)
	{
		token = NULL;
		if ((status = getExpression(token, dataType)) != Token::Status::Done)
		{
			if (status == Token::Status::Parser
				&& token->isDataType(DataType::None))
			{
				status = Token::Status::ExpOpCommaOrParen;
			}
			else if (m_table.isUnaryOperator(token))
			{
				status = Token::Status::ExpBinOpCommaOrParen;
			}
			return status;
		}

		// set reference for appropriate token types
		if (topToken->isType(Token::Type::Paren))
		{
			if (dataType == DataType::Integer)  // array subscript?
			{
				m_doneStack.drop();  // don't need item
			}
			else  // function argument
			{
				Token *operandToken = m_doneStack.top().rpnItem->token();
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
			delete token;  // delete comma token, it's not needed
		}
		else if (token->isCode(CloseParen_Code))
		{
			RpnItemVector attached;
			// save operands for storage in output list
			while (--count >= 0)
			{
				// if array subscript then apppend empty pointer
				// else pop and append operands
				attached.emplace_back(dataType == DataType::Integer
					? RpnItemPtr{} : m_doneStack.pop());
					// TODO will need to keep first/last operands for each
					// TODO (in case expression needs to be reported as error)
					// TODO (RpnItem should have DoneItem operands, not RpnItem)
			}

			// add token to output list and push element pointer on done stack
			m_doneStack.push(m_output.append(topToken, attached), NULL, token);

			m_holdStack.pop();
			token = topToken;  // return original token
			return Token::Status::Good;
		}
		else  // unexpected token
		{
			return Token::Status::ExpOpCommaOrParen;
		}
	}
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
//      an end of expression token is checked for, which causes a done status
//      to be returned
//    - otherwise, the first operand of the operator is processed, which also
//      handles pushing the operator to the hold stack

Token::Status Translator::processOperator(Token *&token)
{
	// determine precedence of incoming token
	// (set highest precedence for unary operators,
	// which don't force other operators from hold stack)
	int tokenPrecedence = m_table.isUnaryOperator(token)
		? HighestPrecedence : m_table.precedence(token);

	// process and drop unary or binary operators on hold stack
	// while they have higher or same precedence as incoming token
	Token *topToken;
	while (m_table.precedence(topToken = m_holdStack.top().token)
		>= tokenPrecedence && m_table.isUnaryOrBinaryOperator(topToken))
	{
		checkPendingParen(topToken, true);

		// change token operator code or insert conversion codes as needed
		Token::Status status = processFinalOperand(topToken,
			m_holdStack.top().first, m_table.isUnaryOperator(topToken) ? 0 : 1);

		if (status != Token::Status::Good)
		{
			delete token;
			token = topToken;  // return token with error
			return status;
		}

		// save precedence of operator being added
		// (doesn't matter if not currently within parentheses,
		// it will be reset upon next open parentheses)
		m_lastPrecedence = m_table.precedence(topToken->code());

		m_holdStack.pop();
	}

	checkPendingParen(token, false);

	// if a unary or binary operator then process first operand and return
	if (m_table.isUnaryOrBinaryOperator(token))
	{
		return processFirstOperand(token);
	}

	return Token::Status::Done;  // otherwise, end of expression
}


// function to process the first operand for an operator
//
//  - called from processOperator()
//  - attaches first operand token from operand on top of done stack
//  - no first operand token for unary operators

Token::Status Translator::processFirstOperand(Token *&token)
{
	Token *first = NULL;		// first operand token

	// check first operand of binary operators
	if (!m_table.isUnaryOperator(token))
	{
		// changed token operator code or insert conversion codes as needed
		Token *orgToken = token;
		Token::Status status = processDoneStackTop(token, 0, &first);
		if (status != Token::Status::Good)
		{
			// check if different token has error
			if (token != orgToken)
			{
				delete orgToken;  // delete operator token
			}
			return status;
		}
	}

	// push it onto the holding stack and attach first operand
	m_holdStack.emplace(token, first);

	return Token::Status::Good;
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
//   - for internal code, token2 is NULL

Token::Status Translator::processFinalOperand(Token *&token, Token *token2,
	int operandIndex)
{
	Token *first;			// first operand token
	Token *last;			// last operand token

	Token::Status status = processDoneStackTop(token, operandIndex, &first,
		&last);
	if (status != Token::Status::Good)
	{
		return status;
	}

	DoneItem::deleteOpenParen(first);  // don't need first operand
	if (token->isType(Token::Type::Operator))
	{
		// set first token
		// unary operator: unary operator token
		// binary operator: first operator of operator (token2)
		first = operandIndex == 0 ? token : token2;
		// last operand set from token that was on done stack
	}
	else  // internal code (won't be pushed to done stack)
	{
		DoneItem::deleteCloseParen(last);  // don't need last operand
	}

	// add token to output list
	RpnItemPtr rpnItem = m_output.append(token);

	// push operator token to the done stack
	if (token->isType(Token::Type::Operator))
	{
		m_doneStack.push(rpnItem, first, last);
	}

	return Token::Status::Good;
}


// function to process the item on top of the done stack for a given operand
// of an operator or only operand of an internal code token
//
//   - if requested, the first and last operands of the item are returned
//   - calls Table::findCode() to check the data type of item:
//     changes token code to a matching associated code if available
//     else returns a conversion code if conversion is possible
//   - if no match found or conversion not possible, an error is returned
//   - if conversion possible, a conversion code token is appended to output

Token::Status Translator::processDoneStackTop(Token *&token, int operandIndex,
	Token **first, Token **last)
{
	if (m_doneStack.isEmpty())
	{
		// oops, there should have been operands on done stack
		return Token::Status::BUG_DoneStackEmptyFindCode;
	}
	Token *topToken = m_doneStack.top().rpnItem->token();
	// get first and last operands for top token
	Token *localFirst = m_doneStack.top().first;
	if (localFirst == NULL)
	{
		localFirst = topToken;  // first operand not set, set to operand token
	}
	if (first != NULL)  // requested by caller?
	{
		*first = localFirst;
		m_doneStack.top().first = NULL;  // prevent deletion below
	}
	Token *localLast = m_doneStack.top().last;
	if (localLast == NULL)
	{
		localLast = topToken;  // last operand not set, set to operand token
	}
	if (last != NULL)  // requested by caller?
	{
		*last = localLast;
		m_doneStack.top().last = NULL;  // prevent deletion below
	}

	// see if main code's data type matches
	Code cvtCode = m_table.findCode(token, topToken, operandIndex);

	if (cvtCode != Invalid_Code)
	{
		m_doneStack.drop();  // remove from done stack (remove paren tokens)

		// is there an actual conversion code to insert?
		if (cvtCode != Null_Code)
		{
			// INSERT CONVERSION CODE
			// create convert token with convert code
			// append token to end of output list (after operand)
			m_output.append(m_table.newToken(cvtCode));
		}

		return Token::Status::Good;
	}

	// no match found, report error
	// use main code's expected data type for operand
	// (if data type is No, then double constant can't be converted to integer)
	Token::Status status = topToken->isDataType(DataType{})
		? Token::Status::ExpIntConst : expectedErrStatus(topToken->dataType());

	// change token to token with invalid data type and return error
	// report entire expression
	token = localFirst->setThrough(localLast);

	// delete last token if close paren
	DoneItem::deleteCloseParen(localLast);

	return status;
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

void Translator::checkPendingParen(Token *token, bool popped)
{
	if (m_pendingParen != NULL)  // is a closing parentheses token pending?
	{
		// may need to add a dummy token if the precedence of the last
		// operator added within the last parentheses sub-expression
		// is higher than or same as (popped tokens only) the operator
		int precedence = m_table.precedence(token);
		if (m_lastPrecedence > precedence
			|| (!popped && m_lastPrecedence == precedence))
		{
			Token *lastToken = m_output.lastToken();
			if (!lastToken->hasSubCode(Paren_SubCode))
			{
				// mark last code for unnecessary parentheses
				lastToken->addSubCode(Paren_SubCode);
			}
			else   // already has parentheses sub-code set
			{
				// add dummy token
				m_output.append(m_pendingParen);
				// reset pending token and return (don't delete token)
				m_pendingParen = NULL;
				return;
			}
		}
		// check if being used as last operand before deleting
		if (m_pendingParen->hasSubCode(Last_SubCode))
		{
			// still used as last operand token, just clear used flag
			m_pendingParen->removeSubCode(Used_SubCode);
		}
		else  // don't need pending token
		{
			delete m_pendingParen;  // release it's memory
		}
		m_pendingParen = NULL;  // reset pending token
	}
}


// function to pop the top item from the done stack and return the token
// from the first to the last operand appropriate for an error token

Token *Translator::doneStackPopErrorToken(void)
{
	Token *token = m_doneStack.top().first;
	if (token == NULL)
	{
		// if no first operand token, set to token itself
		token = m_doneStack.top().rpnItem->token();
	}
	else
	{
		m_doneStack.top().first = NULL;  // prevent delete when top dropped
	}
	// if last operand token set, set error token through last token
	if (m_doneStack.top().last != NULL)
	{
		token->setThrough(m_doneStack.top().last);
	}
	m_doneStack.drop();  // (removes any paren tokens)
	return token;  // return error token
}


// function to return the token error status for an expected data type
// and reference type
Token::Status Translator::expectedErrStatus(DataType dataType,
	Reference reference)
{
	switch (dataType)
	{
	case DataType::Double:
		return reference == Reference::None
			? Token::Status::ExpNumExpr : Token::Status::ExpDblVar;
	case DataType::Integer:
		return reference == Reference::None
			? Token::Status::ExpNumExpr : Token::Status::ExpIntVar;
	case DataType::String:
		switch (reference)
		{
		case Reference::None:
			return Token::Status::ExpStrExpr;
		case Reference::Variable:
		case Reference::VarDefFn:
			return Token::Status::ExpStrVar;
		case Reference::All:
			return Token::Status::ExpStrItem;
		}
	case DataType::None:
		return reference == Reference::None
			? Token::Status::ExpExpr : Token::Status::ExpAssignItem;
	case DataType::Number:
		return reference == Reference::None
			? Token::Status::ExpNumExpr : Token::Status::BUG_InvalidDataType;
	case DataType::Any:
		switch (reference)
		{
		case Reference::None:
			return Token::Status::ExpExpr;
		case Reference::Variable:
			return Token::Status::ExpVar;
		case Reference::VarDefFn:
		case Reference::All:
			return Token::Status::ExpAssignItem;
		}
	}
	return Token::Status::BUG_InvalidDataType;  // won't get here
}


// function to clean up the Translator variables after an error is detected
//
//   - must be called after add_token() returns an error

void Translator::cleanUp(void)
{
	// clean up from error
	while (!m_holdStack.empty())
	{
		// delete first token in command stack item
		Token *token = m_holdStack.top().first;
		if (token != NULL)
		{
			DoneItem::deleteOpenParen(token);
		}
		// delete to free the token that was on the stack
		delete m_holdStack.top().token;
		m_holdStack.pop();
	}
	while (!m_doneStack.isEmpty())
	{
		m_doneStack.drop();  // remove from done stack (remove paren tokens)
	}

	// clear the RPN output list of all items
	m_output.clear();

	// need to delete pending parentheses
	if (m_pendingParen != NULL)
	{
		delete m_pendingParen;
		m_pendingParen = NULL;
	}
}


// end: translator.cpp
