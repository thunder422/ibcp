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

project(ibcp)

cmake_minimum_required(VERSION 2.8)

# find Qt package
find_package(Qt4 REQUIRED QtCore QtGui)
include(${QT_USE_FILE})

# check GCC versionn
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
	OUTPUT_VARIABLE GCC_VERSION
)

# ckeck is a specific build type is set in the environment
if (NOT $ENV{CMAKE_BUILD_TYPE} STREQUAL "")
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE $ENV{CMAKE_BUILD_TYPE})
    endif (NOT CMAKE_BUILD_TYPE)
endif (NOT $ENV{CMAKE_BUILD_TYPE} STREQUAL "")
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif (NOT CMAKE_BUILD_TYPE)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(ibcp_COPYRIGHT_YEAR 2012)

set(ibcp_RELEASE_MAJOR 0)
set(ibcp_RELEASE_MINOR 2)
set(ibcp_RELEASE_PATCH 0)

set(ibcp_RELEASE_STRING release${ibcp_RELEASE_MAJOR}.${ibcp_RELEASE_MINOR})
if (ibcp_RELEASE_PATCH LESS 0)
	set(ibcp_RELEASE_STRING ${ibcp_RELEASE_STRING}${ibcp_RELEASE_PATCH})
else (ibcp_RELEASE_PATCH LESS 0)
	set(ibcp_RELEASE_STRING ${ibcp_RELEASE_STRING}.${ibcp_RELEASE_PATCH})
endif (ibcp_RELEASE_PATCH LESS 0)

find_package(Git)
if (GIT_FOUND)
	execute_process(COMMAND ${GIT_EXECUTABLE} describe
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		OUTPUT_VARIABLE git_release_string
		OUTPUT_STRIP_TRAILING_WHITESPACE
		RESULT_VARIABLE GIT_RESULT
		ERROR_VARIABLE PREVENT_STDERR_OUTPUT
	)
	if (${GIT_RESULT} EQUAL 0)
		string(REGEX MATCH ${ibcp_RELEASE_STRING} match ${git_release_string})
		if ("${match}" STREQUAL "")
			message(FATAL_ERROR "Release string '${ibcp_RELEASE_STRING}' "
				"does not match git '${git_release_string}'"
			)
		endif ("${match}" STREQUAL "")
		set(ibcp_RELEASE_STRING ${git_release_string})
	endif (${GIT_RESULT} EQUAL 0)
endif (GIT_FOUND)
message(STATUS "Build version: ${ibcp_RELEASE_STRING}")

# find awk executable
find_program (PROGRAM_AWK awk)
if (NOT PROGRAM_AWK)
	message(FATAL_ERROR "required 'awk' program was not found")
endif (NOT PROGRAM_AWK)

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

# configure a memtest script to pass source directory to the script
if (NOT WIN32)
	configure_file(
		"${PROJECT_SOURCE_DIR}/memtest.in"
		"${PROJECT_BINARY_DIR}/memtest" @ONLY
	)
endif (NOT WIN32)

# add the binary tree to the search path for include files
# so that ibcp_config.h, autoenums.h, test_names.h can be found
include_directories("${PROJECT_BINARY_DIR}")


# custom target
add_custom_command(OUTPUT autoenums.h
	COMMAND ${PROGRAM_AWK} -f ${PROJECT_SOURCE_DIR}/enums.awk
		${PROJECT_SOURCE_DIR}/
	MAIN_DEPENDENCY enums.awk
	DEPENDS table.cpp token.h
)

# custom target
add_custom_command(OUTPUT test_names.h
	COMMAND ${PROGRAM_AWK} -f ${PROJECT_SOURCE_DIR}/test_names.awk
		${PROJECT_SOURCE_DIR}/
	MAIN_DEPENDENCY test_names.awk
	DEPENDS table.cpp token.h ibcp.h
)

# list the header files
set(ibcp_HEADERS
	commandline.h
	ibcp.h
	parser.h
	table.h
	test_ibcp.h
	token.h
	translator.h
)

# list the main program sources
set(ibcp_SOURCES
	commandhandlers.cpp
	commandline.cpp
	main.cpp
	mainwindow.cpp
	parser.cpp
	table.cpp
	token.cpp
	tokenhandlers.cpp
	translator.cpp
	test_ibcp.cpp
)

# list the header files containing QOBJECT
set(ibcp_MOC_SOURCES
	mainwindow.h
)
qt4_wrap_cpp(ibcp_MOCS ${ibcp_MOC_SOURCES})

# list the program forms
set(ibcp_UIS
	mainwindow.ui
)
qt4_wrap_ui(ibcp_UI_HEADERS ${ibcp_UIS})

# add the executable
add_executable(ibcp
	autoenums.h
	test_names.h
	${ibcp_HEADERS}
	${ibcp_SOURCES}
	${ibcp_MOCS}
	${ibcp_UI_HEADERS}
)
target_link_libraries(ibcp ${QT_LIBRARIES})
