// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_nums.cpp - contains test code for testing number conversions
//	Copyright (C) 2010-2012  Thunder422
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	For a copy of the GNU General Public License,
//	see <http://www.gnu.org/licenses/>.
// 
//
//	Change History:
//
//	2010-01-13	initial release
//
//	2012-10-16	corrected format specifier for sizeof()
//	2012-10-17	added integer range checks to bad conversion check so that the
//				  range error is consistent for 32-bits and 64-bits
//				modified double output for consistent output of exponent digits
//				  for all platforms via new function unified_double()
//				changed last test from 1e-308 to 1e-324 so that range error
//				  also occurs on Windows 7 platform

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

void print_gpl_header(void)
{
	printf("test_nums.exe  Copyright (C) 2010-2012  Thunder422\n");
	printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	printf("This is free software, and you are welcome to\n");
	printf("redistribute it under certain conditions.\n\n");
}

char *unified_double(double value)
{
	static char bfr[20];

	sprintf(bfr, "%g", value);
	char *exp = strchr(bfr, 'e');
	if (exp != NULL)  // contains an exponent?
	{
		if (exp[4] != '\0' && exp[2] == '0')  // 3 digits and first '0'?
		{
			strcpy(exp + 2, exp + 3);  // move last 2 digits over first
		}
	}
	return bfr;
}

int main(void)
{
	print_gpl_header();

	printf("sizeof(int)=%lu sizeof(long long)=%lu\n", sizeof(int),
		sizeof(long long));
	
	printf("\ntesting integers...\n");
	const char *s = "2147483647";
	char *end;
	long n = strtol(s, &end, 10);
	double g;
	bool bad = errno == ERANGE || n > INT_MAX;
	printf("s=%s n=%ld *end=%d bad=%d\n", s, bad ? -1 : n, *end, bad);
	errno = 0;
	
	s = "2147483648";
	n = strtol(s, &end, 10);
	bad = errno == ERANGE || n > INT_MAX;
	printf("s=%s n=%ld *end=%d bad=%d\n", s, bad ? -1 : n, *end, bad);
	if (bad)
	{
		errno = 0;
		g = strtod(s, &end);
		bad = errno == ERANGE;
		printf("\ts=%s d=%s *end=%d bad=%d\n", s, unified_double(g), *end, bad);
	}
	errno = 0;
	
	s = "2147483999";
	n = strtol(s, &end, 10);
	bad = errno == ERANGE || n > INT_MAX;
	printf("s=%s n=%ld *end=%d bad=%d\n", s, bad ? -1 : n, *end, bad);
	if (bad)
	{
		errno = 0;
		g = strtod(s, &end);
		bad = errno == ERANGE;
		printf("\ts=%s d=%s *end=%d bad=%d\n", s, unified_double(g), *end, bad);
	}
	errno = 0;

	printf("\ntesting doubles exponents...\n");
	s = "1e308";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%s *end=%d bad=%d\n", s, bad ? "--" : unified_double(g),
		*end, bad);
	errno = 0;

	s = "9.9999999e308";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%s *end=%d bad=%d\n", s, bad ? "--" : unified_double(g),
		*end, bad);
	errno = 0;

	s = "1e309";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%s *end=%d bad=%d\n", s, bad ? "--" : unified_double(g),
		*end, bad);
	errno = 0;

	printf("\ntesting doubles +exponents...\n");
	s = "1e+308";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%s *end=%d bad=%d\n", s, bad ? "--" : unified_double(g),
		*end, bad);
	errno = 0;

	s = "9.9999999e+308";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%s *end=%d bad=%d\n", s, bad ? "--" : unified_double(g),
		*end, bad);
	errno = 0;

	s = "1e+309";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%s *end=%d bad=%d\n", s, bad ? "--" : unified_double(g),
		*end, bad);
	errno = 0;

	printf("\ntesting doubles -exponents...\n");
	s = "1e-307";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%s *end=%d bad=%d\n", s, bad ? "--" : unified_double(g),
		*end, bad);
	errno = 0;

	s = "9.9999999e-308";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%s *end=%d bad=%d\n", s, bad ? "--" : unified_double(g),
		*end, bad);
	errno = 0;

	s = "1e-324";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%s *end=%d bad=%d\n", s, bad ? "--" : unified_double(g),
		*end, bad);
	errno = 0;
}
