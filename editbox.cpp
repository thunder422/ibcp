// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: editbox.h - edit box class source file
//	Copyright (C) 2012-2013  Thunder422
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
#include <QMessageBox>
#include <QPainter>
#include <QTextBlock>

#include "editbox.h"
#include "errorlist.h"


EditBox::EditBox(QWidget *parent) :
	QPlainTextEdit(parent),
	m_modifiedLine(-1),
	m_modifiedLineIsNew(false),
	m_lineCount(0),
	m_cursorValid(false)
{
	// set the edit box to a fixed width font
	QFont font = this->font();
	font.setFixedPitch(true);
	font.setFamily("Monospace");
	font.setStyleHint(QFont::Monospace);
	setFont(font);

	// connect to catch specific document changes
	connect(document(), SIGNAL(contentsChange(int, int, int)),
		this, SLOT(documentChanged(int, int, int)));

	// connect to catch cursor position changes
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(cursorMoved()));

	// create line number area width and connect signal to update it
	m_lineNumberWidget = new LineNumberWidget(this);

	connect(this, SIGNAL(blockCountChanged(int)),
		this, SLOT(lineNumberWidgetUpdateWidth()));
	connect(this, SIGNAL(updateRequest(QRect, int)),
		this, SLOT(lineNumberWidgetUpdate(QRect, int)));

	lineNumberWidgetUpdateWidth();
}


// function to handle key press events

void EditBox::keyPressEvent(QKeyEvent *event)
{
	QTextCursor cursor = textCursor();

	switch (event->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
		if (event->modifiers() & Qt::ShiftModifier)
		{
			moveCursor(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
			return;
		}
		// intercept Control+Return and change it to a Return event
		else if (event->modifiers() & Qt::ControlModifier
			|| cursor.atBlockEnd())
		{
			event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Return,
				Qt::NoModifier);
			QPlainTextEdit::keyPressEvent(event);
			delete event;
			return;
		}
		else  // intercept Return when cursor is not at the end of a line
		{
			if (!cursor.hasSelection())
			{
				moveCursor(QTextCursor::NextBlock);
				return;
			}
		}
		break;

	case Qt::Key_Insert:
		if (event->modifiers() == Qt::NoModifier)
		{
			setOverwriteMode(!overwriteMode());
			return;
		}
		if (event->modifiers() & Qt::ShiftModifier)
		{
			if (event->modifiers() & Qt::ControlModifier)
			{
				if (pasteSelection())
				{
					return;
				}
			}
			else
			{
				paste();  // intercept paste
				return;
			}
		}
		break;

	case Qt::Key_Period:
		if (event->modifiers() & Qt::ControlModifier)
		{
			return;  // intercept next error key shortcut
		}
		break;

	case Qt::Key_Comma:
		if (event->modifiers() & Qt::ControlModifier)
		{
			return;  // intercept next error key shortcut
		}
		break;

	default:  // check for key sequences
		if (event->matches(QKeySequence::SelectAll))
		{
			selectAll();
			return;
		}
	}
	QPlainTextEdit::keyPressEvent(event);
}


// function to delete the currently selected text (not to the clipboard)

void EditBox::remove(void)
{
	textCursor().removeSelectedText();
}


// function to paste text from the clipboard (and correctly update the screen)

void EditBox::paste(QClipboard::Mode mode)
{
	QClipboard *clipboard = QApplication::clipboard();
	QTextCursor cursor = textCursor();

	// clear any selection on screen first with a temporary cursor
	if (cursor.hasSelection())
	{
		QTextCursor tmpCursor = cursor;
		tmpCursor.clearSelection();
		setTextCursor(tmpCursor);
	}

	cursor.insertText(clipboard->text(mode));
}


// function to paste the current selection if supported into the program
//
//   - return true on success, false if selection is not supported

bool EditBox::pasteSelection(const QPoint &pos)
{
	QClipboard *clipboard = QApplication::clipboard();
	if (clipboard->supportsSelection())
	{
		if (pos != QPoint())
		{
			setTextCursor(cursorForPosition(pos));
		}
		paste(QClipboard::Selection);
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


// function to move cursor to the next error

void EditBox::goNextError(void)
{
	int errIndex = m_errors.find(lineNumber());
	if (errIndex >= m_errors.count()
		|| lineNumber() > m_errors.at(errIndex).lineNumber()
		|| lineNumber() == m_errors.at(errIndex).lineNumber()
		&& column() >= m_errors.at(errIndex).column())
	{
		// past current error, go to next error
		errIndex++;
	}
	if (errIndex >= m_errors.count())  // past last error?
	{
		errIndex = 0;
		if (m_errors.count() == 1 && lineNumber() == m_errors.at(0).lineNumber()
			&& column() >= m_errors.at(0).column()
			&& column() < m_errors.at(0).column() + m_errors.at(0).length()
			|| QMessageBox::question(this, "Next Error",
			"No more errors in program,\nmove to first error?",
			QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
			!= QMessageBox::Yes)
		{
			return;
		}
	}
	moveCursorToError(errIndex);
}


// function to move cursor to the previous error

void EditBox::goPrevError(void)
{
	int errIndex = m_errors.find(lineNumber());
	if (errIndex >= m_errors.count()
		|| lineNumber() < m_errors.at(errIndex).lineNumber()
		|| lineNumber() == m_errors.at(errIndex).lineNumber()
		&& column() < m_errors.at(errIndex).column()
		+ m_errors.at(errIndex).length())
	{
		// before current error, go to previous error
		errIndex--;
	}
	if (errIndex < 0)
	{
		errIndex = m_errors.count() - 1;
		if (m_errors.count() == 1
			&& lineNumber() == m_errors.at(errIndex).lineNumber()
			&& column() == m_errors.at(errIndex).column()
			|| QMessageBox::question(this, "Previous Error",
			"No more errors in program,\nmove to last error?",
			QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
			!= QMessageBox::Yes)
		{
			return;
		}
	}
	moveCursorToError(errIndex);
}


// function to move cursor to an error

void EditBox::moveCursorToError(int errIndex)
{
	ErrorItem error = m_errors.at(errIndex);
	QTextBlock block = document()->findBlockByLineNumber(error.lineNumber());
	QTextCursor cursor = textCursor();
	cursor.setPosition(block.position() + error.column());
	setTextCursor(cursor);
}


// function to reset the document modified flag and other variables

void EditBox::resetModified(void)
{
	document()->setModified(false);
	m_modifiedLine = -1;
}


// function to detect lines changed when the document was changed
//
//   - determines the number of lines that were modified
//   - determines the net line count changed of the document
//   - determines if lines were inserted, changed or deleted

void EditBox::documentChanged(int position, int charsRemoved, int charsAdded)
{
	int linesInserted = 0;
	int linesDeleted = 0;
	QStringList lines;

	// get information about position at start of change
	QTextCursor cursor = textCursor();
	cursor.setPosition(position);
	int changeLine = cursor.blockNumber();
	bool changeAtLineBegin = cursor.atBlockStart();

	// move cursor to end of change and get information about end position
	bool cursorMoved = cursor.movePosition(QTextCursor::Right,
		QTextCursor::MoveAnchor, charsAdded);
	int linesModified = cursor.blockNumber() - changeLine;
	int newLineCount = document()->blockCount();
	int netLineCount = newLineCount - m_lineCount;

	if (document()->isEmpty())
	{
		if (!cursorMoved)
		{
			if (charsAdded == 1)
			{
				m_modifiedLine = -2;
				return;
			}
			if (m_lineCount == 0)
			{
				return;
			}
		}
		if (!cursorMoved || m_modifiedLine == -2)
		{
			netLineCount = -m_lineCount;
			newLineCount = 0;
			changeAtLineBegin = true;
		}
	}

	// capture modified line if it is not related to change (undo/redo)
	if (m_modifiedLine >= 0 && m_modifiedLine != changeLine)
	{
		// adjust if modified line is after change line
		captureModifiedLine(m_modifiedLine > changeLine ? netLineCount: 0);
	}

	if (linesModified != 0 || netLineCount != 0)  // multiple lines affected?
	{
		if (linesModified == netLineCount && changeAtLineBegin)
		{
			// single line changed and at begin of line
			linesInserted = netLineCount;
			linesModified = 0;
		}
		else  // changed multiple lines or single line and not at begin of line
		{
			bool changeLineIsNew = m_modifiedLineIsNew;

			// check if last line is new before linesModified is changed
			if (linesModified == netLineCount && !changeAtLineBegin
				|| cursor.atBlockEnd() && netLineCount > 0)
			{
				m_modifiedLineIsNew = true;
			}

			if (netLineCount <= 0)  // lines deleted from document?
			{
				// check if current line is a newly inserted line
				if (m_modifiedLineIsNew)
				{
					// (backspace at begin of new modified line
					// or delete at end of new modified line
					// or multiple line undo at a new modified line)
					// reset new line status, and one less line deleted
					m_modifiedLineIsNew = false;
					netLineCount++;

					if (linesModified == 0)  // no lines changed?
					{
						// check if multiple line undo
						if (changeLine + 1 < m_modifiedLine)
						{
							// need to report one line (next line) as changed
							linesModified = 1;
							changeLine++;
						}
						else if (netLineCount == 0)  // no lines deleted?
						{
							changeLine = -1;  // prevent empty signal
						}
						else  // lines were deleted (multiple line undo)
						{
							changeLine++;  // first line deleted is next line
						}
					}
				}
				// check if result is a single line and was not at begin of line
				else if (linesModified == 0 && !changeAtLineBegin)
				{
					changeLine++;  // first line deleted is after current line
				}
				linesDeleted = -netLineCount;
			}
			else  // lines were added to document
			{
				// adjust number of lines changed
				if (cursorMoved || linesModified >= netLineCount)
				{
					linesModified -= netLineCount;
				}
				linesInserted = netLineCount;
				if (!changeLineIsNew && m_modifiedLineIsNew)
				{
					// adjust lines changed/inserted if last line is new
					linesModified++;
					linesInserted--;
				}
			}
		}
		if (changeLine != -1)
		{
			if (linesModified + linesInserted > 0)
			{
				// get list of lines changed and inserted
				for (int i = 0; i < linesModified + linesInserted; i++)
				{
					lines << document()->findBlockByNumber(changeLine
						+ i).text();
				}
			}
			emit linesChanged(changeLine, linesDeleted, linesInserted, lines);
		}
		m_lineCount = newLineCount;
	}
	else  // single line modified, reset error if line has an error
	{
		int errIndex = m_errors.findIndex(changeLine);
		if (errIndex != -1)
		{
			int errCol = m_errors.at(errIndex).column();
			int errLen = m_errors.at(errIndex).length();
			int col = cursor.positionInBlock();
			if (col - charsAdded <= errCol + errLen)
			{
				// cursor is within or before error
				if (cursor.atBlockEnd() && col + charsRemoved >= errCol
					|| col - charsAdded + charsRemoved > errCol)
				{
					// cursor and error at end of line or cursor is within error
					// remove error
					m_errors.resetChange();
					m_errors.removeAt(errIndex);
				}
				else  // cursor is before error, update error
				{
					m_errors.resetChange();
					m_errors.moveColumn(errIndex, charsAdded - charsRemoved);
				}
				updateErrors(m_errors);
				// cause error message to be removed from status line
				emit cursorChanged();
			}
		}
	}
	m_modifiedLine = m_modifiedLine == -2 ? -1 : cursor.blockNumber();
}


// function to check if cursor was moved from the modified line

void EditBox::cursorMoved(void)
{
	if (!m_cursorValid)
	{
		m_cursorValid = true;
		updateErrors(m_errors);
	}
	if (m_modifiedLine >= 0 && m_modifiedLine != textCursor().blockNumber())
	{
		// there is a modified line and cursor moved from that line
		captureModifiedLine();
	}
	emit cursorChanged();
}


// function to get error message for current line
//
//   - default to no message if current line does not have error

const QString EditBox::message(void) const
{
	// look for an error message for current line
	int errIndex = m_errors.findIndex(lineNumber());
	QString message;
	if (errIndex != -1)
	{
		message = m_errors.at(errIndex).message();
	}
	return message;
}


// overloaded function to set plain text of the document

void EditBox::setPlainText(const QString &text)
{
	// invalidate cursor until text is set
	m_cursorValid = false;
	QPlainTextEdit::setPlainText(text);
}


// function to check if current line was modified and to process it

void EditBox::captureModifiedLine(int offset)
{
	if (m_modifiedLine >= 0)
	{
		emit linesChanged(m_modifiedLine, 0, m_modifiedLineIsNew ? 1 : 0,
			QStringList() << document()->findBlockByNumber(m_modifiedLine
			+ offset).text());

		if (m_modifiedLineIsNew)  // was this a new line without a number?
		{
			// redraw line numbers to fill in previous blank line number
			// and adjust the line numbers below the new line
			m_lineNumberWidget->update(0, 0, m_lineNumberWidget->width(),
				height());
		}

		m_modifiedLine = -1;  // line processed, reset modified line number
		m_modifiedLineIsNew = false;
	}
}


// function to update errors when program error list changes
void EditBox::updateErrors(const ErrorList &errors)
{
	bool wasEmpty;
	int inserted;
	int removed;
	int changed;
	int start;
	int end;
	int i;

	m_errors = errors;
	if (!m_cursorValid)
	{
		return;  // wait until cursor is valid
	}

	wasEmpty = m_extraSelections.isEmpty();
	inserted = errors.size() - m_extraSelections.size();
	if (inserted > 0)
	{
		removed = 0;
	}
	else
	{
		removed = -inserted;
		inserted = 0;
	}

	if (errors.size() == 0)
	{
		start = 0;
		end = 0;
		changed = 0;
	}
	else
	{
		start = errors.changeIndexStart();
		end = errors.changeIndexEnd();
		changed = end - start + 1 - inserted;
		if (start + changed > errors.size())
		{
			changed = errors.size() - start;
		}
	}

	for (i = start; --changed >= 0; i++)
	{
		m_extraSelections.replace(i, extraSelection(errors.at(i)));
	}
	while (--inserted >= 0)
	{
		m_extraSelections.insert(i, extraSelection(errors.at(i)));
		i++;
	}
	while (--removed >= 0)
	{
		m_extraSelections.removeAt(i);
	}
	setExtraSelections(m_extraSelections);

	if (wasEmpty != m_extraSelections.isEmpty())
	{
		emit errorsAvailable(!m_extraSelections.isEmpty());
	}
}


// function to convert an error item to an extra selection
const QTextEdit::ExtraSelection
	EditBox::extraSelection(const ErrorItem &errorItem)
{
	QTextEdit::ExtraSelection selection;
	QColor lineColor;
	QTextBlock block;

	int column = errorItem.column();
	int length = errorItem.length();
	lineColor = QColor(Qt::red).lighter(150);
	selection.format.setBackground(lineColor);

	block = document()->findBlockByLineNumber(errorItem.lineNumber());
	selection.cursor = textCursor();
	selection.cursor.setPosition(block.position() + column);
	selection.cursor.movePosition(QTextCursor::NextCharacter,
		QTextCursor::KeepAnchor, length);

	return selection;
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

void EditBox::lineNumberWidgetUpdateWidth(void)
{
	setViewportMargins(lineNumberWidgetWidth(), 0, 0, 0);
}


// function to update the line number area when display update requested

void EditBox::lineNumberWidgetUpdate(const QRect &rect, int dy)
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
		lineNumberWidgetUpdateWidth();
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

void EditBox::lineNumberWidgetPaint(QPaintEvent *event)
{
	QPainter painter(m_lineNumberWidget);
	painter.fillRect(event->rect(), Qt::lightGray);

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int)blockBoundingGeometry(block).translated(contentOffset())
		.top();
	int bottom = top + (int)blockBoundingRect(block).height();

	int offset = BaseLineNumber;
	while (block.isValid() && top <= event->rect().bottom())
	{
		if (block.isVisible() && bottom >= event->rect().top())
		{
			QString number;
			if (blockNumber == m_modifiedLine && m_modifiedLineIsNew)
			{
				number = QString("+");	// draw '+' with no line number
				offset--;				// offset rest of line numbers by 1
			}
			else  // draw line number with '*' if modified
			{
				number = QString("%1%2").arg(blockNumber + offset)
					.arg(blockNumber == m_modifiedLine ? '*' : ' ');
			}
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
