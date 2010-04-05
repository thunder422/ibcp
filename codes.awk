# vim:ts=4:sw=4:
#
#	Interactive BASIC Compiler Project
#	File: codes.awk - awk script for creating codes.txt file from ibcp.h
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
#  2010-03-20  initial release
#
#  2010-04-04  added comment header with GPL, change history and usage
#              modified script to handle digits in code names
# 
#  Usage:
#    awk -f codes.awk <ibcp.h >codes.txt

BEGIN { n = 0 }

$1 ~ /_Code/ {
	# 2010-04-03: allow digits in the name of the code
	match($1,"[A-Za-z0-9]*_Code")
	c[n] = substr($1,RSTART,RLENGTH)
	printf "%d: %s\n",i n, c[n]
	n++
}

END {
	print ""
	for (i = 0; i < n; i++)
		printf "%s = %d\n", c[i], i | "sort"
}
