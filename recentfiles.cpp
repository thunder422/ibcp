// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: recentfiles.cpp - recent files list functions
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
//	2013-01-05	initial version

#include <QFileInfo>

#include "recentfiles.h"


// constructor for recent files list
//
//   - retrieves the first entry of the menu for the recent files
//     (assumes that the first entry in a separator
//   - creates all the actions for the open recent file menu
//   - connects all open file actions to the same open file slot

RecentFiles::RecentFiles(QMenu *menu, QObject *parent) :
	QObject(parent)
{
	QAction *separatorAction = menu->actions().at(0);

	for (int i = 0; i < MaxCount; ++i) {
		m_actions[i] = new QAction(menu);
		m_actions[i]->setVisible(false);
		m_actions[i]->setIcon(QIcon(":/images/file-open.png"));
		connect(m_actions[i], SIGNAL(triggered()),
			this, SLOT(on_openFile_triggered()));

		menu->insertAction(separatorAction, m_actions[i]);
	}

	m_menu = menu;
	m_count = MaxCount;
}


// function to add a new file path to the recent list
//
//   - removes all instances of the file path already in the list
//   - adds the file path to the beginning of the list
//   - removes any extra file paths from the list
//   - updates the menu actions to reflect the current list
//   - returns the base file name of the path

QString RecentFiles::addFile(const QString &path)
{
	m_files.removeAll(path);
	m_files.prepend(path);
	while (m_files.count() > MaxCount)
	{
		m_files.removeLast();
	}
	updateActions();

	return fileName(path);
}


// function to clear all the files in the recent list
//
//   - updates the menu actions to reflect the empty list

void RecentFiles::clear(void)
{
	m_files.clear();
	updateActions();
}


// names of the settings for the recent file list
const char *countSettingsName = "recentProgramsCount";
const char *filesSettingsName = "recentPrograms";


// function to restore the recent file list from the settings
//
//   - updates the menu actions to reflect the restored recent file list

void RecentFiles::restore(QSettings &settings)
{
	m_count = settings.value(countSettingsName, 4).toInt();
	m_files = settings.value(filesSettingsName).toStringList();
	updateActions();
}


// function to save the recent file list to the settings

void RecentFiles::save(QSettings &settings)
{
	settings.setValue(countSettingsName, m_count);
	settings.setValue(filesSettingsName, m_files);
}


// slot function to receive one of the recent file open menu triggered signals
//
//   - first retrieves the action from the sender
//   - emits the open file signal with the file path of the action

void RecentFiles::on_openFile_triggered(void)
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
	{
		emit openFile(action->data().toString());
	}
}


// function to update the menu actions from the current recent list
//
//   - first removes any files in the recent list that don't exist
//   - sets the text and data to each action from the recent list
//   - unused actions are set invisible
//   - disables the open recent menu if the recent list is empty

void RecentFiles::updateActions(void)
{
	QMutableStringListIterator path(m_files);
	while (path.hasNext())
	{
		if (!QFile::exists(path.next()))
		{
			path.remove();
		}
	}

	for (int i = 0; i < m_count; i++)
	{
		if (i < m_files.count())
		{
			QString number = tr("%1").arg(i + 1);
			number.insert(number.length() - 1, QChar('&'));
			QString text = tr("%1 %2").arg(number)
				.arg(fileName(m_files[i]));
			m_actions[i]->setText(text);
			m_actions[i]->setData(m_files[i]);
			m_actions[i]->setVisible(true);
		}
		else
		{
			m_actions[i]->setVisible(false);
		}
	}
	m_menu->setEnabled(!m_files.isEmpty());
}


// function to return the base file name of the file path

QString RecentFiles::fileName(const QString &path)
{
	return QFileInfo(path).fileName();
}


// end: recentfiles.cpp
