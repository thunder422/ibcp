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
//  2010-03-10  corrected multiple entry for operators <, <=, and <>
//              (was OneChar, changed to TwoChar)
//
//  2010-03-11  split IntFunc_TokenType into IntFuncN_TokenType and
//              IntFuncP_TokenType; changed tables entries accordingly
//
//  2010-03-17  added data for new unary_code field with all set to Null_Code
//                except for Sub_Code and Not_Code
//              added new table entries for Null_Code, Neg_Code and EOL_Code
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
		"L", NULL, Blank_Flag | Line_Flag | Range_Flag, Null_Code
	},
	{
		Edit_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"E", NULL, Blank_Flag | Line_Flag, Null_Code
	},
	{
		Delete_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"D", NULL, Line_Flag | Range_Flag, Null_Code
	},
	{
		Run_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"R", NULL, Blank_Flag, Null_Code
	},
	{
		Renum_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"R", NULL, Range_Flag | RangeIncr_Flag, Null_Code
	},
	{
		Save_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"S", NULL, Blank_Flag | String_Flag, Null_Code
	},
	{
		Load_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"L", NULL, String_Flag, Null_Code
	},
	{
		New_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"N", NULL, Blank_Flag, Null_Code
	},
	{
		Auto_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"A", NULL, Blank_Flag | Line_Flag | LineIncr_Flag, Null_Code
	},
	{
		Cont_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"C", NULL, Blank_Flag, Null_Code
	},
	{
		Quit_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"Q", NULL, Blank_Flag, Null_Code
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
		"LET", NULL, Null_Flag, Null_Code
	},
	{
		Print_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"PRINT", NULL, Null_Flag, Null_Code
	},
	{
		Input_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"INPUT", NULL, Null_Flag, Null_Code
	},
	{
		Dim_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"DIM", NULL, Null_Flag, Null_Code
	},
	{
		Def_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"DEF", NULL, Null_Flag, Null_Code
	},
	{
		Rem_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"REM", NULL, Null_Flag, Null_Code
	},
	{
		If_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"IF", NULL, Null_Flag, Null_Code
	},
	{
		Then_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"THEN", NULL, Null_Flag, Null_Code
	},
	{
		Else_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"ELSE", NULL, Null_Flag, Null_Code
	},
	{
		End_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"END", NULL, Null_Flag, Null_Code
	},
	{
		EndIf_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"END", "IF", Null_Flag, Null_Code
	},
	{
		For_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"FOR", NULL, Null_Flag, Null_Code
	},
	{
		To_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"TO", NULL, Null_Flag, Null_Code
	},
	{
		Step_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"STEP", NULL, Null_Flag, Null_Code
	},
	{
		Next_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"NEXT", NULL, Null_Flag, Null_Code
	},
	{
		Do_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"Do", NULL, Null_Flag, Null_Code
	},
	{
		DoWhile_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"DO", "WHILE", Null_Flag, Null_Code
	},
	{
		DoUntil_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"DO", "UNTIL", Null_Flag, Null_Code
	},
	{
		While_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"WHILE", NULL, Null_Flag, Null_Code
	},
	{
		Until_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"UNTIL", NULL, Null_Flag, Null_Code
	},
	{
		Loop_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"LOOP", NULL, Null_Flag, Null_Code
	},
	{
		LoopWhile_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"LOOP", "WHILE", Null_Flag, Null_Code
	},
	{
		LoopUntil_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"LOOP", "UNTIL", Null_Flag, Null_Code
	},
	//-----------------------------------------
	//   Internal Functions (No Parentheses)
	//-----------------------------------------
	{
		Rnd_Code, IntFuncN_TokenType, Double_DataType, OneWord_Multiple,
		"RND", NULL, Null_Flag, Null_Code
	},
	//--------------------
	//   Word Operators
	//--------------------
	{
		Mod_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"MOD", NULL, Null_Flag, Null_Code
	},
	{
		And_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"AND", NULL, Null_Flag, Null_Code
	},
	{
		Or_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"OR", NULL, Null_Flag, Null_Code
	},
	{
		Not_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"NOT", NULL, Null_Flag, Not_Code
	},
	{
		Eqv_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"EQV", NULL, Null_Flag, Null_Code
	},
	{
		Imp_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"IMP", NULL, Null_Flag, Null_Code
	},
	{
		Xor_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"XOR", NULL, Null_Flag, Null_Code
	},
	//*********************
	//   END PLAIN WORDS
	//*********************
	{
		EndPlainWord_Code, Error_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag, Null_Code
	},
	//*****************************
	//   BEGIN PARENTHESES WORDS
	//*****************************
	{
		BegParenWord_Code, Error_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag, Null_Code
	},
	//--------------------------------------
	//   INTERNAL FUNCTIONS (Parentheses)
	//--------------------------------------
	{
		Abs_Code, IntFuncP_TokenType, None_DataType, OneWord_Multiple,
		"ABS(", NULL, Null_Flag, Null_Code
	},
	{
		Fix_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"FIX(", NULL, Null_Flag, Null_Code
	},
	{
		Int_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"INT(", NULL, Null_Flag, Null_Code
	},
	{
		RndArg_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"RND(", NULL, Null_Flag, Null_Code
	},
	{
		Sgn_Code, IntFuncP_TokenType, None_DataType, OneWord_Multiple,
		"SGN(", NULL, Null_Flag, Null_Code
	},
	{
		Cint_Code, IntFuncP_TokenType, Integer_DataType, OneWord_Multiple,
		"CINT(", NULL, Null_Flag, Null_Code
	},
	{
		Sqr_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"SQR(", NULL, Null_Flag, Null_Code
	},
	{
		Atn_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"ATN(", NULL, Null_Flag, Null_Code
	},
	{
		Cos_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"COS(", NULL, Null_Flag, Null_Code
	},
	{
		Sin_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"SIN(", NULL, Null_Flag, Null_Code
	},
	{
		Tan_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"TAN(", NULL, Null_Flag, Null_Code
	},
	{
		Exp_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"EXP(", NULL, Null_Flag, Null_Code
	},
	{
		Log_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"LOG(", NULL, Null_Flag, Null_Code
	},
	{
		Tab_Code, IntFuncP_TokenType, None_DataType, OneWord_Multiple,
		"TAB(", NULL, Null_Flag, Null_Code
	},
	{
		Spc_Code, IntFuncP_TokenType, None_DataType, OneWord_Multiple,
		"SPC(", NULL, Null_Flag, Null_Code
	},
	{
		Asc_Code, IntFuncP_TokenType, None_DataType, OneWord_Multiple,
		"ASC(", NULL, Null_Flag, Null_Code
	},
	{
		Chr_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"CHR$(", NULL, Null_Flag, Null_Code
	},
	{
		Instr_Code, IntFuncP_TokenType, None_DataType, OneWord_Multiple,
		"INSTR(", NULL, Null_Flag, Null_Code
	},
	{
		Left_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"LEFT$(", NULL, Null_Flag, Null_Code
	},
	{
		Len_Code, IntFuncP_TokenType, None_DataType, OneWord_Multiple,
		"LEN(", NULL, Null_Flag, Null_Code
	},
	{
		Mid_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"MID$(", NULL, Null_Flag, Null_Code
	},
	{
		Repeat_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"REPEAT$(", NULL, Null_Flag, Null_Code
	},
	{
		Right_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"RIGHT$(", NULL, Null_Flag, Null_Code
	},
	{
		Space_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"SPACE$(", NULL, Null_Flag, Null_Code
	},
	{
		Str_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"STR$(", NULL, Null_Flag, Null_Code
	},
	{
		Val_Code, IntFuncP_TokenType, None_DataType, OneWord_Multiple,
		"Val(", NULL, Null_Flag, Null_Code
	},
	//***************************
	//   END PARENTHESES WORDS
	//***************************
	{
		EndParenWord_Code, Error_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag, Null_Code
	},
	//***************************
	//   BEGIN DATA TYPE WORDS
	//***************************
	{
		BegDataTypeWord_Code, Error_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag, Null_Code
	},
	// Currently None
	
	//*************************
	//   END DATA TYPE WORDS
	//*************************
	{
		EndDataTypeWord_Code, Error_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag, Null_Code
	},
	//*******************
	//   BEGIN SYMBOLS
	//*******************
	{
		BegSymbol_Code, Error_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag, Null_Code
	},
	//----------------------
	//   Symbol Operators
	//----------------------
	{
		Add_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"+", NULL, Null_Flag, Null_Code
	},
	{
		Sub_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"-", NULL, Null_Flag, Neg_Code
	},
	{
		Mul_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"*", NULL, Null_Flag, Null_Code
	},
	{
		Div_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"/", NULL, Null_Flag, Null_Code
	},
	{
		IntDiv_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"\\", NULL, Null_Flag, Null_Code
	},
	{
		Power_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"^", NULL, Null_Flag, Null_Code
	},
	{
		Eq_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"=", NULL, Null_Flag, Null_Code
	},
	{
		Gt_Code, Operator_TokenType, None_DataType, TwoChar_Multiple,
		">", NULL, Null_Flag, Null_Code
	},
	{
		GtEq_Code, Operator_TokenType, None_DataType, TwoChar_Multiple,
		">=", NULL, Null_Flag, Null_Code
	},
	{
		Lt_Code, Operator_TokenType, None_DataType, TwoChar_Multiple,
		"<", NULL, Null_Flag, Null_Code
	},
	{
		LtEq_Code, Operator_TokenType, None_DataType, TwoChar_Multiple,
		"<=", NULL, Null_Flag, Null_Code
	},
	{
		NotEq_Code, Operator_TokenType, None_DataType, TwoChar_Multiple,
		"<>", NULL, Null_Flag, Null_Code
	},
	{
		OpenParen_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"(", NULL, Null_Flag, Null_Code
	},
	{
		CloseParen_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		")", NULL, Null_Flag, Null_Code
	},
	{
		Comma_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		",", NULL, Null_Flag, Null_Code
	},
	{
		SemiColon_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		";", NULL, Null_Flag, Null_Code
	},
	{
		Colon_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		":", NULL, Null_Flag, Null_Code
	},
	{
		RemOp_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"'", NULL, Null_Flag, Null_Code
	},
	//*****************
	//   END SYMBOLS
	//*****************
	{
		EndSymbol_Code, Error_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag, Null_Code
	},
	//***************************
	//   MISCELLANEOUS ENTRIES
	//***************************
	// 2010-03-16: added entries for new codes
	{
		Null_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag, Null_Code
	},
	{
		Neg_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"-", NULL, Null_Flag, Null_Code
	},
	{
		EOL_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag, Null_Code
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
