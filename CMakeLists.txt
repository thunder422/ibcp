# vim:ts=4:sw=4
#
#  Interactive BASIC Compiler Project
#  File: CMakeLists.txt - CMake lists file for generating make file
#  Copyright (C) 2011-2013  Thunder422
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

# enable all compiler warnings as errors, enable c++11 compilation
set(CMAKE_CXX_FLAGS
	"${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic-errors -std=c++11"
)

# check for c++11 support
message(STATUS "Checking for working C++11 compiler: ${CMAKE_CXX_COMPILER}")
try_compile(CPP11_GOOD
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/test/cpp11.cpp
	OUTPUT_VARIABLE OUTPUT
)
if (${CPP11_GOOD})
	message(STATUS
		"Checking for working C++11 compiler: ${CMAKE_CXX_COMPILER} -- works"
	)
else ()
	message(FATAL_ERROR
		"The C++ compiler is not able to compile a simple C++11 test program.\n"
		"It fails with the following output:\n"
		"${OUTPUT}"
	)
endif ()

# prevent in source directory builds
if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
	message(FATAL_ERROR
		"\nbuilding in the source directory not allowed"
		"\nplease create a separate build directory"
	)
endif (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)

# find Qt package
find_package(Qt4 REQUIRED QtCore QtGui)
include(${QT_USE_FILE})

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

set(ibcp_COPYRIGHT_YEAR 2014)

set(ibcp_RELEASE_MAJOR 0)
set(ibcp_RELEASE_MINOR 6)
set(ibcp_RELEASE_PATCH 1)

set(ibcp_RELEASE_STRING v${ibcp_RELEASE_MAJOR}.${ibcp_RELEASE_MINOR})
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
find_program (PROGRAM_AWK gawk)
if (NOT PROGRAM_AWK)
	find_program (PROGRAM_AWK awk)
	if (NOT PROGRAM_AWK)
		message(FATAL_ERROR "required 'awk' program was not found")
	endif (NOT PROGRAM_AWK)
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
	# determine if ld-2.19.so is present
	find_file(LD219 ld-2.19.so PATHS /lib/x86_64-linux-gnu)
	if (NOT LD219)
		find_file(LD219 ld-2.19.so PATHS /lib/i386-linux-gnu)
	endif ()
	if (NOT LD219)
		# build memory error suppression file for specific Qt version
		set(ibcp_QT_VERSION
			${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}
		)
		configure_file(
			"${PROJECT_SOURCE_DIR}/test/ibcp.supp.in"
			"${PROJECT_BINARY_DIR}/ibcp.supp" @ONLY
		)
	else ()
		# copy memory error suppression file for ld2.19.so
		configure_file(
			"${PROJECT_SOURCE_DIR}/test/ibcp-ld2.19.supp"
			"${PROJECT_BINARY_DIR}/ibcp.supp" @ONLY
		)
	endif ()
endif (NOT WIN32)

# add the binary tree to the search path for include files
# so that ibcp_config.h, autoenums.h, test_names.h can be found
include_directories(
	${PROJECT_SOURCE_DIR}
	${PROJECT_BINARY_DIR}
)


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
	DEPENDS table.cpp
)

# list the main program sources
set(ibcp_SOURCES
	basic/basic.cpp
	basic/input.cpp
	basic/let.cpp
	basic/print.cpp
	commandline.cpp
	dictionary.cpp
	errorlist.cpp
	editbox.cpp
	main.cpp
	mainwindow.cpp
	parser.cpp
	program.cpp
	programlinedelegate.cpp
	programmodel.cpp
	recentfiles.cpp
	recreator.cpp
	rpnlist.cpp
	statusmessage.cpp
	table.cpp
	token.cpp
	translator.cpp
	test_ibcp.cpp
	utility.cpp
)

# list the header files containing QOBJECT
set(ibcp_MOC_SOURCES
	editbox.h
	mainwindow.h
	program.h
	programlinedelegate.h
	programmodel.h
	recentfiles.h
)
qt4_wrap_cpp(ibcp_MOCS ${ibcp_MOC_SOURCES})

# list the program forms
set(ibcp_UIS
	mainwindow.ui
)
qt4_wrap_ui(ibcp_UI_HEADERS ${ibcp_UIS})

# list of program resources
set(ibcp_RESOURCES
	ibcp.qrc
)
qt4_add_resources(ibcp_RCC_SOURCES ${ibcp_RESOURCES})

# test and other files
file(GLOB other_files
	RELATIVE ${PROJECT_SOURCE_DIR} test/*.dat test/*.txt test/*.out
)
list(SORT other_files)
list(APPEND other_files LICENSE README rel_notes.txt)

# convert path to DOS style for MinGW generator
function (ConvertPath output input)
	if ("${CMAKE_GENERATOR}" STREQUAL "MinGW Makefiles")
		string(REPLACE "/" "\\" work ${input})
		set(${output} "\"${work}\"" PARENT_SCOPE)
	else ()
		set(${output} ${input} PARENT_SCOPE)
	endif ()
endfunction (ConvertPath output input)

# set make test directory command and copy command
if ("${CMAKE_GENERATOR}" STREQUAL "MinGW Makefiles")
	ConvertPath(TEST_DIR ${PROJECT_BINARY_DIR}/test)
	set(MAKE_TESTDIR_CMD "if")
	set(MAKE_TESTDIR_OPT "")
	set(MAKE_TESTDIR_ARG "not exist ${TEST_DIR} md ${TEST_DIR}")
	set(COPY_COMMAND "copy")
else ()
	set(MAKE_TESTDIR_CMD "mkdir")
	set(MAKE_TESTDIR_OPT "-p")
	set(MAKE_TESTDIR_ARG "${PROJECT_BINARY_DIR}/test")
	set(COPY_COMMAND "cp")
endif ()

# create custom commands for test files
foreach(file ${other_files})
	ConvertPath(src_file ${PROJECT_SOURCE_DIR}/${file})
	ConvertPath(dst_file ${PROJECT_BINARY_DIR}/${file})
	add_custom_command(OUTPUT ${file}
		COMMAND ${MAKE_TESTDIR_CMD} ${MAKE_TESTDIR_OPT} ${MAKE_TESTDIR_ARG}
		COMMAND ${COPY_COMMAND} ${src_file} ${dst_file}
		MAIN_DEPENDENCY ${file}
	)
endforeach(file ${other_files})

# create binary release archive script appropriate for build system
string(SUBSTRING ${ibcp_RELEASE_STRING} 1 -1 ibcp_VERSION)
if (UNIX)
	add_custom_command(OUTPUT rel_bin.sh
		COMMAND echo tar cvzf ibcp_${ibcp_VERSION}-bin.tar.gz
			$<TARGET_FILE_NAME:ibcp> regtest memtest ibcp.supp ${other_files}
			>rel_bin.sh
		DEPENDS ${other_files}
	)
endif (UNIX)
if (WIN32)
	configure_file(
		"${PROJECT_SOURCE_DIR}/regtest.bat"
		"${PROJECT_BINARY_DIR}/regtest.bat"
	)
	add_custom_command(OUTPUT rel_bin.sh
		COMMAND echo 7z a -tzip -mx9 ibcp_${ibcp_VERSION}-bin.zip
			$<TARGET_FILE_NAME:ibcp> regtest regtest.bat ${other_files}
			>rel_bin.sh
		DEPENDS ${other_files}
	)
endif (WIN32)

# add the executable
add_executable(ibcp
	ibcp.h
	basic/basic.h
	autoenums.h
	test_names.h
	${ibcp_SOURCES}
	${ibcp_MOCS}
	${ibcp_UI_HEADERS}
	${ibcp_RCC_SOURCES}
	${other_files}
	rel_bin.sh
)
target_link_libraries(ibcp ${QT_LIBRARIES})
