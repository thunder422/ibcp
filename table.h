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

#include <array>
#include <memory>
#include <unordered_map>

#include "ibcp.h"
#include "token.h"
#include "utility.h"


// bit definitions for flags field
enum TableFlag : unsigned
{
	// table entry flags (each must have unique bit set)
	Multiple_Flag		= 1u << 0,	// function has multiple forms
	Reference_Flag		= 1u << 1,	// code requires a reference
	SubStr_Flag			= 1u << 2,	// code supports sub-string assignments
	Hidden_Flag			= 1u << 3,	// code is hidden operator/function
	Print_Flag			= 1u << 4,	// print-only function
	UseConstAsIs_Flag   = 1u << 5,	// use constant data type as is
	Keep_Flag			= 1u << 6,	// sub-string keep assignment
	Two_Flag			= 1u << 7,	// code can have two words or characters
	EndStmt_Flag		= 1u << 31	// end statement
};


struct TableEntry;
class Translator;
class ProgramModel;
class Recreator;
class RpnItem;
using RpnItemPtr = std::shared_ptr<RpnItem>;

typedef void (*TranslateFunction)(Translator &translator);
typedef uint16_t (*EncodeFunction)(ProgramModel *programUnit,
	const TokenPtr &token);
typedef const std::string (*OperandTextFunction)
	(const ProgramModel *programUnit, uint16_t operand);
typedef void (*RemoveFunction)(ProgramModel *programUnit, uint16_t operand);
typedef void (*RecreateFunction)(Recreator &recreator, RpnItemPtr &rpnItem);


class Table
{
public:
	// function to return a reference to the single table instance
	static Table &instance(void);

	// CODE RELATED TABLE FUNCTIONS
	Token::Type type(Code code) const;
	const std::string name(Code code) const;
	const std::string name2(Code code) const;
	const std::string optionName(Code code) const;
	const std::string debugName(Code code) const;
	bool hasFlag(Code code, unsigned flag) const;
	int precedence(Code code) const;
	DataType returnDataType(Code code) const;
	int operandCount(Code code) const;
	DataType operandDataType(Code code, int operand) const;
	int associatedCodeCount(Code code) const;
	Code associatedCode(Code code, int index = 0) const;
	Code *associatedCodeArray(Code code) const;
	int secondAssociatedIndex(Code code) const;
	Code secondAssociatedCode(Code code, int index = 0) const;
	DataType expectedDataType(Code code) const;

	Code alternateCode(Code primaryCode, int index = 0) const;
	int alternateCodeCount(Code code, int index) const;

	TranslateFunction translateFunction(Code code) const;
	EncodeFunction encodeFunction(Code code) const;
	bool hasOperand(Code code) const;
	OperandTextFunction operandTextFunction(Code code) const;
	RemoveFunction removeFunction(Code code) const;
	RecreateFunction recreateFunction(Code code) const;

	// TOKEN RELATED TABLE FUNCTIONS
	Code unaryCode(const TokenPtr &token) const;
	bool isUnaryOperator(const TokenPtr &token) const;
	bool isUnaryOrBinaryOperator(const TokenPtr &token) const;
	int precedence(const TokenPtr &token) const;
	bool hasFlag(const TokenPtr &token, unsigned flag) const;
	int operandCount(const TokenPtr &token) const;
	DataType expectedDataType(const TokenPtr &token) const;
	void setToken(TokenPtr &token, Code code);
	TokenPtr newToken(Code code);
	TokenUniquePtr newToken(int column, int length, Code code,
		const std::string string = {})
	{
		return TokenUniquePtr{new Token {column, length, type(code),
			returnDataType(code), code, string}};
	}
	Code findCode(TokenPtr &token, TokenPtr &operandToken,
		int operandIndex = 0);
	bool setTokenCode(TokenPtr token, Code code, DataType dataType,
		int operandIndex);
	void setTokenCode(TokenPtr token, Code baseCode)
	{
		setTokenCode(token, baseCode, token->dataType(), 0);
	}
	std::string name(const TokenPtr &token) const;

	// TABLE SPECIFIC FUNCTIONS
	static Code find(const std::string &string);
	static Code find(const std::string &word1, const std::string &word2)
	{
		return find(word1 + ' ' + word2);
	}

private:
	// these functions private to prevent multiple instances
	Table(void) {}
	Table(TableEntry *entry, int entryCount);
	Table(Table const &) {}
	Table &operator=(Table const &) {return *this;}

	void add(TableEntry &entry);

	static Table *s_instance;		// single instance of table

	TableEntry *m_entry;			// pointer to table entries

	// case insensitive unordered map alias
	using NameMap = std::unordered_map<std::string, TableEntry *,
		CaseOptionalHash, CaseOptionalEqual>;

	static NameMap s_nameToEntry;	// name to code table map

	// entry to alternate entries arrays
	using EntryVectorArray = std::array<std::vector<TableEntry *>, 3>;
	static std::unordered_map<TableEntry *, EntryVectorArray> s_alternate;
};


#endif  // TABLE_H
