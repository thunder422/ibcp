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
#  2010-04-28  added code to allow for multiple codes on a line
#              allow for Invalid_Code = -1
#              added -d to sort command to ignore "_" in codes names
#
#  Usage:
#    awk -f codes.awk <ibcp.h >codes.txt

# 2010-04-28: allow for Invalid_Code=-1
BEGIN { n = -1 }

$1 ~ /_Code/ {
	# 2010-04-28: allow for multiple codes on line
	for (i = 1; i <= NR; i++) {
		# 2010-04-03: allow digits in the name of the code
		if (match($i,"[A-Za-z0-9]*_Code") > 0) {
			c[n] = substr($i,RSTART,RLENGTH)
			printf "%d: %s\n", n, c[n]
			n++
		}
	}
}

END {
	print ""
	for (i = 0; i < n; i++)
		# 2010-04-28: added -d to ignore "_" in codes names
		printf "%s = %d\n", c[i], i | "sort -d"
}
