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
//  2010-03-01  support for Parser class added, which includes:
//                Token and CmdArgs structures; Parser class
//                new CmdArgs_DataType and Error_Flag
//                renamed ImCommand_TokenType to ImmCmd_TokenType
//
//  2010-03-06  replaced DefFunc_TokenType with DefFuncN_TokenType and
//                DefFuncP_TokenType so that it can be determined if the define
//                function name has a parentheses or not
//
//  2010-03-07  added length of token with error to Token along with two new
//              set_error() functions
//
//  2010-03-11  support for Translator class initiated, which includes:
//              replaced IntFunc_TokenType With IntFuncN_TokenType and
//              IntFuncP_TokenType; and added new static members to Token
//
//  2010-03-17  added codes Null_Code, Neq_Code, and EOL_Code
//              added unary_code to TableEntry
//              renamed code to index in Token to avoid confusion
//
//  2010-03-18  added Translator class;
//              added default to Token constructor
//              added set_token() function to Table class
//
//  2010-03-20  added precedence to TableEntry
//              added more access functions to Table
//              made numerous changes to Translator class
//
//  2010-03-21  initialized length field in Token to 1
//              added is_unary_operator() to Table
//              added additional diagnostic errors to Translator
//
//  2010-03-25  added variables to Table and Translator to support parentheses,
//              added Highest_Precedence constant
//
//  2010-03-26  added do_pending_paren() to Translator class
//
//  2010-04-02  add support to Token and Table to get precedence values for
//                token types that don't have table entries (DefFuncP and Paren)
//              added count_stack of new type SimpleStack to Translator to
//                support commas (for subscripts in arrays and arguments in
//                functions)
//              added more errors to Translator
//
//  2010-04-04  added support for number of argument checking for internal
//                functions including added codes for functions with different
//                number of arguments; adding Multiple_Flag
//              added nargs to TableEntry; new Table access function and search
//                function
//              added new error to Translator
//              made token argument a reference in Translator::add_token()
//              added debug_name() to Table
//

#ifndef IBCP_H
#define IBCP_H

#include "string.h"
#include "list.h"
#include "stack.h"  // 2010-04-02: added for SimpleStack


enum Code {
	Null_Code,	// 2010-03-17: added to indicated no code

	// math operators
	Add_Code,
	Sub_Code,
	Neg_Code,  // 2010-03-17: added unary operator
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
	Asc2_Code,   // 2010-04-04: added code for two argument form
	Chr_Code,
	// 2010-04-04: replaced Instr_Code
	Instr2_Code,
	Instr3_Code,
	Left_Code,
	Len_Code,
	// 2010-04-04: replaced Mid_Code
	Mid2_Code,
	Mid3_Code,
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

	// other codes
	EOL_Code,  // 2010-03-17: added end-of-line code
	
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


// changes to TokenType may require changes to ibcp.cpp: Token::initialize()
enum TokenType {
	ImmCmd_TokenType,
	Command_TokenType,
	Operator_TokenType,
	IntFuncN_TokenType,  // 2010-03-11: replaces IntFunc_TokenType
	IntFuncP_TokenType,  // 2010-03-11: replaces IntFunc_TokenType
	Remark_TokenType,
	Constant_TokenType,
	DefFuncN_TokenType,  // 2010-03-06: replaces DefFunc_TokenType
	DefFuncP_TokenType,  // 2010-03-06: replaces DefFunc_TokenType
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
	CmdArgs_DataType,	// 2010-02-27: for immediate commands
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


// 2010-03-07: added error length and two new set_error()
struct Token {
	static bool paren[sizeof_TokenType];
	static bool op[sizeof_TokenType];
	static int prec[sizeof_TokenType];  // 2010-04-02

	static void initialize(void);

	int column;				// start column of token
	TokenType type;			// type of the token
	DataType datatype;		// data type of token
	String *string;			// pointer to string of token
	// 2010-03-17: changed variable from code to index
	int index;	 			// index into Table (internal code of token)
	union {
		double dbl_value;	// value for double constant token
		int int_value;		// value for integer constant token
		int length;			// length of token
	};
	// 2010-03-18: added default value to constructor
	Token(int col = -1)
	{
		column = col;
		string = NULL;
		length = 1;  // 2010-03-21: initialize length
	}
	~Token(void)
	{
		if (string != NULL)
		{
			delete string;
		}
	}
	void set_error(const char *msg)
	{
		length = 1;
		type = Error_TokenType;
		datatype = None_DataType;
		string = new String(msg);
	}
	void set_error(int col, const char *msg)
	{
		column = col;
		length = 1;
		type = Error_TokenType;
		datatype = None_DataType;
		string = new String(msg);
	}
	void set_error(const char *msg, int len)
	{
		length = len;
		type = Error_TokenType;
		datatype = None_DataType;
		string = new String(msg);
	}
	void set_error(int col, const char *msg, int len)
	{
		column = col;
		length = len;
		type = Error_TokenType;
		datatype = None_DataType;
		string = new String(msg);
	}
	bool is_operator(void)
	{
		return op[type];
	}
	bool has_paren(void)
	{
		return paren[type];
	}
	// 2010-04-02: new function for arrays/functions in Translator
	int precedence(void)
	{
		return prec[type];
	}
};


// bit definitions for flags field
const int Null_Flag           = 0x00000000;  // entry has no flags
const int Error_Flag          = -1;          // error return code
// immediate command flags (each must have unique bit set)
const int Blank_Flag          = 0x00000001;
const int Line_Flag           = 0x00000002;  // xxx
const int LineIncr_Flag       = 0x00000004;  // xxx,zz
const int Range_Flag          = 0x00000008;  // xxx-yyy
const int RangeIncr_Flag      = 0x00000010;  // xxx-yyy,zz  xxx-yyy,nnn,zz
const int String_Flag         = 0x00000020;
// table entry flags (each must have unique bit set, but not unique from above)
// 2010-04-04: added new flag for table entries with multiple codes
const int Multiple_Flag       = 0x00000001;  // function has multiple forms

// 2010-03-25: added highest precedence value
const int Highest_Precedence = 127;
	// this value was selected as the highest value because it is the highest
	// one-byte signed value (in case the precedence member is changed to an
	// char); all precedences in the table must be below this value

struct TableEntry {
	Code code;					// enumeration code of entry
	TokenType type;				// type of token for entry
	DataType datatype;			// data type of entry (if applicable)
	Multiple multiple;			// multiple word command/character operator
	const char *name;			// name for table entry
	const char *name2;			// name of second word of command
	int flags;					// flags for entry
	// 2010-03-17: added unary code flag
	Code unary_code;			// unary code for operator (Null_Code if none)
	// 2010-03-20: added precedence
	int precedence;				// precedence of code
	// 2010-04-04: added number of arguments
	int nargs;					// number of arguments for internal functions
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
	// 2010-03-20: added more access functions
	const char *name(int index)
	{
		return entry[index].name;
	}
	const char *name2(int index)
	{
		return entry[index].name2;
	}
	// 2010-03-20: added debug name access functions
	const char *debug_name(int index)
	{
		const char *name = entry[index].name2;
		if (name == NULL)
		{
			name = entry[index].name;
		}
		return name;
	}
	int flags(int index)
	{
		return entry[index].flags;
	}
	Code unary_code(int index)
	{
		return entry[index].unary_code;
	}
	int precedence(int index)
	{
		return entry[index].precedence;
	}
	// 2010-04-04: added new access function for nargs
	int nargs(int index)
	{
		return entry[index].nargs;
	}
	// 2010-04-02: added new precedence of token function
	int precedence(Token *token)
	{
		int prec = token->precedence();
		return prec != -1 ? prec : precedence(token->index);
	}
	// 2010-04-02: added convenience function to avoid confusion
	bool is_unary_operator(int index)
	{
		return entry[index].code == entry[index].unary_code;
	}
	int search(char letter, int flag);
	int search(TableSearch type, const char *string, int len);
	int search(const char *word1, int len1, const char *word2, int len2);
	// 2010-04-04: added new search function
	int search(int index, int nargs);
	// 2010-03-18: add function to set token for code
	void set_token(Token *token, Code code)
	{
		token->index = index(code);
		token->type = type(token->index);
		token->datatype = datatype(token->index);
	}
};


struct CmdArgs {
	int begin;				// begin line number (Line, Range)
	int end;				// end line number (Range)
	int start;				// start line number (RangeIncr)
	int incr;				// increment (LineIncr, RangeIncr)

	CmdArgs(void)
	{
		begin = end = start = incr = -1;
	}
};


class Parser {
	Table *table;			// pointer to the table object
	char *input;			// pointer to input line being parsed
	char *pos;				// pointer to current position in input string
	Token *token;			// pointer to working token (to be returned)

	// main functions
	bool get_command(void);
	bool get_identifier(void);
	bool get_number(void);
	bool get_string(void);
	bool get_operator(void);

	// support functions
	void skip_whitespace();
	int scan_command(CmdArgs &args);
	char *scan_word(char *p, DataType &datatype, bool &paren);
	int scan_string(char *&p, String *s);
public:
	Parser(Table *t): table(t) {}
	void start(char *i)
	{
		pos = input = i;
	}
	Token *get_token();
};


// 2010-03-18: added Translator class
// 2010-03-20: renamed members
class Translator {
	Table *table;					// pointer to the table object
	List<Token *> *output;			// pointer to RPN list output
	List<Token *> hold_stack;		// holding stack
	List<List<Token *>::Element *> done_stack;	// tokens processed stack
	enum State {
		Initial,					// initial state
		BinOp,						// expecting binary operator
		Operand,					// expecting unary operator or operand
		sizeof_State
	} state;						// current state of translator
	// 2010-03-25: added variables to support parentheses
	Token *pending_paren;			// closing parentheses token is pending
	int last_precedence;			// precedence of last op added during paren
	// 2010-04-02: added variables to support arrays and functions
	SimpleStack<char> count_stack;	// number of operands counter stack

public:
	enum Status {
		Good,
		Done,
		Error_ExpectedOperand,
		Error_ExpectedOperator,
		Error_ExpectedBinOp,
		Error_MissingOpenParen,		// 2010-03-25: added
		Error_MissingCloseParen,	// 2010-03-25: added
		Error_UnexpectedComma,		// 2010-04-02: added
		Error_WrongNumberOfArgs,	// 2010-04-04: added
		// the following statuses used during development
		BUG_NotYetImplemented,		// somethings is not implemented
		BUG_StackEmpty,				// diagnostic message
		BUG_StackNotEmpty,			// diagnostic message
		BUG_StackNotEmpty2,			// diagnostic message
		BUG_StackEmpty1,			// diagnostic error
		BUG_StackEmpty2,			// diagnostic error
		BUG_StackEmpty3,			// diagnostic error
		BUG_StackEmpty4,			// diagnostic error (2010-03-25)
		BUG_StackEmpty5,			// diagnostic error (2010-04-02)
		BUG_UnexpectedCloseParen,	// diagnostic error (2010-04-02)
		BUG_UnexpectedToken,		// diagnostic error (2010-04-02)
		sizeof_status
	};

	Translator(Table *t): table(t), output(NULL), pending_paren(NULL) {}
	void start(void)
	{
		output = new List<Token *>;
		state = Initial;
	}
	// 2010-04-04: made argument a reference so different value can be returned
	Status add_token(Token *&token);
	List<Token *> *get_result(void)	// only call when add_token returns Done
	{
		List<Token *> *list = output;
		output = NULL;
		return list;
	}
	void clean_up(void);			// only call when add_token returns an error
private:
	Status add_operator(Token *token);
	void do_pending_paren(int index);  // 2010-03-26: added for parentheses
};


#endif  // IBCP_H
