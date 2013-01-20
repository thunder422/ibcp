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
#include "recentfiles.h"


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
	m_recentPrograms = new RecentFiles(ui->menuOpenRecent, this);
	connect(m_recentPrograms, SIGNAL(openFile(QString)),
		this, SLOT(programOpen(const QString)));
	settingsRestore();

	// TODO settings will eventually have info about edit boxes that were open
	// TODO that will need to be restored, for now there is a single edit box

	// create the starting program edit box
	m_editBox = new EditBox;
	setCentralWidget(m_editBox);

	connect(m_editBox->document(), SIGNAL(modificationChanged(bool)),
		this, SLOT(setWindowModified(bool)));

	// if a file name was specified on the command line
	// then it overrides the restored program
	if (!m_commandLine->fileName().isEmpty())
	{
		setCurProgram(m_commandLine->fileName());
	}

	// load program if one was saved or specified on command line
	if (!m_curProgram.isEmpty()						// load a program?
			&& (!QFile::exists(m_curProgram)		// program not found?
			|| !programLoad(m_curProgram))			// program not loaded?
			&& m_commandLine->fileName().isEmpty())	// no program argument
	{
		setCurProgram("");  // clear program path that was restored/set
		// TODO should an warning message be issued here?
	}

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


// function called when clear recent program list has been requested

void MainWindow::on_actionClearRecent_triggered(void)
{
	m_recentPrograms->clear();
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

	const char *copyright = m_commandLine->copyrightStatement();
	// add version and copyright year to string
	aboutString.append(tr(copyright).arg(tr("Version %1")
		.arg(m_commandLine->version())).arg(m_commandLine->copyrightYear()));

	// add license statements
	aboutString.append(tr("<p>This program is free software: you can "
		"redistribute it and/or modify it under the terms of the GNU General "
		"Public License as published by the Free Software Foundation, either "
		"version 3 of the License, or (at your option) any later version."));
	aboutString.append(tr("<p>This program is distributed in the hope that it "
		"will be useful, but WITHOUT ANY WARRANTY; without even the implied "
		"warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  "
		"See the GNU General Public License for more details."));
	aboutString.append(tr("<p>For a copy of the GNU General Public License, "
		"<br>see <a href=\"http://www.gnu.org/licenses\" target=\"_blank\">"
		"http://www.gnu.org/licenses</a>."));

	// add oxygen icon license statement
	aboutString.append(tr("<p>The Oxygen icons (see "
		"<a href=\"http://www.oxygen-icons.org\" target=\"_blank\">"
		"http://www.oxygen-icons.org</a>) are used<br>under LGPLv3, "
		"see <a href=\"http://www.gnu.org/copyleft/lesser.html\" "
		"target=\"_blank\">http://www.gnu.org/copyleft/lesser.html</a>."));

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
		program = m_recentPrograms->addFile(m_curProgram);
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


// slot function connected to the open file signal

void MainWindow::programOpen(const QString programPath)
{
	if (isOkToContinue())
	{
		programLoad(programPath);
	}
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
	statusBar()->showMessage(tr("Program loaded"), 2000);
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
	statusBar()->showMessage(tr("Program saved"), 2000);
	return true;
}


// function to restore the settings saved the last time the program was run
//
//   - if there are no saved settings, reasonable defaults are used

void MainWindow::settingsRestore(void)
{
	QSettings settings("Thunder422", "IBCP");

	restoreGeometry(settings.value("geometry").toByteArray());
	m_recentPrograms->restore(settings);
	m_curProgram = settings.value("curProgram").toString();
}


// function to save the settings for the program to be restored later
void MainWindow::settingsSave(void)
{
	QSettings settings("Thunder422", "IBCP");

	settings.setValue("geometry", saveGeometry());
	m_recentPrograms->save(settings);
	settings.setValue("curProgram", m_curProgram);
}


MainWindow::~MainWindow()
{
	delete ui;
	delete m_commandLine;
}


// end: mainwindow.cpp
