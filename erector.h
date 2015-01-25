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
	m_entry->setIndexAndAddEntry();
	m_entry->addToCodeMap();
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


inline bool Erector::addTwoWordCommandToNameMap()
{
	if (not m_entry->isTwoWordCommand())
	{
		return false;
	}
	if (m_entry->findTwoName())
	{
		throw "Multiple two-word command '" + m_entry->commandName() + '\'';
	}
	m_entry->addCommandNameToNameMap();
	return true;
}


inline bool Erector::addNewPrimaryToNameMap() noexcept
{
	if ((m_primary = m_entry->findName()))
	{
		return false;
	}
	addToNameMap();
	return true;
}


inline void Erector::addToNameMap() noexcept
{
	checkIfNotHomogeneousOperator();
	m_entry->addNameToNameMap();
	if (m_entry->hasOperands())
	{
		m_entry->addToExpectedDataType(m_entry->isOperator()
			? m_entry->lastOperand() : m_entry->firstOperand());
	}
}


inline void Erector::replacePrimary() noexcept
{
	Table *alternate = m_primary;
	m_primary = m_entry;
	m_entry->appendAlternate(alternate->lastOperand(), alternate);
	if (m_entry->isFunction())
	{
		m_entry->setMultipleFlag();
		alternate->eraseExpectedDataType();
	}
	m_entry->addToExpectedDataType(m_entry->firstOperand());
}


inline bool Erector::addAlternateForOperatorWithMoreOperands()
{
	if (m_entry->isOperaratorWithMoreOperandsThan(m_primary))
	{
		auto &vector = m_primary->alternateVector(m_entry->lastOperand());
		if (vector.empty())
		{
			checkIfNotHomogeneousOperator();
			vector.push_back(m_entry);
			m_entry->addToExpectedDataType(m_entry->firstOperand());
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
	for (Table *&alternate : m_primary->alternateVector(m_operand))
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
	alternate->eraseExpectedDataType();
	m_entry->addToExpectedDataType(m_operand);
}


inline void Erector::addAlternateToPrimary() noexcept
{
	m_primary->appendAlternate(m_operand, m_alternate);
	if (m_operand == m_entry->firstOperand() && m_entry->isBinaryOperator())
	{
		m_alternate->addToExpectedDataType(m_operand);
	}
	else
	{
		m_primary->addToExpectedDataType(
			m_alternate->operandDataType(m_operand));
	}
}


inline void Erector::checkIfFunctionIsMulipleEntry()
{
	if (m_entry->isFunction() && m_entry->hasMoreOperandsThan(m_primary))
	{
		m_primary->setMultipleFlag();
		m_primary->appendAlternate(m_entry->lastOperand(), m_entry);
	}
	else
	{
		throw "Multiple entries with same operand data types ("
			+ std::to_string(m_entry->index()) + ','
			+ std::to_string(m_primary->index()) + ')';
	}
}


#endif  // ERECTOR_H
