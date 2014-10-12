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

#include <iostream>
#include <sstream>

#include <QFileInfo>

#include "ibcp_config.h"  // for cmake
#include "commandline.h"
#include "test_ibcp.h"


// function to generate the copyright statement
const std::string CommandLine::copyrightStatement()
{
	std::ostringstream ss;

	ss << "  " << tr("Copyright").toStdString() << "(C) 2010-"
		<< ibcp_COPYRIGHT_YEAR << "  Thunder422";
	return ss.str();
}


// function to retrieve a base file name from a file path string
std::string CommandLine::baseFileName(const std::string &filePath)
{
	return QFileInfo(QString::fromStdString(filePath)).baseName().toStdString();
}


CommandLine::CommandLine(std::list<std::string> args) :
	m_cout {nullptr}
{
	// get base file name of program from first argument
	m_programName = baseFileName(args.front());
	args.pop_front();  // remove program name

	m_returnCode = -1;
	// NOTE: leave m_returnCode set to -1 to start GUI,

	// create usage string
	// append any other options here
	std::string usage = "usage: " + m_programName + "<program file>|-h|-?|-v|"
		+ Tester::options();

	if (args.size() == 0)
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

	Tester tester(m_programName, args, cout());
	if (tester.hasError())
	{
		cout(&std::cerr) << tester.errorMessage() << '\n';
		m_returnCode = 1;
		return;
	}
	else if (tester.hasOption())
	{
		if (tester.run(this))
		{
			m_returnCode = 0;
		}
		else
		{
			cout(&std::cerr) << tester.errorMessage() << '\n';
			m_returnCode = 1;
		}
		return;
	}

	// check if a possible file name was specified
	if (args.size() == 1 && !args.front().front() == '-')
	{
		// not an option so assume argument is a file name
		m_fileName = QString::fromStdString(args.front());
		return;
	}

	// unsupported option (NOTE: other options get checked before this)
	m_returnCode = isHelpOption(args) ? 0 : 1;  // error if not help option
	cout(m_returnCode == 0 ? &std::cout : &std::cerr) << usage << '\n';
}


std::ostream &CommandLine::cout(std::ostream *stream)
{
	m_cout = stream;
	return *m_cout;
}


// function to check if version option was specified and to process it
bool CommandLine::isVersionOption(const std::list<std::string> &args)
{
	if (args.size() != 1 || args.front() != "-v")
	{
		return false;  // not our option or extra/invalid options
	}
	cout() << m_programName + " version " + version().toStdString() << '\n';
	return true;
}


// function to check for help options
bool CommandLine::isHelpOption(const std::list<std::string> &args) const
{
	return args.size() == 1 && (args.front() == "-?" || args.front() == "-h");
}


// function to return the version number string
QString CommandLine::version(void) const
{
	QString versionString(ibcp_RELEASE_STRING);
	int start {versionString.indexOf(QRegExp("\\d"))};
	return versionString.mid(start);
}


// end: commandline.cpp
