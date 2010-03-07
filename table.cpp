// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: table.cpp - contains operator/command/function table
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
//  Change History:
//
//  2010-02-18  initial release
//
//  2010-03-01  renamed ImCommand_TokenType to ImmCmd_TokenType
//
//  2010-03-06  changed None_DataType to String_DataType for the internal string
//              functions
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "ibcp.h"


static TableEntry table_entries[] = {
	//*****************************
	//   IMMEDIATE COMMAND FIRST
	//*****************************
	// these match with fewest charaters
	// (these will go away once gui interface is implemented)
	{
		List_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"L", NULL, Blank_Flag | Line_Flag | Range_Flag
	},
	{
		Edit_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"E", NULL, Blank_Flag | Line_Flag
	},
	{
		Delete_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"D", NULL, Line_Flag | Range_Flag
	},
	{
		Run_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"R", NULL, Blank_Flag
	},
	{
		Renum_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"R", NULL, Range_Flag | RangeIncr_Flag
	},
	{
		Save_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"S", NULL, Blank_Flag | String_Flag
	},
	{
		Load_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"L", NULL, String_Flag
	},
	{
		New_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"N", NULL, Blank_Flag
	},
	{
		Auto_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"A", NULL, Blank_Flag | Line_Flag | LineIncr_Flag
	},
	{
		Cont_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"C", NULL, Blank_Flag
	},
	{
		Quit_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"Q", NULL, Blank_Flag
	},
	// end of immediate commands marked by NULL name (next entry)
	//***********************
	//   BEGIN PLAIN WORDS
	//***********************
	{
		BegPlainWord_Code, Error_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL
	},
	//--------------
	//   Commands
	//--------------
	{
		Let_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"LET", NULL, Null_Flag
	},
	{
		Print_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"PRINT", NULL, Null_Flag
	},
	{
		Input_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"INPUT", NULL, Null_Flag
	},
	{
		Dim_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"DIM", NULL, Null_Flag
	},
	{
		Def_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"DEF", NULL, Null_Flag
	},
	{
		Rem_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"REM", NULL, Null_Flag
	},
	{
		If_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"IF", NULL, Null_Flag
	},
	{
		Then_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"THEN", NULL, Null_Flag
	},
	{
		Else_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"ELSE", NULL, Null_Flag
	},
	{
		End_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"END", NULL, Null_Flag
	},
	{
		EndIf_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"END", "IF", Null_Flag
	},
	{
		For_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"FOR", NULL, Null_Flag
	},
	{
		To_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"TO", NULL, Null_Flag
	},
	{
		Step_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"STEP", NULL, Null_Flag
	},
	{
		Next_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"NEXT", NULL, Null_Flag
	},
	{
		Do_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"Do", NULL, Null_Flag
	},
	{
		DoWhile_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"DO", "WHILE", Null_Flag
	},
	{
		DoUntil_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"DO", "UNTIL", Null_Flag
	},
	{
		While_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"WHILE", NULL, Null_Flag
	},
	{
		Until_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"UNTIL", NULL, Null_Flag
	},
	{
		Loop_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"LOOP", NULL, Null_Flag
	},
	{
		LoopWhile_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"LOOP", "WHILE", Null_Flag
	},
	{
		LoopUntil_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"LOOP", "UNTIL", Null_Flag
	},
	//-----------------------------------------
	//   Internal Functions (No Parentheses)
	//-----------------------------------------
	{
		Rnd_Code, IntFunc_TokenType, Double_DataType, OneWord_Multiple,
		"RND", NULL, Null_Flag
	},
	//--------------------
	//   Word Operators
	//--------------------
	{
		Mod_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"MOD", NULL, Null_Flag
	},
	{
		And_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"AND", NULL, Null_Flag
	},
	{
		Or_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"OR", NULL, Null_Flag
	},
	{
		Not_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"NOT", NULL, Null_Flag
	},
	{
		Eqv_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"EQV", NULL, Null_Flag
	},
	{
		Imp_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"IMP", NULL, Null_Flag
	},
	{
		Xor_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"XOR", NULL, Null_Flag
	},
	//*********************
	//   END PLAIN WORDS
	//*********************
	{
		EndPlainWord_Code, Error_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag
	},
	//*****************************
	//   BEGIN PARENTHESES WORDS
	//*****************************
	{
		BegParenWord_Code, Error_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag
	},
	//--------------------------------------
	//   INTERNAL FUNCTIONS (Parentheses)
	//--------------------------------------
	{
		Abs_Code, IntFunc_TokenType, None_DataType, OneWord_Multiple,
		"ABS(", NULL, Null_Flag
	},
	{
		Fix_Code, IntFunc_TokenType, Double_DataType, OneWord_Multiple,
		"FIX(", NULL, Null_Flag
	},
	{
		Int_Code, IntFunc_TokenType, Double_DataType, OneWord_Multiple,
		"INT(", NULL, Null_Flag
	},
	{
		RndArg_Code, IntFunc_TokenType, Double_DataType, OneWord_Multiple,
		"RND(", NULL, Null_Flag
	},
	{
		Sgn_Code, IntFunc_TokenType, None_DataType, OneWord_Multiple,
		"SGN(", NULL, Null_Flag
	},
	{
		Cint_Code, IntFunc_TokenType, Integer_DataType, OneWord_Multiple,
		"CINT(", NULL, Null_Flag
	},
	{
		Sqr_Code, IntFunc_TokenType, Double_DataType, OneWord_Multiple,
		"SQR(", NULL, Null_Flag
	},
	{
		Atn_Code, IntFunc_TokenType, Double_DataType, OneWord_Multiple,
		"ATN(", NULL, Null_Flag
	},
	{
		Cos_Code, IntFunc_TokenType, Double_DataType, OneWord_Multiple,
		"COS(", NULL, Null_Flag
	},
	{
		Sin_Code, IntFunc_TokenType, Double_DataType, OneWord_Multiple,
		"SIN(", NULL, Null_Flag
	},
	{
		Tan_Code, IntFunc_TokenType, Double_DataType, OneWord_Multiple,
		"TAN(", NULL, Null_Flag
	},
	{
		Exp_Code, IntFunc_TokenType, Double_DataType, OneWord_Multiple,
		"EXP(", NULL, Null_Flag
	},
	{
		Log_Code, IntFunc_TokenType, Double_DataType, OneWord_Multiple,
		"LOG(", NULL, Null_Flag
	},
	{
		Tab_Code, IntFunc_TokenType, None_DataType, OneWord_Multiple,
		"TAB(", NULL, Null_Flag
	},
	{
		Spc_Code, IntFunc_TokenType, None_DataType, OneWord_Multiple,
		"SPC(", NULL, Null_Flag
	},
	{
		Asc_Code, IntFunc_TokenType, None_DataType, OneWord_Multiple,
		"ASC(", NULL, Null_Flag
	},
	{
		Chr_Code, IntFunc_TokenType, String_DataType, OneWord_Multiple,
		"CHR$(", NULL, Null_Flag
	},
	{
		Instr_Code, IntFunc_TokenType, None_DataType, OneWord_Multiple,
		"INSTR(", NULL, Null_Flag
	},
	{
		Left_Code, IntFunc_TokenType, String_DataType, OneWord_Multiple,
		"LEFT$(", NULL, Null_Flag
	},
	{
		Len_Code, IntFunc_TokenType, None_DataType, OneWord_Multiple,
		"LEN(", NULL, Null_Flag
	},
	{
		Mid_Code, IntFunc_TokenType, String_DataType, OneWord_Multiple,
		"MID$(", NULL, Null_Flag
	},
	{
		Repeat_Code, IntFunc_TokenType, String_DataType, OneWord_Multiple,
		"REPEAT$(", NULL, Null_Flag
	},
	{
		Right_Code, IntFunc_TokenType, String_DataType, OneWord_Multiple,
		"RIGHT$(", NULL, Null_Flag
	},
	{
		Space_Code, IntFunc_TokenType, String_DataType, OneWord_Multiple,
		"SPACE$(", NULL, Null_Flag
	},
	{
		Str_Code, IntFunc_TokenType, String_DataType, OneWord_Multiple,
		"STR$(", NULL, Null_Flag
	},
	{
		Val_Code, IntFunc_TokenType, None_DataType, OneWord_Multiple,
		"Val(", NULL, Null_Flag
	},
	//***************************
	//   END PARENTHESES WORDS
	//***************************
	{
		EndParenWord_Code, Error_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag
	},
	//***************************
	//   BEGIN DATA TYPE WORDS
	//***************************
	{
		BegDataTypeWord_Code, Error_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag
	},
	// Currently None
	
	//*************************
	//   END DATA TYPE WORDS
	//*************************
	{
		EndDataTypeWord_Code, Error_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag
	},
	//*******************
	//   BEGIN SYMBOLS
	//*******************
	{
		BegSymbol_Code, Error_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag
	},
	//----------------------
	//   Symbol Operators
	//----------------------
	{
		Add_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"+", NULL, Null_Flag
	},
	{
		Sub_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"-", NULL, Null_Flag
	},
	{
		Mul_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"*", NULL, Null_Flag
	},
	{
		Div_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"/", NULL, Null_Flag
	},
	{
		IntDiv_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"\\", NULL, Null_Flag
	},
	{
		Power_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"^", NULL, Null_Flag
	},
	{
		Eq_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"=", NULL, Null_Flag
	},
	{
		Gt_Code, Operator_TokenType, None_DataType, TwoChar_Multiple,
		">", NULL, Null_Flag
	},
	{
		GtEq_Code, Operator_TokenType, None_DataType, TwoChar_Multiple,
		">=", NULL, Null_Flag
	},
	{
		Lt_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"<", NULL, Null_Flag
	},
	{
		LtEq_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"<=", NULL, Null_Flag
	},
	{
		NotEq_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"<>", NULL, Null_Flag
	},
	{
		OpenParen_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"(", NULL, Null_Flag
	},
	{
		CloseParen_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		")", NULL, Null_Flag
	},
	{
		Comma_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		",", NULL, Null_Flag
	},
	{
		SemiColon_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		";", NULL, Null_Flag
	},
	{
		Colon_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		":", NULL, Null_Flag
	},
	{
		RemOp_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"'", NULL, Null_Flag
	},
	//*****************
	//   END SYMBOLS
	//*****************
	{
		EndSymbol_Code, Error_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag
	},
};


// constructor function that initializes the table variables

Table::Table(void)
{
	List<TableError> *error_list = new List<TableError>;
	int i;
	int type;

	entry = table_entries;

	// allocate and initialize code to index conversion array
	index_code = new int[sizeof_Code];
	for (i = 0; i < sizeof_Code; i++)
	{
		index_code[i] = -1;
	}
	// initialize bracketing code range indexes
	for (type = 0; type < sizeof_TableSearch; type++)
	{
		range[type].beg = range[type].end = -1;
	}

	// scan table and record indexes
	bool error = false;
	int nentries = sizeof(table_entries) / sizeof(TableEntry);
	for (i = 0; i < nentries; i++)
	{
		Code code = entry[i].code;
		if (index_code[code] == -1)
		{
			index_code[code] = i;
		}
		else  // already assigned
		{
			// record duplicated code error
			TableError error(code, index_code[code], i);
			error_list->append(&error);
		}
	}

	// check for missing codes
	for (i = 0; i < sizeof_Code; i++)
	{
		if (index_code[i] == -1)
		{
			// record code missing error
			TableError error((Code)i);
			error_list->append(&error);
		}
	}

	// setup indexes for bracketing codes
	// (will be set to -1 if missing - missing errors were recorded above)
	range[PlainWord_TableSearch].beg = index_code[BegPlainWord_Code];
	range[PlainWord_TableSearch].end = index_code[EndPlainWord_Code];
	range[ParenWord_TableSearch].beg = index_code[BegParenWord_Code];
	range[ParenWord_TableSearch].end = index_code[EndParenWord_Code];
	range[DataTypeWord_TableSearch].beg = index_code[BegDataTypeWord_Code];
	range[DataTypeWord_TableSearch].end = index_code[EndDataTypeWord_Code];
	range[Symbol_TableSearch].beg = index_code[BegSymbol_Code];
	range[Symbol_TableSearch].end = index_code[EndSymbol_Code];

	// check for missing bracketing codes and if properly positioned in table
	// (missing codes recorded above, however,
	// need to make sure all types were set, i.e. no missing assignments above)
	for (type = 0; type < sizeof_TableSearch; type++)
	{
		if (range[type].beg == -1 || range[type].end == -1
			|| range[type].beg > range[type].end)
		{
			// record bracket range error
			TableError error((TableSearch)type, range[type].beg,
				range[type].end);
			error_list->append(&error);
		}
		else
		{
			// check to make sure no bracketing codes overlap
			for (int type2 = 0; type2 < sizeof_TableSearch; type2++)
			{
				if (type != type2
					&& range[type2].beg != -1 && range[type2].end != -1
					&& (range[type].beg > range[type2].beg
					&& range[type].beg < range[type2].end
					|| range[type].end > range[type2].beg
					&& range[type].end < range[type2].end))
				{
					// record bracket overlap error
					TableError error((TableSearch)type, (TableSearch)type2,
						range[type].beg, range[type].end);
					error_list->append(&error);
				}
			}
		}
	}

	// throw exception if error_list is not empty
	if (!error_list->empty())
	{
		throw error_list;
	}

	delete error_list;
}


// this search function will look for an immediate command (located at
// the beginning of the table), which are only one letter
//
// the flags argument is for searching for an immediate command with a
// particular form (the letter and the appropriate flag in the table
// needs to match)
//
//     - returns -1 if the letter (and flags) is not found

int Table::search(char letter, int flag)
{
	for (int i = 0; entry[i].name != NULL; i++)
	{
		if (toupper(letter) == entry[i].name[0]
			&& (flag == Null_Flag || flag & entry[i].flags))
		{
			return i;
		}
	}
	return -1;  // not found
}


// this search function will look for a string of a particular type in
// the Table, the search is case insensitive
//
//     - returns the index of the entry that is found
//     - returns -1 if the string was not found in the table

int Table::search(TableSearch type, const char *string, int len)
{
	int i = range[type].beg;
	int end = range[type].end;
	while (++i < end)
	{
		if (strncasecmp(string, entry[i].name, len) == 0
			&& entry[i].name[len] == '\0')
		{
			return i;
		}
	}
	return -1;
}


// this search function will look for a two word command in the Table,
// the search is case insensitive an only entries containing a second
// word is checked
//
//     - returns the index of the entry that is found
//     - returns -1 if the string was not found in the table

int Table::search(const char *word1, int len1, const char *word2, int len2)
{
	for (int i = range[PlainWord_TableSearch].beg;
		i < range[PlainWord_TableSearch].end; i++)
	{
		if (entry[i].name2 != NULL
			&& strncasecmp(word1, entry[i].name, len1) == 0
			&& entry[i].name[len1] == '\0'
			&& strncasecmp(word2, entry[i].name2, len2) == 0
			&& entry[i].name2[len2] == '\0')
		{
			return i;
		}
	}
	return -1;
}


// end: table.cpp
