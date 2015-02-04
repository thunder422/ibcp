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
#include "programcode.h"
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
	Operand_Flag		= 1u << 11,	// code has an operand word
	EndStmt_Flag		= 1u << 31	// end statement
};


constexpr int FirstOperand = 0;
constexpr int KeepString = 0;
constexpr int SecondOperand = 1;
constexpr int SubStringAssignment = 1;
constexpr int ListAssignment = 1;
constexpr int BinaryOperator = 1;


struct ExprInfo;
class Translator;
class ProgramModel;
class Recreator;
class RpnItem;
using RpnItemPtr = std::shared_ptr<RpnItem>;

typedef void (*TranslateFunction)(Translator &translator);
typedef void (*EncodeFunction)(ProgramModel *programUnit,
	ProgramCode::BackInserter backInserter, Token *token);
typedef const std::string (*OperandTextFunction)
	(const ProgramModel *programUnit, ProgramLineReader &programLineReader);
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
	Table(Code code, const char *name, const char *name2, const char *option,
		unsigned flags, int precedence, ExprInfo *exprInfo,
		TranslateFunction _translate, EncodeFunction _encode,
		OperandTextFunction _operandText, RemoveFunction _remove,
		RecreateFunction _recreate);

	Table &operator=(const Table &) = delete;
	Table(const Table &) = delete;
	Table &operator=(Table &&) = delete;
	Table(Table &&) = delete;

	// TODO temporarily defined functions; will be pure virtual
	virtual void translate(Translator &translator)
	{
		auto translate = m_translate;
		if (!translate)
		{
			translate = Table::entry(Code::Let)->m_translate;
		}
		(*translate)(translator);
	}
	virtual void encode(ProgramModel *programUnit,
		ProgramCode::BackInserter backInserter, Token *token)
	{
		if (m_encode)
		{
			(*m_encode)(programUnit, backInserter, token);
		}
	}
	virtual const std::string operandText(const ProgramModel *programUnit,
		ProgramLineReader &programLineReader)
	{
		if (m_operandText)
		{
			return (*m_operandText)(programUnit, programLineReader);
		}
		return {};
	}
	virtual void remove(ProgramModel *programUnit, uint16_t operand)
	{
		(*m_remove)(programUnit, operand);
	}
	virtual bool recreate(Recreator &recreator, RpnItemPtr &rpnItem)
	{
		if (m_recreate)
		{
			(*m_recreate)(recreator, rpnItem);
			return true;
		}
		return false;
	}

	// INSTANCE ACCESS FUNCTIONS
	Code code() const
	{
		return m_code;
	}
	bool isCode(Code code)
	{
		return m_code == code;
	}
	bool hasCode()
	{
		return m_code != Code{};
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
	bool isCodeWithOperand() const
	{
		return hasFlag(Operand_Flag);
	}
	int precedence() const
	{
		return m_precedence;
	}
	int index() const
	{
		return m_index;
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
	int hasBinaryOperator();

	Table *alternateForReturn(DataType returnDataType);
	Table *firstAlternate(int operand = FirstOperand);
	Table *alternateForOperand(int operand, DataType operandDataType);

	TranslateFunction translateFunction() const
	{
		return m_translate;
	}
	EncodeFunction encodeFunction() const
	{
		return m_encode;
	}
	OperandTextFunction operandTextFunction() const
	{
		return m_operandText;
	}
	RemoveFunction removeFunction() const
	{
		return m_remove;
	}
	RecreateFunction recreateFunction() const
	{
		return m_recreate;
	}

	// STATIC ACCESS FUNCTIONS
	static Table *entry(Code code);
	static Table *entry(Code code, DataType dataType);
	static Table *entry(int index)
	{
		return s_indexToEntry[index];
	}
	static Table *find(const std::string &string);
	static Table *find(const std::string &word1, const std::string &word2)
	{
		return find(word1 + ' ' + word2);
	}

	friend class Erector;

private:
	bool hasName() const
	{
		return not m_name.empty();
	}
	bool isTwoWordCommand() const
	{
		return isCommand() && not m_name2.empty();
	}
	bool isTwoCharacterOperator() const
	{
		return isOperator() && not isalpha(m_name.front())
			&& m_name.length() == 2;
	}
	bool isNotAssignmentOperator() const
	{
		return !hasFlag(Reference_Flag);
	}
	void setMultipleFlag()
	{
		m_flags |= Multiple_Flag;
	}
	void setTwoWordFlag()
	{
		m_flags |= Two_Flag;
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

	// INSTANCE MEMBERS
	Code m_code;
	const std::string m_name;
	const std::string m_name2;
	const std::string m_option;
	unsigned m_flags;
	int m_precedence;
	u_int16_t m_index;
	ExprInfo *m_exprInfo;

	// TODO temporary function pointers; to be replaced with virtual functions
	TranslateFunction m_translate;
	EncodeFunction m_encode;
	OperandTextFunction m_operandText;
	RemoveFunction m_remove;
	RecreateFunction m_recreate;

	// STATIC ACCESS FUNCTIONS
	using EntryVector  = std::vector<Table *>;
	void setIndexAndAddEntry()
	{
		m_index = s_indexToEntry.size();
		s_indexToEntry.push_back(this);
	}

	using NameMap = std::unordered_map<std::string, Table *,
		CaseOptionalHash, CaseOptionalEqual>;
	void addCommandNameToNameMap()
	{
		s_nameToEntry.emplace(commandName(), this);
	}
	void addNameToNameMap()
	{
		s_nameToEntry.emplace(m_name, this);
	}
	Table *findName()
	{
		return find(m_name);
	}
	Table *findTwoName()
	{
		return find(m_name, m_name2);
	}

	using CodeMap = std::unordered_map<Code, Table *, EnumClassHash>;
	bool isNotInCodeMap()
	{
		return s_codeToEntry.find(m_code) == s_codeToEntry.end();
	}
	void addToCodeMap()
	{
		if (hasCode() && isNotInCodeMap())
		{
			s_codeToEntry[code()] = this;
		}
	}

	using EntryVectorArray = std::array<EntryVector, 3>;
	using AlternateMap = std::unordered_map<Table *, EntryVectorArray>;
	void appendAlternate(int operand, Table *alternate)
	{
		s_alternate[this][operand].push_back(alternate);
	}
	EntryVector &alternateVector(int operand)
	{
		return s_alternate[this][operand];
	}
	AlternateMap::const_iterator findAlternate()
	{
		return s_alternate.find(this);
	}
	AlternateMap::const_iterator alternateNotFound()
	{
		return s_alternate.end();
	}

	using ExpectedDataTypeMap = std::unordered_map<Table *, DataType> ;
	ExpectedDataTypeMap::iterator findExpectedDataType()
	{
		return s_expectedDataType.find(this);
	}
	ExpectedDataTypeMap::iterator expectedDataTypeNotFound()
	{
		return s_expectedDataType.end();
	}
	void setExpectedDataType(DataType dataType)
	{
		s_expectedDataType[this] = dataType;
	}
	void eraseExpectedDataType()
	{
		s_expectedDataType.erase(this);
	}
	void addToExpectedDataType(DataType dataType) noexcept;
	void addToExpectedDataType(int operand) noexcept
	{
		addToExpectedDataType(operandDataType(operand));
	}

	// STATIC MEMBERS
	static EntryVector s_indexToEntry;
	static NameMap s_nameToEntry;
	static CodeMap s_codeToEntry;
	static AlternateMap s_alternate;
	static ExpectedDataTypeMap s_expectedDataType;
};


#endif  // TABLE_H
