// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: recentfiles.h - recent files class header file
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
//	2013-01-05	initial version

#ifndef RECENTFILES_H
#define RECENTFILES_H

#include <QObject>
#include <QMenu>
#include <QSettings>

class RecentFiles : public QObject
{
	Q_OBJECT

public:
	explicit RecentFiles(QMenu *menu, QObject *parent = 0);

	QString addFile(const QString &path);
	void clear(void);
	void restore(QSettings &settings);
	void save(QSettings &settings);

signals:
	void openFile(QString path);

public slots:
	void on_openFile_triggered(void);

private:
	void updateActions(void);
	QString fileName(const QString &path);

	enum
	{
		MaxCount = 10				// maximum possible menu items
	};

	QMenu *m_menu;					// menu that owns recent file menu items
	QAction *m_actions[MaxCount];	// actions for all recent files
	int m_count;					// current set number of recent files
	QStringList m_files;			// list of recent files
};

#endif // RECENTFILES_H
