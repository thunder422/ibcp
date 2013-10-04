// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: basic.h - basic directory definitions header file
//	Copyright (C) 2013  Thunder422
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
//	2013-07-06	initial version

#ifndef BASIC_H
#define BASIC_H

#include <QtGlobal>

#include "ibcp.h"

class Translator;
class Token;
class ProgramUnit;


// translate functions
TokenStatus inputTranslate(Translator &translator, Token *commandToken,
	Token *&token);
TokenStatus letTranslate(Translator &translator, Token *commandToken,
	Token *&token);
TokenStatus printTranslate(Translator &translator, Token *commandToken,
	Token *&token);


// encode function
quint16 RemEncode(ProgramUnit *programUnit, Token *token);


#endif // BASIC_H
