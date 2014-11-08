// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: utility.h - utility class header file
//	Copyright (C) 2014
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
//	2014-10-18	initial version (parts removed from commandline.h)

#ifndef UTILITY_H
#define UTILITY_H

#include <string>


class Utility final  // final prevents derived classes
{
	Utility() = delete;  // prevent instances;
public:
	static std::string baseFileName(const std::string &filePath);
};


// lambda function for doing a case insentive character equal comparison
auto noCaseCharEqual = [](char c1, char c2)
{
	return toupper(c1) == toupper(c2);
};


// function for doing case insentive standard string equal comparison
inline bool noCaseStringEqual(const std::string &s1, const std::string &s2)
{
	return s1.size() == s2.size()
		&& std::equal(s1.begin(), s1.end(), s2.begin(), noCaseCharEqual);
}


// function for doing case insentive standard string begins with comparison
inline bool noCaseStringBeginsWith(const std::string &s, const std::string &sbw)
{
	return s.size() >= sbw.size()
		&& std::equal(sbw.begin(), sbw.end(), s.begin(), noCaseCharEqual);
}


#endif // UTILITY_H
