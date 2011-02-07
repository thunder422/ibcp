// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_nums.cpp - contains test code for testing number conversions
//	Copyright (C) 2010  Thunder422
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void print_gpl_header(void)
{
	printf("test_nums.exe  Copyright (C) 2010  Thunder422\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
    printf("This is free software, and you are welcome to\n");
    printf("redistribute it under certain conditions.\n\n");
}

int main(void)
{
	print_gpl_header();

	printf("sizeof(int)=%d sizeof(long)=%d\n", sizeof(int), sizeof(long));
	
	printf("\ntesting integers...\n");
	const char *s = "2147483647";
	char *end;
	int n = strtol(s, &end, 10);
	double g;
	bool bad = errno == ERANGE;
	printf("s=%s n=%d *end=%d bad=%d\n", s, n, *end, bad);
	errno = 0;
	
	s = "2147483648";
	n = strtol(s, &end, 10);
	bad = errno == ERANGE;
	printf("s=%s n=%d *end=%d bad=%d\n", s, n, *end, bad);
	if (errno == ERANGE)
	{
		errno = 0;
		g = strtod(s, &end);
		bad = errno == ERANGE;
		printf("\ts=%s d=%g *end=%d bad=%d\n", s, g, *end, bad);
	}
	errno = 0;
	
	s = "2147483999";
	n = strtol(s, &end, 10);
	bad = errno == ERANGE;
	printf("s=%s n=%d *end=%d bad=%d\n", s, n, *end, bad);
	if (errno == ERANGE)
	{
		errno = 0;
		g = strtod(s, &end);
		bad = errno == ERANGE;
		printf("\ts=%s d=%g *end=%d bad=%d\n", s, g, *end, bad);
	}
	errno = 0;

	printf("\ntesting doubles exponents...\n");
	s = "1e308";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%g *end=%d bad=%d\n", s, g, *end, bad);
	errno = 0;

	s = "9.9999999e308";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%g *end=%d bad=%d\n", s, g, *end, bad);
	errno = 0;

	s = "1e309";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%g *end=%d bad=%d\n", s, g, *end, bad);
	errno = 0;

	printf("\ntesting doubles +exponents...\n");
	s = "1e+308";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%g *end=%d bad=%d\n", s, g, *end, bad);
	errno = 0;

	s = "9.9999999e+308";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%g *end=%d bad=%d\n", s, g, *end, bad);
	errno = 0;

	s = "1e+309";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%g *end=%d bad=%d\n", s, g, *end, bad);
	errno = 0;

	printf("\ntesting doubles -exponents...\n");
	s = "1e-307";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%g *end=%d bad=%d\n", s, g, *end, bad);
	errno = 0;

	s = "9.9999999e-308";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%g *end=%d bad=%d\n", s, g, *end, bad);
	errno = 0;

	s = "1e-308";
	g = strtod(s, &end);
	bad = errno == ERANGE;
	printf("s=%s d=%g *end=%d bad=%d\n", s, g, *end, bad);
	errno = 0;
}
