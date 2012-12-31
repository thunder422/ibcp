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
#include <QFileDialog>
#include <QFileInfo>
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
	setCurProgram("");

	// create the starting program edit box
	m_editBox = new EditBox;
	setCentralWidget(m_editBox);

	connect(m_editBox->document(), SIGNAL(modificationChanged(bool)),
		this, SLOT(setWindowModified(bool)));

	m_guiActive = true;
}


// function called when the user is attempting to close the main window
//
//   - the signal is accepted if it is ok to continue, ignored otherwise
//   - the current settings are saved

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (isOkToContinue())
	{
		settingsSave();
		event->accept();
	}
	else
	{
		event->ignore();
	}
}


// function called when new program has been requested
//
//   - if it is ok to continue, the current program is cleared

void MainWindow::on_actionNew_triggered(void)
{
	if (isOkToContinue())
	{
		m_editBox->clear();
		setWindowModified(false);
		setCurProgram("");
	}
}


// function called when open program has been requested
//
//   - if it is ok to continue, an open file box requests the program to load
//   - if a valid file path was selected, the program is loaded

void MainWindow::on_actionOpen_triggered(void)
{
	if (isOkToContinue())
	{
		QString programPath = QFileDialog::getOpenFileName(this,
			tr("Open Program"), ".", tr("Program files (*.*)"));
		if (!programPath.isEmpty())
		{
			programLoad(programPath);
		}
	}
}


// function called when save program has been requested
//
//   - if the current program is not set, the save as function is called
//   - otherwise the program is saved under the current program

bool MainWindow::on_actionSave_triggered(void)
{
	if (m_curProgram.isEmpty())
	{
		return on_actionSaveAs_triggered();
	}
	else
	{
		return programSave(m_curProgram);
	}
}


// function called when save as program has been requested
//
//  - a save file box requests the file to save the program to
//  - if a valid file path was selected, the program is saved

bool MainWindow::on_actionSaveAs_triggered(void)
{
	QString programPath = QFileDialog::getSaveFileName(this,
		tr("Save Program"), ".", tr("Program files (*.*)"));
	if (programPath.isEmpty())
	{
		return false;
	}
	return programSave(programPath);
}


// function called when help about has been requested

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


// function called when help about Qt has been requested
void MainWindow::on_actionAboutQt_triggered(void)
{
	qApp->aboutQt();
}


// function the sets the current program path and sets the window title
void MainWindow::setCurProgram(const QString &programPath)
{
	m_curProgram = programPath;
	QString program = tr("Untitled");
	if (!m_curProgram.isEmpty())
	{
		program = QFileInfo(m_curProgram).fileName();
	}
	setWindowTitle(tr("%1[*] - %2").arg(program).arg(tr("IBCP")));
}


// function that checks if it is ok to continue when the current program
// has been modified; return false if the user cancels the request to save
// the modified program
bool MainWindow::isOkToContinue(void)
{
	if (isWindowModified())
	{
		int answer = QMessageBox::warning(this, tr("IBCP"), tr("The program "
			"has been modified.\nDo you want to save the changes?"),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (answer == QMessageBox::Yes)
		{
			return on_actionSave_triggered();
		}
		else if (answer == QMessageBox::Cancel)
		{
			return false;
		}
	}
	return true;
}


// function to load a program into memory
//
//   - for now just reads a text file and puts the text in the edit box
//   - eventually it will need to be parsed, translated, encoded and stored

bool MainWindow::programLoad(const QString &programPath)
{
	QFile file(programPath);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, tr("IBCP"), tr("Failed to open file."));
		return false;
	}
	QTextStream input(&file);

	m_editBox->setPlainText(input.readAll());
	m_editBox->document()->setModified(false);

	setCurProgram(programPath);
	return true;
}


// function to save the program to the indicated path
//
//   - for now just grabs the text from the edit box and saves it
//   - eventually this made be sufficient as the edit box will contain the
//     text representation of the stored program, but may also need to
//     collect the text from other edit boxes and/or subroutine/functions

bool MainWindow::programSave(const QString &programPath)
{
	QFile file(programPath);

	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, tr("IBCP"), tr("Failed to save file."));
		return false;
	}
	QTextStream output(&file);
	output << m_editBox->toPlainText();
	m_editBox->document()->setModified(false);

	setCurProgram(programPath);
	return true;
}


// function to restore the settings saved the last time the program was run
//
//   - if there are no saved settings, reasonable defaults are used

void MainWindow::settingsRestore(void)
{
	QSettings settings("Thunder422", "IBCP");

	restoreGeometry(settings.value("geometry").toByteArray());
}


// function to save the settings for the program to be restored later
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
