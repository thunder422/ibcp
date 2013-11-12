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

#include <QString>

#include "ibcp.h"
#include "dictionary.h"

class Translator;
class Token;
class ProgramModel;
class Recreator;
class RpnItem;


// constant definitions

class ConstNumInfo
{
public:
	ConstNumInfo(void);
	ConstNumInfo(Token *token);

	double value(void) const
	{
		return m_value;
	}
	int valueInt(void) const
	{
		return m_valueInt;
	}

private:
	double m_value;
	int m_valueInt;
};


class ConstStrInfo
{
public:
	ConstStrInfo(void);
	ConstStrInfo(Token *token);

	QString *value(void) const
	{
		return m_value;
	}

private:
	QString *m_value;
};


class ConstStrDictionary : public InfoDictionary<ConstStrInfo>
{
public:
	~ConstStrDictionary(void);
};


// translate functions
TokenStatus inputTranslate(Translator &translator, Token *commandToken,
	Token *&token);
TokenStatus letTranslate(Translator &translator, Token *commandToken,
	Token *&token);
TokenStatus printTranslate(Translator &translator, Token *commandToken,
	Token *&token);


// encode functions
quint16 remEncode(ProgramModel *programUnit, Token *token);
quint16 constNumEncode(ProgramModel *programUnit, Token *token);
quint16 constStrEncode(ProgramModel *programUnit, Token *token);
quint16 varDblEncode(ProgramModel *programUnit, Token *token);
quint16 varIntEncode(ProgramModel *programUnit, Token *token);
quint16 varStrEncode(ProgramModel *programUnit, Token *token);


// operand text functions
const QString remOperandText(const ProgramModel *programUnit, quint16 operand);
const QString constNumOperandText(const ProgramModel *programUnit,
	quint16 operand);
const QString constStrOperandText(const ProgramModel *programUnit,
	quint16 operand);
const QString varDblOperandText(const ProgramModel *programUnit,
	quint16 operand);
const QString varIntOperandText(const ProgramModel *programUnit,
	quint16 operand);
const QString varStrOperandText(const ProgramModel *programUnit,
	quint16 operand);


// remove functions
void remRemove(ProgramModel *programUnit, quint16 operand);
void constNumRemove(ProgramModel *programUnit, quint16 operand);
void constStrRemove(ProgramModel *programUnit, quint16 operand);
void varDblRemove(ProgramModel *programUnit, quint16 operand);
void varIntRemove(ProgramModel *programUnit, quint16 operand);
void varStrRemove(ProgramModel *programUnit, quint16 operand);


// recreate functions
void operandRecreate(Recreator &recreator, RpnItem *rpnItem);
void unaryOperatorRecreate(Recreator &recreator, RpnItem *rpnItem);
void binaryOperatorRecreate(Recreator &recreator, RpnItem *rpnItem);
void parenRecreate(Recreator &recreator, RpnItem *rpnItem);
void internalFunctionRecreate(Recreator &recreator, RpnItem *rpnItem);
void arrayRecreate(Recreator &recreator, RpnItem *rpnItem);
void functionRecreate(Recreator &recreator, RpnItem *rpnItem);
void defineFunctionRecreate(Recreator &recreator, RpnItem *rpnItem);
void blankRecreate(Recreator &recreator, RpnItem *rpnItem);
void constStrRecreate(Recreator &recreator, RpnItem *rpnItem);

void assignRecreate(Recreator &recreator, RpnItem *rpnItem);
void assignStrRecreate(Recreator &recreator, RpnItem *rpnItem);


#endif // BASIC_H
