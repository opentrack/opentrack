#pragma once
#include <QtGlobal>
#include <QObject>
#include <QStringList>
#include <QIODevice>
#include <QTextCodec>
#include <QRegExp>
#include <QtGlobal>

class CSV
{
public:
    QString readLine();
    bool parseLine(QStringList& ret);

    static bool getGameData(int gameID, unsigned char* table, QString& gamename);
private:
    CSV(QIODevice* device);

    QIODevice* m_device;
    QString m_string;
    int m_pos;

    static const QTextCodec* m_codec;
    static const QRegExp m_rx;
    static const QRegExp m_rx2; // Silly M$ compiler! It will generate an error if both of these variables are declared on the same line! (M$ Visual Studio Community 2015, Update 3)
};
