// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: editbox.h - edit box header file
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

#ifndef EDITBOX_H
#define EDITBOX_H

#include <QPlainTextEdit>
#include <QTextBlock>

class QEvent;


class Selection
{
	int m_start;					// start position of selection
	int m_end;						// end position of selection
	int m_startLine;				// start line of selection
	int m_endLine;					// end line of selection
	int m_startLineBegin;			// position of begin of start line
	int m_startLineEnd;				// position of end of start line
	int m_endLineBegin;				// position of begin of end line
	int m_endLineEnd;				// position of end of end line

public:
	Selection(void) {}
	// set selction info from a text cursor
	void setFromCursor(const QTextCursor &cursor)
	{
		m_start = cursor.selectionStart();
		m_end = cursor.selectionEnd();

		QTextBlock block = cursor.document()->findBlock(m_start);
		m_startLine = block.blockNumber();
		m_startLineBegin = block.position();
		m_startLineEnd = m_startLineBegin + block.length() - 1;
		block = cursor.document()->findBlock(m_end);
		m_endLine = block.blockNumber();
		m_endLineBegin = block.position();
		m_endLineEnd = m_endLineBegin + block.length() - 1;
	}
	// return if there is no selection
	bool isEmpty(void)
	{
		return m_start == m_end;
	}
	// return number of new lines in selection
	int newLines(void)
	{
		return m_endLine - m_startLine;
	}
	// return start selection line number
	int startLine(void)
	{
		return m_startLine;
	}
	// return end selection line number
	int endLine(void)
	{
		return m_endLine;
	}
	// return if start selection position is at the begin of the line
	bool startAtLineBegin(void)
	{
		return m_start == m_startLineBegin;
	}
	// return if start selection position is at the end of the line
	bool startAtLineEnd(void)
	{
		return m_start == m_startLineEnd;
	}
	// return if end selection position is at being of the line
	bool endAtLineBegin(void)
	{
		return m_end == m_endLineBegin;
	}
	// return if end selection position is at end of the line
	bool endAtLineEnd(void)
	{
		return m_end == m_endLineEnd;
	}
};


class EditBox : public QPlainTextEdit
{
	Q_OBJECT
public:
	explicit EditBox(QWidget *parent = 0);
	void cut(void);
	void remove(void);
	void selectAll(void);
	void resetModified(void);
	void setPlainText(const QString &text);

	int lineNumberWidgetWidth(void);
	void paintLineNumberWidget(QPaintEvent *event);

protected:
	void keyPressEvent(QKeyEvent *event);
	void resizeEvent(QResizeEvent *event);

signals:
	void linesChanged(int lineNumber, int linesDeleted, int linesInserted,
		QStringList lines);

public slots:
	void documentChanged(int position, int charsRemoved, int charsAdded);
	void cursorMoved(void);
	void undo(void);
	void redo(void);
	void undoAdded(void);

private slots:
	void updateLineNumberWidgetWidth(void);
	void updateLineNumberWidget(const QRect &rect, int dy);

private:
	bool pasteSelection(const QPoint &pos = QPoint());
	void captureModifiedLine(void);

	enum LineType {
		LineChanged,
		LineInserted,
		sizeof_LineType
	};
	enum {
		BaseLineNumber = 0			// number of first line
	};

	int m_lineModified;				// current line that has been modified
	int m_lineModCount;				// number of modifications to line
	LineType m_lineModType;			// lone modification type
	bool m_undoActive;				// line modified due to undo
	bool m_ignoreChange;			// ignore next document change flag
	QWidget *m_lineNumberWidget;	// widget to display line numbers
	Selection m_beforeSelection;	// selection info before edit operation
	int m_lineCount;				// total document line count
};


class LineNumberWidget : public QWidget
{
public:
	LineNumberWidget(EditBox *editBox) : QWidget(editBox)
	{
		m_editBox = editBox;
	}
	QSize sizeHint() const
	{
		return QSize(m_editBox->lineNumberWidgetWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent *event)
	{
		m_editBox->paintLineNumberWidget(event);
	}

private:
	EditBox *m_editBox;
};


#endif // EDITBOX_H
