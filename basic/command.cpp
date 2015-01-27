// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: command.cpp - other table commands source file
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

#include "command.h"


static Command Dim("DIM");
static Command Def("DEF");
static Command If("IF");
static Command Then("THEN");
static Command Else("ELSE");
static Command End("END");
static Command EndIf("END", "IF");
static Command For("FOR");
static Command To("TO");
static Command Step("STEP");
static Command Next("NEXT");
static Command Do("DO");
static Command DoWhile("DO", "WHILE");
static Command DoUntil("DO", "UNTIL");
static Command While("WHILE");
static Command Until("UNTIL");
static Command Loop("LOOP");
static Command LoopWhile("LOOP", "WHILE");
static Command LoopUntil("LOOP", "UNTIL");


// end: command.cpp
