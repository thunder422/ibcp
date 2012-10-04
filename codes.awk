# vim:ts=4:sw=4:
#
#	Interactive BASIC Compiler Project
#	File: codes.awk - awk script for creating codes.txt file from ibcp.h
#	Copyright (C) 2010-2011  Thunder422
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
#  2010-03-20  initial release
#
#  2010-04-04  added comment header with GPL, change history and usage
#              modified script to handle digits in code names
#
#  2010-04-28  added code to allow for multiple codes on a line
#              allow for Invalid_Code = -1
#              added -d to sort command to ignore "_" in codes names
#
#  2010-09-06  modified to only look for codes in the Code enumeration
#
#  2011-02-26  rewrote script to generate codes.h and codes.txt directly from
#              the table.cpp source file (all code performed in BEGIN block)
#
#  2012-10-03  added RS and ORS set to "\r\n" so that awak script will work
#              properly on Windows
#
#
#  Usage: awk -f codes.awk
#
#  Note: The beginning of each table entry in table.cpp must be in the
#        format "\t{  // xxx_Code\n" to be read by this script correctly.
#        This script will check for duplicate codes, and if found, the
#        output files will not be written (compiler errors will occur when
#        building the ibcp program).

BEGIN {
	n = 0
	code_enum = 0
	dups = 0
	RS = "\r\n"  # read dos format file

	while ((getline line < "table.cpp") > 0)
	{
		if (code_enum == 0)
		{
			if (line ~ /static TableEntry table_entries/)
			{
				# found start of table entries
				code_enum = 1
			}
		}
		else if (line ~ /};/)
		{
			# found end of table entries
			code_enum = 0
		}
		else
		{
			nf = split(line, field)
			if (field[1] == "{" && field[2] == "//" && field[3] ~ /_Code/)
			{
				# check for duplicates
				if (field[3] in codes)
				{
					print "Duplicate code found:", field[3]
					dups++
				}
				else
				{
					codes[field[3]] = 1
				}
				c[n++] = field[3]
			}
		}
	}

	if (dups == 0)
	{
		ORS = "\r\n"  # write dos format files

		# write 'codes.h'
		printf "// File: codes.h - code enumeration\n" > "codes.h"
		printf "//\n" > "codes.h"
		printf "// This file generated automatically by codes.awk\n" > "codes.h"
		printf "//\n" > "codes.h"
		printf "// ***  DO NOT EDIT  ***\n" > "codes.h"
		printf "\n" > "codes.h"
		printf "enum Code {\n" > "codes.h"
		printf "\tInvalid_Code = -1,\n" > "codes.h"
		for (i = 0; i < n; i++)
		{
			printf "\t%s,\n", c[i] > "codes.h"
		}
		printf "\tsizeof_Code\n" > "codes.h"
		printf "};\n" > "codes.h"

		# write 'codes.txt'
		for (i = 0; i < n; i++)
		{
			printf "%d: %s\n", i, c[i] > "codes.txt"
		}
		printf "\n" > "codes.txt"
		for (i = 0; i < n; i++)
		{
			# 2010-04-28: added -d to ignore "_" in codes names
			printf "%s = %d\n", c[i], i | "sort -d >> codes.txt"
		}

		# output summary
		print "Size of Code enumeration:", n
	}
	else  # error found
	{
		print "" > "codes.h"
		print "" > "codes.txt"
		print ""
		print "Duplicate codes found in table.cpp - please fix"
	}
}
