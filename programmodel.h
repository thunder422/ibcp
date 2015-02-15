// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: programmodel.h - program model class header file
//	Copyright (C) 2013-2015  Thunder422
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

#ifndef PROGRAMMODEL_H
#define PROGRAMMODEL_H

#include <array>

#include <QAbstractListModel>

#include "ibcp.h"
#include "dictionary.h"
#include "errorlist.h"
#include "programcode.h"
#include "recreator.h"
#include "translator.h"
#include "basic/basic.h"

class RpnList;
class Table;


enum class Operation
{
	Append,
	Insert,
	Change,
	Remove
};


// class for holding a program unit model
class ProgramModel : public QAbstractListModel
{
	Q_OBJECT

public:
	explicit ProgramModel(QObject *parent = 0);
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	void clear(void);

	Dictionary *dictionary(OperandType operandType) const
	{
		return m_dictionary[operandType].get();
	}

	// NOTE temporary functions for testing
	const ErrorItem *lineError(int lineIndex) const
	{
		return m_lineInfo[lineIndex].errIndex == -1
			? nullptr : &m_errors[m_lineInfo[lineIndex].errIndex];
	}
	std::string debugText(int lineIndex, bool fullInfo = false) const;

	// program model access functions
	std::string lineText(int lineIndex);
	void update(int lineNumber, int linesDeleted, int linesInserted,
		std::vector<std::string> &&lines);
	void lineEdited(int lineNumber, int column, bool atLineEnd, int charsAdded,
		int charsRemoved);
	int errorCount(void) const
	{
		return m_errors.count();
	}
	const ErrorItem &errorItem(int errIndex) const
	{
		return m_errors.at(errIndex);
	}

	// error access functions
	bool errorFindNext(int &lineNumber, int &column, bool &wrapped) const;
	bool errorFindPrev(int &lineNumber, int &column, bool &wrapped) const;
	Status errorStatus(int lineNumber) const;

signals:
	void lineCountChanged(int newLineCount);
	void programCleared(void);
	void programChange(int lineNumber);
	void errorInserted(int errIndex, const ErrorItem &errorItem);
	void errorChanged(int errIndex, const ErrorItem &errorItem);
	void errorRemoved(int errIndex);
	void errorListChanged(void);

private:
	struct LineInfo
	{
		int offset;						// offset of line in program
		int size;						// size of line in program
		int errIndex;					// index to error list
		std::string text;				// text of line when line has error
	};
	class LineInfoList
	{
	public:
		// replace size of line at index (adjust line offsets after index)
		void replace(int i, int size)
		{
			adjust(i, size - m_vector[i].size);
			m_vector[i].size = size;
		}

		// insert new line at index (adjust line offsets after index)
		void insert(int i, const LineInfo &t)
		{
			m_vector.insert(m_vector.begin() + i, t);
			adjust(i, t.size);
		}

		// remove line at index (adjust line offsets after index)
		void erase(int i)
		{
			adjust(i, -m_vector[i].size);
			m_vector.erase(m_vector.begin() + i);
		}

		// vector pass through access functions
		std::vector<LineInfo>::const_reference operator[](size_t index) const
		{
			return m_vector[index];
		}
		std::vector<LineInfo>::reference operator[](size_t index)
		{
			return m_vector[index];
		}
		int size() const
		{
			return m_vector.size();
		}
		void clear()
		{
			m_vector.clear();
		}

		// vector element access functions
		int offset(int lineNumber) const
		{
			return m_vector[lineNumber].offset;
		}
		int size(int lineNumber) const
		{
			return m_vector[lineNumber].size;
		}

	private:
		// adjust offset of all lines after index by size
		void adjust(unsigned i, int size)
		{
			while (++i < m_vector.size())
			{
				m_vector[i].offset += size;
			}
		}

		std::vector<LineInfo> m_vector;		// line information vector
	};

	bool updateLine(Operation operation, int lineNumber,
		std::string &&line = {});
	void updateError(int lineNumber, LineInfo &lineInfo,
		const ErrorItem &errorItem, bool lineInserted);
	void removeError(int lineNumber, LineInfo &lineInfo, bool lineDeleted);
	ProgramCode encode(RpnList &&input);
	void dereference(const LineInfo &lineInfo);
	RpnList decode(const LineInfo &lineInfo);

	ProgramLineReader createProgramLineReader(const LineInfo &lineInfo) const
	{
		return ProgramLineReader {this, m_code.begin(), lineInfo.offset,
			lineInfo.size};
	}

	// program code variables
	LineInfoList m_lineInfo;			// program line information list
	ProgramCode m_code;					// code for program unit lines
	ErrorList m_errors;					// list of program errors

	std::array<std::unique_ptr<Dictionary>, NumberOf_OperandType> m_dictionary;
};


#endif // PROGRAMMODEL_H
