// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: table.cpp - contains operator/command/function table
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
//	2010-03-01	renamed ImCommand_TokenType to ImmCmd_TokenType
//
//	2010-03-06	changed None_DataType to String_DataType for the internal string
//				functions
//
//	2010-03-10	corrected multiple entry for operators <, <=, and <>
//				(was OneChar, changed to TwoChar)
//
//	2010-03-11	split IntFunc_TokenType into IntFuncN_TokenType and
//				IntFuncP_TokenType; changed tables entries accordingly
//
//	2010-03-17	added data for new unary_code field with all set to Null_Code
//				  except for Sub_Code and Not_Code
//				added new table entries for Null_Code, Neg_Code and EOL_Code
//
//	2010-03-20	added precedence values to the table entries
//
//	2010-03-21	for Neg_Code temporarily replaced "-" with "Neq" for testing
//				and changed unary_code from Null_Code to Neq_Code
//
//	2010-03-25	added unary code for (, changed precedence values for ( & )
//
//	2010-04-02	changed precedence value in comma entry
//				corrected name for VAL( from "Val(" to "VAL("
//				for operators, name is actual (recreator) output string,
//				  name2 is debug output string (only affects Neg for now)
//
//	2010-04-04	added values for number of arguments variable
//				added entry for Asc2_Code
//				replaced Mid_Code entry Mid2_Code and Mid3_Code entries
//				replaced Instr_Code entry Instr2_Code and Instr3_Code entries
//				added Multiple_Flag for Asc_Code, Mid2_Code and Instr2_Code
//				implemented new number of arguments search function
//
//	2010-04-11	added table entries for assignment operators
//				set correct precedence for internal functions
//				removed unnecessary 0 initializers from entries
//
//	2010-04-24	added values for the number of operands to the operator entries
//				added values for new operand_datatype[] and assoc_code[] arrays
//				corrected data type for many operators and internal functions
//				added entries for the associated codes
//				moved the Null_Code to the end of the table
//				added entries for FRAC, CDBL, CvtDbl and CvtDbl
//
//	2010-05-03	began to add support for initialization of expression
//				  information structures for the table entries
//	2010-05-04	updated table entries for the expression information structures
//	2010-05-05	modified the assignment operator entries for data type handling
//				added entries for associated assignment operators
//				added Operands and AssocCode macros
//	2010-05-08	added expression information structures for unary operators
//
//	2010-05-15	modified codes that return string to return TmpStr (Chr, Repeat,
//				  Space, Str, CatStr, and StrInt) - new ExprInfos were added
//				added String_Flag to all codes that have string operands
//
//	2010-05-19	changed data type of LEFT, MID2, MID3 and RIGHT to SubStr
//				added new associated code AssignSubStr_Code to Assign_Code along
//				  with their new table entries
//	2010-05-20	added code to check if the Max_Operands and Max_Assoc_Codes
//				agree with the tables entries, reporting errors if not
//
//	2010-05-22	removed String_Flag from codes with string operands and replaced
//				  this with the automatic setting the flag by looking at the
//				  entries during table initialization
//				added new associated code AssignListMixStr along with its new
//				  table entry
//
//	2010-05-27	changed precedence values for CloseParen_Code and Comma_Code
//				  from 4 to 6 so commands are not emptied from the hold stack
//				added precedence value of 4 (and NUll_Flag) to all commands
//	2010-05-28	added value for new token_mode to Let_Code
//				added values for new token_handler to Eq_Code, CloseParen_Code,
//				  Comma_Code, and EOL_Code
//	2010-05-29	added Hidden_Flag to CvtInt_Code and CvtDbl_Code
//
//	2010-06-01	added print-only function support (Spc_Code and Tab_Code)
//				initiated PRINT command development
//	2010-06-02	added data type specific print hidden code entries
//				changed precedence and added token handler value to SemiColon
//	2010-06-05	added command handler function pointers for PRINT and assign
//				table entries
//	2010-06-06	added end expression flag to Comma, SemiColon and EOL
//	2010-06-09	swapped table entries of MID$, INSTR, and ASC so that the larger
//				number of arguments entry is first, which is necessary for the
//				new number of arguments checking at comma
//	2010-06-13	added command handler to Let entry
//	2010-06-14	removed AssignList_Flag from AssignList table entries
//
//	2010-06-25	Replaced TableError with generic Error template
//				renamed TableSearch enum to SearchType
//	2010-06-26	added end statment flag to EOL
//	2010-06-29	added expr_type value to PRINT code
//	2010-07-02	changed assign and assign list operators expression information
//				  from two operands to one - the assign operators are no longer
//				  handled by the standard operator routines
//
//	2010-07-18	added second associate code index to support checking each
//				  operand when first processed
//	2010-08-10	swapped function with multiple arguments (ASC, INSTR, MID) back
//				  where least number of argument form is first
//				validate multiple argument codes (ASC, INSTR, MID)
//	2010-12-23	automatically generate new table entry number of string
//				  arguments
//
//	2010-12-29	added Print_Flag to the print type codes
//	2011-01-05	removed expr_type from table entries, added datatype values
//	2011-01-07	moved all datatype values from exprinfo to main entry
//				updated all ExprInfo constants (and removed duplicates)
//	2011-02-03	started implementing associated entries for TmpStr type
//	2011-02-04	continue implementing TmpStr associated entries
//	2011-02-05	changed all AssocCode2 macro calls with 0 for index with the
//				  AssocCode macro call, which is the same for index = 0
//				changed table initialization to not count temporary strings
//				changed table initialization to allow assoc2_index to equal
//				  nassoc_codes so that no associated codes are searched for
//				  matching the second operand if not desired (for assigns)
//				changed assign string code entries to have their own expression
//				  info instances because the first string operand does not get
//				  get counted for the number of strings
//				modified table initialization to not could the first string
//				  operand if reference flag is set (assignment operators)
//
//	2011-02-26	removed code from first memory in table entries to comment at
//				  beginning of entry, which is used by codes.awk to generated
//				  codes.h containing the Code enumeration
//				moved Null_Code entry from end to beginning so that Null_Code
//				  enumeration value will be zero
//				removed table index/code duplicate and missing checking (not
//				  necessary since codes are new generated directly from table)
//				added bracketing around immediate commands
//				updated search() functions to return Code enumeration value
//	2011-03-01	started INPUT command handler, added INPUT PROMPT table entry,
//				  add InputBegin tale entries
//				removed String flags - automatically set in Table::Table()
//	2011-03-19	added input assign codes plus their associated code lists,
//				  which sets the input parse codes as second associated codes
//				renamed EndStatment_Flag to EndStmt_Flag
//
//	2011-03-26	corrected Assoc2Code_ErrorType error
//
//	2012-10-28	Table::Table() now Table::initialize() and instead of throwing
//				  an exception for any errors found, returns a list of error
//				  strings as a QStringList (no longer uses Error Template)
//	2012-11-01	replaced some char* with QStringRef
//				removed immediate commands from table
//	2012-11-04	moved all table specific (private) definitions from table.h
//				moved code for access functions from table.h
//				renamed variables and functions to Qt style naming
//				made all array initalizers static
//				changed the rest of char* to QString, removed use of strcmp()
//	2012-11-04	modified table class so that only one instance can be created

#include <QChar>
#include <QString>

#include "table.h"
#include "commandhandlers.h"
#include "tokenhandlers.h"


const int MaxOperands = 3;
	// this value contains the maximum number of operands
	// (arguments) for any operator or internal function (there are currently
	// no internal function with more than 3 arguments)

const int MaxAssocCodes = 4;
	// this value contains the maximum number of associated codes,
	// codes in additional to the main code for different possible data types
	// for the code (no code currently has more the 4 total codes)


// expression information for operators and internal functions
struct ExprInfo
{
	Code m_unaryCode;				// operator unary code (Null_Code if none)
	short m_nOperands;				// number of operands (operators/functions)
	short m_nAssocCodes;			// number of associated codes
	short m_assoc2Index;			// second operand assoc codes start index
	short m_nStrings;				// number of string arguments
	DataType *m_operandDataType;	// data type of each operand
	Code *m_assocCode;				// associated codes

	ExprInfo(Code unaryCode = Null_Code, short nOperands = 0,
		DataType *operandDataType = NULL, short nAssocCodes = 0,
		short assoc2Index = 0, Code *assocCode = NULL) :
	    m_unaryCode(unaryCode), m_nOperands(nOperands),
	    m_operandDataType(operandDataType), m_nAssocCodes(nAssocCodes),
		m_assoc2Index(assoc2Index), m_assocCode(assocCode), m_nStrings(0)
		// default for m_nStrings; will be filled in later
	{
	}
};


struct TableEntry
{
	TokenType type;					// type of token for entry
	Multiple multiple;				// multiple word command/character operator
	const QString name;				// name for table entry
	const QString name2;			// name of second word of command
	int flags;						// flags for entry
	int precedence;					// precedence of code
	DataType dataType;				// next expression data type for command
	ExprInfo *exprInfo;				// expression info pointer (NULL for none)
	TokenHandler tokenHandler;		// translator token handler pointer
	TokenMode tokenMode;			// next token mode for command
	CommandHandler commandHandler;	// translator command handler pointer
};


Table *Table::m_instance;			// pointer to single table instance


// this macro produces two entries for the ExprInfo constructor,
// one for the number of operands, one for the pointer to the
// operand data type array
#define Operands(type)  (sizeof(type ## _OperandArray) \
	/ sizeof(type ## _OperandArray[0])), type ## _OperandArray

// macro produces associated code fields for table entries
#define AssocCode(code)  (sizeof(code ## _AssocCode) \
	/ sizeof(code ## _AssocCode[0])), 0, code ## _AssocCode

// macro produces associated code fields with secondary index for table entries
#define AssocCode2(code, index2)  (sizeof(code ## _AssocCode) \
	/ sizeof(code ## _AssocCode[0])), index2, code ## _AssocCode


// operand data type arrays
static DataType Dbl_OperandArray[] = {
	Double_DataType
};
static DataType DblDbl_OperandArray[] = {
	Double_DataType, Double_DataType
};
static DataType DblInt_OperandArray[] = {
	Double_DataType, Integer_DataType
};

static DataType Int_OperandArray[] = {
	Integer_DataType
};
static DataType IntDbl_OperandArray[] = {
	Integer_DataType, Double_DataType
};
static DataType IntInt_OperandArray[] = {
	Integer_DataType, Integer_DataType
};

static DataType Str_OperandArray[] = {
	String_DataType
};
static DataType StrInt_OperandArray[] = {
	String_DataType, Integer_DataType
};
static DataType StrIntInt_OperandArray[] = {
	String_DataType, Integer_DataType, Integer_DataType
};
static DataType StrStr_OperandArray[] = {
	String_DataType, String_DataType
};
static DataType StrTmp_OperandArray[] = {
	String_DataType, TmpStr_DataType
};
static DataType StrStrInt_OperandArray[] = {
	String_DataType, String_DataType, Integer_DataType
};
static DataType StrTmpInt_OperandArray[] = {
	String_DataType, TmpStr_DataType, Integer_DataType
};
static DataType Sub_OperandArray[] = {
	SubStr_DataType
};
static DataType SubStr_OperandArray[] = {
	SubStr_DataType, String_DataType
};
static DataType SubTmp_OperandArray[] = {
	SubStr_DataType, TmpStr_DataType
};

static DataType Tmp_OperandArray[] = {
	TmpStr_DataType
};
static DataType TmpInt_OperandArray[] = {
	TmpStr_DataType, Integer_DataType
};
static DataType TmpStr_OperandArray[] = {
	TmpStr_DataType, String_DataType
};
static DataType TmpStrInt_OperandArray[] = {
	TmpStr_DataType, String_DataType, Integer_DataType
};
static DataType TmpTmp_OperandArray[] = {
	TmpStr_DataType, TmpStr_DataType
};
static DataType TmpTmpInt_OperandArray[] = {
	TmpStr_DataType, TmpStr_DataType, Integer_DataType
};


// associated code data type arrays
static Code Abs_AssocCode[]				= {AbsInt_Code};
static Code Add_AssocCode[]				= {
	AddI1_Code, CatStr_Code, CatStrT1_Code, AddI2_Code
};
static Code AddI1_AssocCode[]			= {AddInt_Code};
static Code Asc_AssocCode[]				= {AscTmp_Code};
static Code Asc2_AssocCode[]			= {Asc2Tmp_Code};
static Code Assign_AssocCode[]			= {
	AssignInt_Code, AssignStr_Code, AssignSubStr_Code
};
static Code AssignStr_AssocCode[]		= {AssignTmp_Code};
static Code AssignSubStr_AssocCode[]	= {AssignSubTmp_Code};
static Code AssignList_AssocCode[]		= {
	AssignListInt_Code, AssignListStr_Code, AssignListMix_Code
};
static Code AssignListStr_AssocCode[]	= {AssignListTmp_Code};
static Code AssignListMix_AssocCode[]	= {AssignListMixTmp_Code};
static Code CatStr_AssocCode[]			= {CatStrT2_Code};
static Code CatStrT1_AssocCode[]		= {CatStrTT_Code};
static Code Div_AssocCode[]				= {DivI1_Code, DivI2_Code};
static Code DivI1_AssocCode[]			= {DivInt_Code};
static Code Eq_AssocCode[]				= {
	EqI1_Code, EqStr_Code, EqStrT1_Code, EqI2_Code
};
static Code EqI1_AssocCode[]			= {EqInt_Code};
static Code EqStr_AssocCode[]			= {EqStrT2_Code};
static Code EqStrT1_AssocCode[]			= {EqStrTT_Code};
static Code Gt_AssocCode[]				= {
	GtI1_Code, GtStr_Code, GtStrT1_Code, GtI2_Code
};
static Code GtI1_AssocCode[]			= {GtInt_Code};
static Code GtStr_AssocCode[]			= {GtStrT2_Code};
static Code GtStrT1_AssocCode[]			= {GtStrTT_Code};
static Code GtEq_AssocCode[]			= {
	GtEqI1_Code, GtEqStr_Code, GtEqStrT1_Code, GtEqI2_Code
};
static Code GtEqI1_AssocCode[]			= {GtEqInt_Code};
static Code GtEqStr_AssocCode[]			= {GtEqStrT2_Code};
static Code GtEqStrT1_AssocCode[]		= {GtEqStrTT_Code};
static Code InputBeginStr_AssocCode[]	= {InputBeginTmp_Code};
static Code InputAssign_AssocCode[]		= {
	InputAssignInt_Code, InputAssignStr_Code, InputParse_Code
};
static Code InputAssignInt_AssocCode[]	= {InputParseInt_Code};
static Code InputAssignStr_AssocCode[]	= {InputParseStr_Code};
static Code Instr2_AssocCode[]			= {Instr2T1_Code, Instr2T2_Code};
static Code Instr2T1_AssocCode[]		= {Instr2TT_Code};
static Code Instr3_AssocCode[]			= {Instr3T1_Code, Instr3T2_Code};
static Code Instr3T1_AssocCode[]		= {Instr3TT_Code};
static Code Len_AssocCode[]				= {LenTmp_Code};
static Code Lt_AssocCode[]				= {
	LtI1_Code, LtStr_Code, LtStrT1_Code, LtI2_Code
};
static Code LtI1_AssocCode[]			= {LtInt_Code};
static Code LtStr_AssocCode[]			= {LtStrT2_Code};
static Code LtStrT1_AssocCode[]			= {LtStrTT_Code};
static Code LtEq_AssocCode[]			= {
	LtEqI1_Code, LtEqStr_Code, LtEqStrT1_Code, LtEqI2_Code
};
static Code LtEqI1_AssocCode[]			= {LtEqInt_Code};
static Code LtEqStr_AssocCode[]			= {LtEqStrT2_Code};
static Code LtEqStrT1_AssocCode[]		= {LtEqStrTT_Code};
static Code Mod_AssocCode[]				= {ModI1_Code, ModI2_Code};
static Code ModI1_AssocCode[]			= {ModInt_Code};
static Code Mul_AssocCode[]				= {MulI1_Code, MulI2_Code};
static Code MulI1_AssocCode[]			= {MulInt_Code};
static Code Neg_AssocCode[]				= {NegInt_Code};
static Code NotEq_AssocCode[]			= {
	NotEqI1_Code, NotEqStr_Code, NotEqStrT1_Code, NotEqI2_Code
};
static Code NotEqI1_AssocCode[] 	   	= {NotEqInt_Code};
static Code NotEqStr_AssocCode[]		= {NotEqStrT2_Code};
static Code NotEqStrT1_AssocCode[]		= {NotEqStrTT_Code};
static Code Power_AssocCode[]			= {PowerI1_Code, PowerMul_Code};
static Code PowerI1_AssocCode[]			= {PowerInt_Code};
static Code Print_AssocCode[]			= {
    PrintInt_Code, PrintStr_Code, PrintTmp_Code
};
static Code Repeat_AssocCode[]			= {RepeatTmp_Code};
static Code RndArgs_AssocCode[]			= {RndArgInt_Code};
static Code Sgn_AssocCode[]				= {SgnInt_Code};
static Code Str_AssocCode[]				= {StrInt_Code};
static Code Sub_AssocCode[]				= {SubI1_Code, SubI2_Code};
static Code SubI1_AssocCode[]			= {SubInt_Code};
static Code Val_AssocCode[]				= {ValTmp_Code};


// standard expression information structures
static ExprInfo Dbl_ExprInfo(Null_Code, Operands(Dbl));
static ExprInfo DblDbl_ExprInfo(Null_Code, Operands(DblDbl));
static ExprInfo DblInt_ExprInfo(Null_Code, Operands(DblInt));

static ExprInfo Int_ExprInfo(Null_Code, Operands(Int));
static ExprInfo IntInt_ExprInfo(Null_Code, Operands(IntInt));

static ExprInfo Str_ExprInfo(Null_Code, Operands(Str));
static ExprInfo StrInt_ExprInfo(Null_Code, Operands(StrInt));
static ExprInfo StrStr_ExprInfo(Null_Code, Operands(StrStr));  //?
static ExprInfo StrStrInt_ExprInfo(Null_Code, Operands(StrStrInt));
static ExprInfo StrTmp_ExprInfo(Null_Code, Operands(StrTmp));
static ExprInfo Assign_StrTmp_ExprInfo(Null_Code, Operands(StrTmp));
static ExprInfo StrTmpInt_ExprInfo(Null_Code, Operands(StrTmpInt));
static ExprInfo Assign_SubTmp_ExprInfo(Null_Code, Operands(SubTmp));

static ExprInfo Tmp_ExprInfo(Null_Code, Operands(Tmp));
static ExprInfo TmpInt_ExprInfo(Null_Code, Operands(TmpInt));
static ExprInfo TmpStr_ExprInfo(Null_Code, Operands(TmpStr));
static ExprInfo TmpStrInt_ExprInfo(Null_Code, Operands(TmpStrInt));
static ExprInfo TmpTmp_ExprInfo(Null_Code, Operands(TmpTmp));
static ExprInfo TmpTmpInt_ExprInfo(Null_Code, Operands(TmpTmpInt));

static ExprInfo StrIntInt_ExprInfo(Null_Code, Operands(StrIntInt));
static ExprInfo Sub_ExprInfo(Null_Code, Operands(Sub));

// code enumeration names in comments after opening brace
// (code enumeration generated from these by enums.awk)
static TableEntry tableEntries[] =
{
	// Null_Code entry at beginning so Null_Code == 0
	{	// Null_Code
		Operator_TokenType
	},
	//***********************
	//   BEGIN PLAIN WORDS
	//***********************
	{	// BegPlainWord_Code
		Error_TokenType
	},
	//--------------
	//   Commands
	//--------------
	{	// Let_Code
		Command_TokenType, OneWord_Multiple,
		"LET", NULL, Null_Flag, 4, None_DataType, NULL, NULL,
		Assignment_TokenMode, Let_CmdHandler
	},
	{	// Print_Code
		Command_TokenType, OneWord_Multiple,
		"PRINT", NULL, Null_Flag, 4, None_DataType, NULL, NULL,
		Expression_TokenMode, Print_CmdHandler
	},
	{	// Input_Code
		Command_TokenType, TwoWord_Multiple,
		"INPUT", NULL, Null_Flag, 4, None_DataType, NULL, NULL,
		Reference_TokenMode, Input_CmdHandler

	},
	{	// InputPrompt_Code
		Command_TokenType, TwoWord_Multiple,
		"INPUT", "PROMPT", Null_Flag, 4, String_DataType, NULL, NULL,
		Expression_TokenMode, Input_CmdHandler

	},
	{	// Dim_Code
		Command_TokenType, OneWord_Multiple,
		"DIM", NULL, Null_Flag, 4, None_DataType
	},
	{	// Def_Code
		Command_TokenType, OneWord_Multiple,
		"DEF", NULL, Null_Flag, 4, None_DataType
	},
	{	// Rem_Code
		Command_TokenType, OneWord_Multiple,
		"REM", NULL, Null_Flag, 4, None_DataType
	},
	{	// If_Code
		Command_TokenType, OneWord_Multiple,
		"IF", NULL, Null_Flag, 4, None_DataType
	},
	{	// Then_Code
		Command_TokenType, OneWord_Multiple,
		"THEN", NULL, Null_Flag, 4, None_DataType
	},
	{	// Else_Code
		Command_TokenType, OneWord_Multiple,
		"ELSE", NULL, Null_Flag, 4, None_DataType
	},
	{	// End_Code
		Command_TokenType, TwoWord_Multiple,
		"END", NULL, Null_Flag, 4, None_DataType
	},
	{	// EndIf_Code
		Command_TokenType, TwoWord_Multiple,
		"END", "IF", Null_Flag, 4, None_DataType
	},
	{	// For_Code
		Command_TokenType, OneWord_Multiple,
		"FOR", NULL, Null_Flag, 4, None_DataType
	},
	{	// To_Code
		Command_TokenType, OneWord_Multiple,
		"TO", NULL, Null_Flag, 4, None_DataType
	},
	{	// Step_Code
		Command_TokenType, OneWord_Multiple,
		"STEP", NULL, Null_Flag, 4, None_DataType
	},
	{	// Next_Code
		Command_TokenType, OneWord_Multiple,
		"NEXT", NULL, Null_Flag, 4, None_DataType
	},
	{	// Do_Code
		Command_TokenType, TwoWord_Multiple,
		"DO", NULL, Null_Flag, 4, None_DataType
	},
	{	// DoWhile_Code
		Command_TokenType, TwoWord_Multiple,
		"DO", "WHILE", Null_Flag, 4, None_DataType
	},
	{	// DoUntil_Code
		Command_TokenType, TwoWord_Multiple,
		"DO", "UNTIL", Null_Flag, 4, None_DataType
	},
	{	// While_Code
		Command_TokenType, TwoWord_Multiple,
		"WHILE", NULL, Null_Flag, 4, None_DataType
	},
	{	// Until_Code
		Command_TokenType, TwoWord_Multiple,
		"UNTIL", NULL, Null_Flag, 4, None_DataType
	},
	{	// Loop_Code
		Command_TokenType, TwoWord_Multiple,
		"LOOP", NULL, Null_Flag, 4, None_DataType
	},
	{	// LoopWhile_Code
		Command_TokenType, TwoWord_Multiple,
		"LOOP", "WHILE", Null_Flag, 4, None_DataType
	},
	{	// LoopUntil_Code
		Command_TokenType, TwoWord_Multiple,
		"LOOP", "UNTIL", Null_Flag, 4, None_DataType
	},
	//-----------------------------------------
	//   Internal Functions (No Parentheses)
	//-----------------------------------------
	{	// Rnd_Code
		IntFuncN_TokenType, OneWord_Multiple,
		"RND", NULL, Null_Flag, 2, Double_DataType, new ExprInfo()

	},
	//--------------------
	//   Word Operators
	//--------------------
	{	// Mod_Code
		Operator_TokenType, OneWord_Multiple,
		"MOD", NULL, Null_Flag, 42, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Mod, 1))
	},
	{	// And_Code
		Operator_TokenType, OneWord_Multiple,
		"AND", NULL, Null_Flag, 18, Integer_DataType, &IntInt_ExprInfo
	},
	{	// Or_Code
		Operator_TokenType, OneWord_Multiple,
		"OR", NULL, Null_Flag, 14, Integer_DataType, &IntInt_ExprInfo
	},
	{	// Not_Code
		Operator_TokenType, OneWord_Multiple,
		"NOT", NULL, Null_Flag, 20, Integer_DataType,
		new ExprInfo(Not_Code, Operands(Int))
	},
	{	// Eqv_Code
		Operator_TokenType, OneWord_Multiple,
		"EQV", NULL, Null_Flag, 12, Integer_DataType, &IntInt_ExprInfo
	},
	{	// Imp_Code
		Operator_TokenType, OneWord_Multiple,
		"IMP", NULL, Null_Flag, 10, Integer_DataType, &IntInt_ExprInfo
	},
	{	// Xor_Code
		Operator_TokenType, OneWord_Multiple,
		"XOR", NULL, Null_Flag, 16, Integer_DataType, &IntInt_ExprInfo
	},
	//*********************
	//   END PLAIN WORDS
	//*********************
	{	// EndPlainWord_Code
		Error_TokenType
	},
	//*****************************
	//   BEGIN PARENTHESES WORDS
	//*****************************
	{	// BegParenWord_Code
		Error_TokenType
	},
	//--------------------------------------
	//   INTERNAL FUNCTIONS (Parentheses)
	//--------------------------------------
	{	// Abs_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ABS(", NULL, Null_Flag, 2, Double_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(Abs))
	},
	{	// Fix_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"FIX(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Frac_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"FRAC(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Int_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INT(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo

	},
	{	// RndArg_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"RND(", NULL, Null_Flag, 2, Double_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(RndArgs))
	},
	{	// Sgn_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SGN(", NULL, Null_Flag, 2, Double_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(Sgn))
	},
	{	// Cint_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"CINT(", NULL, Null_Flag, 2, Integer_DataType, &Dbl_ExprInfo
	},
	{	// Cdbl_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"CDBL(", NULL, Null_Flag, 2, Double_DataType, &Int_ExprInfo
	},
	{	// Sqr_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SQR(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Atn_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ATN(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Cos_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"COS(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Sin_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SIN(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Tan_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"TAN(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Exp_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"EXP(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Log_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"LOG(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Tab_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"TAB(", NULL, Print_Flag, 2, None_DataType, &Int_ExprInfo
	},
	{	// Spc_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SPC(", NULL, Print_Flag, 2, None_DataType, &Int_ExprInfo
	},
	{	// Asc_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ASC(", NULL, Multiple_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(Str), AssocCode(Asc))
	},
	{	// Asc2_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ASC(", "ASC2(", Null_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrInt), AssocCode(Asc2))
	},
	{	// Chr_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"CHR$(", NULL, Null_Flag, 2, TmpStr_DataType, &Int_ExprInfo
	},
	{	// Instr2_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR2(", Multiple_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode2(Instr2, 1))
	},
	{	// Instr3_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR3(", Null_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStrInt), AssocCode2(Instr3, 1))
	},
	{	// Left_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"LEFT$(", NULL, Null_Flag, 2, SubStr_DataType, &StrInt_ExprInfo
	},
	{	// Len_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"LEN(", NULL, Null_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(Str), AssocCode(Len))
	},
	{	// Mid2_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"MID$(", "MID2$(", Multiple_Flag, 2, SubStr_DataType, &StrInt_ExprInfo
	},
	{	// Mid3_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"MID$(", "MID3$(", Null_Flag, 2, SubStr_DataType, &StrIntInt_ExprInfo
	},
	{	// Repeat_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"REPEAT$(", NULL, Null_Flag, 2, TmpStr_DataType,
		new ExprInfo(Null_Code, Operands(StrInt), AssocCode(Repeat))
	},
	{	// Right_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"RIGHT$(", NULL, Null_Flag, 2, SubStr_DataType, &StrInt_ExprInfo
	},
	{	// Space_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SPACE$(", NULL, Null_Flag, 2, TmpStr_DataType, &Int_ExprInfo
	},
	{	// Str_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"STR$(", NULL, Null_Flag, 2, TmpStr_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(Str))
	},
	{	// Val_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"VAL(", NULL, Null_Flag, 2, Double_DataType, &Str_ExprInfo
	},
	//***************************
	//   END PARENTHESES WORDS
	//***************************
	{	// EndParenWord_Code
		Error_TokenType
	},
	//***************************
	//   BEGIN DATA TYPE WORDS
	//***************************
	{	// BegDataTypeWord_Code
		Error_TokenType
	},
	// Currently None

	//*************************
	//   END DATA TYPE WORDS
	//*************************
	{	// EndDataTypeWord_Code
		Error_TokenType
	},
	//*******************
	//   BEGIN SYMBOLS
	//*******************
	{	// BegSymbol_Code
		Error_TokenType
	},
	//----------------------
	//   Symbol Operators
	//----------------------
	{	// Add_Code
		Operator_TokenType, OneChar_Multiple,
		"+", NULL, Null_Flag, 40, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Add, 3))
	},
	{	// Sub_Code
		Operator_TokenType, OneChar_Multiple,
		"-", NULL, Null_Flag, 40, Double_DataType,
		new ExprInfo(Neg_Code, Operands(DblDbl), AssocCode2(Sub, 1))
	},
	{	// Mul_Code
		Operator_TokenType, OneChar_Multiple,
		"*", NULL, Null_Flag, 46, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Mul, 1))
	},
	{	// Div_Code
		Operator_TokenType, OneChar_Multiple,
		"/", NULL, Null_Flag, 46, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Div, 1))
	},
	{	// IntDiv_Code
		Operator_TokenType, OneChar_Multiple,
		"\\", NULL, Null_Flag, 44, Integer_DataType, &DblDbl_ExprInfo
	},
	{	// Power_Code
		Operator_TokenType, OneChar_Multiple,
		"^", NULL, Null_Flag, 50, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Power, 1))
	},
	{	// Eq_Code
		Operator_TokenType, OneChar_Multiple,
		"=", NULL, Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Eq, 3)),
		Equal_Handler
	},
	{	// Gt_Code
		Operator_TokenType, TwoChar_Multiple,
		">", NULL, Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Gt, 3))
	},
	{	// GtEq_Code
		Operator_TokenType, TwoChar_Multiple,
		">=", NULL, Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(GtEq, 3))
	},
	{	// Lt_Code
		Operator_TokenType, TwoChar_Multiple,
		"<", NULL, Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Lt, 3))
	},
	{	// LtEq_Code
		Operator_TokenType, TwoChar_Multiple,
		"<=", NULL, Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(LtEq, 3))
	},
	{	// NotEq_Code
		Operator_TokenType, TwoChar_Multiple,
		"<>", NULL, Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(NotEq, 3))
	},
	{	// OpenParen_Code
		Operator_TokenType, OneChar_Multiple,
		"(", NULL, Null_Flag, 2, None_DataType,
		new ExprInfo(OpenParen_Code)
	},
	{	// CloseParen_Code
		Operator_TokenType, OneChar_Multiple,
		")", NULL, Null_Flag, 4, None_DataType, NULL,
		CloseParen_Handler
	},
	{	// Comma_Code
		Operator_TokenType, OneChar_Multiple,
		",", NULL, EndExpr_Flag, 6, None_DataType,
		NULL,//new ExprInfo(Comma_Code),
		Comma_Handler
	},
	{	// SemiColon_Code
		Operator_TokenType, OneChar_Multiple,
		";", NULL, EndExpr_Flag, 6, None_DataType,
		NULL,//new ExprInfo(SemiColon_Code),
		SemiColon_Handler
	},
	{	// Colon_Code
		Operator_TokenType, OneChar_Multiple,
		":", NULL, Null_Flag, 0, None_DataType
	},
	{	// RemOp_Code
		Operator_TokenType, OneChar_Multiple,
		"'", NULL, Null_Flag, 0, None_DataType
	},
	//*****************
	//   END SYMBOLS
	//*****************
	{	// EndSymbol_Code
		Error_TokenType
	},
	//***************************
	//   MISCELLANEOUS ENTRIES
	//***************************
	{	// Neg_Code
		Operator_TokenType, OneWord_Multiple,
		// FIXME temporarily replaced "-" with "Neq" for testing
		"-", "Neg", Null_Flag, 48, Double_DataType,
		new ExprInfo(Neg_Code, Operands(Dbl), AssocCode(Neg))
	},
	{	// Assign_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "Assign", Reference_Flag, 4, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Assign, 3)),
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignInt_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "Assign%", Reference_Flag, 4, Integer_DataType, &IntInt_ExprInfo,
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignStr_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "Assign$", Reference_Flag, 4, String_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(AssignStr)),
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignTmp_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "Assign$T", Reference_Flag, 4, TmpStr_DataType,
		&Assign_StrTmp_ExprInfo, NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignSubStr_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignSub$", Reference_Flag, 4, String_DataType,
		new ExprInfo(Null_Code, Operands(SubStr), AssocCode(AssignSubStr)),
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignSubTmp_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignSub$T", Reference_Flag, 4, TmpStr_DataType,
		&Assign_SubTmp_ExprInfo, NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignList_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignList", Reference_Flag, 4, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(AssignList, 3)),
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignListInt_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignList%", Reference_Flag, 4, Integer_DataType,
		&IntInt_ExprInfo, NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignListStr_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignList$", Reference_Flag, 4, String_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(AssignListStr)),
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignListTmp_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignList$T", Reference_Flag, 4, TmpStr_DataType,
		&Assign_StrTmp_ExprInfo, NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignListMix_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignListMix$", Reference_Flag, 4, String_DataType,
		new ExprInfo(Null_Code, Operands(SubStr), AssocCode(AssignListMix)),
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignListMixTmp_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignListMix$T", Reference_Flag, 4,
		TmpStr_DataType, &Assign_SubTmp_ExprInfo, NULL, Null_TokenMode,
		Assign_CmdHandler
	},
	{	// EOL_Code
		Operator_TokenType, OneWord_Multiple,
		NULL, NULL, EndExpr_Flag | EndStmt_Flag, 4, None_DataType, NULL,
		EndOfLine_Handler
	},
	{	// AscTmp_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ASC(", "ASCT(", Multiple_Flag, 2, Integer_DataType, &Tmp_ExprInfo
	},
	{	// Asc2Tmp_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ASC(", "ASC2T(", Null_Flag, 2, Integer_DataType, &TmpInt_ExprInfo
	},
	{	// Instr2T1_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR2T1(", Multiple_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(Instr2T1))
	},
	{	// Instr3T1_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR3T1(", Null_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStrInt), AssocCode(Instr3T1))
	},
	{	// Instr2T2_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR2T2(", Multiple_Flag, 2, Integer_DataType,
		&StrTmp_ExprInfo
	},
	{	// Instr3T2_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR3T2(", Null_Flag, 2, Integer_DataType,
		&StrTmpInt_ExprInfo
	},
	{	// Instr2TT_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR2TT(", Multiple_Flag, 2, Integer_DataType,
		&TmpTmp_ExprInfo
	},
	{	// Instr3TT_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR3TT(", Null_Flag, 2, Integer_DataType,
		&TmpTmpInt_ExprInfo
	},
	{	// LenTmp_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"LEN(", "LENT(", Null_Flag, 2, Integer_DataType, &Tmp_ExprInfo
	},
	{	// RepeatTmp_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"REPEAT$(", "REPEAT$T(", Null_Flag, 2, TmpStr_DataType, &TmpInt_ExprInfo
	},
	{	// ValTmp_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"VAL(", "VALT(", Null_Flag, 2, Double_DataType, &Str_ExprInfo
	},
	{	// AddI1_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+%1", Null_Flag, 40, Double_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(AddI1))
	},
	{	// AddI2_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+%2", Null_Flag, 40, Double_DataType, &DblInt_ExprInfo
	},
	{	// AddInt_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+%", Null_Flag, 40, Integer_DataType, &IntInt_ExprInfo
	},
	{	// CatStr_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+$", Null_Flag, 40, TmpStr_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(CatStr))
	},
	{	// CatStrT1_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+$1", Null_Flag, 40, TmpStr_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(CatStrT1))
	},
	{	// CatStrT2_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+$2", Null_Flag, 40, TmpStr_DataType, &StrTmp_ExprInfo
	},
	{	// CatStrTT_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+$T", Null_Flag, 40, TmpStr_DataType, &TmpTmp_ExprInfo
	},
	{	// SubI1_Code
		Operator_TokenType, OneChar_Multiple,
		"-", "-%1", Null_Flag, 40, Double_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(SubI1))
	},
	{	// SubI2_Code
		Operator_TokenType, OneChar_Multiple,
		"-", "-%2", Null_Flag, 40, Double_DataType, &DblInt_ExprInfo
	},
	{	// SubInt_Code
		Operator_TokenType, OneChar_Multiple,
		"-", "-%", Null_Flag, 40, Integer_DataType, &IntInt_ExprInfo
	},
	{	// NegInt_Code
		Operator_TokenType, OneChar_Multiple,
		"-", "Neg%", Null_Flag, 40, Integer_DataType,
		new ExprInfo(NegInt_Code, Operands(Int))
	},
	{	// MulI1_Code
		Operator_TokenType, OneChar_Multiple,
		"*", "*%1", Null_Flag, 46, Double_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(MulI1))
	},
	{	// MulI2_Code
		Operator_TokenType, OneChar_Multiple,
		"*", "*%2", Null_Flag, 46, Double_DataType, &DblInt_ExprInfo
	},
	{	// MulInt_Code
		Operator_TokenType, OneChar_Multiple,
		"*", "*%", Null_Flag, 46, Integer_DataType, &IntInt_ExprInfo
	},
	{	// DivI1_Code
		Operator_TokenType, OneChar_Multiple,
		"/", "/%1", Null_Flag, 46, Double_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(DivI1))
	},
	{	// DivI2_Code
		Operator_TokenType, OneChar_Multiple,
		"/", "/%2", Null_Flag, 46, Double_DataType, &DblInt_ExprInfo
	},
	{	// DivInt_Code
		Operator_TokenType, OneChar_Multiple,
		"/", "/%", Null_Flag, 46, Integer_DataType, &IntInt_ExprInfo
	},
	{	// ModI1_Code
		Operator_TokenType, OneChar_Multiple,
		"MOD", "MOD%1", Null_Flag, 42, Double_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(ModI1))
	},
	{	// ModI2_Code
		Operator_TokenType, OneChar_Multiple,
		"MOD", "MOD%2", Null_Flag, 42, Double_DataType, &DblInt_ExprInfo
	},
	{	// ModInt_Code
		Operator_TokenType, OneWord_Multiple,
		"MOD", "MOD%", Null_Flag, 42, Integer_DataType, &IntInt_ExprInfo
	},
	{	// PowerI1_Code
		Operator_TokenType, OneChar_Multiple,
		"^", "^%1", Null_Flag, 50, Double_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(PowerI1))
	},
	{	// PowerMul_Code
		Operator_TokenType, OneChar_Multiple,
		"^", "^*", Null_Flag, 50, Double_DataType, &DblInt_ExprInfo
	},
	{	// PowerInt_Code
		Operator_TokenType, OneChar_Multiple,
		"^", "^%", Null_Flag, 50, Integer_DataType, &IntInt_ExprInfo
	},
	{	// EqI1_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=%1", Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(EqI1))
	},
	{	// EqI2_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=%2", Null_Flag, 30, Integer_DataType, &DblInt_ExprInfo
	},
	{	// EqInt_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=%", Null_Flag, 30, Integer_DataType, &IntInt_ExprInfo
	},
	{	// EqStr_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=$", Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(EqStr))
	},
	{	// EqStrT1_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=$1", Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(EqStrT1))
	},
	{	// EqStrT2_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=$2", Null_Flag, 30, Integer_DataType, &StrTmp_ExprInfo
	},
	{	// EqStrTT_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=$T", Null_Flag, 30, Integer_DataType, &TmpTmp_ExprInfo
	},
	{	// GtI1_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">%1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(GtI1))
	},
	{	// GtI2_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">%2", Null_Flag, 32, Integer_DataType, &DblInt_ExprInfo
	},
	{	// GtInt_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">%", Null_Flag, 32, Integer_DataType, &IntInt_ExprInfo
	},
	{	// GtStr_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">$", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(GtStr))
	},
	{	// GtStrT1_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">$1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(GtStrT1))
	},
	{	// GtStrT2_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">$2", Null_Flag, 32, Integer_DataType, &StrTmp_ExprInfo
	},
	{	// GtStrTT_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">$T", Null_Flag, 32, Integer_DataType, &TmpTmp_ExprInfo
	},
	{	// GtEqI1_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=%1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(GtEqI1))
	},
	{	// GtEqI2_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=%2", Null_Flag, 32, Integer_DataType, &DblInt_ExprInfo
	},
	{	// GtEqInt_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=%", Null_Flag, 32, Integer_DataType, &IntInt_ExprInfo
	},
	{	// GtEqStr_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=$", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(GtEqStr))
	},
	{	// GtEqStrT1_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=$1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(GtEqStrT1))
	},
	{	// GtEqStrT2_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=$2", Null_Flag, 32, Integer_DataType, &StrTmp_ExprInfo
	},
	{	// GtEqStrTT_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=$T", Null_Flag, 32, Integer_DataType, &TmpTmp_ExprInfo
	},
	{	// LtI1_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<%1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(LtI1))
	},
	{	// LtI2_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<%2", Null_Flag, 32, Integer_DataType, &DblInt_ExprInfo
	},
	{	// LtInt_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<%", Null_Flag, 32, Integer_DataType, &IntInt_ExprInfo
	},
	{	// LtStr_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<$", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(LtStr))
	},
	{	// LtStrT1_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<$1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(LtStrT1))
	},
	{	// LtStrT2_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<$2", Null_Flag, 32, Integer_DataType, &StrTmp_ExprInfo
	},
	{	// LtStrTT_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<$T", Null_Flag, 32, Integer_DataType, &TmpTmp_ExprInfo
	},
	{	// LtEqI1_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=%1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(LtEqI1))
	},
	{	// LtEqI2_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=%2", Null_Flag, 32, Integer_DataType, &DblInt_ExprInfo
	},
	{	// LtEqInt_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=%", Null_Flag, 32, Integer_DataType, &IntInt_ExprInfo
	},
	{	// LtEqStr_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=$", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(LtEqStr))
	},
	{	// LtEqStrT1_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=$1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(LtEqStrT1))
	},
	{	// LtEqStrT2_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=$2", Null_Flag, 32, Integer_DataType, &StrTmp_ExprInfo
	},
	{	// LtEqStrTT_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=$T", Null_Flag, 32, Integer_DataType, &TmpTmp_ExprInfo
	},
	{	// NotEqI1_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>%1", Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(NotEqI1))
	},
	{	// NotEqI2_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>%2", Null_Flag, 30, Integer_DataType, &DblInt_ExprInfo
	},
	{	// NotEqInt_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>%", Null_Flag, 30, Integer_DataType, &IntInt_ExprInfo
	},
	{	// NotEqStr_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>$", Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(NotEqStr))
	},
	{	// NotEqStrT1_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>$1", Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(NotEqStrT1))
	},
	{	// NotEqStrT2_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>$2", Null_Flag, 30, Integer_DataType, &StrTmp_ExprInfo
	},
	{	// NotEqStrTT_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>$T", Null_Flag, 30, Integer_DataType, &TmpTmp_ExprInfo
	},
	{	// AbsInt_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ABS(", "ABS%(", Null_Flag, 2, Integer_DataType, &Int_ExprInfo
	},
	{	// RndArgInt_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"RND(", "RND%(", Null_Flag, 2, Integer_DataType, &Int_ExprInfo
	},
	{	// SgnInt_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SGN(", "SGN%(", Null_Flag, 2, Integer_DataType, &Int_ExprInfo
	},
	{	// CvtInt_Code
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "CvtInt", Hidden_Flag, 2, None_DataType
	},
	{	// CvtDbl_Code
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "CvtDbl", Hidden_Flag, 2, None_DataType
	},
	{	// StrInt_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"STR$(", "STR%$(", Null_Flag, 2, TmpStr_DataType, &Int_ExprInfo
	},
	{	// PrintDbl_Code
		IntFuncN_TokenType, OneWord_Multiple,
		"", "PrintDbl", Print_Flag, 2, None_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(Print))
	},
	{	// PrintInt_Code
		IntFuncN_TokenType, OneWord_Multiple,
		"", "PrintInt", Print_Flag, 2, None_DataType, &Int_ExprInfo
	},
	{	// PrintStr_Code
		IntFuncN_TokenType, OneWord_Multiple,
		"", "PrintStr", Print_Flag, 2, None_DataType, &Str_ExprInfo
	},
	{	// PrintTmp_Code
		IntFuncN_TokenType, OneWord_Multiple,
		"", "PrintTmp", Print_Flag, 2, None_DataType, &Tmp_ExprInfo
	},
    {	// InputBegin_Code
        IntFuncN_TokenType, OneWord_Multiple,
        "", "InputBegin", Null_Flag, 2, None_DataType
    },
	{	// InputBeginStr_Code
		IntFuncN_TokenType, OneWord_Multiple,
		"", "InputBeginStr", Null_Flag, 2, None_DataType,
		new ExprInfo(Null_Code, Operands(Str), AssocCode(InputBeginStr))
	},
	{	// InputBeginTmp_Code
		IntFuncN_TokenType, OneWord_Multiple,
		"", "InputBeginTmp", Null_Flag, 2, None_DataType,
		&Tmp_ExprInfo
	},
	{	// InputAssign_Code
		IntFuncN_TokenType, OneWord_Multiple,
		"", "InputAssign", Reference_Flag, 2, None_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode2(InputAssign, 2))
	},
	{	// InputAssignInt_Code
		IntFuncN_TokenType, OneWord_Multiple,
		"", "InputAssignInt", Reference_Flag, 2, None_DataType,
		new ExprInfo(Null_Code, Operands(Int), AssocCode(InputAssignInt))
	},
	{	// InputAssignStr_Code
		IntFuncN_TokenType, OneWord_Multiple,
		"", "InputAssignStr", Reference_Flag, 2, None_DataType,
		new ExprInfo(Null_Code, Operands(Str), AssocCode(InputAssignStr))
	},
	{	// InputParse_Code
		IntFuncN_TokenType, OneWord_Multiple,
		"", "InputParse", Null_Flag, 2, None_DataType
	},
	{	// InputParseInt_Code
		IntFuncN_TokenType, OneWord_Multiple,
		"", "InputParseInt", Null_Flag, 2, None_DataType
	},
	{	// InputParseStr_Code
		IntFuncN_TokenType, OneWord_Multiple,
		"", "InputParseStr", Null_Flag, 2, None_DataType
	}
};


// function to create the single instance of the table
//
//   - fatally aborts if called more than once
//   - returns list of errors if any detected during initialization
//   - returns empty list upon successful initialization

QStringList Table::create(void)
{
	if (m_instance != NULL)
	{
		qFatal("Only one Table instance may be created!");
	}
	m_instance = new Table(tableEntries);

	QStringList errors = m_instance->initialize();
	if (!errors.isEmpty())
	{
		delete m_instance;
		m_instance = NULL;
	}
	return errors;
}


// function to return a refernce to the single table instance
//
//   - fatally aborts if called before the instance is created

Table &Table::instance(void)
{
	if (m_instance == NULL)
	{
		qFatal("Table instance was not created!");
	}
	return *m_instance;
}


// function that initializes the table variables
//
//   - if any error found then list of error messages returned
//   - an empty list is returned if no errors were detected

QStringList Table::initialize(void)
{
	QStringList errorList;
	int i;
	int type;

	// scan table and record indexes
	int nEntries = sizeof(tableEntries) / sizeof(TableEntry);
	// find maximum number of operands and associated codes
	int maxOperands = 0;
	int maxAssocCodes = 0;
	for (i = 0; i < nEntries; i++)
	{
		// check if found new maximums
		ExprInfo *exprInfo = m_entry[i].exprInfo;
		if (exprInfo != NULL)
		{
			if (maxOperands < exprInfo->m_nOperands)
			{
				maxOperands = exprInfo->m_nOperands;
			}
			if (maxAssocCodes < exprInfo->m_nAssocCodes)
			{
				maxAssocCodes = exprInfo->m_nAssocCodes;
			}

			// check if assoc2_index is valid
			if (exprInfo->m_assoc2Index > 0
				&& exprInfo->m_assoc2Index > exprInfo->m_nAssocCodes)
			{
				errorList.append(QString("Entry:%1 Assoc2Index=%2 too large, "
					"maximum is %3").arg(i).arg(exprInfo->m_assoc2Index)
					.arg(exprInfo->m_nAssocCodes));
			}

			// generate number of string arguments value
			exprInfo->m_nStrings = 0;
			for (int j = 0; j < exprInfo->m_nOperands; j++)
			{
				// don't count temporary strings
				// don't count first string of assignment operators
				if (exprInfo->m_operandDataType[j] == String_DataType
					&& (j > 0 || !(m_entry[i].flags & Reference_Flag)))
				{
					exprInfo->m_nStrings++;
				}
			}
			// set String_Flag in entries automatically
			if (exprInfo->m_nStrings > 0)
			{
				m_entry[i].flags |= String_Flag;
			}

			// validate multiple entries
			if (m_entry[i].flags & Multiple_Flag != 0)
			{
				ExprInfo *exprInfo2 = m_entry[i + 1].exprInfo;
				if (m_entry[i].name.compare(m_entry[i + 1].name) != 0)
				{
					errorList.append(QString("Multiple entry '%1' name "
						"mis-match '%2'").arg(m_entry[i].name)
						.arg(m_entry[i + 1].name));
				}
				else if (exprInfo2 == NULL)
				{
					errorList.append(QString("Multiple entry '%1' next entry "
						"no expression info").arg(m_entry[i + 1].name));
				}
				else if (exprInfo2->m_nOperands != exprInfo->m_nOperands + 1)
				{
					errorList.append(QString("Multiple entry '%1' incorrect "
						"number of operands (%2, %3)").arg(m_entry[i].name)
						.arg(exprInfo->m_nOperands)
						.arg(exprInfo2->m_nOperands));
				}
			}
		}
	}

	// check maximums found against constants
	if (maxOperands > MaxOperands)
	{
		errorList.append(QString("Max_Operands=%1 too small, actual is %2")
			.arg(MaxOperands).arg(maxOperands));
	}
	if (maxAssocCodes > MaxAssocCodes)
	{
		errorList.append(QString("Max_Assoc_Codes=%1 too small, actual is %2")
			.arg(MaxAssocCodes).arg(maxAssocCodes));
	}

	// setup indexes for bracketing codes
	// (will be set to -1 if missing - missing errors were recorded above)
	m_range[PlainWord_SearchType].beg = BegPlainWord_Code;
	m_range[PlainWord_SearchType].end = EndPlainWord_Code;
	m_range[ParenWord_SearchType].beg = BegParenWord_Code;
	m_range[ParenWord_SearchType].end = EndParenWord_Code;
	m_range[DataTypeWord_SearchType].beg = BegDataTypeWord_Code;
	m_range[DataTypeWord_SearchType].end = EndDataTypeWord_Code;
	m_range[Symbol_SearchType].beg = BegSymbol_Code;
	m_range[Symbol_SearchType].end = EndSymbol_Code;

	// check for missing bracketing codes and if properly positioned in table
	// (missing codes recorded above, however,
	// need to make sure all types were set, i.e. no missing assignments above)
	for (type = 0; type < sizeof_SearchType; type++)
	{
		if (m_range[type].beg > m_range[type].end)
		{
			// record bracket range error
			errorList.append(QString("Search type %1 indexes (%2, %3) not "
				"correct").arg(type).arg(m_range[type].beg)
				.arg(m_range[type].end));
		}
		else
		{
			// check to make sure no bracketing codes overlap
			for (int type2 = 0; type2 < sizeof_SearchType; type2++)
			{
				if (type != type2
					&& (m_range[type].beg > m_range[type2].beg
					&& m_range[type].beg < m_range[type2].end
					|| m_range[type].end > m_range[type2].beg
					&& m_range[type].end < m_range[type2].end))
				{
					// record bracket overlap error
					errorList.append(QString("Search type %1 indexes (%2, %3) "
						"overlap with search type %4").arg(type)
						.arg(m_range[type].beg).arg(m_range[type].end)
						.arg(type2));
				}
			}
		}
	}

	// return list of error messages if any
	return errorList;
}


//================================
//  CODE RELATED TABLE FUNCTIONS
//================================

// returns token type for code
TokenType Table::type(Code code) const
{
	return m_entry[code].type;
}

// returns data type for code
DataType Table::dataType(Code code) const
{
	return m_entry[code].dataType;
}

// returns multiple word or character type for code
Multiple Table::multiple(Code code) const
{
	return m_entry[code].multiple;
}

// returns primary name for code
const QString Table::name(Code code) const
{
	return m_entry[code].name;
}

// returns second name of a two word command for code
const QString Table::name2(Code code) const
{
	return m_entry[code].name2;
}

// returns the debug name for code, which is the primary name
// except for internal functions with multiple argument footprints
const QString Table::debugName(Code code) const
{
	QString name = m_entry[code].name2;
	if (name.isEmpty())
	{
		name = m_entry[code].name;
	}
	return name;
}

// returns the flags for code
int Table::flags(Code code) const
{
	return m_entry[code].flags;
}

// returns the token mode to set after command for code
TokenMode Table::tokenMode(Code code) const
{
	return m_entry[code].tokenMode;
}

// returns the unary operator code (or Null_Code if none) for code
Code Table::unaryCode(Code code) const
{
	ExprInfo *ei = m_entry[code].exprInfo;
	return ei == NULL ? Null_Code : ei->m_unaryCode;
}

// returns the precedence for code
int Table::precedence(Code code) const
{
	return m_entry[code].precedence;
}

// returns the number of operators (arguments) for code
int Table::nOperands(Code code) const
{
	return m_entry[code].exprInfo->m_nOperands;
}

// returns the data type for a specific operator for code
DataType Table::operandDataType(Code code, int operand) const
{
	return m_entry[code].exprInfo->m_operandDataType[operand];
}

// returns the number of associate codes for code
int Table::nAssocCodes(Code code) const
{
	return m_entry[code].exprInfo->m_nAssocCodes;
}

// returns the associate code for a specific index for code
Code Table::assocCode(Code code, int index) const
{
	return m_entry[code].exprInfo->m_assocCode[index];
}

// returns the start index of the secondary associated codes for code
int Table::assoc2Index(Code code) const
{
	return m_entry[code].exprInfo->m_assoc2Index;
}

// returns the secondary associated code for a specific index for code
Code Table::assoc2Code(Code code, int index) const
{
	return m_entry[code].exprInfo->m_assocCode[assoc2Index(code) + index];
}

// returns the number of string operands for code
int Table::nStrings(Code code) const
{
	return m_entry[code].exprInfo->m_nStrings;
}

// returns whether the code is a unary operator code
// (convenience function to avoid confusion)
bool Table::isUnaryOperator(Code code) const
{
	return code == unaryCode(code);
}

// returns the pointer to the token handler (if any) for code
TokenHandler Table::tokenHandler(Code code) const
{
	return m_entry[code].tokenHandler;
}

// returns the pointer to the command handler (if anyy) for code
CommandHandler Table::commandHandler(Code code) const
{
	return m_entry[code].commandHandler;
}


//=================================
//  TOKEN RELATED TABLE FUNCTIONS
//=================================

// returns the precedence of the code contained in a token
//
//   - the precedence is obtained from the token
//   - if this is -1 then the precedences if obtained for the token's code
int Table::precedence(Token *token) const
{
	int prec = token->precedence();
	return prec != -1 ? prec : precedence(token->code());
}

// returns the flags of the code contained in a token
//
//   - returns Null_Flag is the token does not contain a code
int Table::flags(Token *token) const
{
	// (non-table entry token types have no flags)
	return token->hasTableEntry() ? flags(token->code()) : Null_Flag;
}

// function to set a token for a code (code, token type and data type)
void Table::setToken(Token *token, Code code)
{
	token->setCode(code);
	token->setType(type(code));
	token->setDataType(dataType(code));
}

// function to create a new token and initialize it for a code
Token *Table::newToken(Code code)
{
	Token *token = new Token;	// allocates and initializes base members
	setToken(token, code);		// initializes code related members
	return token;
}


//============================
//  TABLE SPECIFIC FUNCTIONS
//============================

// search function to look for a string of a particular type in
// the Table, the search is case insensitive
//
//     - returns the index of the entry that is found
//     - returns -1 if the string was not found in the table

Code Table::search(SearchType type, const QStringRef &string) const
{
	Code i = m_range[type].beg;
	Code end = m_range[type].end;
	while (++i < end)
	{
		if (string.compare(m_entry[i].name, Qt::CaseInsensitive) == 0)
		{
			return i;
		}
	}
	return Invalid_Code;
}


// search function to look for a two word command in the Table,
// the search is case insensitive an only entries containing a second
// word are checked
//
//     - returns the index of the entry that is found
//     - returns -1 if the string was not found in the table

Code Table::search(const QStringRef &word1, const QStringRef &word2) const
{
	for (Code i = m_range[PlainWord_SearchType].beg;
		i < m_range[PlainWord_SearchType].end; i++)
	{
		if (m_entry[i].name2 != NULL
			&& word1.compare(m_entry[i].name, Qt::CaseInsensitive) == 0
			&& word2.compare(m_entry[i].name2, Qt::CaseInsensitive) == 0)
		{
			return i;
		}
	}
	return Invalid_Code;
}


// search function to look for a function matching the same function
// name as the index specified and matching the same number of
// arguments specified
//
//     - returns the index of the entry that is found
//     - returns -1 if the function with same noperands was not found
//     - case sensitive compare used (all entry names must match)
//     - name of function found at index specified
//     - search begins at entry after index
//     - search ends at end of section

Code Table::search(Code index, int nArguments) const
{
	for (Code i = index + 1; m_entry[i].name != NULL; i++)
	{
		if (m_entry[index].name.compare(m_entry[i].name) == 0
			&& nArguments == nOperands(i))
		{
			return i;
		}
	}
	return Invalid_Code;
}


// search function to look for a code matching the same operand
// data types as specified in the argument, the main code is
// checked first and then each of the associated codes
//
//     - returns the index of the code that is found
//     - returns -1 if there is not matching code
//     - the code specified must have associated codes
//     - the number of data types must match that of the code

Code Table::search(Code code, DataType *datatype) const
{
	if (match(code, datatype))
	{
		return code;  // main code matches
	}

	for (int n = nAssocCodes(code); --n >= 0;)
	{
		Code assoc_code = this->assocCode(code, n);
		if (match(assoc_code, datatype))
		{
			return assoc_code;  // associated code matches
		}
	}
	return Invalid_Code;  // no matches
}

// function to check to see if data types specified match the data
// types of the code at the index specified
//
//    - returns true if there is match, otherwise returns false

bool Table::match(Code code, DataType *datatype) const
{
	for (int n = nOperands(code); --n >= 0;)
	{
		if (datatype[n] != operandDataType(code, n))
		{
			return false;
		}
	}
	return true;
}


// end: table.cpp
