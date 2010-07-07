rem vim:ts=4:sw=4:
rem
rem	Interactive BASIC Compiler Project
rem	File: regtest.bat - Windows batch file to run all tests
rem	Copyright (C) 2010  Thunder422
rem
rem	This program is free software: you can redistribute it and/or modify
rem	it under the terms of the GNU General Public License as published by
rem	the Free Software Foundation, either version 3 of the License, or
rem	(at your option) any later version.
rem
rem	This program is distributed in the hope that it will be useful,
rem	but WITHOUT ANY WARRANTY; without even the implied warranty of
rem	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem	GNU General Public License for more details.
rem
rem	For a copy of the GNU General Public License,
rem	see <http://www.gnu.org/licenses/>.
rem 
rem
rem  Change History:
rem
rem  2010-05-29  initial release
rem  2010-06-10  added tests 10 and 11
rem  2010-06-29  added test 12
rem  2010-07-04  added test 13
rem
rem
rem  Usage:
rem    regtest
rem
rem  Details:
rem    output files are directed to current (IBCP) directory (ibcp.exe
rem    is required to produce same output as from MSYS/bash). The files
rem    are then compared comparing to the established output files in
rem    the test directory using the comp Windows command, while will
rem    report if files are the same or have differences (once complete,
rem    the comp command asks if any more files should be compared, must
rem    be answered No for batch to continue - comp has no option to not
rem    ask this question)
rem

del parser*.txt translator*.txt
for %%i in (1 2 3 4 5) do ibcp.exe -p %%i >parser%%i.txt
for %%i in (1 2 3 4 5 6 7 8 9 10 11 12 13) do ibcp.exe -t %%i >translator%%i.txt
comp test\parser*.txt parser*.txt
comp test\translator*.txt translator*.txt
