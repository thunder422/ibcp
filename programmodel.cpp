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

ProgramModel::ProgramModel(QObject *parent) :
	QAbstractListModel(parent)
{
}


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
			return m_lines.at(index.row());
		}
	}
	return QVariant();
}


// function to return the number of program lines

int ProgramModel::rowCount(const QModelIndex &parent) const
{
	return m_lines.count();
}


// slot function to received program updates

void ProgramModel::update(int lineNumber, int linesDeleted, int linesInserted,
	QStringList lines)
{
	int i;
	int oldCount = m_lines.count();
	int count = lines.count();
	for (i = 0; i < count - linesInserted; i++)
	{
		// update changed program lines if they actually changed
		if (lines.at(i) != m_lines.at(lineNumber))
		{
			m_lines[lineNumber] = lines.at(i);

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
			m_lines.insert(lineNumber++, lines.at(i++));
		}
		endInsertRows();
	}

	if (m_lines.count() != oldCount)
	{
		// emit new line count if changed
		emit lineCountChanged(m_lines.count());
	}
}
