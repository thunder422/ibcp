// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: programmodel.h - program model class source file
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

#include "programmodel.h"
#include "rpnlist.h"
#include "table.h"
#include "translator.h"


ProgramModel::ProgramModel(QObject *parent) :
	QAbstractListModel(parent),
	m_translator(new Translator(Table::instance()))
{
}


ProgramModel::~ProgramModel(void)
{
	delete m_translator;
}


// function to return data for a given program line

QVariant ProgramModel::data(const QModelIndex &index, int role) const
{
	if (index.isValid())
	{
		if (role == Qt::TextAlignmentRole)
		{
			return int(Qt::AlignRight | Qt::AlignVCenter);
		}
		else if (role == Qt::DisplayRole)
		{
			RpnList *rpnList = m_lineInfo.at(index.row()).rpnList;
			if (!rpnList->hasError())
			{
				return rpnList->text();
			}
			else  // translate error occurred
			{
				Token *token = rpnList->errorToken();
				return QString("%1:%2 %3").arg(token->column())
					.arg(token->length()).arg(rpnList->errorMessage());
			}
		}
	}
	return QVariant();
}


// function to return the number of program lines

int ProgramModel::rowCount(const QModelIndex &parent) const
{
	return m_lineInfo.count();
}


// slot function to receive program updates

void ProgramModel::update(int lineNumber, int linesDeleted, int linesInserted,
	QStringList lines)
{
	int i;
	int oldCount = m_lineInfo.count();
	int count = lines.count();
	for (i = 0; i < count - linesInserted; i++)
	{
		// update changed program lines if they actually changed
		if (updateLine(Change, lineNumber, lines.at(i)))
		{
			// need to emit signal that data changed
			QModelIndex index = this->index(lineNumber);
			emit dataChanged(index, index);
		}
		lineNumber++;
	}
	if (linesDeleted > 0)
	{
		// delete lines from the program
		int lastLineNumber = lineNumber + linesDeleted - 1;
		beginRemoveRows(QModelIndex(), lineNumber, lastLineNumber);
		while (--linesDeleted >= 0)
		{
			updateLine(Delete, lineNumber);
		}
		endRemoveRows();
	}
	else if (linesInserted > 0)
	{
		// insert new lines into the program
		int lastLineNumber = lineNumber + linesInserted - 1;
		beginInsertRows(QModelIndex(), lineNumber, lastLineNumber);
		while (i < count)
		{
			updateLine(Insert, lineNumber++, lines.at(i++));
		}
		endInsertRows();
	}

	if (m_lineInfo.count() != oldCount)
	{
		// emit new line count if changed
		emit lineCountChanged(m_lineInfo.count());
	}
}


bool ProgramModel::updateLine(ModifyMode mode, int lineNumber,
	const QString &line)
{
	RpnList *rpnList;

	if (mode != Delete)
	{
		// compile line (for now just translate)
		rpnList = m_translator->translate(line);
	}

	if (mode == Change)
	{
		LineInfo &lineInfo = m_lineInfo[lineNumber];
		if (*rpnList == *lineInfo.rpnList)
		{
			return false;  // line not changed; nothing more to do here
		}

		// replace the list with the new line
		delete lineInfo.rpnList;
		lineInfo.rpnList = rpnList;

		setError(lineNumber, lineInfo, false);
	}
	else if (mode == Insert)
	{
		LineInfo lineInfo;
		lineInfo.rpnList = rpnList;
		lineInfo.errIndex = -1;

		setError(lineNumber, lineInfo, true);

		m_lineInfo.insert(lineNumber, lineInfo);
	}
	else if (mode == Delete)
	{
		LineInfo &lineInfo = m_lineInfo[lineNumber];

		removeError(lineNumber, lineInfo, true);

		// delete rpn list and remove from list
		delete lineInfo.rpnList;
		m_lineInfo.removeAt(lineNumber);
	}
	return true;
}


// function to insert error into list if line has an error
void ProgramModel::setError(int lineNumber, LineInfo &lineInfo,
	bool lineInserted)
{
	bool hasError = lineInfo.rpnList->hasError();
	if (!hasError)
	{
		removeError(lineNumber, lineInfo, false);
	}
	else if (lineInfo.errIndex != -1)
	{
		// replace current error
		m_errors[lineInfo.errIndex] = ErrorItem(ErrorItem::Translator,
			lineNumber, lineInfo.rpnList->errorToken(),
			lineInfo.rpnList->errorMessage());
	}

	// find location in error list for line number
	int errIndex = m_errors.find(lineNumber);

	if (hasError)
	{
		// insert new error into error list
		m_errors.insert(errIndex, ErrorItem(ErrorItem::Translator,
			lineNumber, lineInfo.rpnList->errorToken(),
			lineInfo.rpnList->errorMessage()));

		lineInfo.errIndex = errIndex++;
	}
	else if (!lineInserted)
	{
		return;  // nothing else to do
	}

	// loop thru rest of errors in list
	for (; errIndex < m_errors.count(); errIndex++)
	{
		if (hasError)
		{
			// adjust error index for inserted error
			m_lineInfo[m_errors[errIndex].lineNumber()].errIndex++;
		}
		if (lineInserted)
		{
			// adjust error line number for inserted line
			m_errors[errIndex].incrementLineNumber();
		}
	}
}


// function to remove an error from the list if line had an error
void ProgramModel::removeError(int lineNumber, LineInfo &lineInfo,
	bool lineDeleted)
{
	int errIndex;
	bool hadError;

	if (lineInfo.errIndex != -1)
	{
		errIndex = lineInfo.errIndex;

		// remove error (for changed line with no error or deleted line)
		m_errors.removeAt(errIndex);

		lineInfo.errIndex = -1;
		hadError = true;
	}
	else if (lineDeleted)
	{
		// find location in error list for line number
		errIndex = m_errors.find(lineNumber);
		hadError = false;
	}
	else
	{
		return;  // nothing else to do
	}

	// loop thru rest of errors in list
	for (; errIndex < m_errors.count(); errIndex++)
	{
		if (hadError)
		{
			// adjust error index for removed error
			m_lineInfo[m_errors[errIndex].lineNumber()].errIndex--;
		}
		if (lineDeleted)
		{
			// adjust error line number for deleted line
			m_errors[errIndex].decrementLineNumber();
		}
	}
}


// end: programmodel.cpp
