# vim:ts=4:sw=4:
#
#	Interactive BASIC Compiler Project
#	File: test_codes.awk - awk script for creating test_codes.h from ibcp.h
#	Copyright (C) 2010  Thunder422
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
#  2011-02-26  rewrote script to generate test_codes.h directly from the
#              table.cpp source file (all code performed in BEGIN block)
#
#  2011-06-12  for use with cmake, added an optional path to the source files
#              as the first argument on the awk command line
#
#
#  Usage: awk -f test_codes.awk
#
#  Note: The beginning of each table entry in table.cpp must be in the
#        format "\t{  // xxx_Code\n" to be read by this script correctly.
#        This script should be run after the codes.awk script

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

	printf "// File: test_codes.h - text of code enumeration values\n" > "test_codes.h"
	printf "//\n" > "test_codes.h"
	printf "// This file generated automatically by test_codes.awk\n" > "test_codes.h"
	printf "//\n" > "test_codes.h"
	printf "// ***  DO NOT EDIT  ***\n" > "test_codes.h"
	printf "\n" > "test_codes.h"

	while ((getline line < table_source) > 0)
	{
		if (code_enum == 0)
		{
			if (line ~ /static TableEntry table_entries/)
			{
				# found start of table entries
				code_enum = 1
			}
		}
		else if (line !~ /};/)
		{
			nf = split(line, field)
			if (field[1] == "{" && field[2] == "//" && field[3] ~ /_Code/)
			{
				if (c != "")
				{
					printf ",\n" > "test_codes.h"
				}
				c = substr(field[3], 1, length(field[3]) - 5)
				printf "\"%s\"", c > "test_codes.h"
			}
		}
		else
		{
			# found end of table entries
			printf "\n" > "test_codes.h"
			code_enum = 0
		}
	}
}
