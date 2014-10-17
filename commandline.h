// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: commandline.h - command line class header file
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

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <iostream>
#include <list>
#include <string>

class CommandLine
{
public:
	explicit CommandLine(std::list<std::string> args);

	bool processed(void) const  // if processed then exit program
	{
		return m_returnCode >= 0;
	}
	int returnCode(void) const  // valid only if processed
	{
		return m_returnCode;
	}

	const std::string programName() const
	{
		return m_programName;
	}
	std::string fileName(void) const
	{
		return m_fileName;
	}

	static const std::string copyrightStatement(const char *copyright
		= "Copyright");
	static std::string version(void);
	static std::string baseFileName(const std::string &filePath);

private:
	std::ostream &cout(std::ostream *stream = &std::cout);
	bool isVersionOption(const std::list<std::string> &args);
	bool isHelpOption(const std::list<std::string> &args) const;

	std::string m_programName;
	int m_returnCode;
	std::ostream *m_cout;
	std::string m_fileName;
};


#endif // COMMANDLINE_H
