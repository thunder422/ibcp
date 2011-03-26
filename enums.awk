# vim:ts=4:sw=4:
#
#	Interactive BASIC Compiler Project
#	File: enums.awk - awk script for creating autoenums.h from source files
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
#  2011-03-26  renamed from codes.awk to enums.awk
#              updated to write autoenums.h that includes enum Code and
#                enum TokenStatus (which comes from ibcp.cpp)
#              added exit code upon failure to terminate build
#
#
#  Usage: awk -f enums.awk
#
#  Note: The beginning of each table entry in table.cpp must be in the
#        format "\t{  // xxx_Code\n" to be read by this script correctly.
#        This script will check for duplicate codes, and if found, the
#        output files will not be written (compiler errors will occur when
#        building the ibcp program).
#
#  Note: The token status enumeration values are obtained from the
#        Token::message_array[] in the ibcp.cpp source file, each line must
#        start with the string, followed by the "//" comment, a space and the
#        name of the TokenStatus value (the "_TokenStatus" string will be added
#        to to the end of this name).  If the string starts with "BUG", the
#        string value is is a developmental TokenStatus value (the "BUG_"
#        string will be prefixed to the name).

BEGIN {
	n = 0
	code_enum = 0
	dups = 0

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
					print "Duplicate code found: ", field[3]
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

	msg_array = 0
	nts = 0
	while ((getline line < "ibcp.cpp") > 0)
	{
		if (msg_array == 0)
		{
			if (line ~ /const char \*Token::message_array/)
			{
				# found start of token message array
				msg_array = 1
			}
		}
		else if (line ~ /};/)
		{
			# found end of token message array
			msg_array = 0
		}
		else {
			nf = split(line, field)
			if (field[nf - 1] == "//")
			{
				if (field[1] !~ /BUG/)
				{
					ts = field[nf] "_TokenStatus"
				}
				else
				{
					ts = "BUG_" field[nf]
				}

				# check for duplicates
				if (ts in tokensts)
				{
					print "Duplicate token status found: ", ts
					dups++
				}
				else
				{
					tokensts[ts] = 1
				}
				ts_array[nts++] = ts
			}
		}
	}

	if (dups == 0)
	{
		# write header to 'autoenums.h'
		printf "// File: autoenums.h - ibcp enumerations\n" > "autoenums.h"
		printf "//\n" > "autoenums.h"
		printf "// This file generated automatically by enums.awk\n" > "autoenums.h"
		printf "//\n" > "autoenums.h"
		printf "// ***  DO NOT EDIT  ***\n" > "autoenums.h"

		# write 'enum Code' to 'autoenums.h'
		printf "\n" > "autoenums.h"
		printf "enum Code {\n" > "autoenums.h"
		printf "\tInvalid_Code = -1,\n" > "autoenums.h"
		for (i = 0; i < n; i++)
		{
			printf "\t%s,\n", c[i] > "autoenums.h"
		}
		printf "\tsizeof_Code\n" > "autoenums.h"
		printf "};\n" > "autoenums.h"

		# write 'enum TokenStatus' to 'autoenums.h'
		printf "\n" > "autoenums.h"
		printf "enum TokenStatus {\n" > "autoenums.h"
		for (i = 0; i < nts; i++)
		{
			printf "\t%s,\n", ts_array[i] > "autoenums.h"
		}
		printf "\tsizeof_TokenStatus\n" > "autoenums.h"
		printf "};\n" > "autoenums.h"

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
		print "Size of TokenStatus enumeration:", nts
	}
	else  # error found
	{
		print "" > "autoenums.h"
		print "" > "codes.txt"
		print ""
		print "Duplicate found in table.cpp or ibcp.cpp - please correct"
		# return failure code (2011-03-26)
		exit 1
	}
}
