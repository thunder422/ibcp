// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: errorlist.h - error classes header file
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
//	2013-04-10	initial version

#ifndef ERRORLIST_H
#define ERRORLIST_H

#include <QList>
#include <QString>


// class for holding details for an error of a program line
class ErrorItem
{
public:
	enum class Type
	{
		None,
		Input,
		Code
	};
	ErrorItem(void);
	ErrorItem(Type type, int lineNumber, int column, int length,
		const QString &message);

	bool isEmpty(void) const
	{
		return m_type == Type::None;
	}
	Type type(void) const
	{
		return m_type;
	}
	int lineNumber(void) const
	{
		return m_lineNumber;
	}
	void incrementLineNumber(void)
	{
		m_lineNumber++;
	}
	void decrementLineNumber(void)
	{
		m_lineNumber--;
	}
	void moveColumn(int chars)
	{
		m_column += chars;
	}
	int column(void) const
	{
		return m_length >= 0 ? m_column : -m_length;
	}
	int length(void) const
	{
		return m_length >= 0 ? m_length : 1;
	}
	QString message(void) const
	{
		return m_message;
	}

private:
	Type m_type;					// type of error
	int m_lineNumber;				// line number of error
	int m_column;					// column of error
	int m_length;					// length of error
	QString m_message;				// message of error
};


// class for holding a list of all the errors of a program
class ErrorList : public QList<ErrorItem>
{
public:
	ErrorList(void);
	int find(int lineNumber) const;
	int findIndex(int lineNumber) const;
	void insert(int index, const ErrorItem &value);
	void removeAt(int index);
	void replace(int index, const ErrorItem &value);
	void incrementLineNumber(int index);
	void decrementLineNumber(int index);
	void moveColumn(int index, int chars);

	bool hasChanged(void)
	{
		bool changed = m_changed;
		m_changed = false;  // reset after read
		return changed;
	}

private:
	bool m_changed;						// error list changed
};


#endif // ERRORLIST_H
