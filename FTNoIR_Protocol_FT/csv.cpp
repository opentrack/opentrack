/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2013	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*																				*
********************************************************************************/
#include "csv.h"
#include <QTextDecoder>

CSV::CSV(QIODevice * device)
{
	m_device = device;
	m_codec = QTextCodec::codecForLocale();
	m_pos = 0;
	m_rx = QRegExp("((?:(?:[^;\\n]*;?)|(?:\"[^\"]*\";?))*)\\n");
}
CSV::CSV(QString &string){	
	m_device = NULL;
	m_codec = QTextCodec::codecForLocale();
	m_string = string;	
	m_pos = 0;
	m_rx = QRegExp("((?:(?:[^;\\n]*;?)|(?:\"[^\"]*\";?))*)\\n");
}

CSV::~CSV()
{
	//delete m_codec;
}


void CSV::setCodec(const char* codecName){
	//delete m_codec;
	m_codec = QTextCodec::codecForName(codecName);
}

QString CSV::readLine(){
	QString line;

	if(m_string.isNull()){
		//READ DATA FROM DEVICE
		if(m_device && m_device->isReadable()){
			QTextDecoder dec(m_codec);
			m_string = dec.toUnicode(m_device->readAll());
		}else{
			return QString();
		}
	}

	//PARSE
	if((m_pos = m_rx.indexIn(m_string,m_pos)) != -1) {
		line = m_rx.cap(1);		
		m_pos += m_rx.matchedLength();
	}
	return line;
	
}
QStringList CSV::parseLine(){
	return parseLine(readLine());
}
QStringList CSV::parseLine(QString line){
	QStringList list;
	int pos2 = 0;
	QRegExp rx2("(?:\"([^\"]*)\";?)|(?:([^;]*);?)");
	if(line.size()<1){
		list << "";		
	}else while (line.size()>pos2 && (pos2 = rx2.indexIn(line, pos2)) != -1) {
		QString col;
		if(rx2.cap(1).size()>0)
			col = rx2.cap(1);
		else if(rx2.cap(2).size()>0)
			col = rx2.cap(2);
		
		list << col;			

		if(col.size())
			pos2 += rx2.matchedLength();
		else
			pos2++;
	}
	return list;
}