// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: programmodel.h - program model class header file
//	Copyright (C) 2013  Thunder422
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
//	2013-03-15	initial version

#ifndef PROGRAMMODEL_H
#define PROGRAMMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QStringList>
#include <QVector>

#include "ibcp.h"
#include "dictionary.h"
#include "errorlist.h"
#include "basic/basic.h"

class Recreator;
class RpnList;
class Table;
class Translator;


// class for holding and accessing a program word
class ProgramWord
{
public:
	// instruction access functions
	Code instructionCode(void) const
	{
		return (Code)(m_word & ProgramMask_Code);
	}
	int instructionSubCode(void) const
	{
		return m_word & ProgramMask_SubCode;
	}
	bool instructionHasSubCode(int subCode) const
	{
		return (m_word & subCode) != 0;
	}
	void setInstruction(Code code, unsigned subCode)
	{
		m_word = (unsigned)code | subCode & ProgramMask_SubCode;
	}
	QString instructionDebugText(void) const;

	// operand word access functions
	unsigned short operand(void) const
	{
		return m_word;
	}
	void setOperand(unsigned short operand)
	{
		m_word = operand;
	}
	QString operandDebugText(QString text) const;

private:
	unsigned short m_word;				// one program word
};


// class for holding the program code
class ProgramCode : public QVector<ProgramWord>
{
public:
	ProgramCode(void): QVector<ProgramWord>() { }
	ProgramCode(int size): QVector<ProgramWord>(size) { }

	void insertLine(int i, const ProgramCode &line)
	{
		if (line.size() > 0)
		{
			int oldSize = size();
			resize(oldSize + line.size());
			ProgramWord *lineBegin = data() + i;
			if (i != oldSize)  // not inserting line at end?
			{
				// make hole in code for new line
				memmove(lineBegin + line.size(), lineBegin, (oldSize - i)
					* sizeof(ProgramWord));
			}
			// copy new line into program
			memmove(lineBegin, line.data(), line.size() * sizeof(ProgramWord));
		}
	}

	void removeLine(int i, int n)
	{
		if (n > 0)  // something to remove?
		{
			remove(i, n);
		}
	}

	void replaceLine(int i, int n, const ProgramCode &line)
	{
		if (line.count() == 0)
		{
			// no new line, just remove old line
			removeLine(i, n);
			return;
		}
		ProgramWord *offset;
		if (line.count() > n)  // new line larger?
		{
			// make program larger before moving program after line up
			resize(count() - n + line.count());
			offset = data() + i;  // get data, resize may move data

			int moveCount = count() - i - line.count();
			if (moveCount > 0)  // not at end?
			{
				memmove(offset + line.count(), offset + n,
					moveCount * sizeof(ProgramWord));
			}
		}
		else  // new line smaller or same size
		{
			offset = data() + i;
			if (line.count() < n)  // new line smaller?
			{
				// move program after line down before making program smaller
				offset = data() + i;
				int moveCount = count() - i - n;
				if (moveCount > 0)  // not at end?
				{
					memmove(offset + line.count(), offset + n,
						moveCount * sizeof(ProgramWord));
				}
				resize(count() - n + line.count());
				offset = data() + i;  // get data again, resize may move data
			}
		}
		// copy new line into program
		memmove(offset, line.data(), line.count() * sizeof(ProgramWord));
	}
};


// class for holding a program unit model
class ProgramModel : public QAbstractListModel
{
	Q_OBJECT

public:
	explicit ProgramModel(QObject *parent = 0);
	~ProgramModel(void);
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	void clear(void);

	Dictionary *remDictionary(void) const
	{
		return m_remDictionary;
	}

	InfoDictionary *constNumDictionary(void) const
	{
		return m_constNumDictionary;
	}
	InfoDictionary *constStrDictionary(void) const
	{
		return m_constStrDictionary;
	}

	Dictionary *varDblDictionary(void) const
	{
		return m_varDblDictionary;
	}
	Dictionary *varIntDictionary(void) const
	{
		return m_varIntDictionary;
	}
	Dictionary *varStrDictionary(void) const
	{
		return m_varStrDictionary;
	}

	// NOTE temporary functions for testing
	int lineOffset(int lineIndex) const
	{
		return m_lineInfo.at(lineIndex).offset;
	}
	int lineSize(int lineIndex) const
	{
		return m_lineInfo.at(lineIndex).size;
	}
	const ErrorItem *lineError(int lineIndex) const
	{
		if (m_lineInfo.at(lineIndex).errIndex == -1)
		{
			return NULL;
		}
		else
		{
			return &m_errors[m_lineInfo.at(lineIndex).errIndex];
		}
	}
	QString debugText(int lineIndex, bool fullInfo = false) const;
	QString dictionariesDebugText(void);

	// program model access functions
	QString lineText(int lineIndex);
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
	QString errorMessage(int lineNumber) const;

signals:
	void lineCountChanged(int newLineCount);
	void programCleared(void);
	void programChange(int lineNumber);
	void errorInserted(int errIndex, const ErrorItem &errorItem);
	void errorChanged(int errIndex, const ErrorItem &errorItem);
	void errorRemoved(int errIndex);
	void errorListChanged(void);

public slots:
	void update(int lineNumber, int linesDeleted, int linesInserted,
		QStringList lines);

private:
	struct LineInfo
	{
		int offset;						// offset of line in program
		int size;						// size of line in program
		int errIndex;					// index to error list
	};
	class LineInfoList : public QList<LineInfo>
	{
		// adjust offset of all lines after index by size
		void adjust(int i, int size)
		{
			while (++i < count())
			{
				(*this)[i].offset += size;
			}
		}

	public:
		// replace size of line at index (adjust line offsets after index)
		void replace(int i, int size)
		{
			adjust(i, size - at(i).size);
			(*this)[i].size = size;
		}

		// insert new line at index (adjust line offsets after index)
		void insert(int i, const LineInfo &t)
		{
			QList<LineInfo>::insert(i, t);
			adjust(i, t.size);
		}

		// remove line at index (adjust line offsets after index)
		void removeAt(int i)
		{
			adjust(i, -at(i).size);
			QList<LineInfo>::removeAt(i);
		}
	};

	bool updateLine(Operation operation, int lineNumber,
		const QString &line = QString());
	void updateError(int lineNumber, LineInfo &lineInfo,
		const ErrorItem &errorItem, bool lineInserted);
	void removeError(int lineNumber, LineInfo &lineInfo, bool lineDeleted);
	ProgramCode encode(RpnList *input);
	void dereference(const LineInfo &lineInfo);
	RpnList *decode(const LineInfo &lineInfo);

	Table &m_table;						// reference to the table object
	Translator *m_translator;			// program unit translator instance
	Recreator *m_recreator;				// program unit recreator instance

	// program code variables
	LineInfoList m_lineInfo;			// program line information list
	ProgramCode m_code;					// code for program unit lines
	ErrorList m_errors;					// list of program errors

	// pointers to the global program dictionaries
	Dictionary *m_remDictionary;
	ConstNumDictionary *m_constNumDictionary;
	ConstStrDictionary *m_constStrDictionary;

	// pointers to the local unit dictionaries
	Dictionary *m_varDblDictionary;
	Dictionary *m_varIntDictionary;
	Dictionary *m_varStrDictionary;
};


#endif // PROGRAMMODEL_H
