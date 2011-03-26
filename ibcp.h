// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: ibcp.h - main definitions file
//	Copyright (C) 2010-2011  Thunder422
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
//	2010-02-18	initial release
//
//	2010-03-01	support for Parser class added, which includes:
//				  Token and CmdArgs structures; Parser class
//				  new CmdArgs_DataType and Error_Flag
//				  renamed ImCommand_TokenType to ImmCmd_TokenType
//
//	2010-03-06	replaced DefFunc_TokenType with DefFuncN_TokenType and
//				  DefFuncP_TokenType so that it can be determined if the define
//				  function name has a parentheses or not
//
//	2010-03-07	added length of token with error to Token along with two new
//				set_error() functions
//
//	2010-03-11	support for Translator class initiated, which includes:
//				replaced IntFunc_TokenType With IntFuncN_TokenType and
//				IntFuncP_TokenType; and added new static members to Token
//
//	2010-03-17	added codes Null_Code, Neq_Code, and EOL_Code
//				added unary_code to TableEntry
//				renamed code to index in Token to avoid confusion
//
//	2010-03-18	added Translator class;
//				added default to Token constructor
//				added set_token() function to Table class
//
//	2010-03-20	added precedence to TableEntry
//				added more access functions to Table
//				made numerous changes to Translator class
//
//	2010-03-21	initialized length field in Token to 1
//				added is_unary_operator() to Table
//				added additional diagnostic errors to Translator
//
//	2010-03-25	added variables to Table and Translator to support parentheses,
//				added Highest_Precedence constant
//
//	2010-03-26	added do_pending_paren() to Translator class
//
//	2010-04-02	add support to Token and Table to get precedence values for
//				  token types that don't have table entries (DefFuncP and Paren)
//				added count_stack of new type SimpleStack to Translator to
//				  support commas (for subscripts in arrays and arguments in
//				  functions)
//				added more errors to Translator
//
//	2010-04-04	added support for number of argument checking for internal
//				  functions including added codes for functions with different
//				  number of arguments; adding Multiple_Flag
//				added nargs to TableEntry; new Table access function and search
//				  function
//				added new error to Translator
//				made token argument a reference in Translator::add_token()
//				added debug_name() to Table
//
//	2010-04-11	added assignment operator codes
//				added mode with enumerator to Translator
//				added more error codes to Translator
//	2010-04-12	added reference flag to Token
//	2010-04-13	made token argument a reference in Translator::add_operator()
//	2010-04-16	added more error codes to Translator
//				added expression mode flag option to Translator for testing
//	2010-04-17	added another error code to Translator
//
//	2010-04-24	added the associated codes for the specific data types codes for
//				  all the operators and internal functions where applicable
//				add new Frac_Code
//				rearranged DataType enumeration names so that the actual data
//				  types are listed at the beginning with Double (the default)
//				  listed first; added number_DataType for actual number of data
//				  types
//				renamed nargs to noperands in TableEntry and Table (now
//				  applicable operators in addition to internal functions)
//				added Max_Operands and Max_Assoc_Codes definitions; added
//				  operand_datatype[] and assoc_codes[] arrays to TableEntry;
//				  added related access functions to Table
//	2010-04-25	added Invalid_Code set to -1
//				updated Translator for data type handling:
//				  added exptected data type errors
//				  renamed BUG_StackEmpty to BUG_HoldStackEmpty
//				  added BUG_DoneStackEmpty
//				  added set_default_datatype() to set the data type for
//				    operands that didn't have a specific data type
//				  added find_code() and match_code() with Match enum
//
//	2010-05-03	added ExprInfo to hold expression related information for the
//				table entries; replaced these members with ExprInto pointer in
//				TableEntry; modified the access functions in Table
//	2010-05-05	added Reference_Flag and AssignList_Flag
//	2010-05-07	added associated codes for Assign and AssignList
//	2010-05-08	added check for null exprinfo in datatype() and unary_code()
//				  access functions of Table
//				added last_operand argument to Table::find_code()
//
//	2010-05-15	added TmpStr_DataType
//				added RpnItem structure, changed done_stack and output list from
//				  holding Token* to RpnItem*
//				moved operand[] array from find_code() to Translator as member,
//				  removed operand argument from Translator::match_code()
//				added logic to Translator::set_default_datatype() to set data
//				  type to TmpStr for token types DefFuncN/DefFuncP when the data
//				  type is String
//
//	2010-05-19	added new SubStr_DataType
//				added new associated code AssignSubStr_Code to Assign_Code
//	2010-05-20	increased Max_Assoc_Codes from 2 to 3 because of
//				  AssignSubStr_Code
//				added new maximum table errors
//
//	2010-05-20	added new associated code AssignListMixStr_Code to
//				  AssignList_Code
//				added RpnItem::set() to set rpn item members without allocating
//				  a new array and copying to the array
//
//	2010-05-27	added SubCode enumeration and subcode member to Token
//	2010-05-28	changed hold_stack and done_stack from List to SimpleStack in
//				  Translator (a linked list is unnecessary for these stacks)
//				moved/renamed Translator::Status to TokenStatus
//				added CmdItem struct and cmd_stack to Translator
//				moved/renamed Translator::Mode to TokenMode along with
//				  token_mode to TableEntry (next token mode for command) and
//				  access function to Table
//				added TokenHandler typedef along with token_handler to
//				  TableEntry and access function to Table, added friend
//				  definitions for new translator token handler functions and
//				  extern definitions for actual functions
//	2010-05-29	added static token has table entry flag (table) to Token along
//				  with access function (used by Table access functions)
//				added Hidden_Flag to mark table entry of hidden codes
//				added new flags Table access function with token pointer
//				  argument - returns no flags (0) for non-table entrytokens
//				changed argument to Translator::do_pending_paren() from index
//				  to token pointer so that it will work with tokens that don't
//				  have a table entry (arrays and functions)
//
//	2010-06-01	added support for print-only functions
//				added generic flag member to CmdItem that can be used by
//				  commands (for PRINT it will be used to determine if a PRINT
//				  token should be added at the end of a translated print
//				  statement, i.e. advance to a new line)
//	2010-06-02	added support for semicolon with token handler
//				added Table::new_token() for allocating a new token and setting
//				  it up for a specific code
//	2010-06-03	added Translator::expression_end() for checking if expression
//				  was ended correctly
//	2010-06-04	added Translator::add_print_code() for adding new data type
//				  specific print codes that were also added
//	2010-06-05	added support for command handler function pointers
//				  (was necessary to move CmdItem structure outside Translator)
//	2010-06-06	added EndExpr_Flag for token codes that can end an expression
//				corrected issue with special Translator expression only mode
//	2010-06-08	added PrintFunc_CmdFlag and SemiColon_SubCode
//	2010-06-09	changed count_stack from <char> to new <CountItem>, which
//				  contains the old count and expected number of arguments
//	2010-06-10	added new FirstOperand translator state - used for identifying
//				  if any tokens for expression have been received yet
//	2010-06-13	added token pointer argument to command handlers so that
//				  command handlers have accessed to calling token
//	2010-06-14	added Translator::paren_status() for checking if there is an
//				  outstanding token with parentheses on the hold stack and if
//				  there is, to return the appropriate error
//	2010-06-14	removed AssignList_Flag because it is unnecessary (the code can
//				  be checked for AssignList_Code)
//	2010-06-01/14 added, renamed and deleted many TokenStatus enumeration
//				   values for matching error messages that were changed for
//				   better clarity
//
//	2010-06-24	added another TokenStatus
//	2010-06-25	replaced TableErrType enum and TableError struct with
//				  ErrorType enum and Error struct template so that this can also
//				  be used for TokenStatus
//				TableSearch renamed to SearchType
//	2010-06-26	added EndStatement_Flag
//	2010-06-29	added new ExprType enumeration
//				added expr_type member to Translator (initialize for expr mode)
//				added expr_type member to TableEntry (with access function)
//				updated expected expression related token statuses
//	2010-06-30	updated expected expression related token statuses
//	2010-07-01	added AssignList_CmdFlag back
//				removed last_operand argument from Table::find_code(), which was
//				  needed for assign list operators, but assign list is no
//				  handled by comma, semicolor and Assign handlers
//				added new check_assignlist_token() and set_assign_command()
//				  to Translator
//	2010-07-02	updated TokenStatus
//				added new search and match functions to Table
//	2010-07-04	updated TokenStatus
//
//	2010-08-01	removed support for multiple equal assignments (now only commas
//				  used for multiple assignments)
//	2010-07-18	added second associated index to ExprInfo for support of
//				  checking each operand when processed instead of processing
//				  all operands at the end
//	2010-08-10	added several multiple error types used for additional table
//				  checking during initialization
//	2010-09-03	added new Translator::process_final_operand(),
//				  modified Translator::find_code(),
//				  removed Translator::match_code()
//	2010-12-13	added number of string arguments to ExprInfo for Table entries
//
//	2011-01-02	added Translator::get_expr_datatype()
//	2011-01-05	put datatype back into TableEntry, removed expr_type
//				modified datatype access function to get main datatype if there
//				  is not exprinfo (this may be temporary)
//	2011-01-07	removed datatype from ExprInfo
//
//	2011-01-11	moved length from union in Token so it can be used for all types
//	2011-01-13	implemented new Translator::HoldStackItem and changed hold_stack
//				added argument to Translator:: add_operator()
//				  and process_final_operand() for handling first/last operands
//	2011-01-15	implemented new Translator::DoneStackItem and changed done_stack
//				added argument to Translator::find_code() for handling
//				  first/last operands
//				implemented new Translator:: process_first_operand() and
//				  delete_close_paren()
//	2011-01-16	added Token::include() for reporting range of tokens for errors
//	2011-01-22	added last and used sub-code flags for close paren tokens so
//				  they don't get deleted until they are not used anymore
//				added Translator::delete_open_paren()
//	2011-01-29	added list element pointer to Token for memory leak detection
//				added new and delete operator Token functions for memory leaks
//				added static list and del_list to Token for memory leaks
//				moved all static members of Token to the end so that it is
//				  easier to see the regular members when debugging
//	2011-01-30	reset string pointer to NULL in Token deconstructor
//				made Translator::delete_open_paren() public to be accessible
//				  by caller of Translator (to correct a memory leak)
//	2011-02-04	added associated codes for temporary strings
//
//	2011-02-10	added Translator::call_command_handler()
//				added BUG_CmdStackEmptyCmd token status
//	2011-02-11	added BUG_UnexpToken token status
//	2011-02-12	renamed ExpStatement_TokenStatus to ExpCommand
//	2011-02-20	renamed SimpleStack to Stack
//
//	2011-02-26	removed Code enumeration (now auto gen'd in codes.h)
//				changed Token::index back to code
//				moved Table::index_code[], code() and index()
//				removed TableEntry::code (index now same as code)
//				added new ImmCmd search type
//				added Code operator overload functions
//	2011-03-01	started INPUT command handler implementation
//				added Question_SubCode
//	2011-03-04	added non-IntFunc check to Translator::set_default_datatype()
//	2011-03-05	removed PrintOnlyIntFunc, added ExpSemiCommaOrEnd_TokenStatus
//				added "_State" to enum Translator::State values
//				added Translator::EndExpr_State
//	2011-03-10	renamed ExpCommand_TokenStatus to ExpCmd_TokenStatus
//	2011-03-13	removed ExpAssignRef_TokenStatus, UnExpCommand_TokenStatus,
//				  UnexpAssignComma_TokenStatus, UnexpParenInComma_TokenStatus
//	2011-03-19	renamed EndStatement_Flag to EndStmt_Flag
//				added Keep_SubCode (for INPUT), ExpEndStmt_TokenStatus
//				renamed EndStatment_Flag to EndStmt_Flag
//				added Translator::EndStmt_State
//				added ExpEndStmt_TokenStatus
//				implemented Translator::assoc2_code() access function
//	2011-03-20	added End_SubCode (for INPUT), added input command flags
//	2011-03-22	renamed Translator::FirstOperand_State to OperandOrEnd_State
//	2011-03-24	added ExpOpSemiOrComma_TokenStatus
//

#ifndef IBCP_H
#define IBCP_H

#include "string.h"
#include "list.h"
#include "stack.h"  // 2010-04-02: added for Stack

// 2011-02-26: replaced code enumeration with include
#include "codes.h"

// 2011-02-26: Code operator functions
inline Code operator +(Code code, int number)
{
	return (Code)((int)code + number);
}

inline Code operator ++(Code &code)
{
	return code = code + 1;
}

inline Code operator ++(Code &code, int postfix)
{
	return code = code + 1;
}


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
	// the actual execution data types must be listed first
	// since they will also be used for indexes
	Double_DataType,
	Integer_DataType,
	String_DataType,
	TmpStr_DataType,	// 2010-05-15: temporary string data type
	SubStr_DataType,	// 2010-05-19: sub string data type
	// the end of the actual execution data types
	numberof_DataType,	// 2010-04-24: new entry for number of actual data types
	// the following data types are used internally for other uses
	None_DataType,		// 2010-04-24: moved name after new numberof_DataType
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


enum ExprType {  // 2010-06-29
	Null_ExprType,
	Any_ExprType,
	Num_ExprType,
	Str_ExprType,
	sizeof_ExprType
};


// 2010-05-27: sub-code flags for use in Token and internal program
// 2010-06-08: renamed Null to None and added SemiColon sub-code flag
// 2010-08-01: removed Comma_SubCode
// 2011-01-22: added Used and Last sub-codes for parentheses token flags
// 2011-03-01: added Question sub-code for INPUT PROMPT command
// 2011-03-19: added Keep sub-code for INPUT commands
// 2011-03-20: added End sub-code for INPUT parse codes
const int None_SubCode       = 0x00000000;	// no sub-code present
const int Paren_SubCode      = 0x00000001;	// reproduce unnecessary parenthesis
const int Let_SubCode        = 0x00000002;	// reproduce LET keyword for assign
const int Keep_SubCode       = 0x00000004;	// keep cursor on same line of input
const int SemiColon_SubCode  = 0x00000008;	// semicolon after print function
const int Question_SubCode   = 0x00000010;	// add "? " to input prompt
const int Used_SubCode       = 0x00000020;	// parentheses used in output
const int Last_SubCode       = 0x00000040;	// parentheses used as last token
const int End_SubCode        = 0x00000080;	// end of INPUT parsing codes


// 2010-06-25: moved max constants
const int Max_Operands = 3;
	// 2010-04-24: this value contains the maximum number of operands
	// (arguments) for any operator or internal function (there are currently
	// no internal function with more than 3 arguments)

const int Max_Assoc_Codes = 4;
	// 2010-04-24: this value contains the maximum number of associated codes,
	// codes in additional to the main code for different possible data types
	// for the code (no code currently has more the 3 total codes)
	// 2010-05-20: increased from 2 to 3 because of AssignSubStr_Code
	// 2010-07-18: increased from 3 to 4 for second operand associated codes


//*****************************************************************************
//**                             ERROR TEMPLATE                              **
//*****************************************************************************

enum ErrorType {  // 2010-06-25: renamed from TableErrType
	Unset_ErrorType,
	Duplicate_ErrorType,
	Missing_ErrorType,
	Range_ErrorType,
	Overlap_ErrorType,
	MaxOperands_ErrorType,		// 2010-05-20
	MaxAssocCodes_ErrorType,	// 2010-05-20
	Assoc2Code_ErrorType,		// 2010-07-18
	MultName_ErrorType,			// 2010-08-10
	MultExprInfo_ErrorType,		// 2010-08-10
	MultNOperands_ErrorType,	// 2010-08-10
	sizeof_ErrorType
};

// 2010-06-25: moved and renamed from TableSearch
enum SearchType {  // table search types
	ImmCmd_SearchType,			// 2011-02-26
	PlainWord_SearchType,
	ParenWord_SearchType,
	DataTypeWord_SearchType,
	Symbol_SearchType,
	sizeof_SearchType
};

// print call function for Error<T>::report()
typedef void (*PrintFunction)(const char *fmt, ...);

// generic error structure template
// 2010-08-10: changed ints to shorts, added multiple error types
template <class T> struct Error {  // 2010-06-25: created from TableError
	ErrorType type;			// type of the error
	union {
		struct {
			T item;				// item with duplicate
			short ifirst;		// index first found
			short idup;			// index of duplicate
		} duplicate;
		struct {
			T item;				// missing item
		} missing;
		struct {
			SearchType type;	// search type that is incomplete
			short ibeg;			// index of beginning bracket item
			short iend;			// index of ending bracket item
		} range;
		struct {
			SearchType type1;	// first search type of overlap
			SearchType type2;	// second search type of overlap
			short ibeg;			// index of beginning bracket item
			short iend;			// index of ending bracket item
		} overlap;
		struct {
			int found;			// actual maximum found
		} maximum;
		struct {
			short entry;		// index of entry with error
			short code;			// assoc 2 code
			short n;			// number of assoc codes
		} assoc2;
		struct {
			const char *name;		// name of entry
			union {
				const char *name2;	// mis-match name of next entry
				struct {
					short entry;	// number of operands of entry
					short next;		// number of operands of next entry
				} noperands;
			};
		} multiple;
	};

	Error(T item, int ifirst, int idup)
	{
		type = Duplicate_ErrorType;
		duplicate.item = item;
		duplicate.ifirst = ifirst;
		duplicate.idup = idup;
	}
	Error(T item)
	{
		type = Missing_ErrorType;
		missing.item = item;
	}
	Error(SearchType searchtype, int ibeg, int iend)
	{
		type = Range_ErrorType;
		range.type = searchtype;
		range.ibeg = ibeg;
		range.iend = iend;
	}
	Error(SearchType type1, SearchType type2, int ibeg, int iend)
	{
		type = Overlap_ErrorType;
		overlap.type1 = type1;
		overlap.type2 = type2;
		overlap.ibeg = ibeg;
		overlap.iend = iend;
	}
	// 2010-05-20: added new error
	Error(ErrorType _type, int max)
	{
		type = _type;
		maximum.found = max;
	}
	// 2010-07-18: added new error
	Error(short code, short code2, short nassoc)
	{
		type = Assoc2Code_ErrorType;
		assoc2.entry = code;
		assoc2.code = code2;
		assoc2.n = nassoc;
	}
	// 2010-08-10: added new error
	Error(const char *name, const char *name2)
	{
		type = MultName_ErrorType;
		multiple.name = name;
		multiple.name2 = name2;
	}
	Error(const char *name)
	{
		type = MultExprInfo_ErrorType;
		multiple.name = name;
	}
	Error(const char *name, short noperands, short noperands2)
	{
		type = MultNOperands_ErrorType;
		multiple.name = name;
		multiple.noperands.entry = noperands;
		multiple.noperands.next = noperands2;
	}
	Error(void)			// default constructor
	{
		type = Unset_ErrorType;
	}

	// 2010-06-25: function from parts of ibcp.cpp:main()
	static void report(List<Error<T> > *error_list, PrintFunction print,
		const char *title, const char *item)
	{
		Error<T> error;

		(*print)("Error(s) found in %s:\n", title);
		int n = 0;
		bool more;
		do
		{
			more = error_list->remove(NULL, &error);
			(*print)("Error #%d: ", ++n);
			switch (error.type)
			{
			case Duplicate_ErrorType:
				(*print)("%s %d in table more than once at entries %d and "
					"%d\n", item, error.duplicate.item,
					error.duplicate.ifirst, error.duplicate.idup);
				break;
			case Missing_ErrorType:
				(*print)("%s %d missing from table\n", item,
					error.missing.item);
				break;
			case Range_ErrorType:
				(*print)("Search type %d indexes (%d, %d) not correct\n",
					error.range.type, error.range.ibeg, error.range.iend);
				break;
			case Overlap_ErrorType:
				(*print)("Search type %d indexes (%d, %d) overlap with search "
					"type %d\n", error.overlap.type1, error.overlap.ibeg,
					error.overlap.iend, error.overlap.type2);
				break;
			// 2010-05-20: added new maximum errors
			case MaxOperands_ErrorType:
				(*print)("Max_Operands=%d too small, actual is %d\n",
					Max_Operands, error.maximum.found);
				break;
			case MaxAssocCodes_ErrorType:
				(*print)("Max_Assoc_Codes=%d too small, actual is %d\n",
					Max_Assoc_Codes, error.maximum.found);
				break;
			// 2010-07-18: added new assoc 2 code bad error
			case Assoc2Code_ErrorType:
				(*print)("Entry:%d Assoc2Code=%d too large, maximum is %d\n",
					error.assoc2.entry, error.assoc2.code, error.assoc2.n);
				break;
			// 2010-08-10: added new multiple flag errors
			case MultName_ErrorType:
				(*print)("Multiple entry '%s' name mis-match '%s'\n",
					error.multiple.name, error.multiple.name2);
				break;
			case MultExprInfo_ErrorType:
				(*print)("Multiple entry '%s' next entry no expression info\n",
					error.multiple.name);
				break;
			case MultNOperands_ErrorType:
				(*print)("Multiple entry '%s' incorrect number of operands "
					"(%d, %d)\n", error.multiple.name,
					error.multiple.noperands.entry,
					error.multiple.noperands.next);
				break;
			default:
				(*print)("Unknown error %d\n", error.type);
				break;
			}
		}
		while (more);
	}
};


//*****************************************************************************
//**                                  TOKEN                                  **
//*****************************************************************************

// 2010-05-28: moved from Translator::Status, and renamed the values
// 2010-06-01/14: added, renamed or deleted many status enumeration values
// 2010-06-25: moved to begin of TOKEN section
enum TokenStatus {
	Null_TokenStatus,				// 2010-06-04: added
	Good_TokenStatus,
	Done_TokenStatus,
	ExpCmd_TokenStatus,				// 2011-01-12: renamed again
	ExpExpr_TokenStatus,			// 2010-06-10: renamed
	ExpExprOrEnd_TokenStatus,		// 2010-06-10: added
	ExpOpOrEnd_TokenStatus,			// 2010-06-11: renamed
	ExpBinOpOrEnd_TokenStatus,		// 2010-06-12: renamed
	ExpOpOrComma_TokenStatus,		// 2010-06-11: added
	ExpOpCommaOrParen_TokenStatus,	// 2010-06-11: added
	NoOpenParen_TokenStatus,		// 2010-03-25: added
	ExpOpOrParen_TokenStatus,		// 2010-03-25: added (renamed 2010-06-11)
	// 2010-04-11: replaced Error_UnexpectedComma
	ExpEqualOrComma_TokenStatus,	// 2010-04-11: added
	ExpComma_TokenStatus,			// 2010-07-04: added
	ExpAssignItem_TokenStatus,		// 2010-06-13: added
	ExpDouble_TokenStatus,			// 2010-04-25: added
	ExpInteger_TokenStatus,			// 2010-04-25: added
	ExpString_TokenStatus,			// 2010-04-25: added
	ExpNumExpr_TokenStatus,			// 2010-06-29: added
	ExpStrExpr_TokenStatus,			// 2010-06-29: added
	ExpSemiCommaOrEnd_TokenStatus,	// 2011-03-05: added
	ExpSemiOrComma_TokenStatus,		// 2011-03-06: added
	ExpOpSemiOrComma_TokenStatus,	// 2011-03-24: added
	ExpDblVar_TokenStatus,			// 2010-06-30: added
	ExpIntVar_TokenStatus,			// 2010-06-30: added
	ExpStrVar_TokenStatus,			// 2010-06-24: added
	ExpVar_TokenStatus,				// 2011-03-06: added
	ExpStrItem_TokenStatus,			// 2010-07-04: added
	ExpEndStmt_TokenStatus,			// 2011-03-19: added
	// the following statuses used during development
	BUG_NotYetImplemented,			// somethings is not implemented
	BUG_InvalidMode,				// added 2010-06-13
	BUG_HoldStackEmpty,				// diagnostic message
	BUG_HoldStackNotEmpty,			// diagnostic message
	BUG_DoneStackNotEmpty,			// diagnostic message
	BUG_DoneStackEmptyParen,		// diagnostic error (2010-03-25)
	BUG_DoneStackEmptyOperands,		// diagnostic error (2010-04-02)
	BUG_DoneStackEmptyOperands2,	// diagnostic error (2010-10-11)
	BUG_DoneStackEmptyFindCode,		// diagnostic error (2010-07-01)
	BUG_UnexpectedCloseParen,		// diagnostic error (2010-04-02)
	BUG_UnexpectedToken,			// diagnostic error (2010-04-02)
	BUG_DoneStackEmpty,				// diagnostic error (2010-04-25)
	BUG_CmdStackNotEmpty,			// diagnostic error (2010-05-30)
	BUG_CmdStackEmpty,				// diagnostic error (2010-05-30)
	BUG_NoAssignListCode,			// diagnostic error (2010-07-02)
	BUG_InvalidDataType,			// diagnostic error (2010-07-02)
	BUG_CmdStackEmptyExpr,			// diagnostic error (2011-01-02)
	BUG_CmdStackEmptyCmd,			// diagnostic error (2011-02-10)
	BUG_CountStackEmpty,			// diagnostic error (2011-01-02)
	BUG_UnexpParenExpr,				// diagnostic error (2011-01-02)
	BUG_UnexpToken,					// diagnostic error (2011-02-11)
	BUG_Debug,						// diagnostic error (2010-06-13)
	BUG_Debug1,						// diagnostic error (2010-06-13)
	BUG_Debug2,						// diagnostic error (2010-06-13)
	BUG_Debug3,						// diagnostic error (2010-06-13)
	BUG_Debug4,						// diagnostic error (2010-06-13)
	BUG_Debug5,						// diagnostic error (2010-06-13)
	BUG_Debug6,						// diagnostic error (2010-06-13)
	BUG_Debug7,						// diagnostic error (2010-06-13)
	BUG_Debug8,						// diagnostic error (2010-06-13)
	BUG_Debug9,						// diagnostic error (2010-06-13)
	sizeof_TokenStatus
};

struct TokenStsMsg {  // 2010-06-25
	TokenStatus status;		// status code
	const char *string;		// associate message
};

// 2010-06-25: TokenStsMsgErr/Type replaced with Error template


// 2010-03-07: added error length and two new set_error()
struct Token {
	List<Token *>::Element *element;	// pointer to list element (2011-01-29)
	int column;				// start column of token
	int length;				// length of token (2011-01-11: moved from union)
	TokenType type;			// type of the token
	DataType datatype;		// data type of token
	String *string;			// pointer to string of token
	// 2010-03-17: changed variable from code to index
	// 2011-02-26: changed variable back from index to code
	Code code;	 			// internal code of token (index of TableEntry)
	// 2010-04-12: added reference flag
	bool reference;			// token is a reference flag
	// 2010-05-27: added sub-code
	int subcode;			// sub-code flags of token
	union {
		double dbl_value;	// value for double constant token
		int int_value;		// value for integer constant token
	};
	// 2010-03-18: added default value to constructor
	Token(int col = -1)
	{
		column = col;
		string = NULL;
		length = 1;  // 2010-03-21: initialize length
		reference = false;  // 2010-04-12: initialize reference flag
		subcode = None_SubCode;  // 2010-05-29: initial sub-code flags
	}
	~Token(void)
	{
		if (string != NULL)
		{
			delete string;
			string = NULL;  // in case of double delete (2011-01-30)
		}
	}
	// overload new and delete operators for leak detection (2011-01-29)
	void *operator new(size_t size);
	void operator delete(void *ptr);
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
	int table_entry(void)
	{
		return table[type];
	}
	// 2011-01-16: new function to include range through to a second token
	Token *through(Token *token2)
	{
		length = token2->column - column + token2->length;
		return this;
	}

	// static members (moved 2011-01-29)
	static bool paren[sizeof_TokenType];
	static bool op[sizeof_TokenType];
	static int prec[sizeof_TokenType];  // 2010-04-02
	static bool table[sizeof_TokenType];  // 2010-05-29
	static TokenStsMsg message_array[sizeof_TokenStatus];  // 2010-06-25
	static int index_status[sizeof_TokenStatus];  // 2010-06-25
    static List<Token *> list;  // 2011-01-29
    static List<Token> del_list;  // 2011-01-29

	static void initialize(void);
	// 2010-06-25: new function to get message for status
	static const char *message(TokenStatus status)
	{
		return message_array[index_status[status]].string;
	}
};


// 2010-05-28: moved outside Translator, renamed enumeration and values
// 2010-08-01: removed EqualAssignment, renamed CommaAssignment
enum TokenMode {
	Null_TokenMode,					// no token mode set flag (2010-05-29)
	Command_TokenMode,				// expecting command
	Assignment_TokenMode,			// expecting assignment
	AssignmentList_TokenMode,		// comma separated assignment started
	Expression_TokenMode,			// inside expression
	Reference_TokenMode,			// reference expected (2011-03-07)
	sizeof_TokenMode
};


//*****************************************************************************
//**                               TABLE ENTRY                               **
//*****************************************************************************

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
// 2010-05-05: added Reference and AssignList flag for assignment operators
// 2010-05-29: added hidden operator/function flag
// 2010-06-01: added Print flag for print-only functions
// 2010-06-06: added EndExpr flag for comma, semicolon and EOL functions
// 2010-06-14: removed AssignList flag (not needed)
// 2010-06-26: added EndStatement flag
const int Multiple_Flag       = 0x00000001;  // function has multiple forms
const int Reference_Flag      = 0x00000002;  // code requires a reference
// note: value 0x00000004 is available
const int Hidden_Flag         = 0x00000008;  // code is hidden operator/function
const int Print_Flag          = 0x00000010;  // print-only function
// note: don't use 0x00000020 - String_Flag is being used for codes
const int EndExpr_Flag        = 0x00000040;	 // end expression
const int EndStmt_Flag        = 0x00000080;  // end statement

// 2010-03-25: added highest precedence value
const int Highest_Precedence = 127;
	// this value was selected as the highest value because it is the highest
	// one-byte signed value (in case the precedence member is changed to an
	// char); all precedences in the table must be below this value

// 2010-06-25: moved max constants to above Error template

// 2010-05-03: expression information for operators and internal functions
struct ExprInfo {
	// 2010-05-03: members moved from TableEntry
	// 2011-01-07: moved datatype back to TableEntry
	// 2010-03-17: added unary code flag
	Code unary_code;			// unary code for operator (Null_Code if none)
	// 2010-04-04: added number of arguments
	// 2010-04-24: renamed nargs to number of operands
	short noperands;			// number of operands (operators/int. functions)
	// 2010-05-03: added number of associated codes
	short nassoc_codes;			// number of associated codes
	// 2010-07-18: added index to second operand associated codes
	short assoc2_index;			// index to start of second operand assoc codes
	// 2010-12-23: added number of string arguments
	short nstrings;				// number of string arguments
	// 2010-04-24: added operand data type array
	// 2010-05-03: changed from array to pointer
	DataType *operand_datatype;	// data type of each operand
	// 2010-04-24: added associated codes array
	// 2010-05-03: changed from array to pointer
	Code *assoc_code;			// associated codes

	// 2011-01-07: removed datatype
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
		nstrings = 0;  // 2010-12-23: default, will be filled in later
	}
};


class Translator;  // 2010-05-28: forward reference to Translator class

typedef TokenStatus (*TokenHandler)(Translator &t, Token *&token);


// 2010-06-02: added command stack item flag values
// some flags are used for all commands and some are used only for
// specific commands, the values of command specific flags may be reused
// for different commands so each flag will be assigned a value
// 2011-03-20: added input command flags

// FLAGS FOR ALL COMMANDS
const int None_CmdFlag			= 0x00000000;	// initial value of command flag

// FLAGS FOR PRINT COMMAND
const int PrintStay_CmdFlag		= 0x00010000;	// PRINT stay on line flag
const int PrintFunc_CmdFlag		= 0x00020000;	// print func flag (2010-06-08)

// FLAGS FOR ASSIGNMENT COMMANDS
const int AssignList_CmdFlag	= 0x00010000;	// currently an assign list

// FLAGS FOR INPUT COMMAND
const int InputBegin_CmdFlag	= 0x00010000;	// beginning processed
const int InputStay_CmdFlag		= 0x00020000;	// INPUT stay on line flag


// 2010-05-28: added command item and command stack
// 2010-06-05: moved CmdItem outside Translator for TableEntry
// 2011-03-01: removed code, added element
struct RpnItem;
struct CmdItem {
	Token *token;				// pointer to command token
	int flag;					// 2010-06-01: generic flag for command use
	List<RpnItem *>::Element *element;	// 2011-03-01: pointer for command user
};

// 2010-06-13: added token pointer argument to command handlers
typedef TokenStatus (*CmdHandler)(Translator &t, CmdItem *cmd_item,
	Token *token);


struct TableEntry {
	TokenType type;				// type of token for entry
	Multiple multiple;			// multiple word command/character operator
	const char *name;			// name for table entry
	const char *name2;			// name of second word of command
	int flags;					// flags for entry
	// 2010-03-20: added precedence
	int precedence;				// precedence of code
	// 2011-01-05: added expected expression data type for commands
	DataType datatype;			// next expression data type for command
	// 2010-05-03: replace members with expression information pointer
	ExprInfo *exprinfo;			// pointer to expression info (NULL for none)
	// 2010-05-28: added variables to support commands in translator
	TokenHandler token_handler;	// pointer to translator token handler function
	TokenMode token_mode;		// next token mode for command
	// 2010-06-05: added end-of-statement token handler for commands
	CmdHandler cmd_handler;		// pointer to translator cmd handler function
};


// 2010-06-25: TableErrType/TableError replaced with Error template


//*****************************************************************************
//**                                  TABLE                                  **
//*****************************************************************************

// 2011-02-26: removed index_code[], index(), and code(); changed index to code
class Table {
	TableEntry *entry;				// pointer to table entries
	struct Range {
		Code beg;					// begin index of range
		Code end;					// end index of range
	} range[sizeof_SearchType];	// range for each search type
public:
	Table(void);
	~Table() {}

	// ACCESS FUNCTIONS
	TokenType type(Code code)
	{
		return entry[code].type;
	}
	DataType datatype(Code code)
	{
		// 2010-05-03: get value from expression information structure
		// 2010-05-08: added check for null exprinfo pointer
		// 2010-01-05: return main data type if no expression info
		// 2011-01-07: removed expression info
		return entry[code].datatype;
	}
	Multiple multiple(Code code)
	{
		return entry[code].multiple;
	}
	// 2010-03-20: added more access functions
	const char *name(Code code)
	{
		return entry[code].name;
	}
	const char *name2(Code code)
	{
		return entry[code].name2;
	}
	// 2010-03-20: added debug name access functions
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
	// 2010-05-29: added token mode access function
	TokenMode token_mode(Code code)
	{
		return entry[code].token_mode;
	}
	Code unary_code(Code code)
	{
		// 2010-05-03: get value from expression information structure
		// 2010-05-08: added check for null exprinfo pointer
		ExprInfo *ei = entry[code].exprinfo;
		return ei == NULL ? Null_Code : ei->unary_code;
	}
	int precedence(Code code)
	{
		return entry[code].precedence;
	}
	// 2010-04-04: added new access function for nargs
	// 2010-04-23: renamed from nargs
	int noperands(Code code)
	{
		// 2010-05-03: get value from expression information structure
		return entry[code].exprinfo->noperands;
	}
	// 2010-04-23: added two new access functions
	DataType operand_datatype(Code code, int operand)
	{
		// 2010-05-03: get value from expression information structure
		return entry[code].exprinfo->operand_datatype[operand];
	}
	// 2010-05-08: added new access function for nassoc_codes
	int nassoc_codes(Code code)
	{
		return entry[code].exprinfo->nassoc_codes;
	}
	Code assoc_code(Code code, int index)
	{
		// 2010-05-03: get value from expression information structure
		return entry[code].exprinfo->assoc_code[index];
	}
	// 2010-08-07: added new access function for assoc2_index
	int assoc2_index(Code code)
	{
		return entry[code].exprinfo->assoc2_index;
	}
	// 2011-03-19: new function to get an associate 2 code
	Code assoc2_code(Code code, int index = 0)
	{
		// 2010-05-03: get value from expression information structure
		return entry[code].exprinfo->assoc_code[assoc2_index(code) + index];
	}
	// 2010-12-23: added new access function for nstrings
	int nstrings(Code code)
	{
		return entry[code].exprinfo->nstrings;
	}
	// 2010-04-02: added new precedence of token function
	int precedence(Token *token)
	{
		int prec = token->precedence();
		return prec != -1 ? prec : precedence(token->code);
	}
	// 2010-05-29: added new flags of token function
	int flags(Token *token)
	{
		// (non-table entry token types have no flags)
		return token->table_entry() ? flags(token->code) : 0;
	}
	// 2010-04-02: added convenience function to avoid confusion
	bool is_unary_operator(Code code)
	{
		return code == unary_code(code);
	}
	// 2010-05-28: added token handler function pointer access function
	TokenHandler token_handler(Code code)
	{
		return entry[code].token_handler;
	}
	// 2010-06-05: added command handler function pointer access function
	CmdHandler cmd_handler(Code code)
	{
		return entry[code].cmd_handler;
	}

	// TABLE FUNCTIONS
	Code search(char letter, int flag);
	Code search(SearchType type, const char *string, int len);
	Code search(const char *word1, int len1, const char *word2, int len2);
	// 2010-04-04: added new search function
	Code search(Code code, int nargs);
	// 2010-07-02: added new search and match functions
	Code search(Code code, DataType *datatype);
	bool match(Code code, DataType *datatype);
	// 2010-03-18: add function to set token for code
	void set_token(Token *token, Code code)
	{
		token->code = code;
		token->type = type(token->code);
		token->datatype = datatype(token->code);
	}
	// 2010-06-02: create new token and initialize it from code
	Token *new_token(Code code)
	{
		Token *token = new Token;	// allocates and initializes base members
		set_token(token, code);		// initializes code related members
		return token;
	}
};


//*****************************************************************************
//**                                 PARSER                                  **
//*****************************************************************************

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


//*****************************************************************************
//**                               TRANSLATOR                                **
//*****************************************************************************

// 2010-05-15: add structure for holding RPN output list information
struct RpnItem {
	Token *token;							// pointer to token
	int noperands;							// number of operands
	List<RpnItem *>::Element **operand;		// array operand pointers

	RpnItem(Token *_token, int _noperands = 0,
		List<RpnItem *>::Element **_operand = NULL)
	{
		token = _token;
		noperands = _noperands;
		// 2010-07-18: removed allocation of operand array (now set from arg)
		operand = _operand;
	}
	~RpnItem()
	{
		delete token;
		if (noperands > 0)
		{
			delete[] operand;
		}
	}

	// 2010-05-22: function to set operands without allocating a new array
	void set(int _noperands, List<RpnItem *>::Element **_operand)
	{
		noperands = _noperands;
		operand = _operand;
	}
};

// 2010-03-18: added Translator class
// 2010-03-20: renamed members
// 2010-05-15: changed output list and done stack from Token* to RpnItem*
class Translator {
	Table *table;					// pointer to the table object
	List<RpnItem *> *output;		// pointer to RPN list output
	// 2010-05-28: changed hold_stack and done_stack from List to Stack
	// 2011-01-13: changed hold stack to include first token pointer
	struct HoldStackItem {
		Token *token;				// token pointer on hold stack
		Token *first;				// operator token's first operand pointer
	};
	Stack<HoldStackItem> hold_stack;// operator/function holding stack
	// 2011-01-15: changed done stack to include first and last token pointers
	struct DoneStackItem {
		List<RpnItem *>::Element *element;	// RPN item element pointer
		Token *first;				// operator token's first operand pointer
		Token *last;				// operator token's last operand pointer
	};
	Stack<DoneStackItem> done_stack;// items processed stack
	enum State {
		Initial_State,				// initial state
		BinOp_State,				// expecting binary operator
		Operand_State,				// expecting unary operator or operand
		OperandOrEnd_State,			// expecting operand or end (2010-06-10)
		EndExpr_State,				// expecting end of expression (2011-03-05)
        EndStmt_State,				// expecting end of statement (2011-03-19)
		sizeof_State
	} state;						// current state of translator
	// 2010-03-25: added variables to support parentheses
	Token *pending_paren;			// closing parentheses token is pending
	int last_precedence;			// precedence of last op added during paren
	// 2010-04-02: added variables to support arrays and functions
	// 2010-06-09: change count stack to also hold expected number of operands
	// 2010-02-26: changed int index to Code code
	struct CountItem {
		char noperands;				// number of operands seen
		char nexpected;				// number of arguments expected
		Code code;					// table index of internal function
	};
	Stack<CountItem> count_stack;	// number of operands counter stack
	// 2010-04-11: added mode for handling assignment statements
	// 2010-05-29: moved enum definition outside Translator and renamed it
	TokenMode mode;					// current assignment mode
	// 2010-06-05: moved CmdItem outside Translator for TableEntry
	Stack<CmdItem> cmd_stack;		// stack of commands waiting processing
	// 2010-06-06: variable to save expression only mode in
	bool exprmode;					// expression only mode active flag

public:
	Translator(Table *t): table(t), output(NULL), pending_paren(NULL) {}
	// 2010-04-16: added expression mode flag for testing
	void start(bool _exprmode = false)
	{
		exprmode = _exprmode;  // 2010-06-06: save flag
		output = new List<RpnItem *>;
		state = Initial_State;
		// 2010-04-11: initialize mode to command
		// 2010-04-16: start in expression mode for testing
		mode = exprmode ? Expression_TokenMode : Command_TokenMode;
		// 2010-06-29: if expression mode then set any expression type
	}
	// 2010-04-04: made argument a reference so different value can be returned
	TokenStatus add_token(Token *&token);
	List<RpnItem *> *get_result(void)	// only call when add_token returns Done
	{
		List<RpnItem *> *list = output;
		output = NULL;
		return list;
	}
	void clean_up(void);			// only call when add_token returns an error

private:
    // 2011-02-13: implement four new functions from parts of add_token()
	TokenStatus process_operand(Token *&token);
	TokenStatus end_expression_error(void);
	bool process_unary_operator(Token *&token, TokenStatus &status);
    TokenStatus process_binary_operator(Token *&token);
	// 2010-04-13: made argument a reference so different value can be returned
	// 2011-01-13: added first operand token pointer (from hold stack)
	// 2011-02-13: replaced add_operator()
	TokenStatus process_operator(Token *&token);
	enum Match {
		No_Match,
		Yes_Match,
		Cvt_Match,
		sizeof_Match
	};
    // 2011-03-12: added new function
	TokenStatus operator_error(void);
	// 2010-04-25: add two functions for data type handling
	void set_default_datatype(Token *token)
	{
		// only set to double if not an internal function (2011-03-04)
		if (token->datatype == None_DataType
			&& token->type != IntFuncP_TokenType)
		{
			// TODO for now just set default to double
			token->datatype = Double_DataType;
		}
		// 2010-05-15: change string DefFuncN/P to TmpStr
		else if ((token->type == DefFuncN_TokenType
			|| token->type == DefFuncP_TokenType)
			&& token->datatype == String_DataType)
		{
			token->datatype = TmpStr_DataType;
		}
	}
	// 2011-01-15: added new function
	TokenStatus process_first_operand(Token *&token);
	// 2010-09-03: added new function
	// 2011-01-13: added pointer to first token (for done stack)
	TokenStatus process_final_operand(Token *&token, Token *token2,
		int operand_index, int noperands = 0);
	// 2010-05-08: added last_operand argument
	// 2010-07-01: removed last_operand argument
	// 2010-09-03: added operand_index argument
	// 2011-01-15: added first/last token pointers arguments
	TokenStatus find_code(Token *&token, int operand_index,
		Token **first = NULL, Token **last = NULL);
	// 2010-05-29: changed argument from index to token pointer
	void do_pending_paren(Token *token);  // 2010-03-26: added for parentheses
	// 2010-06-03: function to check if expression ended correctly
	TokenStatus expression_end(void);
	// 2010-06-14: function to get status for an outstanding parentheses token
	TokenStatus paren_status(void);
    // 2011-01-02: function to get current expression data type
	TokenStatus get_expr_datatype(DataType &datatype);
	// 2011-01-15: function to delete a close paren token
	void delete_close_paren(Token *last);
    // 2011-02-10: function to call handler for command on top of command stack
	TokenStatus call_command_handler(Token *&token);

	// COMMAND SPECIFIC FUNCTIONS
	// 2010-06-04: function to added data type specific print code
	TokenStatus add_print_code(void);
	// 2010-07-01: functions for new assignment processing
	TokenStatus check_assignlist_token(Token *&token);
	TokenStatus set_assign_command(Token *&token, Code assign_code);
public:
	// 2011-01-22: function to delete an open paren token
	// 2011-01-30: made function public to be used to delete token (leak)
	void delete_open_paren(Token *first);

	// 2010-05-28: added token handler friend function definitions section
	friend TokenStatus Operator_Handler(Translator &t, Token *&token);
	friend TokenStatus Equal_Handler(Translator &t, Token *&token);
	friend TokenStatus Comma_Handler(Translator &t, Token *&token);
	friend TokenStatus CloseParen_Handler(Translator &t, Token *&token);
	friend TokenStatus EndOfLine_Handler(Translator &t, Token *&token);
	// 2010-06-02: added token handler for semicolon
	friend TokenStatus SemiColon_Handler(Translator &t, Token *&token);

	// 2010-06-05: added command handler friend function definitions section
	// 2010-06-13: added token pointer argument to command handlers
	friend TokenStatus Assign_CmdHandler(Translator &t, CmdItem *cmd_item,
		Token *token);
	friend TokenStatus Print_CmdHandler(Translator &t, CmdItem *cmd_item,
		Token *token);
	// 2010-06-13: added command handler
	friend TokenStatus Let_CmdHandler(Translator &t, CmdItem *cmd_item,
		Token *token);
	// 2011-03-01: added command handler
	friend TokenStatus Input_CmdHandler(Translator &t, CmdItem *cmd_item,
		Token *token);
};


// 2010-05-28: added token handler function definitions section
// 2010-06-13: added token pointer argument to command handlers
extern TokenStatus Operator_Handler(Translator &t, Token *&token);
extern TokenStatus Equal_Handler(Translator &t, Token *&token);
extern TokenStatus Comma_Handler(Translator &t, Token *&token);
extern TokenStatus CloseParen_Handler(Translator &t, Token *&token);
extern TokenStatus EndOfLine_Handler(Translator &t, Token *&token);
// 2010-06-02: added token handler for semicolon
extern TokenStatus SemiColon_Handler(Translator &t, Token *&token);

// 2010-06-05: added command handler friend function definitions section
extern TokenStatus Assign_CmdHandler(Translator &t, CmdItem *cmd_item,
	Token *token);
extern TokenStatus Print_CmdHandler(Translator &t, CmdItem *cmd_item,
	Token *token);
// 2010-06-13: added command handler
extern TokenStatus Let_CmdHandler(Translator &t, CmdItem *cmd_item,
	Token *token);
// 2011-03-01: added command handler
extern TokenStatus Input_CmdHandler(Translator &t, CmdItem *cmd_item,
	Token *token);


#endif  // IBCP_H
