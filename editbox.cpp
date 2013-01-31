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
#include <QTextBlock>

#include "editbox.h"


EditBox::EditBox(QWidget *parent) :
	QTextEdit(parent),
	m_lineModified(-1),
	m_lineModType(LineChanged),
	m_undoActive(false),
	m_ignoreChange(false)
{
	// set to only paste plain text into the edit box
	setAcceptRichText(false);

	// set the edit box to a fixed width font
	QFont font = currentFont();
	font.setFixedPitch(true);
	font.setFamily("Monospace");
	font.setStyleHint(QFont::Monospace);
	setCurrentFont(font);

	// connect to catch document changes
	connect(document(), SIGNAL(contentsChanged()),
		this, SLOT(documentChanged()));

	// connect to catch cursor position changes
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(cursorMoved()));

	// connect to catch when undo commands are added
	connect(document(), SIGNAL(undoCommandAdded()),
		this, SLOT(undoAdded()));
}


void EditBox::keyPressEvent(QKeyEvent *event)
{
	QTextCursor cursor = textCursor();

	switch (event->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
		// intercept Control+Return and change it to a Return event
		if (event->modifiers() & Qt::ControlModifier
			|| cursor.atBlockEnd() && !cursor.atBlockStart())
		{
			insertNewLine();
			return;
		}
		else  // intercept Return when cursor is not at the end of a line
		{
			moveCursor(QTextCursor::NextBlock);
			return;
		}
		break;

	default:  // check for key sequences
		if (event->matches(QKeySequence::Undo))
		{
			undo();  // do undo here
			return;
		}
		if (event->matches(QKeySequence::Redo))
		{
			redo();  // do redo here
			return;
		}
		if (event->matches(QKeySequence::Delete))
		{
			if (cursor.atBlockEnd() && !cursor.atEnd()
				 && !cursor.hasSelection())
			{
				// next line is about to be deleted
				emit linesDeleted(cursor.blockNumber() + 1, 1);
			}
		}
		if (event->key() == Qt::Key_Backspace)
		{
			if (cursor.atBlockStart() && !cursor.atStart()
				 && !cursor.hasSelection())
			{
				// current line is about to be deleted
				emit linesDeleted(cursor.blockNumber(), 1);
			}
		}
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


// function to reset the document modified flag and other variables

void EditBox::resetModified(void)
{
	document()->setModified(false);
	m_lineModified = -1;
}


// function to record the current line number when the document was changed
//
//   - if indicated to ignore change, then just returns
//   - if no modified line, then set modified line to current line
//   - resets undo active flag, don't track undos as line changes

void EditBox::documentChanged(void)
{
	if (m_ignoreChange)
	{
		m_ignoreChange = false;
	}
	else
	{
		if (m_lineModified == -1)
		{
			m_lineModified = textCursor().blockNumber();
			m_lineModCount = 0;
			qDebug("Line #%d MODIFIED", m_lineModified);
		}
		m_undoActive = false;
	}
}


// function to check if cursor was moved from the modified line

void EditBox::cursorMoved(void)
{
	if (m_lineModified >= 0 && m_lineModified != textCursor().blockNumber())
	{
		// there is a modified line and cursor moved from that line
		captureModifiedLine();
	}
}


// function to keep track of when undo commands are added

void EditBox::undoAdded(void)
{
	// if modified line, then increment number of changes on line
	if (m_lineModified >= 0)
	{
		m_lineModCount++;
	}
}


// function to catch the undo action before passing it on to the base class
//
//   - if there is no modified line or undo is active then indicates that
//     document changes from undo should be ignored (don't set modified line)
//   - calls the base class undo to perform the undo
//   - if the undo moved the cursor, then set the modified line and set
//     to indicate that undo is active (count undos as line modifications)
//   - if cursor didn't move and undo is active, increment line change count
//   - else if line change count is zero, then reset the modified line
//   - else decrement the line change count

void EditBox::undo(void)
{
	// if no modified line or undo active, indicate to ignore next change
	if (m_lineModified == -1 || m_undoActive)
	{
		m_ignoreChange = true;
	}
	int line = textCursor().blockNumber();
	QTextEdit::undo();
	m_ignoreChange = false;  // reset flag if still set

	if (line != textCursor().blockNumber())
	{
		// set new modified line and indicate undo active if on a new line
		m_lineModified = textCursor().blockNumber();
		m_lineModCount = 0;
		m_undoActive = true;
	}
	else if (m_undoActive)
	{
		m_lineModCount++;  // increment line change count if undo active
	}
	else if (m_lineModCount == 0)
	{
		m_lineModified = -1;  // if last change on line, reset modified line
	}
	else
	{
		m_lineModCount--;  // otherwise, decrement line change count
	}
}


// function to catch the redo action before passing it on to the base class
//
//   - if there is no modified line then indicates that document changes
//     from redo should be ignored (don't set the modified line)
//   - if there is no modified line, then set the modified line
//   - else if undo active, then if the line change count is zero, deactivates
//     undo, otherwise decrements the line change count
//   - else increments the line change count

void EditBox::redo(void)
{
	if (m_lineModified == -1)
	{
		m_ignoreChange = true;
	}
	QTextEdit::redo();
	m_ignoreChange = false;  // reset flag if still set

	if (m_lineModified == -1)
	{
		// set the modified line if there was none
		m_lineModified = textCursor().blockNumber();
		m_lineModCount = 0;
	}
	else if (m_undoActive)
	{
		if (m_lineModCount == 0)
		{
			m_undoActive = false;  // if all undos use, then deactivate undo
		}
		else
		{
			m_lineModCount--;  // otherwise, decrement line change count
		}
	}
	else
	{
		m_lineModCount++;  // increment line change count if undo not active
	}
}


// function to insert a new line and make sure lines are emitted as modified
//
//   - if cursor is at end of line, capture this line if it has been modified
//   - else mark current line as being modified since it is being split
//   - after new line inserted, mark new line as modified and inserted

void EditBox::insertNewLine(void)
{
	if (textCursor().atBlockEnd())
	{
		// cursor at end of line, capture this line if it has been modified
		captureModifiedLine();
	}
	else
	{
		// mark current line as being modified  since it is being split
		m_lineModified = textCursor().blockNumber();
	}
	m_ignoreChange = true;
	textCursor().insertText("\n");

	// mark this new line as modified and inserted
	m_lineModified = textCursor().blockNumber();
	m_lineModCount = 0;
	m_lineModType = LineInserted;
}


// function to check if current line was modified and to process it

void EditBox::captureModifiedLine(void)
{
	if (m_lineModified >= 0)
	{
		QString line = document()->findBlockByNumber(m_lineModified).text();
		if (m_lineModType == LineChanged)
		{
			emit lineChanged(m_lineModified, line);
		}
		else
		{
			emit linesInserted(m_lineModified, QStringList() << line);
		}

		m_lineModified = -1;  // line processed, reset modified line number
		m_lineModType = LineChanged;
	}
}


// end: editbox.cpp
