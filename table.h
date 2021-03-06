// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: table.h - table class header file
//	Copyright (C) 2012  Thunder422
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
//	2012-11-03	initial version (parts removed from ibcp.h)

#ifndef TABLE_H
#define TABLE_H

#include <QStringList>

#include "ibcp.h"
#include "token.h"


// bit definitions for flags field
enum TableFlag
{
	Null_Flag			= 0x00000000,  // entry has no flags
	// table entry flags (each must have unique bit set)
	Multiple_Flag		= 0x00000001,  // function has multiple forms
	Reference_Flag		= 0x00000002,  // code requires a reference
	SubStr_Flag			= 0x00000004,  // code supports sub-string assignments
	Hidden_Flag			= 0x00000008,  // code is hidden operator/function
	Print_Flag			= 0x00000010,  // print-only function
	UseConstAsIs_Flag   = 0x00000020,  // use constant data type as is
	HasOperand_Flag		= 0x00000040,  // code has operand in program code
	EndStmt_Flag		= 0x00000080   // end statement
};


// categories for searching the table entries
enum SearchType  // table search types
{
	PlainWord_SearchType,
	ParenWord_SearchType,
	DataTypeWord_SearchType,
	Symbol_SearchType,
	sizeof_SearchType
};


// multiple word command or multiple character operator type
enum Multiple
{
	OneWord_Multiple,
	OneChar_Multiple = OneWord_Multiple,
	TwoWord_Multiple,
	TwoChar_Multiple = TwoWord_Multiple,
	ThreeWord_Multiple,
	ThreeChar_Multiple = ThreeWord_Multiple,
	sizeof_Multiple
};


struct TableEntry;
class Translator;
class ProgramModel;

typedef TokenStatus (*TranslateFunction)(Translator &translator,
	Token *commandToken, Token *&token);
typedef quint16 (*EncodeFunction)(ProgramModel *programUnit, Token *token);
typedef const QString (*OperandTextFunction)(const ProgramModel *programUnit,
	quint16 operand);
typedef void (*RemoveFunction)(ProgramModel *programUnit, quint16 operand);


class Table
{
	static Table *s_instance;		// single instance of table
	static QStringList s_errorList;	// list of errors found during initialize

	TableEntry *m_entry;			// pointer to table entries
	struct Range
	{
		Code beg;					// begin index of range
		Code end;					// end index of range
	} m_range[sizeof_SearchType];	// range for each search type

	// these functions private to prevent multiple instances
	Table(void) {}
	Table(TableEntry *entry) : m_entry(entry) {}
	Table(Table const &) {}
	Table &operator=(Table const &) {return *this;}

	// function to setup and check the table entries
	QStringList setupAndCheck(void);
public:
	// function to create the single instance, initialize and check the table
	static void initialize(void);
	// function to return a reference to the single table instance
	static Table &instance(void);
	// function to report if there were any table errors
	static bool hasErrors(void);
	// function to returns list of table errors
	static QStringList errorList(void);

	// CODE RELATED TABLE FUNCTIONS
	TokenType type(Code code) const;
	DataType dataType(Code code) const;
	Multiple multiple(Code code) const;
	const QString name(Code code) const;
	const QString name2(Code code) const;
	const QString optionName(Code code) const;
	const QString debugName(Code code) const;
	int hasFlag(Code code, int flag) const;
	int precedence(Code code) const;
	int operandCount(Code code) const;
	DataType operandDataType(Code code, int operand) const;
	int associatedCodeCount(Code code) const;
	Code associatedCode(Code code, int index = 0) const;
	Code *associatedCodeArray(Code code) const;
	int secondAssociatedIndex(Code code) const;
	Code secondAssociatedCode(Code code, int index = 0) const;
	DataType expectedDataType(Code code) const;
	bool isUnaryOperator(Code code) const;
	TranslateFunction translateFunction(Code code) const;
	EncodeFunction encodeFunction(Code code) const;
	OperandTextFunction operandTextFunction(Code code) const;
	RemoveFunction removeFunction(Code code) const;

	// TOKEN RELATED TABLE FUNCTIONS
	Code unaryCode(Token *token) const;
	bool isUnaryOperator(Token *token) const;
	bool isUnaryOrBinaryOperator(Token *token) const;
	int precedence(Token *token) const;
	int hasFlag(Token *token, int flag) const;
	int operandCount(Token *token) const;
	DataType expectedDataType(Token *token) const;
	void setToken(Token *token, Code code);
	Token *newToken(Code code);
	Code cvtCode(Token *token, DataType dataType) const;
	Code findCode(Token *token, Token *operandToken, int operandIndex = 0);
	bool setTokenCode(Token *token, Code code, DataType dataType,
		int operandIndex);
	void setTokenCode(Token *token, Code baseCode)
	{
		setTokenCode(token, baseCode, token->dataType(), 0);
	}

	// TABLE SPECIFIC FUNCTIONS
	Code search(SearchType type, const QStringRef &string) const;
	Code search(const QStringRef &word1, const QStringRef &word2) const;
	Code search(Code code, int argumentCount) const;
	Code search(Code code, DataType *dataType) const;
	bool match(Code code, DataType *dataType) const;
};


#endif  // TABLE_H
