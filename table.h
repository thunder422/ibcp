// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: table.h - table class definitions file
//	Copyright (C) 2012  Thunder422
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
//	2012-11-03	initial version (parts removed from ibcp.h)

#ifndef TABLE_H
#define TABLE_H

#include <QStringList>

#include "ibcp.h"
#include "token.h"


// bit definitions for flags field
enum {
	Null_Flag           = 0x00000000,  // entry has no flags
	// table entry flags (each must have unique bit set)
	Multiple_Flag       = 0x00000001,  // function has multiple forms
	Reference_Flag      = 0x00000002,  // code requires a reference
	// note: value 0x00000004 is available
	Hidden_Flag         = 0x00000008,  // code is hidden operator/function
	Print_Flag          = 0x00000010,  // print-only function
	String_Flag         = 0x00000020,  // code has string operands
	EndExpr_Flag        = 0x00000040,  // end expression
	EndStmt_Flag        = 0x00000080   // end statement
};


// 2010-05-03: expression information for operators and internal functions
struct ExprInfo {
	Code unary_code;			// unary code for operator (Null_Code if none)
	short noperands;			// number of operands (operators/int. functions)
	short nassoc_codes;			// number of associated codes
	short assoc2_index;			// index to start of second operand assoc codes
	short nstrings;				// number of string arguments
	DataType *operand_datatype;	// data type of each operand
	Code *assoc_code;			// associated codes

	ExprInfo(Code _unary_code = Null_Code, short _noperands = 0,
		DataType *_operand_datatype = NULL, short _nassoc_codes = 0,
		short _assoc2_index = 0, Code *_assoc_code = NULL)
	{
		unary_code = _unary_code;
		noperands = _noperands;
		operand_datatype = _operand_datatype;
		nassoc_codes = _nassoc_codes;
		assoc2_index = _assoc2_index;
		assoc_code = _assoc_code;
		nstrings = 0;  // default, will be filled in later
	}
};


const int Max_Operands = 3;
	// 2010-04-24: this value contains the maximum number of operands
	// (arguments) for any operator or internal function (there are currently
	// no internal function with more than 3 arguments)

const int Max_Assoc_Codes = 4;
	// 2010-04-24: this value contains the maximum number of associated codes,
	// codes in additional to the main code for different possible data types
	// for the code (no code currently has more the 4 total codes)
	// 2010-05-20: increased from 2 to 3 because of AssignSubStr_Code
	// 2010-07-18: increased from 3 to 4 for second operand associated codes


// categories for searching the table entries
enum SearchType {  // table search types
	PlainWord_SearchType,
	ParenWord_SearchType,
	DataTypeWord_SearchType,
	Symbol_SearchType,
	sizeof_SearchType
};


// multiple word command or multiple character operator type
enum Multiple {
	OneWord_Multiple,
	OneChar_Multiple = OneWord_Multiple,
	TwoWord_Multiple,
	TwoChar_Multiple = TwoWord_Multiple,
	ThreeWord_Multiple,
	ThreeChar_Multiple = ThreeWord_Multiple,
	sizeof_Multiple
};


class Translator;  // forward reference to Translator class
struct CmdItem;

// TODO see if these can be forward referenced pointers
typedef TokenStatus (*TokenHandler)(Translator &t, Token *&token);
typedef TokenStatus (*CmdHandler)(Translator &t, CmdItem *cmd_item,
	Token *token);


struct TableEntry {
	TokenType type;				// type of token for entry
	Multiple multiple;			// multiple word command/character operator
	const char *name;			// name for table entry
	const char *name2;			// name of second word of command
	int flags;					// flags for entry
	int precedence;				// precedence of code
	DataType datatype;			// next expression data type for command
	ExprInfo *exprinfo;			// pointer to expression info (NULL for none)
	TokenHandler token_handler;	// pointer to translator token handler function
	TokenMode token_mode;		// next token mode for command
	CmdHandler cmd_handler;		// pointer to translator cmd handler function
};


// 2011-02-26: removed index_code[], index(), and code(); changed index to code
class Table {
	TableEntry *entry;				// pointer to table entries
	struct Range {
		Code beg;					// begin index of range
		Code end;					// end index of range
	} range[sizeof_SearchType];		// range for each search type
public:
	Table(void) {}
	~Table() {}
	// function to initialize and check the table entries
	QStringList initialize(void);

	// ACCESS FUNCTIONS FOR A CODE
	TokenType type(Code code)
	{
		return entry[code].type;
	}
	DataType datatype(Code code)
	{
		return entry[code].datatype;
	}
	Multiple multiple(Code code)
	{
		return entry[code].multiple;
	}
	const char *name(Code code)
	{
		return entry[code].name;
	}
	const char *name2(Code code)
	{
		return entry[code].name2;
	}
	const char *debug_name(Code code)
	{
		const char *name = entry[code].name2;
		if (name == NULL)
		{
			name = entry[code].name;
		}
		return name;
	}
	int flags(Code code)
	{
		return entry[code].flags;
	}
	TokenMode token_mode(Code code)
	{
		return entry[code].token_mode;
	}
	Code unary_code(Code code)
	{
		ExprInfo *ei = entry[code].exprinfo;
		return ei == NULL ? Null_Code : ei->unary_code;
	}
	int precedence(Code code)
	{
		return entry[code].precedence;
	}
	int noperands(Code code)
	{
		return entry[code].exprinfo->noperands;
	}
	DataType operand_datatype(Code code, int operand)
	{
		return entry[code].exprinfo->operand_datatype[operand];
	}
	int nassoc_codes(Code code)
	{
		return entry[code].exprinfo->nassoc_codes;
	}
	Code assoc_code(Code code, int index)
	{
		return entry[code].exprinfo->assoc_code[index];
	}
	int assoc2_index(Code code)
	{
		return entry[code].exprinfo->assoc2_index;
	}
	Code assoc2_code(Code code, int index = 0)
	{
		return entry[code].exprinfo->assoc_code[assoc2_index(code) + index];
	}
	int nstrings(Code code)
	{
		return entry[code].exprinfo->nstrings;
	}
	// convenience function to avoid confusion
	bool is_unary_operator(Code code)
	{
		return code == unary_code(code);
	}
	TokenHandler token_handler(Code code)
	{
		return entry[code].token_handler;
	}
	CmdHandler cmd_handler(Code code)
	{
		return entry[code].cmd_handler;
	}

	// ACCESS FUNCTIONS FOR A TOKEN
	int precedence(Token *token)
	{
		int prec = token->precedence();
		return prec != -1 ? prec : precedence(token->code);
	}
	int flags(Token *token)
	{
		// (non-table entry token types have no flags)
		return token->table_entry() ? flags(token->code) : Null_Flag;
	}

	// TABLE FUNCTIONS
	Code search(SearchType type, const QStringRef &string);
	Code search(const QStringRef &word1, const QStringRef &word2);
	Code search(Code code, int nargs);
	Code search(Code code, DataType *datatype);
	bool match(Code code, DataType *datatype);

	// function to set a token for a code
	void set_token(Token *token, Code code)
	{
		token->code = code;
		token->type = type(token->code);
		token->datatype = datatype(token->code);
	}
	// function to create new token and initialize it from a code
	Token *new_token(Code code)
	{
		Token *token = new Token;	// allocates and initializes base members
		set_token(token, code);		// initializes code related members
		return token;
	}
};


#endif  // TABLE_H
