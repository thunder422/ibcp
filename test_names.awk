# vim:ts=4:sw=4:
#
#	Interactive BASIC Compiler Project
#	File: test_names.awk - awk script for creating test_names.h from table.cpp
#                          and ibcp.h
#	Copyright (C) 2010-2012  Thunder422
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	For a copy of the GNU General Public License,
#	see <http://www.gnu.org/licenses/>.
#
#
#  Change History:
#
#  2010-09-06  initial release (started with codes.awk)
#
#
#  Usage: awk -f test_names.awk
#
#  Note: The beginning of each table entry in table.cpp must be in the
#        format "\t{  // xxx_Code\n" to be read by this script correctly.
#        For TokenType_enum and DataType_enum in ibcp.h, their is no specific
#        format other than the enum value must be the first 'field' and only
#        one enum value per line, any comment lines are ignored plus any
#        comments on the line, and the 'sizeof' value is ignored.

BEGIN {
	code_enum = 0
	c = ""

	# get source path from optional first argument (2011-06-12)
	if (ARGC >= 2)
	{
		path = ARGV[1]
	}
	else
	{
		path = ""
	}
	table_source = path "table.cpp"
	output = "test_names.h"

	printf "// File: test_names.h - text of enumeration values\n" > output
	printf "//\n" > output
	printf "// This file generated automatically by test_names.awk\n" > output
	printf "//\n" > output
	printf "// ***  DO NOT EDIT  ***\n" > output
	printf "\n" > output

	while ((getline line < table_source) > 0)
	{
		if (code_enum == 0)
		{
			if (line ~ /static TableEntry tableEntries/)
			{
				# found start of table entries
				code_enum = 1
				printf "const char *code_name[] = {\n" > output
			}
		}
		else if (line !~ /};/)
		{
			nf = split(line, field)
			if (field[1] == "{" && field[2] == "//" && field[3] ~ /_Code/)
			{
				if (c != "")
				{
					printf ",\n" > output
				}
				# 2012-10-06: changed n argument from length of field[3]
				c = substr(field[3], 1, index(field[3], "_Code") - 1)
				printf "\t\"%s\"", c > output
			}
		}
		else
		{
			# found end of table entries
			printf "\n};\n" > output
			code_enum = 0
		}
	}
}
