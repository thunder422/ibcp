// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: programmodel.h - program model class source file
//	Copyright (C) 2013-2014  Thunder422
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
//	2013-03-15	initial version

#include "programmodel.h"
#include "rpnlist.h"
#include "statusmessage.h"
#include "table.h"


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                           PROGRAM WORD FUNCTIONS                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


// overloaded output stream operator for contents of an instruction word
std::ostream &operator<<(std::ostream &os, ProgramWord word)
{
	Table &table = Table::instance();

	Code code {word.instructionCode()};
	os << table.debugName(code).toStdString();

	if (word.instructionHasSubCode(ProgramMask_SubCode))
	{
		os << '\'';
		if (word.instructionHasSubCode(Paren_SubCode))
		{
			os << ')';
		}
		if (word.instructionHasSubCode(Option_SubCode))
		{
			std::string option {table.optionName(code).toStdString()};
			os << (option.empty() ? "BUG" : option);
		}
		if (word.instructionHasSubCode(Colon_SubCode))
		{
			os << ":";
		}
		os << '\'';
	}
	return os;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                        PROGRAM UNIT MODEL FUNCTIONS                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


ProgramModel::ProgramModel(QObject *parent) :
	QAbstractListModel(parent),
	m_table(Table::instance()),

	m_remDictionary {new Dictionary(CaseSensitive::Yes)},
	m_constNumDictionary {new ConstNumDictionary},
	m_constStrDictionary {new ConstStrDictionary},

	m_varDblDictionary {new Dictionary},
	m_varIntDictionary {new Dictionary},
	m_varStrDictionary {new Dictionary}
{

}


// NOTE temporary function to return the text for a program line
std::string ProgramModel::debugText(int lineIndex, bool fullInfo) const
{
	std::ostringstream oss;

	const LineInfo &lineInfo = m_lineInfo[lineIndex];
	if (fullInfo)
	{
		oss << '[' << lineInfo.offset;
		if (lineInfo.size > 0)
		{
			oss << '-' << lineInfo.offset + lineInfo.size - 1;
		}
		oss << ']';
	}

	auto line = m_code.begin() + m_lineInfo.offset(lineIndex);
	int size {m_lineInfo.size(lineIndex)};
	for (int i {}; i < size; i++)
	{
		if (i > 0 || fullInfo)
		{
			oss << ' ';
		}
		oss << i << ':' << line[i];

		Code code {line[i].instructionCode()};
		if (auto operandText = m_table.operandTextFunction(code))
		{
			const std::string operand {operandText(this, line[++i].operand())};
			oss << ' ' << i << ":|" << line[i].operand() << ':' << operand
				<< '|';
		}
	}

	if (fullInfo && lineInfo.errIndex != -1)
	{
		const ErrorItem &errorItem {m_errors[lineInfo.errIndex]};
		oss << " ERROR " << errorItem.column() << ':' << errorItem.length()
			<< ' ' << StatusMessage::text(errorItem.status()).toStdString();
	}

	return oss.str();
}


// function to return text for a given program line
std::string ProgramModel::lineText(int lineIndex)
{
	LineInfo &lineInfo = m_lineInfo[lineIndex];
	// return original test if line has error
	return lineInfo.errIndex == -1
		? Recreator{}(decode(lineInfo)) : lineInfo.text;
}


// function to return data for a given program line

QVariant ProgramModel::data(const QModelIndex &index, int role) const
{
	if (index.isValid())
	{
		if (role == Qt::TextAlignmentRole)
		{
			return int(Qt::AlignRight | Qt::AlignVCenter);
		}
		else if (role == Qt::DisplayRole)
		{
			return debugText(index.row(), true).c_str();
		}
	}
	return QVariant();
}


// function to return the number of program lines

int ProgramModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return m_lineInfo.size();
}


// function to reset to contents of the program model
void ProgramModel::clear(void)
{
	m_lineInfo.clear();
	m_code.clear();
	m_errors.clear();

	m_remDictionary->clear();
	m_constNumDictionary->clear();
	m_constStrDictionary->clear();

	m_varDblDictionary->clear();
	m_varIntDictionary->clear();
	m_varStrDictionary->clear();

	emit programCleared();
}


// slot function to receive program updates
//   - if lineNumber == 1 then append line to end of program
void ProgramModel::update(int lineNumber, int linesDeleted, int linesInserted,
	std::vector<std::string> &&lines)
{
	int i;
	int oldSize = m_lineInfo.size();
	int size = lines.size();
	for (i = 0; i < size - linesInserted; i++)
	{
		// update changed program lines if they actually changed
		if (updateLine(Operation::Change, lineNumber, std::move(lines[i])))
		{
			// need to emit signal that data changed
			QModelIndex index {this->index(lineNumber)};
			emit dataChanged(index, index);
		}
		lineNumber++;
	}
	if (linesDeleted > 0)
	{
		// delete lines from the program
		int lastLineNumber {lineNumber + linesDeleted - 1};
		beginRemoveRows(QModelIndex(), lineNumber, lastLineNumber);
		while (--linesDeleted >= 0)
		{
			updateLine(Operation::Remove, lineNumber);
		}
		endRemoveRows();
	}
	else if (linesInserted > 0)
	{
		Operation operation;
		if (lineNumber == -1)  // append to end of program?
		{
			lineNumber = oldSize;
			operation = Operation::Append;
		}
		else  // insert new lines into the program
		{
			operation = Operation::Insert;
		}
		int lastLineNumber {lineNumber + linesInserted - 1};
		beginInsertRows(QModelIndex(), lineNumber, lastLineNumber);
		while (i < size)
		{
			updateLine(operation, lineNumber++, std::move(lines[i++]));
		}
		endInsertRows();
	}

	if (m_lineInfo.size() != oldSize)
	{
		// emit new line count if changed
		emit lineCountChanged(m_lineInfo.size());
	}

	if (m_errors.hasChanged())
	{
		emit errorListChanged();
	}
}


bool ProgramModel::updateLine(Operation operation, int lineNumber,
	std::string &&line)
{
	RpnList rpnList;
	ProgramCode lineCode;
	ErrorItem errorItem;

	if (operation != Operation::Remove)
	try
	{
		rpnList = Translator{line}();
	}
	catch (Error &error)
	{
		errorItem = ErrorItem(ErrorItem::Type::Input, lineNumber, error.column,
			error.length, error.status);
	}

	if (operation == Operation::Change)
	{
		LineInfo &lineInfo = m_lineInfo[lineNumber];
		RpnList currentRpnList {decode(lineInfo)};
		bool changed {rpnList != currentRpnList || errorItem};
		if (changed)
		{
			// derefence old line
			// (line gets deleted if new line has an error)
			dereference(lineInfo);

			// line is different, encode it if there was no translation error
			if (!errorItem)
			{
				lineCode = encode(std::move(rpnList));
				lineInfo.text.clear();
			}
			else  // else store line text for error line
			{
				lineInfo.text = std::move(line);
			}

			updateError(lineNumber, lineInfo, errorItem, false);

			// replace with new line
			m_code.replaceLine(lineInfo.offset, lineInfo.size, lineCode);
			m_lineInfo.replace(lineNumber, lineCode.size());
		}

		if (!errorItem)
		{
			emit programChange(lineNumber);  // only for non-error lines
		}
		return changed;
	}
	else if (operation == Operation::Append || operation == Operation::Insert)
	{
		LineInfo lineInfo;
		lineInfo.errIndex = -1;

		updateError(lineNumber, lineInfo, errorItem, true);

		// encode line if there was no translation error
		if (!errorItem)
		{
			lineCode = encode(std::move(rpnList));
		}
		else  // else store line text for error line
		{
			lineInfo.text = std::move(line);
		}

		// find offset to insert line
		if (lineNumber < m_lineInfo.size())
		{
			lineInfo.offset = m_lineInfo.offset(lineNumber);
		}
		else  // append to end
		{
			lineInfo.offset = m_code.size();
		}
		lineInfo.size = lineCode.size();  // zero if line has error

		// insert line into code (nothing if line has error)
		m_code.insertLine(lineInfo.offset, lineCode);

		m_lineInfo.insert(lineNumber, lineInfo);
		if (operation == Operation::Append || !errorItem)
		{
			// only for appended lines or lines with no errors
			emit programChange(lineNumber);
		}
	}
	else if (operation == Operation::Remove)
	{
		LineInfo &lineInfo = m_lineInfo[lineNumber];

		removeError(lineNumber, lineInfo, true);

		// derefence old line and remove from code
		dereference(lineInfo);
		m_code.removeLine(lineInfo.offset, lineInfo.size);

		// remove from line info list
		m_lineInfo.erase(lineNumber);
	}
	return true;
}


// function to update error into list if line has an error
void ProgramModel::updateError(int lineNumber, LineInfo &lineInfo,
	const ErrorItem &errorItem, bool lineInserted)
{
	if (!lineInserted)
	{
		if (!errorItem)
		{
			removeError(lineNumber, lineInfo, false);
			return;  // nothing more to do
		}
		else if (lineInfo.errIndex != -1)  // had error?
		{
			// replace current error
			m_errors.replace(lineInfo.errIndex, errorItem);
			emit errorChanged(lineInfo.errIndex, errorItem);
			return;  // nothing more to do
		}
	}

	// find location in error list for line number
	auto errIndex = m_errors.find(lineNumber);

	if (errorItem)
	{
		// insert new error into error list
		m_errors.insert(errIndex, errorItem);
		emit errorInserted(errIndex, errorItem);

		lineInfo.errIndex = errIndex++;
	}

	// loop thru rest of errors in list
	for (; errIndex < m_errors.count(); errIndex++)
	{
		if (errorItem)
		{
			// adjust error index for inserted error
			m_lineInfo[m_errors[errIndex].lineNumber()].errIndex++;
		}
		if (lineInserted)
		{
			// adjust error line number for inserted line
			m_errors.incrementLineNumber(errIndex);
			emit errorChanged(errIndex, m_errors.at(errIndex));
		}
	}
}


// function to process a line that has been edited
void ProgramModel::lineEdited(int lineNumber, int column, bool atLineEnd,
	int charsAdded, int charsRemoved)
{
	auto errIndex = m_errors.findIndex(lineNumber);
	if (errIndex < m_errors.count())  // line has error?
	{
		int errColumn {m_errors.at(errIndex).column()};
		int errLength {m_errors.at(errIndex).length()};

		if (column - charsAdded <= errColumn + errLength)
		{
			// change is within or before error
			if ((atLineEnd && column + charsRemoved >= errColumn)
				|| column - charsAdded + charsRemoved > errColumn)
			{
				// change and error at end of line or change is within error
				// remove error
				LineInfo &lineInfo = m_lineInfo[lineNumber];
				removeError(lineNumber, lineInfo, false);
			}
			else  // cursor is before error, move error column
			{
				m_errors.moveColumn(errIndex, charsAdded - charsRemoved);
				emit errorChanged(errIndex, m_errors.at(errIndex));
			}
			m_errors.hasChanged();  // clear changed flag
			emit errorListChanged();
		}
	}
}


// function to remove an error from the list if line had an error
void ProgramModel::removeError(int lineNumber, LineInfo &lineInfo,
	bool lineDeleted)
{
	size_t errIndex;
	bool hadError;

	if (lineInfo.errIndex != -1)  // has error?
	{
		errIndex = lineInfo.errIndex;

		// remove error (for changed line with no error or deleted line)
		m_errors.removeAt(errIndex);
		emit errorRemoved(errIndex);

		lineInfo.errIndex = -1;
		hadError = true;
	}
	else if (lineDeleted)
	{
		// find location in error list for line number
		errIndex = m_errors.find(lineNumber);
		hadError = false;
	}
	else  // line not deleted and did not have an error
	{
		return;  // nothing more to do
	}

	// loop thru rest of errors in list
	for (; errIndex < m_errors.count(); errIndex++)
	{
		if (hadError)
		{
			// adjust error index for removed error
			m_lineInfo[m_errors[errIndex].lineNumber()].errIndex--;
		}
		if (lineDeleted)
		{
			// adjust error line number for deleted line
			m_errors.decrementLineNumber(errIndex);
			emit errorChanged(errIndex, m_errors.at(errIndex));
		}
	}
}


// function to find the next error from a given line number and column
//
//   - returns the line number and column of the next error
//   - returns whether the next error is the first error (wrapped)
//   - returns true upon success, false if already at single error

bool ProgramModel::errorFindNext(int &lineNumber, int &column, bool &wrapped)
	const
{
	wrapped = false;
	auto errIndex  = m_errors.find(lineNumber);
	if (errIndex >= m_errors.count()
		|| lineNumber > m_errors.at(errIndex).lineNumber()
		|| (lineNumber == m_errors.at(errIndex).lineNumber()
		&& column >= m_errors.at(errIndex).column()))
	{
		// past current error, go to next error
		errIndex++;
	}
	if (errIndex >= m_errors.count())  // past last error?
	{
		errIndex = 0;
		// check if already at the single error in the program
		if (m_errors.count() == 1 && lineNumber == m_errors.at(0).lineNumber()
			&& column >= m_errors.at(0).column()
			&& column < m_errors.at(0).column() + m_errors.at(0).length())
		{
			return false;
		}
		wrapped = true;
	}
	lineNumber = m_errors.at(errIndex).lineNumber();
	column = m_errors.at(errIndex).column();
	return true;
}


// function to find the previous error from a given line number and column
//
//   - returns the line number and column of the previous error
//   - returns whether the previous error is the last error (wrapped)
//   - returns true upon success, false if already at begin of single error

bool ProgramModel::errorFindPrev(int &lineNumber, int &column, bool &wrapped)
	const
{
	wrapped = false;
	auto errIndex = m_errors.find(lineNumber);
	if (errIndex >= m_errors.count()
		|| lineNumber < m_errors.at(errIndex).lineNumber()
		|| (lineNumber == m_errors.at(errIndex).lineNumber()
		&& column < m_errors.at(errIndex).column()
		+ m_errors.at(errIndex).length()))
	{
		// before current error, go to previous error
		errIndex--;
	}
	if (errIndex > m_errors.count())  // past first error?
	{
		errIndex = m_errors.count() - 1;
		// check if already at beginning of single error in the program
		if (m_errors.count() == 1
			&& lineNumber == m_errors.at(errIndex).lineNumber()
			&& column == m_errors.at(errIndex).column())
		{
			return false;
		}
		wrapped = true;
	}
	lineNumber = m_errors.at(errIndex).lineNumber();
	column = m_errors.at(errIndex).column();
	return true;
}


// function to return error status for a line (default if no error)
Status ProgramModel::errorStatus(int lineNumber) const
{
	auto errIndex = m_errors.findIndex(lineNumber);
	return errIndex == m_errors.count()
		? Status{} : m_errors.at(errIndex).status();
}


// function to encode a translated RPN list
ProgramCode ProgramModel::encode(RpnList &&input)
{
	ProgramCode programLine;

	for (RpnItemPtr rpnItem : input)
	{
		TokenPtr token {rpnItem->token()};
		programLine.emplace_back(token->code(), token->subCodes());
		if (auto encode = m_table.encodeFunction(token->code()))
		{
			programLine.emplace_back(encode(this, token));
		}
	}
	return programLine;
}


// function to dereference contents of line to prepare for its removal
void ProgramModel::dereference(const LineInfo &lineInfo)
{
	auto line = m_code.begin() + lineInfo.offset;
	for (int i {}; i < lineInfo.size; i++)
	{
		Code code {line[i].instructionCode()};
		if (auto remove = m_table.removeFunction(code))
		{
			remove(this, line[++i].operand());
		}
	}
}


// function to decode a program line into an RPN list
RpnList ProgramModel::decode(const LineInfo &lineInfo)
{
	RpnList rpnList;
	auto line = m_code.begin() + lineInfo.offset;
	for (int i {}; i < lineInfo.size; i++)
	{
		TokenPtr token {new Token};
		token->setCode(line[i].instructionCode());
		token->addSubCode(line[i].instructionSubCode());

		if (auto operandText = m_table.operandTextFunction(token->code()))
		{
			token->setString(operandText(this, line[++i].operand()));
		}
		rpnList.append(token);
	}
	return rpnList;
}


// end: programmodel.cpp
