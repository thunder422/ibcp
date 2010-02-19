// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: ibcp.h - main definitions file
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

#ifndef IBCP_H
#define IBCP_H

#include "string.h"
#include "list.h"


enum Code {
	// math operators
	Add_Code,
	Sub_Code,
	Mul_Code,
	Div_Code,
	IntDiv_Code,
	Mod_Code,
	Power_Code,

	// relational operators
	Eq_Code,
	Gt_Code,
	GtEq_Code,
	Lt_Code,
	LtEq_Code,
	NotEq_Code,

	// logical operators
	And_Code,
	Or_Code,
	Not_Code,
	Eqv_Code,
	Imp_Code,
	Xor_Code,

	// math internal functions
	Abs_Code,
	Fix_Code,
	Int_Code,
	Rnd_Code,
	RndArg_Code,
	Sgn_Code,
	Cint_Code,

	// scientific math internal function
	Sqr_Code,
	Atn_Code,
	Cos_Code,
	Sin_Code,
	Tan_Code,
	Exp_Code,
	Log_Code,

	// string functions
	Asc_Code,
	Chr_Code,
	Instr_Code,
	Left_Code,
	Len_Code,
	Mid_Code,
	Repeat_Code,
	Right_Code,
	Space_Code,
	Str_Code,
	Val_Code,

	// other symbol operators
	OpenParen_Code,
	CloseParen_Code,
	Comma_Code,
	SemiColon_Code,
	Colon_Code,
	RemOp_Code,

	// print internal functions
	Tab_Code,
	Spc_Code,

	// commands
	Let_Code,
	Print_Code,
	Input_Code,
	Dim_Code,
	Def_Code,
	Rem_Code,
	If_Code,
	Then_Code,
	Else_Code,
	EndIf_Code,
	For_Code,
	To_Code,
	Step_Code,
	Next_Code,
	Do_Code,
	DoWhile_Code,
	DoUntil_Code,
	While_Code,
	Until_Code,
	Loop_Code,
	LoopWhile_Code,
	LoopUntil_Code,
	End_Code,
	
	// immediate commands (will go away once gui implemented)
	List_Code,
	Edit_Code,
	Delete_Code,
	Run_Code,
	Renum_Code,
	Save_Code,
	Load_Code,
	New_Code,
	Auto_Code,
	Cont_Code,
	Quit_Code,

	// search bracketing codes
	BegPlainWord_Code,
	EndPlainWord_Code,
	BegParenWord_Code,
	EndParenWord_Code,
	BegDataTypeWord_Code,
	EndDataTypeWord_Code,
	BegSymbol_Code,
	EndSymbol_Code,
	sizeof_Code
};


enum TokenType {
	ImCommand_TokenType,
	Command_TokenType,
	Operator_TokenType,
	IntFunc_TokenType,
	Remark_TokenType,
	Constant_TokenType,
	DefFunc_TokenType,
	NoParen_TokenType,
	Paren_TokenType,
	Error_TokenType,
	sizeof_TokenType
};


enum DataType {
	None_DataType,
	Double_DataType,
	Integer_DataType,
	String_DataType,
	sizeof_DataType
};


enum Multiple {
	OneWord_Multiple,
	OneChar_Multiple = OneWord_Multiple,
	TwoWord_Multiple,
	TwoChar_Multiple = TwoWord_Multiple,
	ThreeWord_Multiple,
	ThreeChar_Multiple = ThreeWord_Multiple,
	sizeof_Multiple
};


// bit definitions for flags field
const int Null_Flag         = 0x00;  // entry has no flags
// immediate command flags (each must have unique bit set)
const int Blank_Flag          = 0x00000001;
const int Line_Flag           = 0x00000002;  // xxx
const int LineIncr_Flag       = 0x00000004;  // xxx,zz
const int Range_Flag          = 0x00000008;  // xxx-yyy
const int RangeIncr_Flag      = 0x00000010;  // xxx-yyy,zz  xxx-yyy,nnn,zz
const int String_Flag         = 0x00000020;


struct TableEntry {
	Code code;					// enumeration code of entry
	TokenType type;				// type of token for entry
	DataType datatype;			// data type of entry (if applicable)
	Multiple multiple;			// multiple word command/character operator
	const char *name;			// name for table entry
	const char *name2;			// name of second word of command
	int flags;					// flags for entry
};


enum TableSearch {  // table search types
	PlainWord_TableSearch,
	ParenWord_TableSearch,
	DataTypeWord_TableSearch,
	Symbol_TableSearch,
	sizeof_TableSearch
};


enum TableErrType {
	Unset_TableErrType,
	Duplicate_TableErrType,
	Missing_TableErrType,
	Range_TableErrType,
	Overlap_TableErrType,
	sizeof_TableErrType
};

struct TableError {
	TableErrType type;			// type of the error
	union {
		struct {
			Code code;			// code with duplicate
			int ifirst;			// index first found
			int idup;			// index of duplicate
		} duplicate;
		struct {
			Code code;			// missing code
		} missing;
		struct {
			TableSearch type;	// search type that is incomplete
			int ibeg;			// index of beginning bracket code
			int iend;			// index of ending bracket code
		} range;
		struct {
			TableSearch type1;	// first search type of overlap
			TableSearch type2;	// second search type of overlap
			int ibeg;			// index of beginning bracket code
			int iend;			// index of ending bracket code
		} overlap;
	};

	TableError(Code code, int ifirst, int idup)
	{
		type = Duplicate_TableErrType;
		duplicate.code = code;
		duplicate.ifirst = ifirst;
		duplicate.idup = idup;
	}
	TableError(Code code)
	{
		type = Missing_TableErrType;
		missing.code = code;
	}
	TableError(TableSearch searchtype, int ibeg, int iend)
	{
		type = Range_TableErrType;
		range.type = searchtype;
		range.ibeg = ibeg;
		range.iend = iend;
	}
	TableError(TableSearch type1, TableSearch type2, int ibeg, int iend)
	{
		type = Overlap_TableErrType;
		overlap.type1 = type1;
		overlap.type2 = type2;
		overlap.ibeg = ibeg;
		overlap.iend = iend;
	}
	TableError(void)			// default constructor
	{
		type = Unset_TableErrType;
	}
};


class Table {
	TableEntry *entry;				// pointer to table entries
	int *index_code;				// returns index from code
	struct Range {
		int beg;					// begin index of range
		int end;					// end index of range
	} range[sizeof_TableSearch];	// range for each search type
public:
	Table(void);
	~Table()
	{
		delete index_code;
	}
	int index(Code code)
	{
		return index_code[code];
	}
	Code code(int index)
	{
		return entry[index].code;
	}
	TokenType type(int index)
	{
		return entry[index].type;
	}
	DataType datatype(int index)
	{
		return entry[index].datatype;
	}
	Multiple multiple(int index)
	{
		return entry[index].multiple;
	}
	int search(char letter, int flags);
	int search(TableSearch type, const char *string, int len);
	int search(const char *word1, int len1, const char *word2, int len2);
};


#endif  // IBCP_H
