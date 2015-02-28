// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: operand.cpp - operand table classes and functions source file
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
#include "programwriter.h"
#include "recreator.h"
#include "rpnlist.h"
#include "table.h"
#include "token.h"

extern ExprInfo Null_ExprInfo;
extern ExprInfo Dbl_None_ExprInfo;
extern ExprInfo Int_None_ExprInfo;
extern ExprInfo Str_None_ExprInfo;

constexpr TypeInfo Dbl = {"", &Dbl_None_ExprInfo};
constexpr TypeInfo Int = {"%", &Int_None_ExprInfo};
constexpr TypeInfo Str = {"$", &Str_None_ExprInfo};


//=====================================
//  CODE WITH OPERAND TABLE SUB-CLASS
//=====================================

class Operand : public Table
{
public:
	// constructor for rem sub-class
	Operand(Code code, const char *name, int precedence, unsigned flags,
		OperandType operandType);

	// constructor for no-name sub-classes
	Operand(BaseInfo baseInfo, TypeInfo typeInfo, OperandType operandType,
			const AlternateItem &alternateItem = {});

	virtual void encode(ProgramWriter &programWriter, Token *token) override;
	virtual const std::string operandText(ProgramReader &programReader)
		override;
	virtual void remove(ProgramReader &programReader) override;
	virtual void recreate(Recreator &recreator, RpnItemPtr &rpnItem) override;
};


Operand::Operand(Code code, const char *name, int precedence, unsigned flags,
		OperandType operandType) :
	Table {code, name, "", "", flags, precedence, &Null_ExprInfo,
	   operandType}
{
}

Operand::Operand(BaseInfo baseInfo, TypeInfo typeInfo, OperandType operandType,
		const AlternateItem &alternateItem) :
	Table {baseInfo, typeInfo, 2, operandType, NoName_Flag, alternateItem}
{
}

void Operand::encode(ProgramWriter &programWriter, Token *token)
{
	programWriter.generateAndWriteOperand(token, operandType());
}

const std::string Operand::operandText(ProgramReader &programReader)
{
	return programReader.readOperandAndGetString(operandType());
}

void Operand::remove(ProgramReader &programReader)
{
	programReader.readOperandAndRemoveReference(operandType());
}

void Operand::recreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	recreator.emplace(rpnItem->token()->stringWithDataType());
}


//=======================
//  REM TABLE SUB-CLASS
//=======================

class Rem : public Operand
{
public:
	Rem(Code code, const char *name, int precedence, unsigned flags) :
		Operand(code, name, precedence, flags, Rem_OperandType) {}

	// TODO virtual run() override function for Rem/RemOp (does nothing)
	virtual void recreate(Recreator &recreator, RpnItemPtr &rpnItem) override;
};

void Rem::recreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	std::string string {rpnItem->token()->name()};
	std::string remark {rpnItem->token()->string()};
	if (islower(remark.front()))
	{
		std::transform(string.begin(), string.end(), string.begin(), tolower);
	}
	if (rpnItem->token()->isCode(Code::RemOp) && recreator.backIsNotSpace())
	{
		// FLAG option: space before rem operator (default=yes)
		recreator.append(' ');
	}
	recreator.append(std::move(string));
	recreator.append(std::move(remark));
}


//==============================
//  CONSTANT TABLE SUB-CLASSES
//==============================

constexpr BaseInfo constBase = {Code::Constant, "Const", TableFlag{}};


class ConstDbl : public Operand
{
public:
	ConstDbl() : Operand {constBase, Dbl, ConstNum_OperandType} {}

	// TODO virtual run() override function for ConstDbl
};


class ConstInt : public Operand
{
public:
	ConstInt(const AlternateItem &alteranteItem = {}) :
		Operand {constBase, Int, ConstNum_OperandType, alteranteItem} {}

	// TODO virtual run() override function for ConstInt
};


class ConstStr : public Operand
{
public:
	ConstStr(const AlternateItem &alteranteItem = {}) :
		Operand {constBase, Str, ConstStr_OperandType, alteranteItem} {}

	// TODO virtual run() override function for ConstStr
	virtual void recreate(Recreator &recreator, RpnItemPtr &rpnItem) override;

private:
	std::string replaceDoubleQuotesWithTwo(std::string input);
};

void ConstStr::recreate(Recreator &recreator, RpnItemPtr &rpnItem)
{
	auto constant = replaceDoubleQuotesWithTwo(rpnItem->token()->string());
	recreator.emplace('"' + constant + '"');
}

std::string ConstStr::replaceDoubleQuotesWithTwo(std::string input)
{
	std::string result;
	for (auto c : input)
	{
		result += c;
		if (c == '"')
		{
			result += c;
		}
	}
	return result;
}


//==============================
//  VARIABLE TABLE SUB-CLASSES
//==============================

constexpr BaseInfo varBase = {Code::Variable, "Var", TableFlag{}};
constexpr BaseInfo varRefBase = {Code::Variable, "VarRef", Reference_Flag};


class VarDbl : public Operand
{
public:
	VarDbl() : Operand {varBase, Dbl, VarDbl_OperandType} {}

	// TODO virtual run() override function for VarInt
};

class VarInt : public Operand
{
public:
	VarInt(const AlternateItem &alteranteItem) :
		Operand {varBase, Int, VarInt_OperandType, alteranteItem} {}

	// TODO virtual run() override function for VarInt
};

class VarStr : public Operand
{
public:
	VarStr(const AlternateItem &alteranteItem) :
		Operand {varBase, Str, VarStr_OperandType, alteranteItem} {}

	// TODO virtual run() override function for VarStr
};


class VarRefDbl : public Operand
{
public:
	VarRefDbl(const AlternateItem &alteranteItem) :
		Operand {varRefBase, Dbl, VarDbl_OperandType, alteranteItem} {}

	// TODO virtual run() override function for VarRefInt
};

class VarRefInt : public Operand
{
public:
	VarRefInt(const AlternateItem &alteranteItem) :
		Operand {varRefBase, Int, VarInt_OperandType, alteranteItem} {}

	// TODO virtual run() override function for VarRefInt
};

class VarRefStr : public Operand
{
public:
	VarRefStr(const AlternateItem &alteranteItem) :
		Operand {varRefBase, Str, VarStr_OperandType, alteranteItem} {}

	// TODO virtual run() override function for VarRefStr
};


//======================================
//  CONSTANT INFO DICTIONARY FUNCTIONS
//======================================

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


//=========================
//  TABLE ENTRY INSTANCES
//=========================

Rem remCommand {Code::Rem, "REM", 4, Command_Flag};
Rem remOperator {Code::RemOp, "'", 2, Operator_Flag | EndStmt_Flag};

ConstDbl constDbl;
ConstInt constInt {&constDbl};
ConstStr constStr {&constDbl};

VarDbl varDbl;
VarInt varInt {&varDbl};
VarStr varStr {&varDbl};

VarRefDbl varRefDbl {{&varDbl, 1}};
VarRefInt varRefInt {&varRefDbl};
VarRefStr varRefStr {&varRefDbl};


// end: operand.cpp
