// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: statusmessage.h - status message class header file
//	Copyright (C) 2014
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
//	2014-09-23	initial version (parts removed from token.h)

#ifndef STATUSMESSAGE_H
#define STATUSMESSAGE_H

#include <QCoreApplication>
#include <QString>

#include "ibcp.h"


class StatusMessage final  // final prevents derived classes
{
	Q_DECLARE_TR_FUNCTIONS(StatusMessage)

	StatusMessage() = delete;  // prevent instances
public:
	static const QString text(Status status);
};


#endif  // STATUSMESSAGE_H
