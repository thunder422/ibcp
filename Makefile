## -*- Makefile -*-
##
## User: thunder422
## Time: Jan 9, 2011 9:10:10 AM
## Makefile created by Oracle Solaris Studio.
##
## This file was initially generated automatically, then simplified.
##
##  Change History:
##
##  2011-01-09	initial release
##  2011-02-06	cleaned up make file
##  2011-02-08	corrected mistake on linker line with objects list
##  2011-02-26	awk scripts modified to read and write files directly
##		awk generates codes.h from table.cpp
##  2011-02-27	changed main program codes.h and test_codes.h dependencies to be
##		  listed first


#### Compiler and tool definitions shared by all build targets #####
CCC = g++
CXX = g++
BASICOPTS = -O -s
CCFLAGS = $(BASICOPTS)
CXXFLAGS = $(BASICOPTS)
CCADMIN =


# Define the target directories.
IBCPOBJS_dir=objects
TESTOBJS_dir=objects/test


all: codes.h test_codes.h ibcp.exe

## Target: codes.h (also generates codes.txt)
codes.h: table.cpp codes.awk
	awk -f codes.awk

## Target: test_codes.h
test_codes.h: table.cpp codes.h test_codes.awk
	awk -f test_codes.awk

## Target: ibcp.exe
IBCP_incs = \
	ibcp.h \
	list.h \
	stack.h \
	string.h

IBCP_objs =  \
	$(IBCPOBJS_dir)/ibcp.o \
	$(IBCPOBJS_dir)/parser.o \
	$(IBCPOBJS_dir)/string.o \
	$(IBCPOBJS_dir)/table.o \
	$(IBCPOBJS_dir)/translator.o \
	$(IBCPOBJS_dir)/test_ibcp.o
LDLIBS_ibcp.exe = -static-libgcc


# Link or archive
ibcp.exe: codes.h test_codes.h $(IBCP_incs) $(IBCP_objs)
	$(LINK.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ $(IBCP_objs) \
		$(LDLIBS_ibcp.exe)


# Compile source files into .o files
$(IBCPOBJS_dir)/ibcp.o: $(IBCPOBJS_dir) ibcp.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ ibcp.cpp

$(IBCPOBJS_dir)/parser.o: $(IBCPOBJS_dir) parser.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ parser.cpp

$(IBCPOBJS_dir)/string.o: $(IBCPOBJS_dir) string.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ string.cpp

$(IBCPOBJS_dir)/table.o: $(IBCPOBJS_dir) table.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ table.cpp

$(IBCPOBJS_dir)/translator.o: $(IBCPOBJS_dir) translator.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ translator.cpp

$(IBCPOBJS_dir)/test_ibcp.o: $(IBCPOBJS_dir) test_ibcp.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ test_ibcp.cpp


#### Clean target deletes all generated files ####
clean: cleantests
	rm -f \
		ibcp.exe \
		$(IBCP_objs)
	$(CCADMIN)
	rm -f -r $(IBCPOBJS_dir)


# Create the target directory (if needed)
$(IBCPOBJS_dir):
	mkdir -p $(IBCPOBJS_dir)

###########
## TESTS ##
###########

tests: test_stack test_nums test_string test_cons test_stack2

# test_stack
test_stack: test/test_stack.exe

test/test_stack.exe: $(TESTOBJS_dir)/test_stack.o
	$(LINK.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ $(TESTOBJS_dir)/test_stack.o

$(TESTOBJS_dir)/test_stack.o: $(TESTOBJS_dir) list.h test/test_stack.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ test/test_stack.cpp

# test_nums
test_nums: test/test_nums.exe

test/test_nums.exe: $(TESTOBJS_dir)/test_nums.o
	$(LINK.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ $(TESTOBJS_dir)/test_nums.o

$(TESTOBJS_dir)/test_nums.o: $(TESTOBJS_dir) test/test_nums.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ test/test_nums.cpp

# test_string
test_string: test/test_string.exe

test/test_string.exe: $(TESTOBJS_dir)/test_string.o $(IBCPOBJS_dir)/string.o
	$(LINK.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ $(IBCPOBJS_dir)/string.o \
		$(TESTOBJS_dir)/test_string.o

$(TESTOBJS_dir)/test_string.o: $(TESTOBJS_dir) string.h test/test_string.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ test/test_string.cpp

# test_cons
test_cons: test/test_cons.exe

test/test_cons.exe: $(TESTOBJS_dir)/test_cons.o
	$(LINK.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ $(TESTOBJS_dir)/test_cons.o

$(TESTOBJS_dir)/test_cons.o: $(TESTOBJS_dir) test/test_cons.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ test/test_cons.cpp

# test_stack2
test_stack2: test/test_stack2.exe

test/test_stack2.exe: $(TESTOBJS_dir)/test_stack2.o
	$(LINK.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ $(TESTOBJS_dir)/test_stack2.o

$(TESTOBJS_dir)/test_stack2.o: $(TESTOBJS_dir) stack.h test/test_stack2.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ test/test_stack2.cpp


#### Clean target deletes all generated files ####
cleantests:
	rm -f \
		test/test_stack.exe \
		test/test_nums.exe \
		test/test_string.exe \
		test/test_cons.exe \
		test/test_stack2.exe \
		$(TESTOBJS_dir)/test_stack.o \
		$(TESTOBJS_dir)/test_nums.o \
		$(TESTOBJS_dir)/test_string.o \
		$(TESTOBJS_dir)/test_cons.o \
		$(TESTOBJS_dir)/test_stack2.o \
	$(CCADMIN)
	rm -f -r $(TESTOBJS_dir)


# Create the target directory (if needed)
$(TESTOBJS_dir): $(IBCPOBJS_dir)
	mkdir -p $(TESTOBJS_dir)


# Enable dependency checking
.KEEP_STATE:
.KEEP_STATE_FILE:.make.state.GNU-x86-Windows

