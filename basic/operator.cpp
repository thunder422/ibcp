// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: operator.cpp - operator table classes and functions source file
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

#include "operator.h"


SpecialOperator::SpecialOperator(Code code, const char *name, int precedence,
		unsigned moreFlags) :
	Table {code, name, "", "", Operator_Flag | moreFlags, precedence,
	   &Null_ExprInfo, No_OperandType}
{
}


// end: operator.cpp
