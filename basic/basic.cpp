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

uint16_t remEncode(ProgramModel *programUnit, const TokenPtr &token)
{
	return programUnit->remDictionary()->add(token);
}

const std::string remOperandText(const ProgramModel *programUnit,
	uint16_t operand, SubCode subCode)
{
	(void)subCode;
	return programUnit->remDictionary()->string(operand);
}

void remRemove(ProgramModel *programUnit, uint16_t operand)
{
	programUnit->remDictionary()->remove(operand);
}


// CONSTANT FUNCTIONS

void ConstNumInfo::clear(void)
{
	m_value.clear();
	m_valueInt.clear();
}

void ConstNumInfo::addElement(const TokenPtr &token)
{
	m_value.emplace_back(token->value());
	m_valueInt.emplace_back(token->valueInt());
}

void ConstNumInfo::setElement(int index, const TokenPtr &token)
{
	m_value[index] = token->value();
	m_valueInt[index] = token->valueInt();
}

uint16_t constNumEncode(ProgramModel *programUnit, const TokenPtr &token)
{
	return programUnit->constNumDictionary()->add(token);
}

const std::string constNumOperandText(const ProgramModel *programUnit,
	uint16_t operand, SubCode subCode)
{
	(void)subCode;
	return programUnit->constNumDictionary()->string(operand);
}

void constNumRemove(ProgramModel *programUnit, uint16_t operand)
{
	programUnit->constNumDictionary()->remove(operand);
}


void ConstStrInfo::clear(void)
{
	m_value.clear();
}

void ConstStrInfo::addElement(const TokenPtr &token)
{
	m_value.emplace_back(new std::string {token->string()});
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
	for (auto string : m_value)
	{
		delete string;
	}
}


uint16_t constStrEncode(ProgramModel *programUnit, const TokenPtr &token)
{
	return programUnit->constStrDictionary()->add(token);
}

const std::string constStrOperandText(const ProgramModel *programUnit,
	uint16_t operand, SubCode subCode)
{
	(void)subCode;
	return programUnit->constStrDictionary()->string(operand);
}

void constStrRemove(ProgramModel *programUnit, uint16_t operand)
{
	programUnit->constStrDictionary()->remove(operand);
}

void constStrRecreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	// change all double quote to two double qoutes and surround with quotes
	std::string string(1, '"');
	for (auto c : rpnItem->token()->string())
	{
		string += c;
		if (c == '"')
		{
			string += c;
		}
	}
	string += '"';
	recreator.emplace(string);
}


// VARIABLE FUNCTIONS

uint16_t varDblEncode(ProgramModel *programUnit, const TokenPtr &token)
{
	return programUnit->varDblDictionary()->add(token);
}

uint16_t varIntEncode(ProgramModel *programUnit, const TokenPtr &token)
{
	return programUnit->varIntDictionary()->add(token);
}

uint16_t varStrEncode(ProgramModel *programUnit, const TokenPtr &token)
{
	return programUnit->varStrDictionary()->add(token);
}


const std::string varDblOperandText(const ProgramModel *programUnit,
	uint16_t operand, SubCode subCode)
{
	std::string string = programUnit->varDblDictionary()->string(operand);
	if (subCode & Double_SubCode)
	{
		string.push_back('#');
	}
	return string;
}

const std::string varIntOperandText(const ProgramModel *programUnit,
	uint16_t operand, SubCode subCode)
{
	std::string string = programUnit->varIntDictionary()->string(operand);
	if (!(subCode & Ignore_SubCode))
	{
		string.push_back('%');
	}
	return string;
}

const std::string varStrOperandText(const ProgramModel *programUnit,
	uint16_t operand, SubCode subCode)
{
	std::string string = programUnit->varStrDictionary()->string(operand);
	if (!(subCode & Ignore_SubCode))
	{
		string.push_back('$');
	}
	return string;
}


void varDblRemove(ProgramModel *programUnit, uint16_t operand)
{
	programUnit->varDblDictionary()->remove(operand);
}

void varIntRemove(ProgramModel *programUnit, uint16_t operand)
{
	programUnit->varIntDictionary()->remove(operand);
}

void varStrRemove(ProgramModel *programUnit, uint16_t operand)
{
	programUnit->varStrDictionary()->remove(operand);
}


// end: variable.cpp
