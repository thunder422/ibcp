# vim:ts=4:sw=4
#
#  Interactive BASIC Compiler Project
#  File: CMakeLists.txt - CMake lists file for generating make file
#  Copyright (C) 2011-2011  Thunder422
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

cmake_minimum_required(VERSION 2.8)

project(ibcp)

# check GCC versionn
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
	OUTPUT_VARIABLE GCC_VERSION)
if (GCC_VERSION VERSION_LESS 4.5)
	message(FATAL_ERROR "GCC 4.5+ Required")
endif (GCC_VERSION VERSION_LESS 4.5)

set(ibcp_VERSION_MAJOR 0)
set(ibcp_VERSION_MINOR 1)
set(ibcp_VERSION_PATCH 16)

set(ibcp_COPYRIGHT_YEAR 2011)

# configure a header file to pass some of the CMake settings to the source code
configure_file(
	"${PROJECT_SOURCE_DIR}/ibcp_config.h.in"
	"${PROJECT_BINARY_DIR}/ibcp_config.h"
	)

# add the binary tree to the search path for include files
# so that ibcp_config.h, autoenums.h, test_codes.h can be found
include_directories("${PROJECT_BINARY_DIR}")

# custom target
add_custom_command(OUTPUT autoenums.h
	COMMAND awk -f ${PROJECT_SOURCE_DIR}/enums.awk ${PROJECT_SOURCE_DIR}/
	MAIN_DEPENDENCY enums.awk
	DEPENDS table.cpp
	)

# custom target
add_custom_command(OUTPUT test_codes.h
	COMMAND awk -f ${PROJECT_SOURCE_DIR}/test_codes.awk ${PROJECT_SOURCE_DIR}/
	MAIN_DEPENDENCY test_codes.awk
	DEPENDS table.cpp
	)

# list the include files
set(ibcp_INCS
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

# link in libgcc.a and libstrc++.a
add_definitions(-static-libgcc -static-libstdc++)

# add the executable
add_executable(ibcp.exe
		autoenums.h
		test_codes.h
		${ibcp_INCS}
		${ibcp_SRCS}
		)
