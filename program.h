// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: program.h - program class header file
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
//	2013-12-17	initial version

#ifndef PROGRAM_H
#define PROGRAM_H

#include <QObject>

class QSettings;


class Program : public QObject
{
	Q_OBJECT

public:
	explicit Program(QObject *parent = 0);

	void settingsRestore(QSettings &settings);
	void settingsSave(QSettings &settings);

	// access functions
	const QString fileName(void)
	{
		return m_fileName;
	}
	void setFileName(QString fileName)
	{
		m_fileName = fileName;
	}

signals:

public slots:

private:
	QString m_fileName;						// file name of current program

};


#endif // PROGRAM_H
