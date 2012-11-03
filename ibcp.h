// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: ibcp.h - main definitions file
//	Copyright (C) 2010-2012  Thunder422
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
//	2011-03-26	enum TokenStatus is now automatically generated from the
//				  ibcp.cpp source file by the enums.awk script so it was
//				  removed,
//				The automatic codes.h was renamed autoenums.h
//				removed Duplicate and Missing error types (no longer used)
//				changed several shorts (index values) to type T in Error
//				changed Token::message_array[] to simply string pointer array
//	2011-03-27	added operand_state flag to Parser with set access function
//				added get operand state access function to Translator
//
//	2012-10-25	added new function Token::isNull() to properly check for the
//				  null token (not all token types have a valid code)
//	2012-10-27	converted translator output rpn list from List class to QList
//				also required changes to done_stack (element list pointer to
//				  rpn item pointer), cmd_stack (element list pointer to index
//				  into output rpn list), and RpnItem.operands[] (element list
//				  pointer array to RpnItem pointer array)
//	2012-10-28	removed token lists and new/delete overload functions
//	2012-10-28	removed Error Template (no longer needed)
//				changed Table class (constructor and new initialize() function)
//	2012-10-29	converted translator stacks from Stack class to QStack
//	2012-10-31	added 'const' to 'char *' variables in Parser
//	2012-11-01	removed support for immediate commands
//				changed all String to QString
//				renamed Parser functions and variables to Qt Style naming
//	2012-11-03	separated classes to individual header files

#ifndef IBCP_H
#define IBCP_H

// include auto-generated enumerations
#include "autoenums.h"


// code operator in-line functions for additional, increment and decrement
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


// data type of operands
enum DataType {
	// actual execution data types must be listed first
	// since they will also be used for indexes
	Double_DataType,
	Integer_DataType,
	String_DataType,
	TmpStr_DataType,
	SubStr_DataType,
	// end of the actual execution data types
	numberof_DataType,	// number of actual execution data types
	// the following data types are used internally for other uses
	None_DataType,		// indicates none of the above data types
	sizeof_DataType
};


// sub-code flags for use in Token and internal program
enum {
	None_SubCode       = 0x00000000,	// no sub-code present
	Paren_SubCode      = 0x00000001,	// reproduce unnecessary parenthesis
	Let_SubCode        = 0x00000002,	// reproduce LET keyword for assign
	Keep_SubCode       = 0x00000004,	// keep cursor on same line of input
	SemiColon_SubCode  = 0x00000008,	// semicolon after print function
	Question_SubCode   = 0x00000010,	// add "? " to input prompt
	Used_SubCode       = 0x00000020,	// parentheses used in output
	Last_SubCode       = 0x00000040,	// parentheses used as last token
	End_SubCode        = 0x00000080		// end of INPUT parsing codes
};


// expected token mode after command type
enum TokenMode {
	Null_TokenMode,					// no token mode set flag
	Command_TokenMode,				// expecting command
	Assignment_TokenMode,			// expecting assignment
	AssignmentList_TokenMode,		// comma separated assignment started
	Expression_TokenMode,			// inside expression
	Reference_TokenMode,			// reference expected
	sizeof_TokenMode
};


#endif  // IBCP_H
