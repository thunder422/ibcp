// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: commandhandlers.h - command handler function definitions
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

#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

class Translator;
struct CmdItem;
struct Token;


// command handler function definitions
extern TokenStatus Assign_CmdHandler(Translator &t, CmdItem *cmd_item,
	Token *token);
extern TokenStatus Print_CmdHandler(Translator &t, CmdItem *cmd_item,
	Token *token);
extern TokenStatus Let_CmdHandler(Translator &t, CmdItem *cmd_item,
	Token *token);
extern TokenStatus Input_CmdHandler(Translator &t, CmdItem *cmd_item,
	Token *token);


#endif  // COMMANDHANDLER_H