// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: program.h - program class source file
//	Copyright (C) 2013  Thunder422
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
//	2013-12-17	initial version

#include <QSettings>

#include "program.h"
#include "programmodel.h"


Program::Program(QObject *parent) :
	QObject(parent)
{
	// create empty program unit for main routine
	m_unit = new ProgramModel(this);
}


// names of the settings for the program
const char *curProgramSettingsName = "curProgram";


// function to clear the current program
void Program::clear(void)
{
	// TODO delete all but main program unit, which gets cleared
	m_unit->clear();
}


// function to restore the settings saved the last time the program was run
void Program::settingsRestore(QSettings &settings)
{
	m_fileName = settings.value(curProgramSettingsName).toString()
		.toStdString();
}


// function to save the settings for the program to be restored later
void Program::settingsSave(QSettings &settings)
{
	settings.setValue(curProgramSettingsName, m_fileName.c_str());
}


// end: program.cpp
