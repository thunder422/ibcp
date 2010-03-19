// vim:ts=4:sw=4

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
//  1.0  2010-03-01  initial release

#include "ibcp.h"


// function to get a token at the current position
//
//     - a pointer to the token if returned
//     - the token must be dellocated when it is no longer needed
//     - the token may contain an error message if an error was found

void Translator::start(char *i)
{
	rpn_list = new List<Token *>;
	state = Initial;
	input = i;
}


Translator::Status Translator::add_token(Token *token)
{
	if (state == Initial)
	{
		// push null token to be last operator on stack
		// to prevent from popping past bottom of stack
		Token *null_token = new Token();
		table->set_token(token, Null_Code);
		op_stack.push(&token);

		state = Operand;
	}

}



// end: translator.cpp
