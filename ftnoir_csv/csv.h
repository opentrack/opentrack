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
#include <QtGlobal>

#if defined(INSIDE_CSV)
#   define CSV_API Q_DECL_EXPORT
#else
#   define CSV_API Q_DECL_IMPORT
#endif

class CSV_API CSV
{
	/*Q_OBJECT*/

public:
	~CSV();

	QString readLine();
	QStringList parseLine();
	static QStringList parseLine(QString line);

	void setCodec(const char* codecName);
    static void getGameData( const QString& gameID, bool& tirviews, bool& dummy, unsigned char* table, QString& gamename);
private:
	QIODevice *m_device;
	QTextCodec *m_codec;
	QString m_string;
	int m_pos;
	QRegExp m_rx;
    CSV(QIODevice * device);
	CSV(QString &string);	
};

#endif // CSV_H
