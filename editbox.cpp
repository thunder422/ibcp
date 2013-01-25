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
}


void EditBox::keyPressEvent(QKeyEvent *event)
{
	QTextCursor cursor = textCursor();

	switch (event->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
		// intercept Control+Return and change it to a Return event
		if (event->modifiers() & Qt::ControlModifier)
		{
			cursor.insertText("\n");
			return;
		}
		// intercept Return when cursor is not at the end of a line
		if (!cursor.atBlockEnd() || cursor.atBlockStart())
		{
			moveCursor(QTextCursor::NextBlock);
			return;
		}
		break;
	}
	QTextEdit::keyPressEvent(event);
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
