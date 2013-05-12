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

class RpnList;

enum Operation
{
	Insert_Operation,
	Change_Operation,
	Remove_Operation
};

// class for holding details for an error of a program line
class ErrorItem
{
public:
	enum Type
	{
		Translator,
		Encoder
	};
	ErrorItem(Type type, int lineNumber, RpnList *rpnList);
	enum Type type(void) const
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
	int column(void) const
	{
		return m_column;
	}
	int length(void) const
	{
		return m_length;
	}
	QString message(void) const
	{
		return m_message;
	}

private:
	enum Type m_type;				// type of error
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

	void resetChange(void)
	{
		m_changeIndexStart = m_changeIndexEnd = -1;
	}
	bool hasChanged(void) const
	{
		return m_changeIndexStart != -1;
	}
	int changeIndexStart(void) const
	{
		return m_changeIndexStart;
	}
	int changeIndexEnd(void) const
	{
		return m_changeIndexEnd;
	}

private:
	void setChangeIndex(int index, Operation operation);

	int m_changeIndexStart;				// start index of change
	int m_changeIndexEnd;				// end index of change
	QList<Operation> m_changeList;		// list of changes
};

#endif // ERRORLIST_H
