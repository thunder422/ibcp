// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: ibcp.h - miscellaneous functions
//	Copyright (C) 2010-2011  Thunder422
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
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTimer>

#include "ibcp.h"
#include "ibcp_config.h"  // for cmake
#include "token.h"
#include "table.h"
#include "parser.h"
#include "translator.h"


void printGplHeader(QTextStream &cout, const QString &name)
{
	cout << endl << name << "  Copyright (C) 2010-" << ibcp_COPYRIGHT_YEAR
		<< "  Thunder422" << endl
		<< "This program comes with ABSOLUTELY NO WARRANTY." << endl
		<< "This is free software, and you are welcome to" << endl
		<< "redistribute it under certain conditions." << endl << endl;
}

// Program Name and Length less extension
// TODO this needs to be put into a class somewhere)
QString programName;

// function to print version number
bool ibcpVersion(QTextStream &cout, const QString &name, QStringList &args)
{
	if (args.count() != 2 || args.at(1).compare("-v") != 0)
	{
		return false;  // not our options
	}
	cout << name << QObject::tr(" version %1").arg(ibcp_RELEASE_STRING + 7)
		<< endl;
	return true;
}


// prototype for test function
bool ibcpTest(QTextStream &cout, Translator &translator, QStringList &args);

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	QStringList args = app.arguments();

	// get base file name of program from first argument
	programName = QFileInfo(args.at(0)).baseName();

	// setup standard output stream
	QFile output;
	output.open(stdout, QIODevice::WriteOnly | QIODevice::Unbuffered);
	QTextStream cout(&output);

	if (!ibcpVersion(cout, programName, args))
	{
		printGplHeader(cout, programName);

		Token::initialize();

		QStringList errors = Table::create();
		if (!errors.isEmpty())
		{
			int n = 0;
			foreach (QString error, errors)
			{
				qWarning("%s", qPrintable(QObject::tr("Error #%1: %2").arg(++n)
					.arg(error)));
			}
			qFatal("%s", qPrintable(QObject::tr("Program aborting!")));
		}
		cout << "Table initialization successful." << endl;

		Translator translator(Table::instance());

		if (!ibcpTest(cout, translator, args))
		{
			qWarning("%s: %s -v -t <%s>|-tp|-te|-tt",
				qPrintable(QObject::tr("usage")), qPrintable(programName),
				qPrintable(QObject::tr("test_file")));
		}
	}
	QTimer::singleShot(0, &app, SLOT(quit()));
	return app.exec();
}


// end: ibcp.cpp
