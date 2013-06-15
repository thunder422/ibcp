// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: programlinedelegate.cpp - program line delegate class source file
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

#include <QListView>
#include <QPainter>

#include "programlinedelegate.h"

ProgramLineDelegate::ProgramLineDelegate(int baseLineNumber,
	QListView *programView, QObject *parent) :
	QStyledItemDelegate(parent),
	m_programView(programView),
	m_baseLineNumber(baseLineNumber),
	m_lineNumberWidth(0)
{
	m_digitWidth = m_programView->fontMetrics().width(QLatin1Char('9'));
}


// function to update the width needed for the line number area

void ProgramLineDelegate::lineNumberWidthUpdate(int newLineCount)
{
	int digits = 2;  // plus 2 * 0.5 spacing on either side
	while (newLineCount > 10 - m_baseLineNumber)
	{
		newLineCount /= 10;
		digits++;
	}
	int width = m_digitWidth * digits;
	if (width != m_lineNumberWidth)
	{
		m_lineNumberWidth = width;
		// line number width changed so need to force program view to redraw
		m_programView->update(m_programView->rect());
	}
}


// function that paints one program line to the program view list widget

void ProgramLineDelegate::paint(QPainter *painter,
	const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	// draw line number with gray background
	QRect rect = option.rect;
	rect.setWidth(m_lineNumberWidth);
	painter->fillRect(rect, Qt::lightGray);
	rect.setLeft(rect.left() + m_digitWidth / 2);
	rect.setWidth(m_lineNumberWidth - m_digitWidth);
	QString text = QString("%1").arg(index.row() + m_baseLineNumber);
	painter->drawText(rect, Qt::AlignRight, text);

	// draw program line text
	rect.setLeft(rect.left() + m_lineNumberWidth);
	rect.setWidth(option.rect.width() - m_lineNumberWidth);
	text = index.model()->data(index, Qt::DisplayRole).toString();
	painter->drawText(rect, text);
}


// end: programlinedelegate.cpp
