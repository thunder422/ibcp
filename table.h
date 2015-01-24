// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: table.h - table class header file
//	Copyright (C) 2012-2015  Thunder422
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

#ifndef TABLE_H
#define TABLE_H

#include <array>
#include <memory>
#include <unordered_map>

#include "ibcp.h"
#include "utility.h"

class Token;
using TokenPtr = std::shared_ptr<Token>;
class Erector;


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
	Command_Flag		= 1u << 8,	// code is a command
	Operator_Flag		= 1u << 9,	// code is an operator
	Function_Flag		= 1u << 10,	// code is a BASIC function
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

// expression information for operators and internal functions
struct ExprInfo
{
	DataType m_returnDataType;		// return data type of operator/function
	short m_operandCount;			// number of operands (operators/functions)
	const DataType *m_operandDataType;	// data type of each operand

	ExprInfo(DataType returnDataType = DataType::None,
		const std::initializer_list<const DataType> &operands = {}) :
		m_returnDataType {returnDataType},
		m_operandCount(operands.size()),
		m_operandDataType {operands.begin()}
	{
	}
};

// table entry structure with information for a single code
class Table
{
public:
	Table();
	Table(Code code, const std::string name, const std::string name2,
		const std::string option, unsigned flags, int precedence,
		ExprInfo *exprInfo, TranslateFunction _translate,
		EncodeFunction _encode, OperandTextFunction _operandText,
		RemoveFunction _remove, RecreateFunction _recreate);

	Table &operator=(const Table &) = delete;
	Table(const Table &) = delete;
	Table &operator=(Table &&) = delete;
	Table(Table &&) = delete;

	// TABLE ENTRY FUNCTIONS
	int index() const;
	Code code() const
	{
		return m_code;
	}
	bool isCode(Code code)
	{
		return code == m_code;
	}
	std::string name() const
	{
		return m_name;
	}
	const std::string name2() const
	{
		return m_name2;
	}
	std::string commandName() const;
	const std::string debugName() const
	{
		return m_name + m_name2;
	}
	const std::string optionName() const
	{
		return m_option;
	}
	bool hasFlag(unsigned flag) const
	{
		return m_flags & flag ? true : false;
	}
	bool isOperator() const
	{
		return hasFlag(Operator_Flag);
	}
	bool isCommand() const
	{
		return hasFlag(Command_Flag) && isNotAssignmentOperator()
			&& !isOperator();
	}
	bool isFunction() const
	{
		return hasFlag(Function_Flag);
	}
	int precedence() const
	{
		return m_precedence;
	}
	DataType returnDataType() const
	{
		return m_exprInfo->m_returnDataType;
	}
	int operandCount() const
	{
		return m_exprInfo->m_operandCount;
	}
	DataType operandDataType(int operand) const
	{
		return m_exprInfo->m_operandDataType[operand];
	}
	DataType expectedDataType();

	int firstOperand() const
	{
		return 0;
	}
	int lastOperand() const
	{
		return m_exprInfo->m_operandCount - 1;
	}
	bool isUnaryOperator() const
	{
		return isOperator() && m_exprInfo->m_operandCount == 1;
	}
	bool isUnaryOrBinaryOperator() const
	{
		return isOperator() && hasOperands();
	}

	Table *alternate(DataType returnDataType);
	int alternateCount(int operand);
	Table *alternate(int operand = 0);
	Table *alternate(int operand, DataType operandDataType);

	TranslateFunction translateFunction() const
	{
		return translate;
	}
	EncodeFunction encodeFunction() const
	{
		return encode;
	}
	bool isCodeWithOperand() const
	{
		return operandText;
	}
	OperandTextFunction operandTextFunction() const
	{
		return operandText;
	}
	RemoveFunction removeFunction() const
	{
		return remove;
	}
	RecreateFunction recreateFunction() const
	{
		return recreate;
	}

	// STATIC TABLE FUNCTIONS
	static Table *entry(Code code);
	static Table *entry(Code code, DataType dataType);
	static Table *entry(int index);
	static Table *find(const std::string &string);
	static Table *find(const std::string &word1, const std::string &word2)
	{
		return find(word1 + ' ' + word2);
	}

	friend class Erector;

private:
	bool hasName() const
	{
		return !m_name.empty();
	}
	bool isTwoWordCommand() const
	{
		return hasFlag(Two_Flag) && !m_name2.empty();
	}
	bool isNotAssignmentOperator() const
	{
		return !hasFlag(Reference_Flag);
	}
	void setMultipleFlag()
	{
		m_flags |= Multiple_Flag;
	}
	bool hasOperands() const
	{
		return m_exprInfo->m_operandCount > 0;
	}
	bool hasOperandsAndIsNotAssignmentOperator()
	{
		return hasOperands() && isNotAssignmentOperator();
	}
	bool isBinaryOperator() const
	{
		return isOperator() && m_exprInfo->m_operandCount == 2;
	}
	bool isHomogeneousOperator() const
	{
		return isBinaryOperator() && operandDataType(0) == operandDataType(1);
	}
	bool isNotHomogeneousOperator() const
	{
		return isBinaryOperator() && operandDataType(0) != operandDataType(1);
	}
	bool hasLessOperandsThan(Table *other)
	{
		return operandCount() < other->operandCount();
	}
	bool hasMoreOperandsThan(Table *other)
	{
		return operandCount() > other->operandCount();
	}
	bool isOperaratorWithMoreOperandsThan(Table *other)
	{
		return isOperator() && hasMoreOperandsThan(other);
	}
	bool hasSameOperandDataType(Table *other, int operand)
	{
		return operandDataType(operand) == other->operandDataType(operand);
	}
	bool hasDifferentOperandDataType(Table *other, int operand)
	{
		return not hasSameOperandDataType(other, operand);
	}

	// STATIC MEMBERS
	using NameMap = std::unordered_map<std::string, Table *,
		CaseOptionalHash, CaseOptionalEqual>;
	static NameMap s_nameToEntry;

	using CodeMap = std::unordered_map<Code, Table *, EnumClassHash>;
	static CodeMap s_codeToEntry;

	// entry to alternate entries arrays
	using EntryVector = std::vector<Table *>;
	using EntryVectorArray = std::array<EntryVector, 3>;
	static std::unordered_map<Table *, EntryVectorArray> s_alternate;

	static std::unordered_map<Table *, DataType> s_expectedDataType;

	// INSTANCE MEMBERS
	Code m_code;					// code type of table entry
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


#endif  // TABLE_H
