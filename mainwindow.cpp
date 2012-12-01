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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "commandline.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	CommandLine commandLine(qApp->arguments());
	if (commandLine.processed())
	{
		// don't start GUI and retrieve the return code
		m_guiActive = false;
		m_returnCode = commandLine.returnCode();
		return;
	}

	// start GUI here
	ui->setupUi(this);
	m_guiActive = true;
}


MainWindow::~MainWindow()
{
	delete ui;
}


// end: mainwindow.cpp
