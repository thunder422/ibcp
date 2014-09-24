// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: statusmessage.cpp - status message class source file
//	Copyright (C) 2014  Thunder422
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
//	2014-09-23	initial version (parts removed from token.cpp)

#include "statusmessage.h"


// function to convert status enumerator to translated string
const QString StatusMessage::text(Status status)
{
	switch (status)
	{
	case Status::Good:
		return tr("Good_TokenStatus (BUG)");
	case Status::Done:
		return tr("Done_TokenStatus (BUG)");
	case Status::Parser:
		return tr("Parser_TokenStatus (BUG)");
	case Status::ExpCmd:
		return tr("expected command");
	case Status::ExpExpr:
		return tr("expected expression");
	case Status::ExpExprOrEnd:
		return tr("expected expression or end-of-statement");
	case Status::ExpOpOrEnd:
		return tr("expected operator or end-of-statement");
	case Status::ExpBinOpOrEnd:
		return tr("expected binary operator or end-of-statement");
	case Status::ExpEqualOrComma:
		return tr("expected equal or comma for assignment");
	case Status::ExpComma:
		return tr("expected comma");
	case Status::ExpAssignItem:
		return tr("expected item for assignment");
	case Status::ExpOpOrComma:
		return tr("expected operator or comma");
	case Status::ExpOpCommaOrParen:
		return tr("expected operator, comma or closing parentheses");
	case Status::ExpOpOrParen:
		return tr("expected operator or closing parentheses");
	case Status::ExpBinOpOrComma:
		return tr("expected binary operator or comma");
	case Status::ExpBinOpCommaOrParen:
		return tr("expected binary operator, comma or closing parentheses");
	case Status::ExpBinOpOrParen:
		return tr("expected binary operator or closing parentheses");
	case Status::ExpNumExpr:
		return tr("expected numeric expression");
	case Status::ExpStrExpr:
		return tr("expected string expression");
	case Status::ExpSemiCommaOrEnd:
		return tr("expected semicolon, comma or end-of-statement");
	case Status::ExpCommaSemiOrEnd:
		return tr("expected comma, semicolon or end-of-statement");
	case Status::ExpSemiOrComma:
		return tr("expected semicolon or comma");
	case Status::ExpOpSemiOrComma:
		return tr("expected operator, semicolon or comma");
	case Status::ExpDblVar:
		return tr("expected double variable");
	case Status::ExpIntVar:
		return tr("expected integer variable");
	case Status::ExpStrVar:
		return tr("expected string variable");
	case Status::ExpVar:
		return tr("expected variable");
	case Status::ExpStrItem:
		return tr("expected string item for assignment");
	case Status::ExpEndStmt:
		return tr("expected end-of-statement");
	case Status::ExpExprPfnOrEnd:
		return tr("expected expression, print function or end-of-statement");
	case Status::ExpExprCommaPfnOrEnd:
		return tr("expected expression, comma, print function or "
			"end-of-statement");
	case Status::ExpOpSemiCommaOrEnd:
		return tr("expected operator, semicolon, comma or end-of-statement");
	case Status::ExpIntConst:
		return tr("expected valid integer constant");
	// the following statuses are parser errors
	case Status::UnrecognizedChar:
		return tr("unrecognizable character");
	case Status::ExpNonZeroDigit:
		return tr("expected non-zero leading digit in numeric constant");
	case Status::ExpDigitsOrSngDP:
		return tr("expected digits or single decimal point in floating point "
			"constant");
	case Status::ExpManDigits:
		return tr("expected digits in mantissa of floating point constant");
	case Status::ExpExpDigits:
		return tr("expected sign or digits for exponent in floating point "
			"constant");
	case Status::ExpDigits:
		return tr("expected digits in floating point constant");
	case Status::FPOutOfRange:
		return tr("floating point constant is out of range");
	// the following statuses used during development
	case Status::BUG_NotYetImplemented:
		return tr("BUG: not yet implemented");
	case Status::BUG_HoldStackNotEmpty:
		return tr("BUG: hold stack not empty");
	case Status::BUG_DoneStackNotEmpty:
		return tr("BUG: done stack not empty");
	case Status::BUG_DoneStackEmptyFindCode:
		return tr("BUG: done stack empty - find code");
	case Status::BUG_UnexpectedCloseParen:
		return tr("BUG: unexpected closing parentheses");
	case Status::BUG_DoneStackEmpty:
		return tr("BUG: expected operand on done stack");
	case Status::BUG_InvalidDataType:
		return tr("BUG: invalid data type");
	case Status::BUG_UnexpToken:
		return tr("BUG: unexpected token");
	case Status::BUG_Debug1:
		return tr("BUG: debug #1");
	case Status::BUG_Debug2:
		return tr("BUG: debug #2");
	case Status::BUG_Debug3:
		return tr("BUG: debug #3");
	case Status::BUG_Debug4:
		return tr("BUG: debug #4");
	case Status::BUG_Debug5:
		return tr("BUG: debug #5");
	case Status::BUG_Debug6:
		return tr("BUG: debug #6");
	case Status::BUG_Debug7:
		return tr("BUG: debug #7");
	case Status::BUG_Debug8:
		return tr("BUG: debug #8");
	case Status::BUG_Debug9:
		return tr("BUG: debug #9");
	case Status::BUG_Debug:
		return tr("BUG: debug");
	}
	return "";
}


// end: statusmessage.cpp
