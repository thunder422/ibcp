// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: mainwinow.cpp - main window class source file
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

#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QStringListModel>
#include <QTimer>

#include "mainwindow.h"
#include "editbox.h"
#include "programmodel.h"
#include "recentfiles.h"


MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui {new Ui::MainWindow}
{
	m_commandLine.reset(new CommandLine(qApp->arguments()));
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

	statusBarCreate();

	// create actions for edit box context menu
	m_contextActions.append(ui->actionUndo);
	m_contextActions.append(ui->actionRedo);
	m_contextActions.append(new QAction(this));
	m_contextActions.last()->setSeparator(true);
	m_contextActions.append(ui->actionCut);
	m_contextActions.append(ui->actionCopy);
	m_contextActions.append(ui->actionPaste);
	m_contextActions.append(ui->actionDelete);
	m_contextActions.append(new QAction(this));
	m_contextActions.last()->setSeparator(true);
	m_contextActions.append(ui->actionSelectAll);

	// TODO settings will eventually have info about edit boxes that were open
	// TODO that will need to be restored, for now there is a single edit box

	//==================
	//  SETUP EDIT BOX
	//==================

	// create the starting program edit box
	m_editBox = new EditBox(m_program.unit(), this);
	setCentralWidget(m_editBox);

	connect(m_editBox->document(), SIGNAL(modificationChanged(bool)),
		this, SLOT(setWindowModified(bool)));
	connect(m_editBox->document(), SIGNAL(modificationChanged(bool)),
		ui->actionSave, SLOT(setEnabled(bool)));

	editBoxSetActive(m_editBox);

	//========================================
	//  SETUP PROGRAM LINE DELEGATE AND VIEW
	//========================================

	// setup program line delegate (connect to model line count changes)
	m_programLineDelegate = new ProgramLineDelegate(BaseLineNumber,
		ui->programView, this);
	connect(m_program.unit(), SIGNAL(lineCountChanged(int)),
		m_programLineDelegate, SLOT(lineNumberWidthUpdate(int)));

	// setup program view
	ui->programView->setItemDelegate(m_programLineDelegate);
	ui->programView->setModel(m_program.unit());

	//================
	//  LOAD PROGRAM
	//================

	// if a file name was specified on the command line
	// then it overrides the restored program
	if (!m_commandLine->fileName().isEmpty())
	{
		setCurProgram(m_commandLine->fileName());
	}
	else
	{
		// make sure window title is set before continuing
		setCurProgram(m_program.fileName());
	}

	// load program if one was saved or specified on command line
	if (!m_program.fileName().isEmpty()					// load a program?
			&& (!QFile::exists(m_program.fileName())	// program not found?
			|| !programLoad(m_program.fileName()))		// program not loaded?
			&& m_commandLine->fileName().isEmpty())		// no program argument
	{
		setCurProgram("");  // clear program path that was restored/set
		// TODO should an warning message be issued here?
	}

	m_guiActive = true;
	statusBarUpdate("");
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


// function to create widgets on status bar and make connections

void MainWindow::statusBarCreate(void)
{
	m_statusPositionLabel = new QLabel(this);
	m_statusPositionLabel->setAlignment(Qt::AlignHCenter);
	m_statusPositionLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	m_statusPositionLabel->setLineWidth(1);

	m_statusMessageLabel = new QLabel(this);
	m_statusMessageLabel->setAlignment(Qt::AlignLeft);
	m_statusMessageLabel->setIndent(3);
	m_statusMessageLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	m_statusMessageLabel->setLineWidth(1);

	statusBar()->addWidget(m_statusPositionLabel);
	statusBar()->addWidget(m_statusMessageLabel, 1);
}


// function to update the status bar

void MainWindow::statusBarUpdate(const QString &message)
{
	m_statusPositionLabel->setText(QString(" %1:%2 ")
		.arg(m_editBox->lineNumber()).arg(m_editBox->column() + 1));
	m_statusMessageLabel->setText(message);
}


// function to set an edit box active
void MainWindow::editBoxSetActive(EditBox *editBox)
{
	// TODO first disconnect signals and remove actions from current edit box
#if 0
	m_editBox->disconnect();
	QList<QAction *>actions = m_editBox->actions();
	foreach(QAction *action, actions)
	{
		m_editBox->removeAction(action);
	}
#endif

	// connect available signals to the appropriate edit actions
	connect(editBox, SIGNAL(undoAvailable(bool)),
		ui->actionUndo, SLOT(setEnabled(bool)));
	connect(editBox, SIGNAL(redoAvailable(bool)),
		ui->actionRedo, SLOT(setEnabled(bool)));
	connect(editBox, SIGNAL(copyAvailable(bool)),
		ui->actionCut, SLOT(setEnabled(bool)));
	connect(editBox, SIGNAL(copyAvailable(bool)),
		ui->actionCopy, SLOT(setEnabled(bool)));
	connect(editBox, SIGNAL(copyAvailable(bool)),
		ui->actionDelete, SLOT(setEnabled(bool)));
	connect(editBox, SIGNAL(errorsAvailable(bool)),
		ui->actionGoNextError, SLOT(setEnabled(bool)));
	connect(editBox, SIGNAL(errorsAvailable(bool)),
		ui->actionGoPrevError, SLOT(setEnabled(bool)));

	connect(editBox, SIGNAL(cursorChanged(QString)),
		this, SLOT(statusBarUpdate(QString)));

	// set context actions for edit box context menu
	editBox->addActions(m_contextActions);
	editBox->setContextMenuPolicy(Qt::ActionsContextMenu);
}


// function called when new program has been requested
//
//   - if it is ok to continue, the current program is cleared

void MainWindow::on_actionNew_triggered(void)
{
	programCaptureEditChanges();

	if (isOkToContinue())
	{
		// TODO remove all edit boxes except main edit box
		m_program.clear();
		setWindowModified(false);
		setCurProgram("");
	}
}


// function called when open program has been requested
//
//   - if it is ok to continue, an open file box requests the program to load
//   - if a valid file path was selected, the program is loaded
//   - the directory of the program loaded is saved

void MainWindow::on_actionOpen_triggered(void)
{
	if (isOkToContinue())
	{
		QString programPath = QFileDialog::getOpenFileName(this,
			tr("Open Program"), m_curDirectory, tr("Program files (*.*)"));
		if (!programPath.isEmpty())
		{
			programLoad(programPath);
			m_curDirectory = QFileInfo(programPath).path();
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
	if (m_program.fileName().isEmpty())
	{
		return on_actionSaveAs_triggered();
	}
	else
	{
		return programSave(m_program.fileName());
	}
}


// function called when save as program has been requested
//
//  - a save file box requests the file to save the program to
//  - if a valid file path was selected, the program is saved

bool MainWindow::on_actionSaveAs_triggered(void)
{
	QString programPath = m_program.fileName();
	if (programPath.isEmpty())
	{
		programPath = m_curDirectory;
	}
	programPath = QFileDialog::getSaveFileName(this, tr("Save Program"),
		programPath, tr("Program files (*.*)"));
	if (programPath.isEmpty())
	{
		return false;
	}
	m_curDirectory = QFileInfo(programPath).path();
	return programSave(programPath);
}


// function called when edit undo has been requested

void MainWindow::on_actionUndo_triggered(void)
{
	m_editBox->undo();
}


// function called when edit Redo has been requested

void MainWindow::on_actionRedo_triggered(void)
{
	m_editBox->redo();
}


// function called when edit Cut has been requested

void MainWindow::on_actionCut_triggered(void)
{
	m_editBox->cut();
}


// function called when edit Copy has been requested

void MainWindow::on_actionCopy_triggered(void)
{
	m_editBox->copy();
}


// function called when edit paste has been requested

void MainWindow::on_actionPaste_triggered(void)
{
	m_editBox->paste();
}


// function called when edit delete has been requested

void MainWindow::on_actionDelete_triggered(void)
{
	m_editBox->remove();
}


// function called when edit select all has been requested

void MainWindow::on_actionSelectAll_triggered(void)
{
	m_editBox->selectAll();
}


// function called when move to next error has been requested

void MainWindow::on_actionGoNextError_triggered(void)
{
	m_editBox->goNextError();
}


// function called when move to previous has been requested

void MainWindow::on_actionGoPrevError_triggered(void)
{
	m_editBox->goPrevError();
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
	m_program.setFileName(programPath);
	QString program = tr("Untitled");
	if (!m_program.fileName().isEmpty())
	{
		program = m_recentPrograms->addFile(m_program.fileName());
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
	m_editBox->resetModified();
	// reset modified flag in case file loaded is empty and document wasn't
	setWindowModified(false);

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

	programCaptureEditChanges();

	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::warning(this, tr("IBCP"), tr("Failed to save file."));
		return false;
	}
	QTextStream output(&file);
	output << m_editBox->toPlainText();
	m_editBox->resetModified();

	setCurProgram(programPath);
	statusBar()->showMessage(tr("Program saved"), 2000);
	return true;
}


// function to capture any edit box changed for modified lines
void MainWindow::programCaptureEditChanges(void)
{
	// make sure any current modified line is emitted from the edit box
	m_editBox->captureModifiedLine();
	// TODO this will need to loop through all open edit boxes
}


// names of the settings for the application
const char *geometrySettingsName = "geometry";
const char *windowStateSettingsName = "windowState";
const char *curDirectorySettingsName = "curDirectory";


// function to restore the settings saved the last time the program was run
//
//   - if there are no saved settings, reasonable defaults are used

void MainWindow::settingsRestore(void)
{
	QSettings settings("Thunder422", "IBCP");

	restoreGeometry(settings.value(geometrySettingsName).toByteArray());
	restoreState(settings.value(windowStateSettingsName).toByteArray());
	m_recentPrograms->restore(settings);
	m_program.settingsRestore(settings);
	m_curDirectory = settings.value(curDirectorySettingsName, ".").toString();
}


// function to save the settings for the program to be restored later
void MainWindow::settingsSave(void)
{
	QSettings settings("Thunder422", "IBCP");

	settings.setValue(geometrySettingsName, saveGeometry());
	settings.setValue(windowStateSettingsName, saveState());
	m_recentPrograms->save(settings);
	m_program.settingsSave(settings);
	settings.setValue(curDirectorySettingsName, m_curDirectory);
}


// end: mainwindow.cpp
