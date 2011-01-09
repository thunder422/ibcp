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


#### Compiler and tool definitions shared by all build targets #####
CCC = g++
CXX = g++
BASICOPTS = -O -s
CCFLAGS = $(BASICOPTS)
CXXFLAGS = $(BASICOPTS)
CCADMIN =


# Define the target directories.
TARGETDIR_objs=objects


all: test_codes.h codes.txt ibcp.exe

## Target: test_codes.h
test_codes.h: ibcp.h test_codes.awk
	awk -f test_codes.awk <ibcp.h >test_codes.h

## Target: codes.txt
codes.txt: ibcp.h codes.awk
	awk -f codes.awk <ibcp.h >codes.txt

## Target: ibcp.exe
OBJS_ibcp.exe =  \
	$(TARGETDIR_objs)/ibcp.o \
	$(TARGETDIR_objs)/parser.o \
	$(TARGETDIR_objs)/string.o \
	$(TARGETDIR_objs)/table.o \
	$(TARGETDIR_objs)/translator.o \
	$(TARGETDIR_objs)/test_ibcp.o
LDLIBS_ibcp.exe = -static-libgcc


# Link or archive
ibcp.exe: $(TARGETDIR_objs) $(OBJS_ibcp.exe) test_codes.h codes.txt
	$(LINK.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ $(OBJS_ibcp.exe) $(LDLIBS_ibcp.exe)


# Compile source files into .o files
$(TARGETDIR_objs)/ibcp.o: $(TARGETDIR_objs) ibcp.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ ibcp.cpp

$(TARGETDIR_objs)/parser.o: $(TARGETDIR_objs) parser.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ parser.cpp

$(TARGETDIR_objs)/string.o: $(TARGETDIR_objs) string.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ string.cpp

$(TARGETDIR_objs)/table.o: $(TARGETDIR_objs) table.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ table.cpp

$(TARGETDIR_objs)/translator.o: $(TARGETDIR_objs) translator.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ translator.cpp

$(TARGETDIR_objs)/test_ibcp.o: $(TARGETDIR_objs) test_ibcp.cpp
	$(COMPILE.cc) $(CCFLAGS_ibcp.exe) $(CPPFLAGS_ibcp.exe) -o $@ test_ibcp.cpp



#### Clean target deletes all generated files ####
clean:
	rm -f \
		ibcp.exe \
		$(TARGETDIR_objs)/ibcp.o \
		$(TARGETDIR_objs)/parser.o \
		$(TARGETDIR_objs)/string.o \
		$(TARGETDIR_objs)/table.o \
		$(TARGETDIR_objs)/translator.o \
		$(TARGETDIR_objs)/test_ibcp.o
	$(CCADMIN)
	rm -f -r $(TARGETDIR_objs)


# Create the target directory (if needed)
$(TARGETDIR_objs):
	mkdir -p $(TARGETDIR_objs)


# Enable dependency checking
.KEEP_STATE:
.KEEP_STATE_FILE:.make.state.GNU-x86-Windows

