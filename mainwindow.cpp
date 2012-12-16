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
#include <QMessageBox>
#include <QTimer>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "commandline.h"

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
	createActions();
	createMenus();
	m_guiActive = true;
}


void MainWindow::createActions(void)
{
	m_actionExit = new QAction(tr("E&xit"), this);
	m_actionExit->setShortcut(QKeySequence::Quit);
	m_actionExit->setStatusTip(tr("Exit IBCP"));
	connect(m_actionExit, SIGNAL(triggered()), this, SLOT(close()));

	m_actionAbout = new QAction(tr("&About"), this);
	m_actionAbout->setStatusTip(tr("Show the IBCP About box"));
	connect(m_actionAbout, SIGNAL(triggered()), this, SLOT(about()));

	m_actionAboutQt = new QAction(tr("About &Qt"), this);
	m_actionAboutQt->setStatusTip(tr("Show the Qt library's About box"));
	connect(m_actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}


void MainWindow::createMenus(void)
{
	m_menuFile = menuBar()->addMenu(tr("&File"));
	m_menuFile->addAction(m_actionExit);

	menuBar()->addSeparator();

	m_menuHelp = menuBar()->addMenu(tr("&Help"));
	m_menuHelp->addAction(m_actionAbout);
	m_menuHelp->addAction(m_actionAboutQt);
}


void MainWindow::about(void)
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


MainWindow::~MainWindow()
{
	delete ui;
	delete m_commandLine;
}


// end: mainwindow.cpp
