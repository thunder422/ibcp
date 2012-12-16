// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: mainwinow.h - main window header file
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
//	2012-11-28	initial version

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class CommandLine;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	bool isGuiActive(void) const
	{
		return m_guiActive;
	}
	int returnCode(void) const	// valid only if GUI not active
	{
		return m_returnCode;
	}

protected:
	void closeEvent(QCloseEvent *event);

private slots:
	void about(void);

private:
	void createActions(void);
	void createMenus(void);

	void settingsRestore(void);
	void settingsSave(void);

	Ui::MainWindow *ui;
	bool m_guiActive;
	int m_returnCode;
	CommandLine *m_commandLine;

	QAction *m_actionExit;
	QAction *m_actionAbout;
	QAction *m_actionAboutQt;

	QMenu *m_menuFile;
	QMenu *m_menuHelp;
};

#endif // MAINWINDOW_H
