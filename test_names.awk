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
#  2011-02-26  rewrote script to generate test_codes.h directly from the
#              table.cpp source file (all code performed in BEGIN block)
#
#  2011-06-12  for use with cmake, added an optional path to the source files
#              as the first argument on the awk command line
#  2012-10-06  changed the length argument of the substr() that extracts the
#              name of the code from the "_Code" suffix so that it will also
#              work with a DOS format file that contains the extra CR character
#              on the end of the line, because if the RS variable was set to
#              "/r/n" then this script no longer works on Linux
#  2012-10-14  renamed from test_codes.awk and now writes test_names.h
#              changed to write complete code_name[] array along with generating
#              the tokentype_name[] and datatype_name[] arrays from ibcp.h
#  2012-10-14  continued lines cause problems on Windows, so these were removed
#              by using an output variable to hold the "test_names.h" name
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
	ibcp_header = path "ibcp.h"
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
			if (line ~ /static TableEntry table_entries/)
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

	type_enum = ""
	c = ""
	while ((getline line < ibcp_header) > 0)
	{
		if (type_enum == "")
		{
			if (line ~ /enum TokenType/)
			{
				# found start of token types
				printf "\nconst char *tokentype_name[] = {\n" > output
				type_enum = "_TokenType"
			}
			else if (line ~ /enum DataType/)
			{
				# found start of data types
				printf "\nconst char *datatype_name[] = {\n" > output
				type_enum = "_DataType"
			}
		}
		else if (type_enum != "")
		{
			if (line ~ type_enum && line !~ /sizeof/)
			{
				nf = split(line, field)
				if (c != "")
				{
					printf ",\n" > output
				}
				c = substr(field[1], 1, index(field[1], type_enum) - 1)
				printf "\t\"%s\"", c > output
			}
			else if (line ~ /};/)
			{
				# found end of enum
				printf "\n};\n" > output
				type_enum = ""
				c = ""
			}
		}
	}
}
