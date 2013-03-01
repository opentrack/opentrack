/*dummy CSV reader for QT4*/
/*version 0.1*/
/*11.1.2009*/
#ifndef CSV_H
#define CSV_H

//#include "myclass_api.h"

#include <QObject>
#include <QStringList>
#include <QIODevice>
#include <QTextCodec>
#include <QRegExp>

class /*MYCLASS_API*/ CSV /*: public QObject*/
{
	/*Q_OBJECT*/

public:
	CSV(QIODevice * device);
	CSV(QString &string);
	~CSV();

	QString readLine();
	QStringList parseLine();
	static QStringList parseLine(QString line);

	void setCodec(const char* codecName);
private:
	QIODevice *m_device;
	QTextCodec *m_codec;
	QString m_string;
	int m_pos;
	QRegExp m_rx;
	
};

#endif // CSV_H
