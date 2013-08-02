rem vim:ts=4:sw=4:
rem
rem	Interactive BASIC Compiler Project
rem	File: regtestn.bat - batch file to run some tests with new translator
rem	Copyright (C) 2013  Thunder422
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
rem  2013-07-21  temporary batch file (based on regtest.bat)
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

del expression*.txt translator*.txt
for %%i in (test/expression*.dat test/translator*.dat) do (
	ibcp -n test\%%i >%%~ni.txt
)
comp test\expression*.txt expression*.txt
comp test\translator*.txt translator*.txt
