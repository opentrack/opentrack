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
#define INSIDE_CSV
#include "csv.h"
#include <QTextDecoder>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>

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
}


void CSV::setCodec(const char* codecName){
	m_codec = QTextCodec::codecForName(codecName);
}

QString CSV::readLine(){
	QString line;

	if(m_string.isNull()){
		if(m_device && m_device->isReadable()){
			QTextDecoder dec(m_codec);
			m_string = dec.toUnicode(m_device->readAll());
		}else{
			return QString();
		}
	}
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

void CSV::getGameData( const int id, unsigned char* table, QString& gamename)
{
    QString gameID = QString::number(id);
    
    /* zero table first, in case unknown game is connecting */
    memset(table, 0, 8);
    QStringList gameLine;
	qDebug() << "getGameData, ID = " << gameID;

	QFile file(QCoreApplication::applicationDirPath() + "/settings/facetracknoir supported games.csv");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
		return;
	}
	CSV csv(&file);
	gameLine = csv.parseLine();

	while (gameLine.count() > 2) {
		//qDebug() << "Column 0: " << gameLine.at(0);		// No.
		//qDebug() << "Column 1: " << gameLine.at(1);		// Game Name
		//qDebug() << "Column 2: " << gameLine.at(2);		// Game Protocol
		//qDebug() << "Column 3: " << gameLine.at(3);		// Supported since version
		//qDebug() << "Column 4: " << gameLine.at(4);		// Verified
		//qDebug() << "Column 5: " << gameLine.at(5);		// By
		//qDebug() << "Column 6: " << gameLine.at(6);		// International ID
		//qDebug() << "Column 7: " << gameLine.at(7);		// FaceTrackNoIR ID
		
		if (gameLine.count() > 6) {
			if (gameLine.at(6).compare( gameID, Qt::CaseInsensitive ) == 0) {
                QByteArray id = gameLine.at(7).toLatin1();
                unsigned int tmp[8];
                unsigned int fuzz[3];
                if (gameLine.at(3) == QString("V160"))
                {
                    qDebug() << "no table";
                }
                else if (sscanf(id.constData(),
                           "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                           fuzz + 2,
                           fuzz + 0,
                           tmp + 3,
                           tmp + 2,
                           tmp + 1,
                           tmp + 0,
                           tmp + 7,
                           tmp + 6,
                           tmp + 5,
                           tmp + 4,
                           fuzz + 1) != 11)
                {
                    qDebug() << "scanf failed" << fuzz[0] << fuzz[1] << fuzz[2];
                }
                else
                    for (int i = 0; i < 8; i++)
                        table[i] = tmp[i];
                qDebug() << gameID << "game-id" << gameLine.at(7);
                gamename = gameLine.at(1);
                file.close();
                return;
			}
		}

		gameLine = csv.parseLine();
	}

    qDebug() << "Unknown game connected" << gameID;
    file.close();
}
