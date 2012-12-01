// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: commandline.n - command line functions source file
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
//	2012-11-25	initial version

#include <QFileInfo>

#include "ibcp_config.h"  // for cmake
#include "commandline.h"
#include "test_ibcp.h"


CommandLine::CommandLine(const QStringList &args)
{
	// get base file name of program from first argument
	m_programName = QFileInfo(args.at(0)).baseName();
	m_returnCode = -1;
	// NOTE: leave m_returnCode set to -1 to start GUI,

	// parse command line arguments
	if (version(args))
	{
		m_returnCode = 0;
		return;
	}

	// create GPL statement string list
	m_gplStatement
		.append(QString(QT_TR_NOOP("%1  Copyright (C) 2010-%2  Thunder422"))
		.arg(m_programName).arg(ibcp_COPYRIGHT_YEAR));
	m_gplStatement
		.append(QT_TR_NOOP("This program comes with ABSOLUTELY NO WARRANTY."));
	m_gplStatement
		.append(QT_TR_NOOP("This is free software, and you are welcome to"));
	m_gplStatement
		.append(QT_TR_NOOP("redistribute it under certain conditions."));

	Tester tester(args);
	if (tester.hasError())
	{
		qWarning("%s", qPrintable(tester.errorMessage()));
		m_returnCode = 1;
		return;
	}
	else if (tester.hasOption())
	{
		if (tester.run(cout(), m_gplStatement))
		{
			m_returnCode = 0;
		}
		else
		{
			qWarning("%s", qPrintable(tester.errorMessage()));
			m_returnCode = 1;
		}
		return;
	}

	// unsupported option (NOTE: other options get checked before this)
	QStringList options = tester.options();
	options.prepend("-v");
	// append any other options here
	qWarning("%s: %s %s", qPrintable(tr("usage")), qPrintable(m_programName),
		qPrintable(options.join("|")));
	m_returnCode = 1;
}


CommandLine::~CommandLine()
{
	if (m_cout.device())
	{
		QIODevice *output = m_cout.device();
		m_cout.setDevice(0);
		delete output;
	}
}


QTextStream &CommandLine::cout(void)
{
	if (!m_cout.device())
	{
		// setup standard output stream first time
		QFile *output = new QFile;
		output->open(stdout, QIODevice::WriteOnly | QIODevice::Unbuffered);
		m_cout.setDevice(output);
	}
	return m_cout;
}


// function to print version number
bool CommandLine::version(const QStringList &args)
{
	if (args.count() != 2 || args.at(1) != "-v")
	{
		return false;  // not our option or extra/invalid options
	}
	cout() << tr("%1 version %2").arg(m_programName)
		.arg(ibcp_RELEASE_STRING + 7) << endl;
	return true;
}


// end: commandline.cpp
