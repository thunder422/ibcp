// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: commandline.h - command line header file
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

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QCoreApplication>
#include <QStringList>
#include <QTextStream>

class CommandLine
{
	Q_DECLARE_TR_FUNCTIONS(CommandLine)

	QTextStream &cout(FILE *stream = stdout);
	void coutClose(void);
	bool isVersionOption(const QStringList &args);
	bool isHelpOption(const QStringList &args) const;

	static const char *s_gplStatement[];

	QString m_programName;
	int m_returnCode;
	QTextStream m_cout;
	QString m_usage;
public:
    CommandLine(const QStringList &args);
	~CommandLine();

	bool processed(void) const  // if processed then exit program
	{
		return m_returnCode >= 0;
	}
	int returnCode(void) const  // valid only if processed
	{
		return m_returnCode;
	}
	QString programName(void) const
	{
		return m_programName;
	}
	QString usage(void) const
	{
		return m_usage;
	}
	QString version(void) const;
	int copyrightYear(void) const;
	static const char **gplStatement(void)
	{
		return s_gplStatement;
	}
};


#endif // COMMANDLINE_H
