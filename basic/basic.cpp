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
#include "programreader.h"
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


const std::string Table::operandText(ProgramReader &programReader)
{
	if (not isCodeWithOperand())
	{
		// TODO move to be default table virtual operand text function
		return {};
	}
	// TODO move to code with operands class virtual encode function
	return programReader.getStringForOperand(m_operandType);
}


// TODO do nothing vitual remove will be default table virtual remove function
void Table::remove(ProgramReader &programReader)
{
	if (isCodeWithOperand())
	{
		// TODO move to code with operands class virtual remove function
		programReader.removeReferenceToOperand(m_operandType);
	}
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


// end: variable.cpp
