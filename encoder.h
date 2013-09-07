// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: encoder.h - encoder class header file
//	Copyright (C) 2013  Thunder422
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
//	2013-09-06	initial version

#ifndef ENCODER_H
#define ENCODER_H

class RpnList;
class Table;

class Encoder
{
	Table &m_table;					// reference to the table instance
	RpnList *m_input;				// translated input list

public:
    Encoder(Table &table);

	bool encode(RpnList *&input);

private:
	int prepareTokens(void);
};

#endif // ENCODER_H
