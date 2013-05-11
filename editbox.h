// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: editbox.h - edit box class header file
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

#ifndef EDITBOX_H
#define EDITBOX_H

#include <QClipboard>
#include <QPlainTextEdit>
#include <QTextBlock>

#include "errorlist.h"

class QEvent;


class EditBox : public QPlainTextEdit
{
	Q_OBJECT
public:
	explicit EditBox(QWidget *parent = 0);
	void remove(void);
	void paste(QClipboard::Mode mode = QClipboard::Clipboard);
	void selectAll(void);
	void resetModified(void);
	void captureModifiedLine(int offset = 0);

	int lineNumberWidgetWidth(void);
	void lineNumberWidgetPaint(QPaintEvent *event);

	enum {
		BaseLineNumber = 0			// number of first line
	};

protected:
	void keyPressEvent(QKeyEvent *event);
	void resizeEvent(QResizeEvent *event);

signals:
	void linesChanged(int lineNumber, int linesDeleted, int linesInserted,
		QStringList lines);

public slots:
	void documentChanged(int position, int charsRemoved, int charsAdded);
	void cursorMoved(void);
	void updateErrors(const ErrorList &errors);
	void setPlainText(const QString &text);

private slots:
	void lineNumberWidgetUpdateWidth(void);
	void lineNumberWidgetUpdate(const QRect &rect, int dy);

private:
	bool pasteSelection(const QPoint &pos = QPoint());
	const QTextEdit::ExtraSelection extraSelection(const ErrorItem &errorItem);

	int m_modifiedLine;				// current line that has been modified
	bool m_modifiedLineIsNew;		// modified line is a new line flag
	int m_modifiedLineOffset;		// compensate for lines inserted/deleted
	QWidget *m_lineNumberWidget;	// widget to display line numbers
	int m_lineCount;				// total document line count
	QList<QTextEdit::ExtraSelection>
		m_extraSelections;			// error highlight extra selections
	bool m_cursorValid;				// flag for when text cursor is valid
	ErrorList m_errors;				// list of errors from the program model
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
		m_editBox->lineNumberWidgetPaint(event);
	}

private:
	EditBox *m_editBox;
};


#endif // EDITBOX_H
