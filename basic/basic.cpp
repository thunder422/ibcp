// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: basic.cpp - miscellaneous basic functions source file
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
//	2013-10-05	initial version

#include "dictionary.h"
#include "programmodel.h"
#include "token.h"


// REM FUNCTIONS

quint16 remEncode(ProgramUnit *programUnit, Token *token)
{
	return programUnit->remDictionary()->add(token);
}


QString remOperandText(ProgramUnit *programUnit, quint16 operand)
{
	return programUnit->remDictionary()->string(operand);
}


// CONSTANT FUNCTIONS

ConstNumInfo::ConstNumInfo(void)
{
	// default constructor needed for QVector
}

ConstNumInfo::ConstNumInfo(Token *token)
{
	m_value = token->value();
	m_valueInt = token->valueInt();
}

quint16 constNumEncode(ProgramUnit *programUnit, Token *token)
{
	return programUnit->constNumDictionary()->add(token);
}

QString constNumOperandText(ProgramUnit *programUnit, quint16 operand)
{
	return programUnit->constNumDictionary()->string(operand);
}


// VARIABLE FUNCTIONS

quint16 varDblEncode(ProgramUnit *programUnit, Token *token)
{
	return programUnit->varDblDictionary()->add(token);
}

quint16 varIntEncode(ProgramUnit *programUnit, Token *token)
{
	return programUnit->varIntDictionary()->add(token);
}

quint16 varStrEncode(ProgramUnit *programUnit, Token *token)
{
	return programUnit->varStrDictionary()->add(token);
}


QString varDblOperandText(ProgramUnit *programUnit, quint16 operand)
{
	return programUnit->varDblDictionary()->string(operand);
}

QString varIntOperandText(ProgramUnit *programUnit, quint16 operand)
{
	return programUnit->varIntDictionary()->string(operand);
}

QString varStrOperandText(ProgramUnit *programUnit, quint16 operand)
{
	return programUnit->varStrDictionary()->string(operand);
}


// end: variable.cpp
