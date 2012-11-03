// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: tokenhandlers.h - token handler function definitions
//	Copyright (C) 2012  Thunder422
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
//	2012-11-03	initial version (parts removed from ibcp.h)

#ifndef TOKENHANDLERS_H
#define TOKENHANDLERS_H

class Translator;
struct Token;


// token handler function definitions section
extern TokenStatus Operator_Handler(Translator &t, Token *&token);
extern TokenStatus Equal_Handler(Translator &t, Token *&token);
extern TokenStatus Comma_Handler(Translator &t, Token *&token);
extern TokenStatus CloseParen_Handler(Translator &t, Token *&token);
extern TokenStatus EndOfLine_Handler(Translator &t, Token *&token);
extern TokenStatus SemiColon_Handler(Translator &t, Token *&token);


#endif  // TOKENHANDLERS_H
