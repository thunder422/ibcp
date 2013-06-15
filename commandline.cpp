// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: commandline.n - command line class source file
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
//	2012-11-25	initial version

#include <QFileInfo>

#include "ibcp_config.h"  // for cmake
#include "commandline.h"
#include "test_ibcp.h"


// create copyright statement string
const char *CommandLine::s_copyrightStatement =
	QT_TR_NOOP("%1  Copyright (C) 2010-%2  Thunder422");

// create warranty statement string list
const char *CommandLine::s_warrantyStatement[] = {
	QT_TR_NOOP("This program comes with ABSOLUTELY NO WARRANTY."),
	QT_TR_NOOP("This is free software, and you are welcome to"),
	QT_TR_NOOP("redistribute it under certain conditions."),
    NULL
};


CommandLine::CommandLine(const QStringList &args)
{
	// get base file name of program from first argument
	m_programName = QFileInfo(args.at(0)).baseName();
	m_returnCode = -1;
	// NOTE: leave m_returnCode set to -1 to start GUI,

	// create usage string
	QStringList options = Tester::options();
	// append any other options here
	options.prepend("<program file>|-h|-?|-v");
	m_usage = tr("usage: %1 [%2]").arg(m_programName).arg(options.join("|"));

	if (args.count() == 1)
	{
		// no options, start GUI
		return;
	}

	// parse command line arguments
	if (isVersionOption(args))
	{
		m_returnCode = 0;
		return;
	}

	Tester tester(args);
	if (tester.hasError())
	{
		cout(stderr) << tester.errorMessage() << endl;
		m_returnCode = 1;
		return;
	}
	else if (tester.hasOption())
	{
		if (tester.run(cout(), this))
		{
			m_returnCode = 0;
		}
		else
		{
			coutClose();  // close stdout
			cout(stderr) << tester.errorMessage() << endl;
			m_returnCode = 1;
		}
		return;
	}

	// check if a possible file name was specified
	if (args.count() == 2 && !args.at(1).startsWith("-"))
	{
		// not an option so assume argument is a file name
		m_fileName = args.at(1);
		return;
	}

	// unsupported option (NOTE: other options get checked before this)
	m_returnCode = isHelpOption(args) ? 0 : 1;  // error if not help option
	cout(m_returnCode == 0 ? stdout : stderr) << m_usage << endl;
}


CommandLine::~CommandLine()
{
	coutClose();
}


QTextStream &CommandLine::cout(FILE *stream)
{
	if (!m_cout.device())
	{
		// setup standard output stream first time
		QFile *output = new QFile;
		output->open(stream, QIODevice::WriteOnly | QIODevice::Unbuffered);
		m_cout.setDevice(output);
	}
	return m_cout;
}



void CommandLine::coutClose(void)
{
	if (m_cout.device())
	{
		QIODevice *output = m_cout.device();
		m_cout.setDevice(0);
		delete output;
	}
}


// function to check if version option was specified and to process it
bool CommandLine::isVersionOption(const QStringList &args)
{
	if (args.count() != 2 || args.at(1) != "-v")
	{
		return false;  // not our option or extra/invalid options
	}
	cout() << tr("%1 version %2").arg(m_programName).arg(version()) << endl;
	return true;
}


// function to check for help options
bool CommandLine::isHelpOption(const QStringList &args) const
{
	return args.count() == 2 && (args.at(1) == "-?" || args.at(1) == "-h");
}


// function to return the copyright year value
int CommandLine::copyrightYear(void) const
{
	return ibcp_COPYRIGHT_YEAR;
}


// function to return the version number string
QString CommandLine::version(void) const
{
	QString versionString(ibcp_RELEASE_STRING);
	int start = versionString.indexOf(QRegExp("\\d"));
	return versionString.mid(start);
}


// end: commandline.cpp
