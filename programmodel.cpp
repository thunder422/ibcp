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
	m_translator(new Translator(m_table))
{
	m_remDictionary = new Dictionary;
	m_constNumDictionary = new InfoDictionary<ConstNumInfo>;
	m_constStrDictionary = new ConstStrDictionary;

	m_varDblDictionary = new Dictionary;
	m_varIntDictionary = new Dictionary;
	m_varStrDictionary = new Dictionary;
}


ProgramModel::~ProgramModel(void)
{
	delete m_translator;

	delete m_remDictionary;
	delete m_constNumDictionary;
	delete m_constStrDictionary;

	delete m_varDblDictionary;
	delete m_varIntDictionary;
	delete m_varStrDictionary;

	// REMOVE need to delete all of the stored translated line lists
	for (int i = 0; i < m_lineInfo.count(); i++)
	{
		delete m_lineInfo.at(i).rpnList;
	}
}


QString ProgramModel::operandText(Code code, int operand) const
{
	OperandTextFunction operandText = m_table.operandTextFunction(code);
	return operandText == NULL ? "--": operandText(this, operand);
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
		if (m_table.hasFlag(code, HasOperand_Flag))
		{
			const QString operand = operandText(code, line[++i].operand());
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
	return m_lineInfo.count();
}


// slot function to receive program updates

void ProgramModel::update(int lineNumber, int linesDeleted, int linesInserted,
	QStringList lines)
{
	int i;
	int oldCount = m_lineInfo.count();
	int count = lines.count();
	m_errors.resetChange();
	for (i = 0; i < count - linesInserted; i++)
	{
		// update changed program lines if they actually changed
		if (updateLine(Change_Operation, lineNumber, lines.at(i)))
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
			updateLine(Remove_Operation, lineNumber);
		}
		endRemoveRows();
	}
	else if (linesInserted > 0)
	{
		// insert new lines into the program
		int lastLineNumber = lineNumber + linesInserted - 1;
		beginInsertRows(QModelIndex(), lineNumber, lastLineNumber);
		while (i < count)
		{
			updateLine(Insert_Operation, lineNumber++, lines.at(i++));
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
		// emit error list if changed
		emit errorListChanged(m_errors);
	}
}


bool ProgramModel::updateLine(Operation operation, int lineNumber,
	const QString &line)
{
	RpnList *rpnList;
	ProgramCode lineCode;
	ErrorItem errorItem;

	if (operation != Remove_Operation)
	{
		// compile line
		// if line has error, line code vector will be empty
		rpnList = m_translator->translate(line);
		if (rpnList->hasError())
		{
			errorItem = ErrorItem(ErrorItem::Input, lineNumber,
				rpnList->errorColumn(), rpnList->errorLength(),
				rpnList->errorMessage());
		}
	}

	if (operation == Change_Operation)
	{
		LineInfo &lineInfo = m_lineInfo[lineNumber];
		if (*rpnList == *lineInfo.rpnList)
		{
			return false;  // line not changed; nothing more to do here
		}

		// line is different, encode it if there was no translation error
		if (errorItem.isEmpty())
		{
			lineCode = encode(rpnList);
		}

		// REMOVE replace rpn list with the new list
		delete lineInfo.rpnList;
		lineInfo.rpnList = rpnList;

		updateError(lineNumber, lineInfo, errorItem, false);

		// derefence old line and replace with new line
		// (line gets deleted if new line has an error)
		dereference(lineInfo);
		m_code.replaceLine(lineInfo.offset, lineInfo.size, lineCode);
		m_lineInfo.replace(lineNumber, lineCode.size());
	}
	else if (operation == Insert_Operation)
	{
		LineInfo lineInfo;
		lineInfo.rpnList = rpnList;  // REMOVE replace rpn list
		lineInfo.errIndex = -1;

		updateError(lineNumber, lineInfo, errorItem, true);

		// encode line if there was no translation error
		if (errorItem.isEmpty())
		{
			lineCode = encode(rpnList);
		}

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
	}
	else if (operation == Remove_Operation)
	{
		LineInfo &lineInfo = m_lineInfo[lineNumber];

		removeError(lineNumber, lineInfo, true);

		// REMOVE delete rpn list
		delete lineInfo.rpnList;

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
			return;  // nothing more to do
		}
	}

	// find location in error list for line number
	int errIndex = m_errors.find(lineNumber);

	if (!errorItem.isEmpty())
	{
		// insert new error into error list
		m_errors.insert(errIndex, errorItem);

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
		}
	}
}


// function to remove an error from the list if line had an error
void ProgramModel::removeError(int lineNumber, LineInfo &lineInfo,
	bool lineDeleted)
{
	int errIndex;
	bool hadError;

	if (lineInfo.errIndex != -1)
	{
		errIndex = lineInfo.errIndex;

		// remove error (for changed line with no error or deleted line)
		m_errors.removeAt(errIndex);

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
		}
	}
}


// function to encode a translated RPN list
ProgramCode ProgramModel::encode(RpnList *input)
{
	ProgramCode programLine(input->codeSize());

	for (int i = 0; i < input->count(); i++)
	{
		Token *token = input->at(i)->token();
		programLine[token->index()].setInstruction(token->code(),
			token->subCodes());
		if (m_table.hasFlag(token, HasOperand_Flag))
		{
			quint16 operand;
			EncodeFunction encode = m_table.encodeFunction(token->code());
			if (encode == NULL)
			{
				// TODO for now set set operand to zero
				operand = 0;
			}
			else
			{
				operand = encode(this, token);
			}
			programLine[token->index() + 1].setOperand(operand);
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

// end: programmodel.cpp
