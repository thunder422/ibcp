// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: operand.cpp - operand classes and functions source file
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


//=====================================
//  CODE WITH OPERAND TABLE SUB-CLASS
//=====================================

class Operand : public Table
{
public:
	Operand(Code code, const char *name, const char *name2, unsigned flags,
			int precedence, ExprInfo *exprInfo, OperandType operandType,
			const AlternateItem &alternateItem = {});

	virtual void encode(ProgramWriter &programWriter, Token *token) override;
	virtual const std::string operandText(ProgramReader &programReader)
		override;
	virtual void remove(ProgramReader &programReader) override;
	virtual void recreate(Recreator &recreator, RpnItemPtr &rpnItem) override;
};


Operand::Operand(Code code, const char *name, const char *name2, unsigned flags,
		int precedence, ExprInfo *exprInfo, OperandType operandType,
		const AlternateItem &alternateItem) :
	Table {code, name, name2, "", flags, precedence, exprInfo,
		   operandType, nullptr, nullptr}
{
	appendAlternate(alternateItem);
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
	Rem(Code code, const char *name, unsigned flags, int precedence) :
		Operand(code, name, "", flags, precedence, &Null_ExprInfo,
			Rem_OperandType) {}

	virtual void recreate(Recreator &recreator, RpnItemPtr &rpnItem) override;
	// TODO virtual run() override function for Rem/RemOp (does nothing)
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

class Const : public Operand
{
public:
	Const(const char *name2, OperandType operandType, ExprInfo *exprInfo,
			const AlternateItem &alteranteItem = {}) :
		Operand(Code::Constant, "", name2, TableFlag{}, 2, exprInfo,
			operandType, alteranteItem) {}

	// TODO virtual run() override function for ConstDbl
};

class ConstInt : public Const
{
public:
	using Const::Const;

	// TODO virtual run() override function for ConstInt
};

class ConstStr : public Const
{
public:
	using Const::Const;

	virtual void recreate(Recreator &recreator, RpnItemPtr &rpnItem) override;
	// TODO virtual run() override function for ConstStr

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

class Var : public Operand
{
public:
	Var(const char *name2, OperandType operandType, ExprInfo *exprInfo,
			const AlternateItem &alteranteItem = {},
			unsigned flags = TableFlag{}) :
		Operand(Code::Variable, "", name2, flags, 2, exprInfo, operandType,
			alteranteItem) {}

	// TODO virtual run() override function for VarDbl
};

class VarInt : public Var
{
	using Var::Var;

	// TODO virtual run() override function for VarInt
};

class VarStr : public Var
{
	using Var::Var;

	// TODO virtual run() override function for VarStr
};


class VarRef : public Var
{
public:
	VarRef(const char *name2, OperandType operandType, ExprInfo *exprInfo,
			const AlternateItem &alteranteItem = {}) :
		Var(name2, operandType, exprInfo, alteranteItem, Reference_Flag) {}

	// TODO virtual run() override function for VarRefDbl
};

class VarRefInt : public VarRef
{
	using VarRef::VarRef;

	// TODO virtual run() override function for VarRefInt
};

class VarRefStr : public VarRef
{
	using VarRef::VarRef;

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

Rem remCommand {Code::Rem, "REM", Command_Flag, 4};
Rem remOperator {Code::RemOp, "'", Operator_Flag | EndStmt_Flag, 2};

Const constDbl {"Const", ConstNum_OperandType, &Dbl_None_ExprInfo};
ConstInt constInt {"ConstInt", ConstNum_OperandType, &Int_None_ExprInfo,
	{&constDbl, 0}};
ConstStr constStr {"ConstStr", ConstStr_OperandType, &Str_None_ExprInfo,
	{&constDbl, 0}};

Var var {"Var", VarDbl_OperandType, &Dbl_None_ExprInfo};
VarInt varInt {"VarInt", VarInt_OperandType, &Int_None_ExprInfo, {&var, 0}};
VarStr varStr {"VarStr", VarStr_OperandType, &Str_None_ExprInfo, {&var, 0}};

VarRef varRef {"VarRef", VarDbl_OperandType, &Dbl_None_ExprInfo, {&var, 1}};
VarRefInt varRefInt {"VarRefInt", VarInt_OperandType, &Int_None_ExprInfo,
	{&varRef, 0}};
VarRefStr varRefStr {"VarRefStr", VarStr_OperandType, &Str_None_ExprInfo,
	{&varRef, 0}};


// end: operand.cpp
