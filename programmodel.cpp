// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: programmodel.h - program model functions
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
			RpnList *rpnList = m_linesTranslated.at(index.row());
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
	return m_linesTranslated.count();
}


// slot function to received program updates

void ProgramModel::update(int lineNumber, int linesDeleted, int linesInserted,
	QStringList lines)
{
	int i;
	int oldCount = m_linesTranslated.count();
	int count = lines.count();
	for (i = 0; i < count - linesInserted; i++)
	{
		// update changed program lines if they actually changed
		RpnList *rpnList = m_translator->translate(lines.at(i));
		if (*rpnList != *m_linesTranslated.at(lineNumber))
		{
			m_lines[lineNumber] = lines.at(i);

			// delete old list, translate line and store new list
			delete m_linesTranslated[lineNumber];
			m_linesTranslated[lineNumber] = rpnList;

			// need to emit signal that data changed
			QModelIndex index = this->index(lineNumber);
			emit dataChanged(index, index);
		}
		lineNumber++;
	}
	if (linesDeleted > 0)
	{
		// delete lines from the program
		beginRemoveRows(QModelIndex(), lineNumber, lineNumber + linesDeleted
			- 1);
		while (--linesDeleted >= 0)
		{
			m_lines.removeAt(lineNumber);
			// delete rpn list and remove from list
			delete m_linesTranslated[lineNumber];
			m_linesTranslated.removeAt(lineNumber);
		}
		endRemoveRows();
	}
	else if (linesInserted > 0)
	{
		// insert new lines into the program
		beginInsertRows(QModelIndex(), lineNumber, lineNumber + linesInserted
			- 1);
		while (i < count)
		{
			m_lines.insert(lineNumber, lines.at(i));
			// translate new line and insert into list
            m_linesTranslated.insert(lineNumber++,
				m_translator->translate(lines.at(i++)));
		}
		endInsertRows();
	}

	if (m_linesTranslated.count() != oldCount)
	{
		// emit new line count if changed
		emit lineCountChanged(m_linesTranslated.count());
	}
}
