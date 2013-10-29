// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: programlinedelegate.h - program line delegate class header file
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
//	2013-03-10	initial version

#ifndef PROGRAMLINEDELEGATE_H
#define PROGRAMLINEDELEGATE_H

#include <QStyledItemDelegate>

class QListView;

class ProgramLineDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit ProgramLineDelegate(int baseLineNumber, QListView *programView,
		QObject *parent = 0);
	void paint(QPainter *painter, const QStyleOptionViewItem &option,
		const QModelIndex &index) const;

signals:

public slots:
	void lineNumberWidthUpdate(int newLineCount);

private:
	QListView *m_programView;			// pointer to program view widget
	int m_baseLineNumber;				// first line number
	int m_digitWidth;					// pixel width of one digit
	int m_lineNumberWidth;				// width of line number area
};

#endif // PROGRAMLINEDELEGATE_H
