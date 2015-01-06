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
#include "utility.h"

class Token;
using TokenPtr = std::shared_ptr<Token>;

// TODO temporary until code type enumeration implemented
enum class Type
{
	Command = 1,
	Operator,
	IntFunc,
	Constant,
	DefFunc,
	NoParen,
	Paren
};


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
	Command_Flag		= 1u << 8,	// operator code is really a command
	EndStmt_Flag		= 1u << 31	// end statement
};


struct ExprInfo;
class Translator;
class ProgramModel;
class Recreator;
class RpnItem;
using RpnItemPtr = std::shared_ptr<RpnItem>;

typedef void (*TranslateFunction)(Translator &translator);
typedef uint16_t (*EncodeFunction)(ProgramModel *programUnit,
	const TokenPtr &token);
typedef const std::string (*OperandTextFunction)
	(const ProgramModel *programUnit, uint16_t operand, SubCode subCode);
typedef void (*RemoveFunction)(ProgramModel *programUnit, uint16_t operand);
typedef void (*RecreateFunction)(Recreator &recreator, RpnItemPtr &rpnItem);

// table entry structure with information for a single code
class TableEntry
{
public:
	Code code() const;
	bool isCode(Code code)
	{
		return code == this->code();
	}
	Type type() const
	{
		return m_type;
	}
	std::string name() const
	{
		return m_name;
	}
	bool hasFlag(unsigned flag) const
	{
		return m_flags & flag ? true : false;
	}
	TableEntry *alternate(int index = 0);

public:  // TODO make private once new table fully implemented
	Type m_type;					// type of token for entry
	const std::string m_name;		// name for table entry
	const std::string m_name2;		// name of second word of command
	const std::string m_option;		// name of option sub-code
	unsigned m_flags;				// flags for entry
	int m_precedence;				// precedence of code
	ExprInfo *m_exprInfo;			// expression info pointer (NULL for none)
	TranslateFunction translate;	// pointer to translate function
	EncodeFunction encode;			// pointer to encode function
	OperandTextFunction operandText;// pointer to operand text function
	RemoveFunction remove;			// pointer to remove function
	RecreateFunction recreate;		// pointer to recreate function
};


class Table
{
public:
	// function to return a reference to the single table instance
	static Table &instance(void);

	// CODE RELATED TABLE FUNCTIONS
	Type type(Code code) const;
	const std::string name(Code code) const;
	const std::string name2(Code code) const;
	const std::string optionName(Code code) const;
	const std::string debugName(Code code) const;
	bool hasFlag(Code code, unsigned flag) const;
	int precedence(Code code) const;
	DataType returnDataType(Code code) const;
	int operandCount(Code code) const;
	DataType operandDataType(Code code, int operand) const;
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
	void setToken(Token *token, Code code);
	Code findCode(TokenPtr &token, TokenPtr &operandToken,
		int operandIndex = 0);
	bool setTokenCode(Token *token, Code code, DataType dataType,
		int operandIndex);
	void setTokenCode(Token *token, Code code, DataType dataType);
	std::string name(const TokenPtr &token) const;

	// TABLE SPECIFIC FUNCTIONS
	static TableEntry *entry(int index);
	static TableEntry *find(const std::string &string);
	static TableEntry *find(const std::string &word1, const std::string &word2)
	{
		return find(word1 + ' ' + word2);
	}

	friend class TableEntry;

private:
	// these functions private to prevent multiple instances
	Table(void) {}
	Table(TableEntry *entry, int entryCount);
	Table(Table const &) {}
	Table &operator=(Table const &) {return *this;}

	static void addExpectedDataType(TableEntry *entry, DataType dataType);

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

	// entry to expected data type map
	static std::unordered_map<TableEntry *, DataType> s_expectedDataType;
};


#endif  // TABLE_H
