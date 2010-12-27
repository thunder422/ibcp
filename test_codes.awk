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
#
#  Usage:
#    awk -f test_codes.awk <ibcp.h >test_codes.h

BEGIN {
	code_enum = 0
}

/enum Code/ {
	code_enum = 1
}

/Null_Code/ {
	code_enum = 2
}

$1 ~ /_Code/ {
	if (code_enum == 2) {
		for (i = 1; i <= NR; i++) {
			if (match($i,"[A-Za-z0-9]*_Code") > 0) {
				c = substr($i,RSTART,RLENGTH-5)
				printf "\"%s\"", c
				if (c == "Quit") {
					code_enum = 0
				} else {
					printf ", "
				}
			}
		}
		printf "\n"
	}
}
