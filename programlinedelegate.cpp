// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: programlinedelegate.cpp - program line delegate functions
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

#include <QPainter>

#include "programlinedelegate.h"

ProgramLineDelegate::ProgramLineDelegate(QObject *parent) :
	QItemDelegate(parent)
{
}

void ProgramLineDelegate::paint(QPainter *painter,
	const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QString text = QString("%1: %2").arg(index.row())
		.arg(index.model()->data(index, Qt::DisplayRole).toString());
	painter->drawText(option.rect, text);
}
