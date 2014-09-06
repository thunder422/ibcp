// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: programmodel.h - program model class source file
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
//	2013-03-15	initial version

#include "programmodel.h"
#include "recreator.h"
#include "rpnlist.h"
#include "table.h"
#include "translator.h"


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                           PROGRAM WORD FUNCTIONS                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


// function to return the debug text for an instruction word
QString ProgramWord::instructionDebugText(void) const
{
	Table &table = Table::instance();
	QString string;

	Code code = instructionCode();
	string += table.debugName(code);

	if (instructionHasSubCode(ProgramMask_SubCode))
	{
		string += '\'';
		if (instructionHasSubCode(Paren_SubCode))
		{
			string += ')';
		}
		if (instructionHasSubCode(Option_SubCode))
		{
			QString option = table.optionName(code);
			string += option.isEmpty() ? "BUG" : option;
		}
		if (instructionHasSubCode(Colon_SubCode))
		{
			string += ":";
		}
		string += '\'';
	}
	return string;
}


// function to return the debug text for an instruction word
QString ProgramWord::operandDebugText(QString text) const
{
	return QString("|%2:%3|").arg(operand()).arg(text);
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                        PROGRAM UNIT MODEL FUNCTIONS                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


ProgramModel::ProgramModel(QObject *parent) :
	QAbstractListModel(parent),
	m_table(Table::instance()),
	m_translator(new Translator),
	m_recreator(new Recreator)
{
	m_remDictionary = new Dictionary;
	m_constNumDictionary = new ConstNumDictionary;
	m_constStrDictionary = new ConstStrDictionary;

	m_varDblDictionary = new Dictionary;
	m_varIntDictionary = new Dictionary;
	m_varStrDictionary = new Dictionary;
}


ProgramModel::~ProgramModel(void)
{
	delete m_translator;
	delete m_recreator;

	delete m_remDictionary;
	delete m_constNumDictionary;
	delete m_constStrDictionary;

	delete m_varDblDictionary;
	delete m_varIntDictionary;
	delete m_varStrDictionary;
}


// NOTE temporary function to return the text for a program line
QString ProgramModel::debugText(int lineIndex, bool fullInfo) const
{
	QString string;

	const LineInfo &lineInfo = m_lineInfo[lineIndex];
	if (fullInfo)
	{
		string.append(QString("[%1").arg(lineInfo.offset));
		if (lineInfo.size > 0)
		{
			string.append(QString("-%1").arg(lineInfo.offset + lineInfo.size
				- 1));
		}
		string.append("]");
	}

	const ProgramWord *line = m_code.data() + m_lineInfo.at(lineIndex).offset;
	int count = m_lineInfo.at(lineIndex).size;
	for (int i = 0; i < count; i++)
	{
		if (i > 0 || fullInfo)
		{
			string += ' ';
		}
		string += QString("%1:%2").arg(i).arg(line[i].instructionDebugText());

		Code code = line[i].instructionCode();
		OperandTextFunction operandText = m_table.operandTextFunction(code);
		if (operandText != NULL)
		{
			const QString operand = operandText(this, line[++i].operand());
			string += QString(" %1:%2").arg(i)
				.arg(line[i].operandDebugText(operand));
		}
	}

	if (fullInfo && lineInfo.errIndex != -1)
	{
		const ErrorItem &errorItem = m_errors[lineInfo.errIndex];
		string.append(QString(" ERROR %1:%2 %3").arg(errorItem.column())
			.arg(errorItem.length()).arg(errorItem.message()));
	}

	return string;
}


// NOTE temporary function to return debug text of all the dictionary
QString ProgramModel::dictionariesDebugText(void)
{
	QString string;

	string.append(m_remDictionary->debugText("Remarks"));
	string.append(m_constNumDictionary->debugText("Number Constants"));
	string.append(m_constStrDictionary->debugText("String Constants"));
	string.append(m_varDblDictionary->debugText("Double Variables"));
	string.append(m_varIntDictionary->debugText("Integer Variables"));
	string.append(m_varStrDictionary->debugText("String Variables"));
	return string;
}


// function to return text for a given program line
QString ProgramModel::lineText(int lineIndex)
{
	QString string;

	LineInfo &lineInfo = m_lineInfo[lineIndex];
	if (lineInfo.errIndex == -1)
	{
		RpnList *rpnList = decode(lineInfo);
		string = m_recreator->recreate(rpnList);
		delete rpnList;
	}
	else  // line has error, return original text
	{
		string = lineInfo.text;
	}
	return string;
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
			return debugText(index.row(), true);
		}
	}
	return QVariant();
}


// function to return the number of program lines

int ProgramModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return m_lineInfo.count();
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
	QStringList lines)
{
	int i;
	int oldCount = m_lineInfo.count();
	int count = lines.count();
	for (i = 0; i < count - linesInserted; i++)
	{
		// update changed program lines if they actually changed
		if (updateLine(Operation::Change, lineNumber, lines.at(i)))
		{
			// need to emit signal that data changed
			QModelIndex index = this->index(lineNumber);
			emit dataChanged(index, index);
		}
		lineNumber++;
	}
	if (linesDeleted > 0)
	{
		// delete lines from the program
		int lastLineNumber = lineNumber + linesDeleted - 1;
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
			lineNumber = oldCount;
			operation = Operation::Append;
		}
		else  // insert new lines into the program
		{
			operation = Operation::Insert;
		}
		int lastLineNumber = lineNumber + linesInserted - 1;
		beginInsertRows(QModelIndex(), lineNumber, lastLineNumber);
		while (i < count)
		{
			updateLine(operation, lineNumber++, lines.at(i++));
		}
		endInsertRows();
	}

	if (m_lineInfo.count() != oldCount)
	{
		// emit new line count if changed
		emit lineCountChanged(m_lineInfo.count());
	}

	if (m_errors.hasChanged())
	{
		emit errorListChanged();
	}
}


bool ProgramModel::updateLine(Operation operation, int lineNumber,
	const QString &line)
{
	RpnList *rpnList;
	ProgramCode lineCode;
	ErrorItem errorItem;

	if (operation != Operation::Remove)
	{
		// compile line
		// if line has error, line code vector will be empty
		rpnList = m_translator->translate(line);
		if (rpnList->hasError())
		{
			errorItem = ErrorItem(ErrorItem::Type::Input, lineNumber,
				rpnList->errorColumn(), rpnList->errorLength(),
				rpnList->errorMessage());
		}
	}

	if (operation == Operation::Change)
	{
		LineInfo &lineInfo = m_lineInfo[lineNumber];
		RpnList *currentRpnList = decode(lineInfo);
		bool changed = *rpnList != *currentRpnList;
		delete currentRpnList;
		if (changed)
		{
			// derefence old line
			// (line gets deleted if new line has an error)
			dereference(lineInfo);

			// line is different, encode it if there was no translation error
			if (errorItem.isEmpty())
			{
				lineCode = encode(rpnList);
				lineInfo.text.clear();
			}
			else  // else store line text for error line
			{
				lineInfo.text = line;
			}

			updateError(lineNumber, lineInfo, errorItem, false);

			// replace with new line
			m_code.replaceLine(lineInfo.offset, lineInfo.size, lineCode);
			m_lineInfo.replace(lineNumber, lineCode.size());
		}

		if (errorItem.isEmpty())
		{
			emit programChange(lineNumber);  // only for non-error lines
		}
		delete rpnList;  // no longer needed
		return changed;
	}
	else if (operation == Operation::Append || operation == Operation::Insert)
	{
		LineInfo lineInfo;
		lineInfo.errIndex = -1;

		updateError(lineNumber, lineInfo, errorItem, true);

		// encode line if there was no translation error
		if (errorItem.isEmpty())
		{
			lineCode = encode(rpnList);
		}
		else  // else store line text for error line
		{
			lineInfo.text = line;
		}
		delete rpnList;  // no longer needed

		// find offset to insert line
		if (lineNumber < m_lineInfo.count())
		{
			lineInfo.offset = m_lineInfo.at(lineNumber).offset;
		}
		else  // append to end
		{
			lineInfo.offset = m_code.size();
		}
		lineInfo.size = lineCode.size();  // zero if line has error

		// insert line into code (nothing if line has error)
		m_code.insertLine(lineInfo.offset, lineCode);

		m_lineInfo.insert(lineNumber, lineInfo);
		if (operation == Operation::Append || errorItem.isEmpty())
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
		m_lineInfo.removeAt(lineNumber);
	}
	return true;
}


// function to update error into list if line has an error
void ProgramModel::updateError(int lineNumber, LineInfo &lineInfo,
	const ErrorItem &errorItem, bool lineInserted)
{
	if (!lineInserted)
	{
		if (errorItem.isEmpty())
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
	int errIndex = m_errors.find(lineNumber);

	if (!errorItem.isEmpty())
	{
		// insert new error into error list
		m_errors.insert(errIndex, errorItem);
		emit errorInserted(errIndex, errorItem);

		lineInfo.errIndex = errIndex++;
	}

	// loop thru rest of errors in list
	for (; errIndex < m_errors.count(); errIndex++)
	{
		if (!errorItem.isEmpty())
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
	int errIndex = m_errors.findIndex(lineNumber);
	if (errIndex != -1)  // line has error?
	{
		int errColumn = m_errors.at(errIndex).column();
		int errLength = m_errors.at(errIndex).length();

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
	int errIndex;
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
	int errIndex = m_errors.find(lineNumber);
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
	int errIndex = m_errors.find(lineNumber);
	if (errIndex >= m_errors.count()
		|| lineNumber < m_errors.at(errIndex).lineNumber()
		|| (lineNumber == m_errors.at(errIndex).lineNumber()
		&& column < m_errors.at(errIndex).column()
		+ m_errors.at(errIndex).length()))
	{
		// before current error, go to previous error
		errIndex--;
	}
	if (errIndex < 0)  // past first error?
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


// function to return error message for a line (blank if no error)
QString ProgramModel::errorMessage(int lineNumber) const
{
	int errIndex = m_errors.findIndex(lineNumber);
	return errIndex == -1 ? QString() : m_errors.at(errIndex).message();
}


// function to encode a translated RPN list
ProgramCode ProgramModel::encode(RpnList *input)
{
	ProgramCode programLine(input->codeSize());

	for (RpnItemPtr rpnItem : *input)
	{
		Token *token = rpnItem->token();
		programLine[token->index()].setInstruction(token->code(),
			token->subCodes());
		EncodeFunction encode = m_table.encodeFunction(token->code());
		if (encode != NULL)
		{
			programLine[token->index() + 1].setOperand(encode(this, token));
		}
	}
	return programLine;
}


// function to dereference contents of line to prepare for its removal
void ProgramModel::dereference(const LineInfo &lineInfo)
{
	ProgramWord *line = m_code.data() + lineInfo.offset;
	for (int i = 0; i < lineInfo.size; i++)
	{
		Code code = line[i].instructionCode();
		RemoveFunction remove = m_table.removeFunction(code);
		if (remove != NULL)
		{
			remove(this, line[++i].operand());
		}
	}
}


// function to decode a program line into an RPN list
RpnList *ProgramModel::decode(const LineInfo &lineInfo)
{
	RpnList *rpnList = new RpnList;
	ProgramWord *line = m_code.data() + lineInfo.offset;
	for (int i = 0; i < lineInfo.size; i++)
	{
		Token *token = new Token;
		token->setCode(line[i].instructionCode());
		token->addSubCode(line[i].instructionSubCode());

		OperandTextFunction operandText
			= m_table.operandTextFunction(token->code());
		if (operandText != NULL)
		{
			token->setString(operandText(this, line[++i].operand()));
		}
		rpnList->append(token);
	}
	return rpnList;
}


// end: programmodel.cpp
