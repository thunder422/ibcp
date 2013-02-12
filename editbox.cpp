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
#include <QClipboard>
#include <QEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QTextBlock>

#include "editbox.h"


EditBox::EditBox(QWidget *parent) :
	QPlainTextEdit(parent),
	m_lineModified(-1),
	m_lineModType(LineChanged),
	m_undoActive(false),
	m_ignoreChange(false),
	m_charsRemoved(0),
	m_charsAdded(0)
{
	// set the edit box to a fixed width font
	QFont font = this->font();
	font.setFixedPitch(true);
	font.setFamily("Monospace");
	font.setStyleHint(QFont::Monospace);
	setFont(font);

	// connect to catch document changes
	connect(document(), SIGNAL(contentsChanged()),
		this, SLOT(documentChanged()));

	// connect to catch specific document changes
	connect(document(), SIGNAL(contentsChange(int, int, int)),
		this, SLOT(documentChanged(int, int, int)));

	// connect to catch cursor position changes
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(cursorMoved()));

	// connect to catch when undo commands are added
	connect(document(), SIGNAL(undoCommandAdded()),
		this, SLOT(undoAdded()));

	// create line number area width and connect signal to update it
	m_lineNumberWidget = new LineNumberWidget(this);

	connect(this, SIGNAL(blockCountChanged(int)),
		this, SLOT(updateLineNumberWidgetWidth()));
	connect(this, SIGNAL(updateRequest(QRect, int)),
		this, SLOT(updateLineNumberWidget(QRect, int)));

	updateLineNumberWidgetWidth();
}


// function to handle key press events

void EditBox::keyPressEvent(QKeyEvent *event)
{
	QTextCursor cursor = textCursor();
	m_beforeSelection.setFromCursor(cursor);

	switch (event->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
		// intercept Control+Return and change it to a Return event
		if (event->modifiers() & Qt::ControlModifier
			|| cursor.atBlockEnd() && !cursor.atBlockStart())
		{
			insertText("\n");
			return;
		}
		else  // intercept Return when cursor is not at the end of a line
		{
			moveCursor(QTextCursor::NextBlock);
			return;
		}
		break;

	case Qt::Key_Backspace:
		backspace(cursor);
		break;

	case Qt::Key_Insert:
		if (event->modifiers() & Qt::ShiftModifier
			&& event->modifiers() & Qt::ControlModifier
			&& pasteSelection())
		{
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
				int line = cursor.blockNumber() + 1;
				if (document()->findBlockByLineNumber(line).text().isEmpty())
				{
					// next line blank, prevent delete setting modified line
					m_ignoreChange = true;
				}
				emit linesDeleted(line, 1);
			}
			break;
		}
		if (event->matches(QKeySequence::Paste))
		{
			paste();  // intercept paste
			return;
		}
	}
	QPlainTextEdit::keyPressEvent(event);
	captureDeletedLines();
	m_ignoreChange = false;
}


// function to handle mouse release events

void EditBox::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MiddleButton && pasteSelection(event->pos()))
	{
		return;
	}
	QPlainTextEdit::mouseReleaseEvent(event);
}


// function to cut the currently selected text to the clipboard

void EditBox::cut(void)
{
	m_beforeSelection.setFromCursor(textCursor());
	QPlainTextEdit::cut();
	captureDeletedLines();
}


// function to delete the currently selected text (not to the clipboard)

void EditBox::remove(void)
{
	m_beforeSelection.setFromCursor(textCursor());
	textCursor().removeSelectedText();
	captureDeletedLines();
}


// function to process backspace (check if line will be combined with previous)
//
//   - if not at beginning of line or at beginning of file or there is a
//     selection, then there is nothing to do here
//   - if current line is a new (inserted) line not reported yet,
//     then the modified line and inserted status is reset
//   - else if previous line is blank, then previous line is marked for deletion
//     otherwise the current line is marked for deleted (modified line reset)
//   - emits the line being deleted
//   - if current line is blank, then previous line will not be modified
//     when combined so ignore the change

void EditBox::backspace(QTextCursor &cursor)
{
	if (!cursor.atBlockStart() || cursor.atStart() || cursor.hasSelection())
	{
		return;
	}

	if (m_lineModType == LineInserted)
	{
		// this line has not actually been added yet (reset status)
		m_lineModified = -1;
		m_lineModType = LineChanged;
	}
	else
	{
		int line = cursor.blockNumber();
		if (document()->findBlockByLineNumber(line - 1).text()
		    .isEmpty())
		{
			// previous line blank, mark it for deletion instead
			line--;
			if (m_lineModified >= 0)  // current line modified?
			{
				// adjust modified line to its new line
				m_lineModified--;
			}
			m_ignoreChange = true;
		}
		else
		{
			// line was deleted, do not report as changed
			m_lineModified = -1;
		}
		// indicate line is about to be deleted
		emit linesDeleted(line, 1);
	}

	if (cursor.atBlockEnd())  // is line blank?
	{
		// prevent backspace setting modified line
		// (previous line not actually be modified)
		m_ignoreChange = true;
	}
}


// function to paste the clipboard into the program

void EditBox::paste(void)
{
	insertText(QApplication::clipboard()->text());
}


// function to paste the current selection if supported into the program

bool EditBox::pasteSelection(const QPoint &pos)
{
	QClipboard *clipboard = QApplication::clipboard();
	if (clipboard->supportsSelection())
	{
		if (pos != QPoint())
		{
			setTextCursor(cursorForPosition(pos));
		}
		insertText(clipboard->text(QClipboard::Selection));
		return true;
	}
	return false;
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
	if (!m_ignoreChange)
	{
		if (m_lineModified == -1)
		{
			m_lineModified = textCursor().blockNumber();
			m_lineModCount = 0;
			qDebug("MODIFIED Line #%d", m_lineModified);
		}
		m_undoActive = false;
	}
}


void EditBox::documentChanged(int position, int charsRemoved, int charsAdded)
{
	m_charsRemoved = charsRemoved;
	m_charsAdded = charsAdded;
}


// function to check if cursor was moved from the modified line

void EditBox::cursorMoved(void)
{
	if (!m_ignoreChange && m_lineModified >= 0
		&& m_lineModified != textCursor().blockNumber())
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
	QPlainTextEdit::undo();
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
	QPlainTextEdit::redo();
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

void EditBox::insertText(const QString &text)
{
	// gather info before text is inserted
	int initialLine = textCursor().blockNumber();
	bool initialAtLineStart = textCursor().atBlockStart();
	bool initialAtLineEnd = textCursor().atBlockEnd();

	// set flag if initial line is modified or will be modified by text
	// (at begin of line and text does not end with a new line)
	bool initialLineModified = m_lineModified >= 0
		|| initialAtLineStart && !text.endsWith('\n');

	// check if current line will be modified by inserted text
	if (!initialAtLineStart && (!initialAtLineEnd || !text.startsWith('\n'))
		|| initialAtLineEnd && !text.startsWith('\n') && !text.endsWith('\n'))
	{
		// if in middle of line
		// or not at begin of line and text does not start with a new line
		// or at end of line and text does not start or end with a new line
		// then current line will be modified by the inserted text
		m_lineModified = initialLine;
	}

	// insert the text and ignore the document change signal
	m_ignoreChange = true;
	textCursor().insertText(text);
	m_ignoreChange = false;

	// check if inserted text does not contain new lines
	int newLines = text.count('\n');  // count number of new lines in text
	if (newLines == 0)
	{
		return;  // nothing further to do (no new lines will be inserted)
	}

	// capture the original cursor line if not initially at beginning of line
	if (!initialAtLineStart)
	{
		captureModifiedLine();  // (resets m_lineModified to -1)
	}

	// capture any new lines inserted
	QStringList lines;
	int firstInsertedLineOffset = initialAtLineStart && (!initialAtLineEnd
		|| !text.startsWith('\n') || initialLineModified) ? 0 : 1;
	for (int i = firstInsertedLineOffset; i < newLines; i++)
	{
		QString line = document()->findBlockByNumber(initialLine + i).text();
		lines.append(line);
	}
	if (!lines.isEmpty())
	{
		emit linesInserted(initialLine + firstInsertedLineOffset, lines);
	}

	// determine status of last line
	if (firstInsertedLineOffset == 0)  // inserting at begin of line?
	{
		// if this line was modified or the inserted text does not end
		// with a new line, then mark the new cursor line as modified
		if (initialLineModified || !text.endsWith('\n'))
		{
			m_lineModified = textCursor().blockNumber();
		}
		else  // current unmodifed line was just moved, clear modified line
		{
			m_lineModified = -1;
		}
	}
	else  // new cursor line has not been inserted yet
	{
		// mark this new line as modified and to be inserted
		m_lineModified = textCursor().blockNumber();
		m_lineModCount = 0;
		m_lineModType = LineInserted;
	}
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


// function to check for modified lines when there is a selection

void EditBox::captureDeletedLines(void)
{
	if (!m_beforeSelection.isEmpty() && m_charsRemoved > 0)
	{
		if (m_beforeSelection.lines() > 1)
		{
			emit linesDeleted(m_beforeSelection.startLine(),
				m_beforeSelection.lines() - 1);
		}
	}
	// reset charaters removed and added variables
	m_charsRemoved = 0;
	m_charsAdded = 0;
}


// function to calculate the width needed for the line number area widget

int EditBox::lineNumberWidgetWidth(void)
{
	int digits = 3;  // add one space to each side of the numbers
	int max = blockCount();
	while (max > 10 - BaseLineNumber)
	{
		max /= 10;
		digits++;
	}
	return fontMetrics().width(QLatin1Char('9')) * digits;
}


// function to update the line number area width based on the maximum line

void EditBox::updateLineNumberWidgetWidth(void)
{
	setViewportMargins(lineNumberWidgetWidth(), 0, 0, 0);
}


// function to update the line number area when display update requested

void EditBox::updateLineNumberWidget(const QRect &rect, int dy)
{
	if (dy)
	{
		m_lineNumberWidget->scroll(0, dy);
	}
	else
	{
		m_lineNumberWidget->update(0, rect.y(), m_lineNumberWidget->width(),
			rect.height());
	}

	if (rect.contains(viewport()->rect()))
	{
		updateLineNumberWidgetWidth();
	}
}


// function to set line number area rectangle upon a resize event

void EditBox::resizeEvent(QResizeEvent *event)
{
	QPlainTextEdit::resizeEvent(event);

	QRect rect = contentsRect();
	m_lineNumberWidget->setGeometry(QRect(rect.left(), rect.top(),
		lineNumberWidgetWidth(), rect.height()));
}


// function to draw the line number area (from line number area paint event)

void EditBox::paintLineNumberWidget(QPaintEvent *event)
{
	QPainter painter(m_lineNumberWidget);
	painter.fillRect(event->rect(), Qt::lightGray);

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int)blockBoundingGeometry(block).translated(contentOffset())
		.top();
	int bottom = top + (int)blockBoundingRect(block).height();

	while (block.isValid() && top <= event->rect().bottom())
	{
		if (block.isVisible() && bottom >= event->rect().top())
		{
			QString number = QString("%1 ").arg(blockNumber + BaseLineNumber);
			painter.setPen(Qt::black);
			painter.drawText(0, top, m_lineNumberWidget->width(),
				fontMetrics().height(), Qt::AlignRight, number);
		}
		block = block.next();
		top = bottom;
		bottom = top + (int)blockBoundingRect(block).height();
		blockNumber++;
	}
}


// end: editbox.cpp
