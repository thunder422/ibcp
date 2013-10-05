// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: encoder.cpp - encoder class source file
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

#include "dictionary.h"
#include "encoder.h"
#include "rpnlist.h"
#include "programmodel.h"
#include "table.h"


Encoder::Encoder(Table &table): m_table(table)
{
}


// function to encode a translated RPN list

ProgramLine Encoder::encode(RpnList *input, ProgramUnit *programUnit)
{
	ProgramLine programLine(input->codeSize());

	for (int i = 0; i < input->count(); i++)
	{
		Token *token = input->at(i)->token();
		programLine[token->index()].setInstruction(token->code(),
			token->subCodes());
		if (m_table.hasFlag(token, HasOperand_Flag))
		{
			quint16 operand;
			EncodeFunction encode = m_table.encodeFunction(token->code());
			if (encode == NULL)
			{
				// TODO for now set set operand to zero
				operand = 0;
			}
			else
			{
				operand = encode(programUnit, token);
			}
			programLine[token->index() + 1].setOperand(operand);
		}
	}
	return programLine;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                       MISCELLANEOUS ENCODE FUNCTIONS                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


quint16 remEncode(ProgramUnit *programUnit, Token *token)
{
	return programUnit->remDictionary()->add(token);
}


// end: encoder.cpp
