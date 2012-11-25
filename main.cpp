// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: main.cpp - main function
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
#include <QFileInfo>
#include <QTextStream>
#include <QTimer>

#include "ibcp_config.h"  // for cmake
#include "test_ibcp.h"


void printGplHeader(QTextStream &cout, const QString &name)
{
	cout << endl << name << "  Copyright (C) 2010-" << ibcp_COPYRIGHT_YEAR
		<< "  Thunder422" << endl
		<< "This program comes with ABSOLUTELY NO WARRANTY." << endl
		<< "This is free software, and you are welcome to" << endl
		<< "redistribute it under certain conditions." << endl << endl;
}


// function to print version number
bool ibcpVersion(QTextStream &cout, const QString &name, QStringList &args)
{
	if (args.count() != 2 || args.at(1).compare("-v") != 0)
	{
		return false;  // not our option or extra/invalid options
	}
	cout << name << QObject::tr(" version %1").arg(ibcp_RELEASE_STRING + 7)
		<< endl;
	return true;
}


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	QStringList args = app.arguments();

	// get base file name of program from first argument
	QString programName = QFileInfo(args.at(0)).baseName();

	// setup standard output stream
	QFile output;
	output.open(stdout, QIODevice::WriteOnly | QIODevice::Unbuffered);
	QTextStream cout(&output);

	if (!ibcpVersion(cout, programName, args))
	{
		Tester tester(args);
		if (tester.hasError())
		{
			qWarning("%s", qPrintable(tester.errorMessage()));
		}
		else if (!tester.hasOption())
		{
			QStringList options = tester.options();
			options.prepend("-v");
			qWarning("%s: %s %s", qPrintable(QObject::tr("usage")),
				qPrintable(programName), qPrintable(options.join("|")));
		}
		else if (!tester.run(cout))
		{
			qWarning("%s", qPrintable(tester.errorMessage()));
		}
	}

	// force quit once event processing loop is started
	QTimer::singleShot(0, &app, SLOT(quit()));
	return app.exec();
}


// end: main.cpp
