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
	createActions();
	createMenus();
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


void MainWindow::createActions(void)
{
	struct ActionInfo
	{
		Action action;
		QString name;
		QKeySequence::StandardKey key;
		QString tip;
	} infoArray[] =
	{
		{ New, tr("&New"),
			QKeySequence::New, tr("Create a new program") },
		{ Open, tr("&Open..."),
			QKeySequence::Open, tr("Open an existing program") },
		{ Save, tr("&Save"),
			QKeySequence::Save, tr("Save the program to disk") },
		{ SaveAs, tr("Save &As..."),
			QKeySequence::SaveAs, tr("Save the program to a new file") },
		{ Exit, tr("E&xit"),
			QKeySequence::Quit, tr("Exit IBCP") },
		{ About, tr("&About"),
			QKeySequence::UnknownKey, tr("Show the IBCP About box") },
		{ AboutQt, tr("About &Qt"),
			QKeySequence::UnknownKey, tr("Show the Qt library's About box") },
		{ sizeof_Action }  // marks the end
	};

	for (ActionInfo *info = infoArray; info->action != sizeof_Action; info++)
	{
		QAction *action = new QAction(info->name, this);
		m_action[info->action] = action;
		if (info->key != QKeySequence::UnknownKey)
		{
			action->setShortcut(info->key);
		}
		action->setStatusTip(info->tip);
	}

	// connect action triggered signals to functions
	connect(m_action[New], SIGNAL(triggered()), this, SLOT(programNew()));
	connect(m_action[Open], SIGNAL(triggered()), this, SLOT(programOpen()));
	connect(m_action[Save], SIGNAL(triggered()), this, SLOT(programSave()));
	connect(m_action[SaveAs], SIGNAL(triggered()), this, SLOT(programSaveAs()));
	connect(m_action[Exit], SIGNAL(triggered()), this, SLOT(close()));
	connect(m_action[About], SIGNAL(triggered()), this, SLOT(about()));
	connect(m_action[AboutQt], SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}


void MainWindow::createMenus(void)
{
	m_menuFile = menuBar()->addMenu(tr("&File"));
	m_menuFile->addAction(m_action[New]);
	m_menuFile->addAction(m_action[Open]);
	m_menuFile->addAction(m_action[Save]);
	m_menuFile->addAction(m_action[SaveAs]);
	m_menuFile->addSeparator();
	m_menuFile->addAction(m_action[Exit]);

	menuBar()->addSeparator();

	m_menuHelp = menuBar()->addMenu(tr("&Help"));
	m_menuHelp->addAction(m_action[About]);
	m_menuHelp->addAction(m_action[AboutQt]);
}


void MainWindow::programNew(void)
{
}


void MainWindow::programOpen(void)
{
}


void MainWindow::programSave(void)
{
}


void MainWindow::programSaveAs(void)
{
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
