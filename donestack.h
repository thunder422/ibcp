// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: donestack.h - translator done stack class header file
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
//	2013-07-21	initial version (parts removed from translator.h)

#ifndef DONESTACK_H
#define DONESTACK_H

#include <memory>
#include <stack>

class RpnItem;
using RpnItemPtr = std::shared_ptr<RpnItem>;
class Token;
using TokenPtr = Token *;


class DoneItem
{
public:
	RpnItemPtr rpnItem;			// pointer to RPN item
	TokenPtr first;				// operator token's first operand pointer
	TokenPtr last;				// operator token's last operand pointer

	DoneItem(RpnItemPtr _rpnItem) : rpnItem{_rpnItem}, first{nullptr},
		last{nullptr} {}
	DoneItem(RpnItemPtr _rpnItem, TokenPtr _last) : rpnItem{_rpnItem},
		first{nullptr}, last{_last} {}
	DoneItem(RpnItemPtr _rpnItem, TokenPtr _first, TokenPtr _last) :
		rpnItem{_rpnItem}, first{_first}, last{_last} {}
	~DoneItem()
	{
		deleteOpenParen(first);
		deleteCloseParen(last);
	}

	// delete token if is an open parentheses
	static void deleteOpenParen(TokenPtr token);
	void deleteOpenParen(void)
	{
		deleteOpenParen(first);
	}

	// delete token if is a close parentheses
	static void deleteCloseParen(TokenPtr token);
	void deleteCloseParen(void)
	{
		deleteCloseParen(last);
	}

	// replace the item's first and last operand token
	void replaceFirstLast(TokenPtr _first, TokenPtr _last)
	{
		deleteOpenParen(first);
		deleteCloseParen(last);
		first = _first;
		last = _last;
	}
};


using DoneStack = std::stack<DoneItem>;


#endif  // DONESTACK_H
