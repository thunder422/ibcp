// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: mainwinow.cpp - main window functions
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

#include <QApplication>
#include <QCloseEvent>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "commandline.h"
#include "editbox.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	m_commandLine = new CommandLine(qApp->arguments());
	if (m_commandLine->processed())
	{
		// don't start GUI and retrieve the return code
		m_guiActive = false;
		m_returnCode = m_commandLine->returnCode();
		return;
	}

	// start GUI here
	ui->setupUi(this);
	settingsRestore();
	setWindowTitle("IBCP");

	// create the starting program edit box
	m_editBox = new EditBox;
	setCentralWidget(m_editBox);

	m_guiActive = true;
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    settingsSave();
    event->accept();
}


void MainWindow::on_actionNew_triggered(void)
{
}


void MainWindow::on_actionOpen_triggered(void)
{
}


void MainWindow::on_actionSave_triggered(void)
{
}


void MainWindow::on_actionSaveAs_triggered(void)
{
}


void MainWindow::on_actionAbout_triggered(void)
{
	// build up about box string
	QString aboutString(tr("<h3>Interactive BASIC Compiler Project</h3>"));

	const char **gpl = m_commandLine->gplStatement();
	// add version and copyright year to first string
	aboutString.append(tr(gpl[0]).arg(tr("Version %1")
		.arg(m_commandLine->version())).arg(m_commandLine->copyrightYear())
		.append("<br>"));
	for (int i = 1; gpl[i]; i++)
	{
		aboutString.append("<br>").append(tr(gpl[i]));
	}
	aboutString.append(tr("<p><i>Temporary GUI Placeholder</i>"));
	aboutString.append(tr("<p>Command line %1").arg(m_commandLine->usage()));

	QMessageBox::about(this, tr("About IBCP"), aboutString);
}


void MainWindow::on_actionAboutQt_triggered(void)
{
	qApp->aboutQt();
}


void MainWindow::settingsRestore(void)
{
    QSettings settings("Thunder422", "IBCP");

    restoreGeometry(settings.value("geometry").toByteArray());
}


void MainWindow::settingsSave(void)
{
    QSettings settings("Thunder422", "IBCP");

    settings.setValue("geometry", saveGeometry());
}


MainWindow::~MainWindow()
{
	delete ui;
	delete m_commandLine;
}


// end: mainwindow.cpp
