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

class QEvent;

class EditBox : public QPlainTextEdit
{
	Q_OBJECT
public:
	explicit EditBox(QWidget *parent = 0);
	void remove(void);
	void selectAll(void);
	void resetModified(void);
	int lineNumberWidgetWidth(void);
	void paintLineNumberWidget(QPaintEvent *event);

protected:
	void keyPressEvent(QKeyEvent *event);
	void resizeEvent(QResizeEvent *event);

signals:
	void lineChanged(int number, QString line);
	void linesInserted(int number, QStringList lines);
	void linesDeleted(int number, int count);

public slots:
	void documentChanged(void);
	void cursorMoved(void);
	void undo(void);
	void redo(void);
	void undoAdded(void);

private slots:
	void updateLineNumberWidgetWidth(void);
	void updateLineNumberWidget(const QRect &rect, int dy);

private:
	void insertNewLine(void);
	void captureModifiedLine(void);

	enum LineType {
		LineChanged,
		LineInserted,
		LineDeleted,
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
