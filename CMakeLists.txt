# vim:ts=4:sw=4
#
#  Interactive BASIC Compiler Project
#  File: CMakeLists.txt - CMake lists file for generating make file
#  Copyright (C) 2011-2012  Thunder422
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  For a copy of the GNU General Public License,
#  see <http://www.gnu.org/licenses/>.
#
#  Change History:
#
#  2011-06-11	initial version
#  2012-09-23	changed GCC 4.5+ fatal error to just a warning
#  2012-10-07	add if GCC < 4.5 then don't add static libraries
#  2012-10-08	corrected in source directory builds
#  2012-10-12	added a configure_file command for the regtest script
#  2012-10-14	renamed test_codes.awk to test_names.awk, which now generates
#               test_names.h that also includes text enums from ibcp.h
#  2012-10-16	added build of test programs with separate tests target

cmake_minimum_required(VERSION 2.8)

project(ibcp)

# check GCC versionn
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
	OUTPUT_VARIABLE GCC_VERSION
)
if (GCC_VERSION VERSION_LESS 4.5)
	message(WARNING_ERROR
		"GCC 4.5+ Required for static building with libstdc++"
	)
endif (GCC_VERSION VERSION_LESS 4.5)

set(ibcp_VERSION_MAJOR 0)
set(ibcp_VERSION_MINOR 1)
set(ibcp_VERSION_PATCH 16)

set(ibcp_COPYRIGHT_YEAR 2012)

# find awk executable
find_program (PROGRAM_AWK awk)
if (PROGRAM_AWK STREQUAL "PROGRAM_AWK-NOTFOUND")
	message(FATAL_ERROR "required awk program was not found")
endif (PROGRAM_AWK STREQUAL "PROGRAM_AWK-NOTFOUND")

# configure a header file to pass some of the CMake settings to the source code
configure_file(
	"${PROJECT_SOURCE_DIR}/ibcp_config.h.in"
	"${PROJECT_BINARY_DIR}/ibcp_config.h"
)

# configure a regtest script to pass source directory to the script
configure_file(
	"${PROJECT_SOURCE_DIR}/regtest.in"
	"${PROJECT_BINARY_DIR}/regtest" @ONLY
)

# add the binary tree to the search path for include files
# so that ibcp_config.h, autoenums.h, test_names.h can be found
# which is not necessary for in source build
string(COMPARE NOTEQUAL ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR} OUT_SOURCE)
if (OUT_SOURCE)
	include_directories("${PROJECT_BINARY_DIR}")
endif (OUT_SOURCE)


# custom target
add_custom_command(OUTPUT autoenums.h
	COMMAND ${PROGRAM_AWK} -f ${PROJECT_SOURCE_DIR}/enums.awk
		${PROJECT_SOURCE_DIR}/
	MAIN_DEPENDENCY enums.awk
	DEPENDS table.cpp
)

# custom target
add_custom_command(OUTPUT test_names.h
	COMMAND ${PROGRAM_AWK} -f ${PROJECT_SOURCE_DIR}/test_names.awk
		${PROJECT_SOURCE_DIR}/
	MAIN_DEPENDENCY test_names.awk
	DEPENDS table.cpp ibcp.h
)

# list the include files
set(ibcp_HDRS
	ibcp.h
	list.h
	stack.h
	string.h
)

# list the main program sources
set(ibcp_SRCS
	ibcp.cpp
	parser.cpp
	string.cpp
	table.cpp
	translator.cpp
	test_ibcp.cpp
)

# make linker link in static libgcc.a and libstrc++.a
if (GCC_VERSION VERSION_LESS 4.5)
	add_definitions(-static-libgcc -static-libstdc++)
endif (GCC_VERSION VERSION_LESS 4.5)

# add the executable
add_executable(ibcp
	autoenums.h
	test_names.h
	${ibcp_INCS}
	${ibcp_SRCS}
)

# add "make tests" target for making test programs
add_custom_target(tests
	DEPENDS test_stack test_nums test_string test_cons test_stack2
)

# test program executables for tests target (not built by default)
add_executable(test_stack EXCLUDE_FROM_ALL
	list.h
	test/test_stack.cpp
)
add_executable(test_nums EXCLUDE_FROM_ALL
	test/test_nums.cpp
)
add_executable(test_string EXCLUDE_FROM_ALL
	string.h
	string.cpp
	test/test_string.cpp
)
add_executable(test_cons EXCLUDE_FROM_ALL
	test/test_cons.cpp
)
add_executable(test_stack2 EXCLUDE_FROM_ALL
	stack.h
	test/test_stack2.cpp
)
