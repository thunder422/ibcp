// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: programmodel.h - program model class header file
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
//	2013-03-15	initial version

#ifndef PROGRAMMODEL_H
#define PROGRAMMODEL_H

#include <QAbstractListModel>
#include <QStringList>

#include "errorlist.h"

class RpnList;
class Translator;

class ProgramModel : public QAbstractListModel
{
	Q_OBJECT
public:
	explicit ProgramModel(QObject *parent = 0);
	~ProgramModel(void);
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;

signals:
	void lineCountChanged(int newLineCount);

public slots:
	void update(int lineNumber, int linesDeleted, int linesInserted,
		QStringList lines);

private:
	enum ModifyMode
	{
		Insert,
		Change,
		Delete
	};
	struct LineInfo
	{
		RpnList *rpnList;				// pointer to rpn list
		int errIndex;					// index to error list
	};
	bool updateLine(ModifyMode mode, int lineNumber,
		const QString &line = QString());
	void setError(int lineNumber,
		LineInfo &lineInfo, bool lineInserted);
	void removeError(int lineNumber, LineInfo &lineInfo, bool lineDeleted);

	Translator *m_translator;			// program line translator instance
	QList<LineInfo> m_lineInfo;			// program line information list
	ErrorList m_errors;				// list of program errors
};

#endif // PROGRAMMODEL_H
