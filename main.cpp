// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: main.cpp - main function source file
//	Copyright (C) 2010-2012  Thunder422
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
//	2010-03-13	initial version

#include <QtGui/QApplication>

#include "ibcp_config.h"  // for cmake
#include "table.h"
#include "mainwindow.h"


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	MainWindow mainWindow;
	if (!mainWindow.isGuiActive())
	{
		return mainWindow.returnCode();
	}
	mainWindow.show();
	return app.exec();
}


// end: main.cpp
