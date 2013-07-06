// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: let.cpp - let command functions source file
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

#include "translator.h"
#include "token.h"


TokenStatus letTranslate(Translator &t, Token *commandToken, Token *&token)
{
	TokenStatus status;

	if (commandToken != NULL)
	{
		token = commandToken;
		status = BUG_Debug1;
	}
	else
	{
		status = BUG_Debug2;
	}
	token->setSubCodeMask(UnUsed_SubCode);
	return status;
}


// end: let.cpp
