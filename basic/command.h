// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: command.h - command table intermediate class header file
//	Copyright (C) 2015  Thunder422
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

#ifndef COMMAND_H
#define COMMAND_H

#include "table.h"

extern ExprInfo Null_ExprInfo;


class Command : public Table
{
public:
	Command(const char *name, const char *name2 = "") :
		Table(Code{}, name, name2, "", Command_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL) {}

	virtual void translate(Translator &translator) override;
};


#endif  // COMMAND_H
