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

#include "programmodel.h"
#include "recreator.h"
#include "rpnlist.h"
#include "token.h"


// REM FUNCTIONS

quint16 remEncode(ProgramModel *programUnit, const TokenPtr &token)
{
	return programUnit->remDictionary()->add(token, Qt::CaseSensitive);
}

const QString remOperandText(const ProgramModel *programUnit, quint16 operand)
{
	return programUnit->remDictionary()->string(operand);
}

void remRemove(ProgramModel *programUnit, quint16 operand)
{
	programUnit->remDictionary()->remove(operand, Qt::CaseSensitive);
}


// CONSTANT FUNCTIONS

void ConstNumInfo::clear(void)
{
	m_value.clear();
	m_valueInt.clear();
}

void ConstNumInfo::addElement(void)
{
	m_value.resize(m_value.size() + 1);
	m_valueInt.resize(m_valueInt.size() + 1);
}

void ConstNumInfo::setElement(int index, const TokenPtr &token)
{
	m_value[index] = token->value();
	m_valueInt[index] = token->valueInt();
}

quint16 constNumEncode(ProgramModel *programUnit, const TokenPtr &token)
{
	return programUnit->constNumDictionary()->add(token);
}

const QString constNumOperandText(const ProgramModel *programUnit,
	quint16 operand)
{
	return programUnit->constNumDictionary()->string(operand);
}

void constNumRemove(ProgramModel *programUnit, quint16 operand)
{
	programUnit->constNumDictionary()->remove(operand);
}


void ConstStrInfo::clear(void)
{
	m_value.clear();
}

void ConstStrInfo::addElement(void)
{
	m_value.append(new QString);
}

void ConstStrInfo::setElement(int index, const TokenPtr &token)
{
	*m_value[index] = token->string();
}

void ConstStrInfo::clearElement(int index)
{
	m_value[index]->clear();
}

ConstStrInfo::~ConstStrInfo(void)
{
	for (int i {}; i < m_value.count(); i++)
	{
		delete m_value.at(i);
	}
}


quint16 constStrEncode(ProgramModel *programUnit, const TokenPtr &token)
{
	return programUnit->constStrDictionary()->add(token, Qt::CaseSensitive);
}

const QString constStrOperandText(const ProgramModel *programUnit,
	quint16 operand)
{
	return programUnit->constStrDictionary()->string(operand);
}

void constStrRemove(ProgramModel *programUnit, quint16 operand)
{
	programUnit->constStrDictionary()->remove(operand, Qt::CaseSensitive);
}

void constStrRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	// change all double quote to two double qoutes and surround with quotes
	QString string {rpnItem->token()->string()};
	string.replace('"', "\"\"");
	recreator.push('"' + string + '"');
}


// VARIABLE FUNCTIONS

quint16 varDblEncode(ProgramModel *programUnit, const TokenPtr &token)
{
	return programUnit->varDblDictionary()->add(token);
}

quint16 varIntEncode(ProgramModel *programUnit, const TokenPtr &token)
{
	return programUnit->varIntDictionary()->add(token);
}

quint16 varStrEncode(ProgramModel *programUnit, const TokenPtr &token)
{
	return programUnit->varStrDictionary()->add(token);
}


const QString varDblOperandText(const ProgramModel *programUnit,
	quint16 operand)
{
	return programUnit->varDblDictionary()->string(operand);
}

const QString varIntOperandText(const ProgramModel *programUnit,
	quint16 operand)
{
	return programUnit->varIntDictionary()->string(operand);
}

const QString varStrOperandText(const ProgramModel *programUnit,
	quint16 operand)
{
	return programUnit->varStrDictionary()->string(operand);
}


void varDblRemove(ProgramModel *programUnit, quint16 operand)
{
	programUnit->varDblDictionary()->remove(operand);
}

void varIntRemove(ProgramModel *programUnit, quint16 operand)
{
	programUnit->varIntDictionary()->remove(operand);
}

void varStrRemove(ProgramModel *programUnit, quint16 operand)
{
	programUnit->varStrDictionary()->remove(operand);
}


// end: variable.cpp
