#pragma once
#include <QtGlobal>
#include <QObject>
#include <QStringList>
#include <QIODevice>
#include <QTextCodec>
#include <QRegExp>
#include <QtGlobal>

#include "export.hpp"

class OPENTRACK_CSV_EXPORT CSV
{
public:
    QString readLine();
    bool parseLine(QStringList& ret);

    void setCodec(const char* codecName);
    static bool getGameData(int gameID, unsigned char* table, QString& gamename);
private:
    CSV(QIODevice* device);

    QIODevice* m_device;
    QString m_string;
    int m_pos;

    static const QTextCodec* m_codec;
    static const QRegExp m_rx, m_rx2;
};
