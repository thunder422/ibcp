// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: editbox.h - edit box functions
//	Copyright (C) 2012  Thunder422
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
//	2012-12-29	initial version

#include <QApplication>
#include <QEvent>
#include <QKeyEvent>

#include "editbox.h"


EditBox::EditBox(QWidget *parent) :
	QTextEdit(parent)
{
	// set to only paste plain text into the edit box
	setAcceptRichText(false);

	// set the edit box to a fixed width font
	QFont font = currentFont();
	font.setFixedPitch(true);
	font.setFamily("Monospace");
	font.setStyleHint(QFont::Monospace);
	setCurrentFont(font);

	// install event filter to catch keyboard events
	installEventFilter(this);
}


bool EditBox::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		if ((keyEvent->key() == Qt::Key_Return
			|| keyEvent->key() == Qt::Key_Enter)
			&& keyEvent->modifiers() & Qt::ControlModifier)
		{
			keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Return,
				Qt::NoModifier);
			QApplication::postEvent(this, keyEvent);
			return true;
		}
	}
	return QTextEdit::eventFilter(obj, event);
}


// function to delete the currently select text (not to clipboard)

void EditBox::remove(void)
{
	textCursor().removeSelectedText();
}


// function to select all of the text in the edit box

void EditBox::selectAll(void)
{
	QTextCursor cursor = textCursor();
	cursor.select(QTextCursor::Document);
	setTextCursor(cursor);
}


// end: editbox.cpp
