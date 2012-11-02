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
#  2012-10-21	corrected check whether to add static linking, which should have
#				  been greater than 4.4, not less than 4.5
#				replaced add_definitions, which are for compiler options, with
#				  set_target_properties where link flags are set for added the
#				  static linking options
#
#  2012-10-23	streamlined not found check for PROGRAM_AWK
#				renamed variables from VERSION to RELEASE
#				allow negative patch numbers indicating developmental releases
#				added code to get a unique release string using git, and if git
#				  is not available use the three RELEASE variables
#  2012-10-24	detect the CMAKE_BUILD_TYPE environment variable and set the
#				  cmake variable if not already set, then report the build type
#  2012-10-25	added configuration of new memtest script on non-Windows builds
#  2012-10-26	added changed for Qt libraries
#  2012-10-28	removed list.h (Qt lists used) and test_stack program
#  2012-10-29	removed stack.h (Qt stacks used) and test_stack2 program
#  2012-10-31	corrected problem when git is present but not the git repository
#  2012-11-01	removed string.h, string.cpp and test_string
#  2012-11-02	removed the rest of the test programs

cmake_minimum_required(VERSION 2.8)

project(ibcp)

# find Qt package
find_package(Qt4 REQUIRED QtCore)
include(${QT_USE_FILE})

# check GCC versionn
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
	OUTPUT_VARIABLE GCC_VERSION
)
if (GCC_VERSION VERSION_LESS 4.5)
	message(WARNING_ERROR
		"GCC 4.5+ Required for static building with libstdc++"
	)
endif (GCC_VERSION VERSION_LESS 4.5)

# ckeck is a specific build type is set in the environment
if (NOT $ENV{CMAKE_BUILD_TYPE} STREQUAL "")
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE $ENV{CMAKE_BUILD_TYPE})
    endif (NOT CMAKE_BUILD_TYPE)
endif (NOT $ENV{CMAKE_BUILD_TYPE} STREQUAL "")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(ibcp_RELEASE_MAJOR 0)
set(ibcp_RELEASE_MINOR 2)
set(ibcp_RELEASE_PATCH -1)

set(ibcp_COPYRIGHT_YEAR 2012)

find_program (PROGRAM_GIT git)
if (PROGRAM_GIT)
	execute_process(COMMAND ${PROGRAM_GIT} describe
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		OUTPUT_VARIABLE ibcp_RELEASE_STRING
		OUTPUT_STRIP_TRAILING_WHITESPACE
		RESULT_VARIABLE GIT_RESULT
		ERROR_VARIABLE PREVENT_STDERR_OUTPUT
	)
	if (NOT ${GIT_RESULT} EQUAL 0)
		unset(PROGRAM_GIT CACHE)
	endif (NOT ${GIT_RESULT} EQUAL 0)
endif (PROGRAM_GIT)
if (NOT PROGRAM_GIT)
	set(ibcp_RELEASE_STRING release${ibcp_RELEASE_MAJOR}.${ibcp_RELEASE_MINOR})
	if (ibcp_RELEASE_PATCH LESS 0)
		set(ibcp_RELEASE_STRING ${ibcp_RELEASE_STRING}${ibcp_RELEASE_PATCH})
	else (ibcp_RELEASE_PATCH LESS 0)
		set(ibcp_RELEASE_STRING ${ibcp_RELEASE_STRING}.${ibcp_RELEASE_PATCH})
	endif (ibcp_RELEASE_PATCH LESS 0)
endif (NOT PROGRAM_GIT)
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
)

# list the main program sources
set(ibcp_SRCS
	ibcp.cpp
	parser.cpp
	table.cpp
	translator.cpp
	test_ibcp.cpp
)

# add the executable
add_executable(ibcp
	autoenums.h
	test_names.h
	${ibcp_INCS}
	${ibcp_SRCS}
)
target_link_libraries(ibcp ${QT_LIBRARIES})

# make linker link in static libgcc.a and libstrc++.a
if (GCC_VERSION VERSION_GREATER 4.4)
	set_target_properties(ibcp
		PROPERTIES LINK_FLAGS "-static-libgcc -static-libstdc++"
	)
endif (GCC_VERSION VERSION_GREATER 4.4)
