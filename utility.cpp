// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: utility.cpp - utility class source file
//	Copyright (C) 2014  Thunder422
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
//	2014-10-18	initial version (parts removed from commandline.cpp)

#include <QFileInfo>

#include "utility.h"


// function to retrieve a base file name from a file path string
std::string Utility::baseFileName(const std::string &filePath)
{
	return QFileInfo(filePath.c_str()).baseName().toStdString();
}


// end: utility.cpp
