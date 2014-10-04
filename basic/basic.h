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

#include <QString>
#include <QVector>

#include "ibcp.h"
#include "token.h"
#include "dictionary.h"

class Translator;
class Token;
class ProgramModel;
class Recreator;
class RpnItem;
using RpnItemPtr = std::shared_ptr<RpnItem>;


// constant definitions

class ConstNumInfo : public AbstractInfo
{
public:
	void clear(void);
	void addElement(void);
	void setElement(int index, const TokenPtr &token);

	double *array(void)
	{
		return m_value.data();
	}
	int *arrayInt(void)
	{
		return m_valueInt.data();
	}

private:
	QVector<double> m_value;				// vector of double values
	QVector<int> m_valueInt;				// vector of integer values
};

class ConstNumDictionary : public InfoDictionary
{
public:
	ConstNumDictionary(void)
	{
		m_info.reset(new ConstNumInfo);
	}

	double *array(void)
	{
		return (dynamic_cast<ConstNumInfo *>(m_info.get()))->array();
	}
	int *arrayInt(void)
	{
		return (dynamic_cast<ConstNumInfo *>(m_info.get()))->arrayInt();
	}
};


class ConstStrInfo : public AbstractInfo
{
public:
	~ConstStrInfo(void);
	void clear(void);
	void addElement(void);
	void setElement(int index, const TokenPtr &token);
	void clearElement(int index);

	QString **array(void)
	{
		return m_value.data();
	}

private:
	QVector<QString *> m_value;				// vector of string instances
};

class ConstStrDictionary : public InfoDictionary
{
public:
	ConstStrDictionary(void) : InfoDictionary(CaseSensitive::Yes)
	{
		m_info.reset(new ConstStrInfo);
	}

	QString **array(void)
	{
		return (dynamic_cast<ConstStrInfo *>(m_info.get()))->array();
	}
};


// translate functions
Status inputTranslate(Translator &translator, TokenPtr commandToken,
	TokenPtr &token);
Status letTranslate(Translator &translator, TokenPtr commandToken,
	TokenPtr &token);
Status printTranslate(Translator &translator, TokenPtr commandToken,
	TokenPtr &token);


// encode functions
quint16 remEncode(ProgramModel *programUnit, const TokenPtr &token);
quint16 constNumEncode(ProgramModel *programUnit, const TokenPtr &token);
quint16 constStrEncode(ProgramModel *programUnit, const TokenPtr &token);
quint16 varDblEncode(ProgramModel *programUnit, const TokenPtr &token);
quint16 varIntEncode(ProgramModel *programUnit, const TokenPtr &token);
quint16 varStrEncode(ProgramModel *programUnit, const TokenPtr &token);


// operand text functions
const std::string remOperandText(const ProgramModel *programUnit,
	quint16 operand);
const std::string constNumOperandText(const ProgramModel *programUnit,
	quint16 operand);
const std::string constStrOperandText(const ProgramModel *programUnit,
	quint16 operand);
const std::string varDblOperandText(const ProgramModel *programUnit,
	quint16 operand);
const std::string varIntOperandText(const ProgramModel *programUnit,
	quint16 operand);
const std::string varStrOperandText(const ProgramModel *programUnit,
	quint16 operand);


// remove functions
void remRemove(ProgramModel *programUnit, quint16 operand);
void constNumRemove(ProgramModel *programUnit, quint16 operand);
void constStrRemove(ProgramModel *programUnit, quint16 operand);
void varDblRemove(ProgramModel *programUnit, quint16 operand);
void varIntRemove(ProgramModel *programUnit, quint16 operand);
void varStrRemove(ProgramModel *programUnit, quint16 operand);


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
