// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: operator.h - operator table classes header file
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

#ifndef OPERATOR_H
#define OPERATOR_H

#include "table.h"

extern ExprInfo Null_ExprInfo;


class SpecialOperator : public Table
{
public:
	SpecialOperator(Code code, const char *name, int precedence,
			unsigned moreFlags = {});
};

#endif // OPERATOR_H
