// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: basic.cpp - miscellaneous basic functions source file
//	Copyright (C) 2013-2015  Thunder422
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

#include "programmodel.h"
#include "recreator.h"
#include "rpnlist.h"
#include "token.h"


// TODO do nothing vitual encode will be default table virtual encode function
void Table::encode(ProgramModel *programUnit,
	ProgramCode::BackInserter backInserter, Token *token)
{
	if (isCodeWithOperand())
	{
		// TODO move to code with operands class virtual encode function
		backInserter = programUnit->dictionary(m_operandType)->add(token).first;
	}
}


// REM FUNCTIONS

const std::string remOperandText(ProgramLineReader &programLineReader)
{
	auto operand = programLineReader();
	return programLineReader.unit()->remDictionary()->string(operand);
}

void remRemove(ProgramLineReader &programLineReader)
{
	programLineReader.unit()->remDictionary()->remove(programLineReader());
}


// CONSTANT FUNCTIONS

void ConstNumInfo::clear(void)
{
	m_value.clear();
	m_valueInt.clear();
}

void ConstNumInfo::addElement(Token *token)
{
	m_value.emplace_back(token->value());
	m_valueInt.emplace_back(token->valueInt());
}

void ConstNumInfo::setElement(int index, Token *token)
{
	m_value[index] = token->value();
	m_valueInt[index] = token->valueInt();
}

const std::string constNumOperandText(ProgramLineReader &programLineReader)
{
	auto operand = programLineReader();
	return programLineReader.unit()->constNumDictionary()->string(operand);
}

void constNumRemove(ProgramLineReader &programLineReader)
{
	programLineReader.unit()->constNumDictionary()->remove(programLineReader());
}


void ConstStrInfo::clear(void)
{
	m_value.clear();
}

void ConstStrInfo::addElement(Token *token)
{
	m_value.emplace_back(new std::string {token->string()});
}

void ConstStrInfo::setElement(int index, Token *token)
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


const std::string constStrOperandText(ProgramLineReader &programLineReader)
{
	auto operand = programLineReader();
	return programLineReader.unit()->constStrDictionary()->string(operand);
}

void constStrRemove(ProgramLineReader &programLineReader)
{
	programLineReader.unit()->constStrDictionary()->remove(programLineReader());
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

const std::string varDblOperandText(ProgramLineReader &programLineReader)
{
	auto operand = programLineReader();
	return programLineReader.unit()->varDblDictionary()->string(operand);
}

const std::string varIntOperandText(ProgramLineReader &programLineReader)
{
	auto operand = programLineReader();
	return programLineReader.unit()->varIntDictionary()->string(operand);
}

const std::string varStrOperandText(ProgramLineReader &programLineReader)
{
	auto operand = programLineReader();
	return programLineReader.unit()->varStrDictionary()->string(operand);
}


void varDblRemove(ProgramLineReader &programLineReader)
{
	programLineReader.unit()->varDblDictionary()->remove(programLineReader());
}

void varIntRemove(ProgramLineReader &programLineReader)
{
	programLineReader.unit()->varIntDictionary()->remove(programLineReader());
}

void varStrRemove(ProgramLineReader &programLineReader)
{
	programLineReader.unit()->varStrDictionary()->remove(programLineReader());
}


// end: variable.cpp
