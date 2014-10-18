// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: mainwinow.h - main window class header file
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
//	2012-11-28	initial version

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>

#include <QLabel>
#include <QMainWindow>

#include "ibcp.h"
#include "commandline.h"
#include "editbox.h"
#include "program.h"
#include "programlinedelegate.h"
#include "ui_mainwindow.h"

namespace Ui {
class MainWindow;
}

class ProgramModel;
class CommandLine;
class RecentFiles;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);

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
	void statusBarUpdate(Status status);
	void on_actionNew_triggered(void);
	void on_actionOpen_triggered(void);
	void on_actionClearRecent_triggered(void);
	bool on_actionSave_triggered(void);
	bool on_actionSaveAs_triggered(void);
	void on_actionAbout_triggered(void);
	void on_actionAboutQt_triggered(void);
	void programOpen(const QString programPath);
	void on_actionUndo_triggered(void);
	void on_actionRedo_triggered(void);
	void on_actionCut_triggered(void);
	void on_actionCopy_triggered(void);
	void on_actionPaste_triggered(void);
	void on_actionDelete_triggered(void);
	void on_actionSelectAll_triggered(void);
	void on_actionGoNextError_triggered(void);
	void on_actionGoPrevError_triggered(void);

private:
	void statusBarCreate(void);
	void editBoxSetActive(EditBox *editBox);

	bool isOkToContinue(void);
	void setCurProgram(const std::string &programPath);

	bool programLoad(const std::string &programPath);
	bool programSave(const std::string &programPath);
	void programCaptureEditChanges(void);

	void settingsRestore(void);
	void settingsSave(void);

	// member variables
	std::unique_ptr<Ui::MainWindow> ui;
	bool m_guiActive;
	int m_returnCode;
	std::unique_ptr<CommandLine> m_commandLine;

	Program m_program;
	EditBox *m_editBox;
	RecentFiles *m_recentPrograms;
	QString m_curDirectory;
	ProgramLineDelegate *m_programLineDelegate;
	QLabel *m_statusPositionLabel;
	QLabel *m_statusMessageLabel;
	QList<QAction *> m_contextActions;
};


#endif // MAINWINDOW_H
