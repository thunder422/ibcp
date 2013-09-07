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

#include "encoder.h"
#include "rpnlist.h"
#include "table.h"


Encoder::Encoder(Table &table): m_table(table)
{
}


// function to encode a translated RPN list

bool Encoder::encode(RpnList *&input)
{
	m_input = input;
	int size = prepareTokens();
	if (size < 0)
	{
		return false;
	}
	return true;
}


// function to prepare tokens for encoding
//
//   - assigns codes to token types without codes
//   - assigns position indexes to each token
//   - returns number of program words required for line

int Encoder::prepareTokens(void)
{
	int count = 0;
	for (int i = 0; i < m_input->count(); i++)
	{
		// assign codes to token types without codes
		Token *token = m_input->at(i)->token();
		switch (token->type())
		{
		case Constant_TokenType:
			m_table.findCode(token, Const_Code);
			break;
		case NoParen_TokenType:
			if (token->reference())
			{
				m_table.findCode(token, VarRef_Code);
				token->setReference(false);  // FIXME remove (need for testing)
			}
			else
			{
				m_table.findCode(token, Var_Code);
			}
			break;
		case Command_TokenType:
		case Operator_TokenType:
		case IntFuncN_TokenType:
		case IntFuncP_TokenType:
			break;  // these token types already have a code
		default:
			m_input->setError(token);
			m_input->setErrorMessage(token->message(BUG_NotYetImplemented));
			m_input->clear();
			return -1;
		}

		// assign position index to tokens (increment count for instruction)
		token->setIndex(count++);
		if (m_table.hasFlag(token, HasOperand_Flag))
		{
			count++;  // increment count for operand
		}
	}
	return count;
}


// end: encoder.cpp
