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
#include <QStringList>
#include <QVector>

#include "ibcp.h"
#include "errorlist.h"

class RpnList;
class Translator;


// class for holding and accessing a program word
class ProgramWord
{
	unsigned short m_word;				// one program word

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
	QString instructionText(void) const;

	// operand word access functions
	unsigned short operand(void) const
	{
		return m_word;
	}
	void setOperand(unsigned short operand)
	{
		m_word = operand;
    }
    QString operandText(void) const;
};


// class for hold a vector of program words representing a program line
class ProgramLine : public QVector<ProgramWord>
{
public:
	ProgramLine(int size): QVector<ProgramWord>(size)
	{

	}
	QString text(void);
};


class ProgramModel : public QAbstractListModel
{
	Q_OBJECT
public:
	explicit ProgramModel(QObject *parent = 0);
	~ProgramModel(void);
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;

signals:
	void lineCountChanged(int newLineCount);
	void errorListChanged(const ErrorList &errorList);

public slots:
	void update(int lineNumber, int linesDeleted, int linesInserted,
		QStringList lines);

private:
	struct LineInfo
	{
		RpnList *rpnList;				// pointer to rpn list
		int errIndex;					// index to error list
	};
	bool updateLine(Operation operation, int lineNumber,
		const QString &line = QString());
	void setError(int lineNumber, LineInfo &lineInfo, bool lineInserted);
	void removeError(int lineNumber, LineInfo &lineInfo, bool lineDeleted);

	Translator *m_translator;			// program line translator instance
	QList<LineInfo> m_lineInfo;			// program line information list
	ErrorList m_errors;					// list of program errors
};

#endif // PROGRAMMODEL_H
