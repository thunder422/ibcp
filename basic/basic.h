// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: basic.h - basic directory definitions header file
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
//	2013-07-06	initial version

#ifndef BASIC_H
#define BASIC_H

#include <memory>
#include <string>
#include <vector>

#include "ibcp.h"
#include "dictionary.h"
#include "programcode.h"

class Translator;
class Token;
using TokenPtr = std::shared_ptr<Token>;
class ProgramModel;
class Recreator;
class RpnItem;
using RpnItemPtr = std::shared_ptr<RpnItem>;


// constant definitions

class ConstNumInfo : public AbstractInfo
{
public:
	void clear(void) override;
	void addElement(Token *token) override;
	void setElement(int index, Token *token) override;

	std::vector<double> &array(void)
	{
		return m_value;
	}
	std::vector<int> &arrayInt(void)
	{
		return m_valueInt;
	}

private:
	std::vector<double> m_value;			// vector of double values
	std::vector<int> m_valueInt;			// vector of integer values
};

class ConstNumDictionary : public InfoDictionary
{
public:
	ConstNumDictionary(void)
	{
		m_info.reset(new ConstNumInfo);
	}

	std::vector<double> &array(void)
	{
		return (dynamic_cast<ConstNumInfo *>(m_info.get()))->array();
	}
	std::vector<int> &arrayInt(void)
	{
		return (dynamic_cast<ConstNumInfo *>(m_info.get()))->arrayInt();
	}
};


class ConstStrInfo : public AbstractInfo
{
public:
	~ConstStrInfo(void) override;
	void clear(void) override;
	void addElement(Token *token) override;
	void setElement(int index, Token *token) override;
	void clearElement(int index) override;

	std::vector<std::string *> &array(void)
	{
		return m_value;
	}

private:
	std::vector<std::string *> m_value;		// vector of string instances
};

class ConstStrDictionary : public InfoDictionary
{
public:
	ConstStrDictionary(void) : InfoDictionary(CaseSensitive::Yes)
	{
		m_info.reset(new ConstStrInfo);
	}

	std::vector<std::string *> &array(void)
	{
		return (dynamic_cast<ConstStrInfo *>(m_info.get()))->array();
	}
};


// translate functions
void inputTranslate(Translator &translator);
void letTranslate(Translator &translator);
void printTranslate(Translator &translator);


// encode functions
void remEncode(ProgramModel *programUnit,
	ProgramCode::BackInserter backInserter, Token *token);
void constNumEncode(ProgramModel *programUnit,
	ProgramCode::BackInserter backInserter, Token *token);
void constStrEncode(ProgramModel *programUnit,
	ProgramCode::BackInserter backInserter, Token *token);
void varDblEncode(ProgramModel *programUnit,
	ProgramCode::BackInserter backInserter, Token *token);
void varIntEncode(ProgramModel *programUnit,
	ProgramCode::BackInserter backInserter, Token *token);
void varStrEncode(ProgramModel *programUnit,
	ProgramCode::BackInserter backInserter, Token *token);


// operand text functions
const std::string remOperandText(const ProgramModel *programUnit,
	uint16_t operand, SubCode subCode);
const std::string constNumOperandText(const ProgramModel *programUnit,
	uint16_t operand, SubCode subCode);
const std::string constStrOperandText(const ProgramModel *programUnit,
	uint16_t operand, SubCode subCode);
const std::string varDblOperandText(const ProgramModel *programUnit,
	uint16_t operand, SubCode subCode);
const std::string varIntOperandText(const ProgramModel *programUnit,
	uint16_t operand, SubCode subCode);
const std::string varStrOperandText(const ProgramModel *programUnit,
	uint16_t operand, SubCode subCode);


// remove functions
void remRemove(ProgramModel *programUnit, uint16_t operand);
void constNumRemove(ProgramModel *programUnit, uint16_t operand);
void constStrRemove(ProgramModel *programUnit, uint16_t operand);
void varDblRemove(ProgramModel *programUnit, uint16_t operand);
void varIntRemove(ProgramModel *programUnit, uint16_t operand);
void varStrRemove(ProgramModel *programUnit, uint16_t operand);


// recreate functions
void operandRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void unaryOperatorRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void binaryOperatorRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void parenRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void internalFunctionRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void arrayRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void functionRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void defineFunctionRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void blankRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void remRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void constStrRecreate(Recreator &recreator, RpnItemPtr &rpnItem);

void assignRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void assignStrRecreate(Recreator &recreator, RpnItemPtr &rpnItem);

void printItemRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void printCommaRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void printFunctionRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void printSemicolonRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void printRecreate(Recreator &recreator, RpnItemPtr &rpnItem);

void inputPromptBeginRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void inputAssignRecreate(Recreator &recreator, RpnItemPtr &rpnItem);
void inputRecreate(Recreator &recreator, RpnItemPtr &rpnItem);


#endif // BASIC_H
