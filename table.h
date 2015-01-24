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
class TableEntry
{
public:
	TableEntry(Code code, const std::string name, const std::string name2,
		const std::string option, unsigned flags, int precedence,
		ExprInfo *exprInfo, TranslateFunction _translate,
		EncodeFunction _encode, OperandTextFunction _operandText,
		RemoveFunction _remove, RecreateFunction _recreate) :
		m_code {code},
		m_name {name},
		m_name2 {name2},
		m_option {option},
		m_flags {flags},
		m_precedence {precedence},
		m_exprInfo {exprInfo},
		translate {_translate},
		encode {_encode},
		operandText {_operandText},
		remove {_remove},
		recreate {_recreate}
		{}

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
	DataType operandDataType(int operandIndex) const
	{
		return m_exprInfo->m_operandDataType[operandIndex];
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

	TableEntry *alternate(DataType returnDataType);
	int alternateCount(int operandIndex);
	TableEntry *alternate(int operandIndex = 0);
	TableEntry *alternate(int operandIndex, DataType operandDataType);

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
	friend class Table;

private:
	void addToTable() noexcept;
	void addToCodeMap() noexcept;
	void ifTwoWordCommandAddToNameMap();
	TableEntry *getPrimaryOrAddToNameMap() noexcept;
	void addToNameMap();
	TableEntry *setNewPrimaryOrGetPrimary(TableEntry *primary);
	TableEntry *addAlternateOrGetNewPrimary(TableEntry *primary);
	void checkIfBinaryOperatorAndOperandsNotSameDataType() const;
	bool iterateOperandsAndAddAlternate(TableEntry *primary) noexcept;
	bool addAlternate(TableEntry *primary, int operand) noexcept;
	bool updatePrimaryAndAlternate(TableEntry *&primary, TableEntry *&alternate,
		int operand) noexcept;
	void addAlternateToPrimary(TableEntry *primary, int operand,
		TableEntry *alternate) noexcept;
	void replaceExpectedDataTypeWithEntry(TableEntry *oldAlternate,
		int operand) noexcept;
	TableEntry *swapAlternateWithEntry(TableEntry *&alternateIterator) noexcept;
	void addToExpectedDataType(int operandIndex) noexcept;
	void addToExpectedDataType(DataType dataType) noexcept;
	void checkIfFunctionIsMulipleEntry(TableEntry *primary);

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
	bool isBinaryOperator() const
	{
		return isOperator() && m_exprInfo->m_operandCount == 2;
	}
	bool isBinaryOperatorAndHomogeneous() const
	{
		return isBinaryOperator()
			&& m_exprInfo->m_operandDataType[0]
			== m_exprInfo->m_operandDataType[1];
	}
	bool isBinaryOperatorAndNotHomogeneous() const
	{
		return isBinaryOperator()
			&& m_exprInfo->m_operandDataType[0]
			!= m_exprInfo->m_operandDataType[1];
	}
	bool hasLessOperandsThan(TableEntry *other)
	{
		return operandCount() < other->operandCount();
	}
	bool hasMoreOperandsThan(TableEntry *other)
	{
		return operandCount() > other->operandCount();
	}
	bool hasSameOperandDataType(TableEntry *other, int operandIndex)
	{
		return operandDataType(operandIndex)
			== other->operandDataType(operandIndex);
	}
	bool hasDifferentOperandDataType(TableEntry *other, int operandIndex)
	{
		return !hasSameOperandDataType(other, operandIndex);
	}

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


class Table
{
public:
	Table();

	// TABLE SPECIFIC FUNCTIONS
	static TableEntry *entry(Code code);
	static TableEntry *entry(Code code, DataType dataType);
	static TableEntry *entry(int index);
	static TableEntry *find(const std::string &string);
	static TableEntry *find(const std::string &word1, const std::string &word2)
	{
		return find(word1 + ' ' + word2);
	}

	friend class TableEntry;

private:
	// these functions private to prevent multiple instances
	Table(Table const &) {}
	Table &operator=(Table const &) {return *this;}

	// case insensitive unordered map alias
	using NameMap = std::unordered_map<std::string, TableEntry *,
		CaseOptionalHash, CaseOptionalEqual>;

	static NameMap s_nameToEntry;	// name to code table map

	using CodeMap = std::unordered_map<Code, TableEntry *, EnumClassHash>;

	static CodeMap s_codeToEntry;

	// entry to alternate entries arrays
	using EntryVector = std::vector<TableEntry *>;
	using EntryVectorArray = std::array<EntryVector, 3>;
	static std::unordered_map<TableEntry *, EntryVectorArray> s_alternate;

	// entry to expected data type map
	static std::unordered_map<TableEntry *, DataType> s_expectedDataType;
};


#endif  // TABLE_H
