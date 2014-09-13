#pragma once
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
    static void getGameData(const int gameID, unsigned char* table, QString& gamename);
private:
	QIODevice *m_device;
	QTextCodec *m_codec;
	QString m_string;
	int m_pos;
	QRegExp m_rx;
    CSV(QIODevice * device);
	CSV(QString &string);	
};
