// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: table.h - table class definitions file
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
	// note: value 0x00000004 is available
	Hidden_Flag			= 0x00000008,  // code is hidden operator/function
	Print_Flag			= 0x00000010,  // print-only function
	String_Flag			= 0x00000020,  // code has string operands
	EndExpr_Flag		= 0x00000040,  // end expression
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
struct CmdItem;

// TODO see if these can be forward referenced pointers
typedef TokenStatus (*TokenHandler)(Translator &t, Token *&token);
typedef TokenStatus (*CommandHandler)(Translator &t, CmdItem *cmdItem,
	Token *token);


// 2011-02-26: removed index_code[], index(), and code(); changed index to code
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
	const QString debugName(Code code) const;
	int flags(Code code) const;
	TokenMode tokenMode(Code code) const;
	Code unaryCode(Code code) const;
	int precedence(Code code) const;
	int nOperands(Code code) const;
	DataType operandDataType(Code code, int operand) const;
	int nAssocCodes(Code code) const;
	Code assocCode(Code code, int index) const;
	int assoc2Index(Code code) const;
	Code assoc2Code(Code code, int index = 0) const;
	int nStrings(Code code) const;
	bool isUnaryOperator(Code code) const;
	TokenHandler tokenHandler(Code code) const;
	CommandHandler commandHandler(Code code) const;

	// TOKEN RELATED TABLE FUNCTIONS
	int precedence(Token *token) const;
	int flags(Token *token) const;
	void setToken(Token *token, Code code);
	Token *newToken(Code code);

	// TABLE SPECIFIC FUNCTIONS
	Code search(SearchType type, const QStringRef &string) const;
	Code search(const QStringRef &word1, const QStringRef &word2) const;
	Code search(Code code, int nArguments) const;
	Code search(Code code, DataType *dataType) const;
	bool match(Code code, DataType *dataType) const;
};


#endif  // TABLE_H
