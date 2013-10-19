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
			// copy new line in
			memmove(lineBegin, line.data(), line.size() * sizeof(ProgramWord));
		}
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

	Dictionary *remDictionary(void) const
	{
		return m_remDictionary;
	}

	InfoDictionary<ConstNumInfo> *constNumDictionary(void) const
	{
		return m_constNumDictionary;
	}
	InfoDictionary<ConstStrInfo> *constStrDictionary(void) const
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

	QString operandText(Code code, int operand);

	// NOTE temporary functions for testing
	RpnList *rpnList(int lineIndex) const
	{
		return m_lineInfo.at(lineIndex).rpnList;
	}
	int lineOffset(int lineIndex) const
	{
		return m_lineInfo.at(lineIndex).offset;
	}
	int lineSize(int lineIndex) const
	{
		return m_lineInfo.at(lineIndex).size;
	}
	QString debugText(int lineIndex);

signals:
	void lineCountChanged(int newLineCount);
	void errorListChanged(const ErrorList &errorList);

public slots:
	void update(int lineNumber, int linesDeleted, int linesInserted,
		QStringList lines);

private:
	struct LineInfo
	{
		RpnList *rpnList;				// REMOVE pointer to rpn list
		int offset;						// offset of line in program
		int size;						// size of line in program
		int errIndex;					// index to error list
	};

	bool updateLine(Operation operation, int lineNumber,
		const QString &line = QString());
	void updateError(int lineNumber, LineInfo &lineInfo, bool lineInserted);
	void removeError(int lineNumber, LineInfo &lineInfo, bool lineDeleted);
	ProgramCode encode(RpnList *input);

	Table &m_table;						// reference to the table object
	Translator *m_translator;			// program line translator instance

	// program code variables
	QList<LineInfo> m_lineInfo;			// program line information list
	ProgramCode m_code;					// code for program unit lines
	ErrorList m_errors;					// list of program errors

	// pointers to the global program dictionaries
	Dictionary *m_remDictionary;
	InfoDictionary<ConstNumInfo> *m_constNumDictionary;
	ConstStrDictionary *m_constStrDictionary;

	// pointers to the local unit dictionaries
	Dictionary *m_varDblDictionary;
	Dictionary *m_varIntDictionary;
	Dictionary *m_varStrDictionary;
};


#endif // PROGRAMMODEL_H
