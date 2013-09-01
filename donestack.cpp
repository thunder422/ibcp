// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: donestack.cpp - translator done stack class source file
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
//	2013-07-21	initial version (parts removed from translator.h/.cpp)

#include "donestack.h"
#include "rpnlist.h"
#include "token.h"


// function to delete a opening parentheses token that is no longer being used
// as the first operand token of another token
//
//   - no action if first operand token pointer is not set
//   - no action if first operand token is not an open parentheses token

void DoneItem::deleteOpenParen(Token *token)
{
	if (token != NULL && token->isCode(OpenParen_Code))
	{
		delete token;  // delete OpenParen token of operand
	}
}


// function to delete a closing parentheses token that is no longer being used
// as the last operand token of another token
//
//   - no action if last operand token pointer is not set
//   - no action if last operand token is not a closing parentheses token
//   - if closing parentheses is being used by pending parentheses or in output
//     as a dummy parentheses token, then last operand flag is cleared
//   - if closing parentheses token is not being used, then it is deleted

void DoneItem::deleteCloseParen(Token *token)
{
	if (token != NULL && token->isCode(CloseParen_Code))
	{
		// check if parentheses token is still being used
		if (token->hasSubCode(Used_SubCode))
		{
			// no longer used as last token
			token->removeSubCode(Last_SubCode);
		}
		else  // not used, close parentheses token can be deleted
		{
			delete token;  // delete CloseParen token of operand
		}
	}
}


// end: donestack.cpp
