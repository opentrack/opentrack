#pragma once
#include <QtGlobal>
#include <QObject>
#include <QStringList>
#include <QIODevice>
#include <QTextCodec>
#include <QRegExp>
#include <QtGlobal>

#ifdef BUILD_opentrack_csv
#   define CSV_EXPORT Q_DECL_EXPORT
#else
#   define CSV_EXPORT Q_DECL_IMPORT
#endif

class CSV_EXPORT CSV
{
public:
    QString readLine();
    QStringList parseLine();
    static QStringList parseLine(QString line);

    void setCodec(const char* codecName);
    static bool getGameData(const int gameID, unsigned char* table, QString& gamename);
private:
    QIODevice *m_device;
    QTextCodec *m_codec;
    QString m_string;
    int m_pos;
    QRegExp m_rx;
    CSV(QIODevice * device);
    CSV(QString &string);
};
