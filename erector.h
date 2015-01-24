// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: erector.h - table erector class header file
//	Copyright (C) 2015  Thunder422
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

#ifndef ERECTOR_H
#define ERECTOR_H

#include <iostream>

#include "table.h"


class Erector
{
public:
	Erector(Table *entry) : m_entry {entry} {}
	void operator()();

private:
	void addToCodeMap() noexcept;
	bool addTwoWordCommandToNameMap();
	bool addNewPrimaryToNameMap() noexcept;
	void addToNameMap() noexcept;
	void replacePrimary() noexcept;
	bool addAlternateForOperatorWithMoreOperands();
	void checkIfNotHomogeneousOperator() const;
	bool addAlternateWithDifferentOperandDataType() noexcept;
	bool addAlternate() noexcept;
	bool sameOperandDataTypeOrShouldBeFirstAlternate() noexcept;
	bool replaceFirstAlternateIfHomogeneousOperator(Table *alternate) noexcept;
	void replaceExpectedDataType(Table *alternate) noexcept;
	void addAlternateToPrimary() noexcept;
	void checkIfFunctionIsMulipleEntry();
	void addToExpectedDataType(Table *entry, int operand) noexcept;
	void addToExpectedDataType(Table *entry, DataType dataType) noexcept;

	Table *m_entry;
	Table *m_alternate;
	Table *m_primary;
	int m_operand;
};


//===============================
//  TABLE ERECTOR FUNCTIONS
//===============================

inline void Erector::operator()()
try
{
	addToCodeMap();
	if (not addTwoWordCommandToNameMap()
		&& m_entry->hasName() && not addNewPrimaryToNameMap()
		&& m_entry->hasOperandsAndIsNotAssignmentOperator())
	{
		if (m_entry->hasLessOperandsThan(m_primary))
		{
			replacePrimary();
		}
		else if (not addAlternateForOperatorWithMoreOperands()
			&& not addAlternateWithDifferentOperandDataType())
		{
			checkIfFunctionIsMulipleEntry();
		}
	}
}
catch (std::string &error)
{
	std::cerr << "Table Error: " << error << std::endl;
	abort();
}


inline void Erector::addToCodeMap() noexcept
{
	if (m_entry->code() != Code{}
		&& Table::s_codeToEntry.find(m_entry->code())
		== Table::s_codeToEntry.end())
	{
		Table::s_codeToEntry[m_entry->code()] = m_entry;
	}
}


inline bool Erector::addTwoWordCommandToNameMap()
{
	if (not m_entry->isTwoWordCommand())
	{
		return false;
	}
	if (Table::find(m_entry->name(), m_entry->name2()))
	{
		throw "Multiple two-word command '" + m_entry->commandName() + '\'';
	}
	Table::s_nameToEntry.emplace(m_entry->commandName(), m_entry);
	return true;
}


inline bool Erector::addNewPrimaryToNameMap() noexcept
{
	if ((m_primary = Table::find(m_entry->name())))
	{
		return false;
	}
	addToNameMap();
	return true;
}


inline void Erector::addToNameMap() noexcept
{
	checkIfNotHomogeneousOperator();
	Table::s_nameToEntry.emplace(m_entry->name(), m_entry);
	if (m_entry->hasOperands())
	{
		addToExpectedDataType(m_entry, m_entry->isOperator()
			? m_entry->lastOperand() : m_entry->firstOperand());
	}
}


inline void Erector::replacePrimary() noexcept
{
	Table *alternate = m_primary;
	m_primary = m_entry;
	Table::s_alternate[m_entry][alternate->lastOperand()].push_back(alternate);
	if (m_entry->isFunction())
	{
		m_entry->setMultipleFlag();
		Table::s_expectedDataType.erase(alternate);
	}
	addToExpectedDataType(m_entry, m_entry->firstOperand());
}


inline bool Erector::addAlternateForOperatorWithMoreOperands()
{
	if (m_entry->isOperaratorWithMoreOperandsThan(m_primary))
	{
		Table::EntryVector &vector = Table::s_alternate[m_primary]
			[m_entry->lastOperand()];
		if (vector.empty())
		{
			checkIfNotHomogeneousOperator();
			vector.push_back(m_entry);
			addToExpectedDataType(m_entry, m_entry->firstOperand());
			return true;
		}
		m_primary = vector.front();
	}
	return false;
}


inline bool Erector::addAlternateWithDifferentOperandDataType() noexcept
{
	for (m_operand = m_entry->firstOperand();
		m_operand <= m_primary->lastOperand(); ++m_operand)
	{
		if (m_entry->hasDifferentOperandDataType(m_primary, m_operand)
			&& addAlternate())
		{
			return true;
		}
	}
	return false;
}


inline bool Erector::addAlternate() noexcept
{
	m_alternate = m_entry;
	do
	{
		if (not sameOperandDataTypeOrShouldBeFirstAlternate())
		{
			addAlternateToPrimary();
			return true;
		}
	}
	while (++m_operand <= m_entry->lastOperand());
	return false;
}


inline bool Erector::sameOperandDataTypeOrShouldBeFirstAlternate() noexcept
{
	for (Table *&alternate : Table::s_alternate[m_primary][m_operand])
	{
		if (m_entry->hasSameOperandDataType(alternate, m_operand))
		{
			if (replaceFirstAlternateIfHomogeneousOperator(alternate))
			{
				alternate = m_entry;
			}
			return true;
		}
	}
	return false;
}


inline bool Erector::replaceFirstAlternateIfHomogeneousOperator(
	Table *alternate) noexcept
{
	if (m_entry->isHomogeneousOperator())
	{
		replaceExpectedDataType(alternate);
		m_alternate = alternate;
		m_primary = m_entry;
		return true;
	}
	return false;
}


inline void Erector::replaceExpectedDataType(Table *alternate)
	noexcept
{
	Table::s_expectedDataType.erase(alternate);
	addToExpectedDataType(m_entry, m_operand);
}


inline void Erector::addAlternateToPrimary() noexcept
{
	Table::s_alternate[m_primary][m_operand].push_back(m_alternate);
	if (m_operand == m_entry->firstOperand() && m_entry->isBinaryOperator())
	{
		addToExpectedDataType(m_alternate, m_operand);
	}
	else
	{
		addToExpectedDataType(m_primary,
			m_alternate->operandDataType(m_operand));
	}
}


inline void Erector::checkIfFunctionIsMulipleEntry()
{
	if (m_entry->isFunction() && m_entry->hasMoreOperandsThan(m_primary))
	{
		m_primary->setMultipleFlag();
		Table::s_alternate[m_primary][m_entry->lastOperand()]
			.push_back(m_entry);
	}
	else
	{
		throw "Multiple entries with same operand data types ("
			+ std::to_string(m_entry->index()) + ','
			+ std::to_string(m_primary->index()) + ')';
	}
}


inline void Erector::addToExpectedDataType(Table *entry, int operand)
	noexcept
{
	addToExpectedDataType(entry, entry->operandDataType(operand));
}


#endif  // ERECTOR_H
