// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: token.h - token class header file
//	Copyright (C) 2012-2013  Thunder422
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
//	2012-11-02	initial version (parts removed from ibcp.h)

#ifndef TOKEN_H
#define TOKEN_H

#include <unordered_map>

#include <QCoreApplication>
#include <QStack>
#include <QString>
#include <QStringList>

#include "ibcp.h"


class Token
{
	Q_DECLARE_TR_FUNCTIONS(Token)

public:
	// changes to Type may require changes to initializers
	// for s_hasParen and s_precedence maps
	enum class Type
	{
		Command,
		Operator,
		IntFuncN,
		IntFuncP,
		Constant,
		DefFuncN,
		DefFuncP,
		NoParen,
		Paren,
		Error
	};

	enum class Status
	{
		Good,
		Done,
		Parser,
		ExpCmd,
		ExpExpr,
		ExpExprOrEnd,
		ExpOpOrEnd,
		ExpBinOpOrEnd,
		ExpEqualOrComma,
		ExpComma,
		ExpAssignItem,
		ExpOpOrComma,
		ExpOpCommaOrParen,
		ExpOpOrParen,
		ExpBinOpOrComma,
		ExpBinOpCommaOrParen,
		ExpBinOpOrParen,
		ExpNumExpr,
		ExpStrExpr,
		ExpSemiCommaOrEnd,
		ExpCommaSemiOrEnd,
		ExpSemiOrComma,
		ExpOpSemiOrComma,
		ExpDblVar,
		ExpIntVar,
		ExpStrVar,
		ExpVar,
		ExpStrItem,
		ExpEndStmt,
		ExpExprPfnOrEnd,
		ExpExprCommaPfnOrEnd,
		ExpOpSemiCommaOrEnd,
		ExpIntConst,
		// the following statuses used during development
		BUG_NotYetImplemented,
		BUG_HoldStackNotEmpty,
		BUG_DoneStackNotEmpty,
		BUG_DoneStackEmptyFindCode,
		BUG_UnexpectedCloseParen,
		BUG_DoneStackEmpty,
		BUG_InvalidDataType,
		BUG_UnexpToken,
		BUG_Debug1,
		BUG_Debug2,
		BUG_Debug3,
		BUG_Debug4,
		BUG_Debug5,
		BUG_Debug6,
		BUG_Debug7,
		BUG_Debug8,
		BUG_Debug9,
		BUG_Debug
	};

	explicit Token(int column = -1)
	{
		m_column = column;
		m_length = 1;
		m_reference = false;
		m_code = Invalid_Code;
		m_subCode = None_SubCode;
	}
	Token(const Token &token)  // copy constructor
	{
		*this = token;
	}
	~Token(void) {}

	bool operator==(const Token &other) const;
	bool operator!=(const Token &other) const
	{
		return !(*this == other);
	}
	// overload new and delete operators for memory management
	void *operator new(size_t size);
	void operator delete(void *ptr);

	// column and length access functions
	int column(void) const
	{
		return m_column;
	}
	int length(void) const
	{
		return m_length;
	}
	void setLength(int length)
	{
		m_length = length;
	}
	void addLengthToColumn(void)
	{
		m_column += m_length;
	}

	// type access functions
	Type type(void) const
	{
		return m_type;
	}
	void setType(Type type)
	{
		m_type = type;
	}
	bool isType(Type type) const
	{
		return type == m_type;
	}

	// data type access functions
	DataType dataType(bool actual = false) const
	{
		return !actual && hasSubCode(Double_SubCode)
			? DataType::Double : m_dataType;
	}
	void setDataType(DataType dataType)
	{
		m_dataType = dataType;
	}
	bool isDataType(DataType dataType) const
	{
		return dataType == m_dataType;
	}
	void setDataType(void);  // set default data type if not already set

	// string access function
	QString string(void) const
	{
		return m_string;
	}
	void setString(const QString &string)
	{
		m_string = string;
	}
	void setString(int pos, QChar character)
	{
		m_string[pos] = character;
	}
	int stringLength(void) const
	{
		return m_string.length();
	}

	// code access functions
	Code code(void) const
	{
		return m_code;
	}
	void setCode(Code code)
	{
		m_code = code;
	}
	Code nextCode(void)
	{
		return ++m_code;
	}
	bool isCode(Code code) const
	{
		return code == m_code;
	}
	bool hasValidCode(void) const
	{
		return m_code != Invalid_Code;
	}

	// reference access functions
	bool reference(void) const
	{
		return m_reference;
	}
	void setReference(bool reference = true)
	{
		m_reference = reference;
	}

	// sub-code access functions
	int hasSubCode(int subCode) const
	{
		return m_subCode & subCode;
	}
	void addSubCode(int subCode)
	{
		m_subCode |= subCode;
	}
	void removeSubCode(int subCode)
	{
		m_subCode &= ~subCode;
	}
	int subCodes(void) const
	{
		return m_subCode;
	}

	// value access functions
	double value(void) const
	{
		return m_value;
	}
	int valueInt(void) const
	{
		return m_valueInt;
	}
	void setValue(double value)
	{
		m_value = value;
	}
	void setValue(int value)
	{
		m_valueInt = value;
	}

	// index access functions
	int index(void)
	{
		return m_index;
	}
	void setIndex(int index)
	{
		m_index = index;
	}

	// set error functions
	void setError(const QString &msg, DataType dataType = DataType::Double)
	{
		m_length = 1;
		m_type = Type::Error;
		m_dataType = dataType;
		m_string = msg;
	}
	void setError(int column, const QString  &msg)
	{
		m_length = -column;  // assume length=1, specifies alternate column
		m_type = Type::Error;
		m_dataType = DataType::Double;
		m_string = msg;
	}
	void setError(const QString &msg, int len)
	{
		m_length = len;
		m_type = Type::Error;
		m_dataType = DataType::Double;
		m_string = msg;
	}

	// token information functions
	bool hasParen(void) const
	{
		return s_hasParen[m_type];
	}
	int precedence(void) const
	{
		return s_precendence[m_type];
	}
	bool isNull(void) const
	{
		return m_code == Null_Code;
	}

	// set length to include second token
	Token *setThrough(Token *token2)
	{
		m_length = token2->m_column - m_column + token2->m_length;
		return this;
	}

	// other functions
	Code convertCode(DataType toDataType) const;

	// recreate text for token
	QString text(bool withIndex = false);

	// static member functions
	static const QString message(Status status);
	static void reportErrors(void)
	{
		s_used.reportErrors();
		s_deleted.reportErrors();
	}

private:
	QString textOperand(bool withIndex);

	// static members
	static std::unordered_map<Type, bool, EnumClassHash> s_hasParen;
	static std::unordered_map<Type, int, EnumClassHash> s_precendence;

	class FreeStack : public QStack<Token *>
	{
	public:
		~FreeStack(void);
	};
	static FreeStack s_freeStack;

	class UsedVector : public QVector<Token *>
	{
	public:
		~UsedVector(void);
		void reportErrors(void);
	};
	static UsedVector s_used;

	class DeletedList : public QStringList
	{
	public:
		~DeletedList(void);
		void reportErrors(void);
	};
	static DeletedList s_deleted;

	// instance members
	int m_id;				// private ID (index) for detecting token leaks
	int m_column;			// start column of token
	int m_length;			// length of token
	Type m_type;			// type of the token
	DataType m_dataType;	// data type of token
	QString m_string;		// pointer to string of token
	Code m_code;			// internal code of token (index of TableEntry)
	bool m_reference;		// token is a reference flag
	int m_subCode;			// sub-code flags of token
	double m_value;			// double value for constant token
	int m_valueInt;			// integer value for constant token (if possible)
	int m_index;			// index within encoded program code line
};


// stack to hold tokens
class TokenStack : public QStack<Token *>
{
public:
	~TokenStack(void)
	{
		// delete any tokens left in stack when stack goes out of scope
		while (!isEmpty())
		{
			delete pop();
		}
	}
};


#endif  // TOKEN_H
