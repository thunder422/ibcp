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

	// NOTE need to delete all of the stored translated line lists
	for (int i = 0; i < m_lineInfo.count(); i++)
	{
		delete m_lineInfo.at(i).rpnList;
	}
}


QString ProgramModel::operandText(Code code, int operand)
{
	OperandTextFunction operandText = m_table.operandTextFunction(code);
	return operandText == NULL ? "--": operandText(this, operand);
}


// NOTE temporary function to return the text for a program line
QString ProgramModel::debugText(int lineIndex)
{
	QString string;

	ProgramWord *line = m_code.data() + m_lineInfo.at(lineIndex).offset;
	int count = m_lineInfo.at(lineIndex).size;
	for (int i = 0; i < count; i++)
	{
		if (i > 0)
		{
			string += ' ';
		}
		string += QString("%1:%2").arg(i).arg(line[i].instructionDebugText());

		Code code = line[i].instructionCode();
		if (m_table.hasFlag(code, HasOperand_Flag))
		{
			QString operand = operandText(code, line[++i].operand());
			string += QString(" %1:%2").arg(i)
				.arg(line[i].operandDebugText(operand));
		}
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
			RpnList *rpnList = m_lineInfo.at(index.row()).rpnList;
			if (!rpnList->hasError())
			{
				return rpnList->text();
			}
			else  // translate error occurred
			{
				return QString("ERROR %1:%2 %3").arg(rpnList->errorColumn())
					.arg(rpnList->errorLength()).arg(rpnList->errorMessage());
			}
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

	if (operation != Remove_Operation)
	{
		// compile line (for now just translate)
		rpnList = m_translator->translate(line);
		if (!rpnList->hasError())
		{
			lineCode = encode(rpnList);
		}
	}

	if (operation == Change_Operation)
	{
		LineInfo &lineInfo = m_lineInfo[lineNumber];
		if (*rpnList == *lineInfo.rpnList)
		{
			return false;  // line not changed; nothing more to do here
		}

		// replace the list with the new line
		delete lineInfo.rpnList;
		lineInfo.rpnList = rpnList;

		updateError(lineNumber, lineInfo, false);
	}
	else if (operation == Insert_Operation)
	{
		LineInfo lineInfo;
		lineInfo.rpnList = rpnList;
		lineInfo.errIndex = -1;

		updateError(lineNumber, lineInfo, true);

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

		// delete rpn list and remove from list
		delete lineInfo.rpnList;
		m_lineInfo.removeAt(lineNumber);
	}
	return true;
}


// function to update error into list if line has an error
void ProgramModel::updateError(int lineNumber, LineInfo &lineInfo,
	bool lineInserted)
{
	bool hasError = lineInfo.rpnList->hasError();
	if (!lineInserted)
	{
		if (!hasError)
		{
			removeError(lineNumber, lineInfo, false);
		}
		else if (lineInfo.errIndex != -1)  // had error?
		{
			// replace current error
			m_errors.replace(lineInfo.errIndex, ErrorItem(ErrorItem::Translator,
				lineNumber, lineInfo.rpnList));
		}
	}
	else  // line inserted, need to adjust all errors after new line
	{
		// find location in error list for line number
		int errIndex = m_errors.find(lineNumber);

		if (hasError)
		{
			// insert new error into error list
			m_errors.insert(errIndex, ErrorItem(ErrorItem::Translator,
				lineNumber, lineInfo.rpnList));

			lineInfo.errIndex = errIndex++;
		}

		// loop thru rest of errors in list
		for (; errIndex < m_errors.count(); errIndex++)
		{
			if (hasError)
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


// end: programmodel.cpp
