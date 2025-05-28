#include "csv.h"
#include "compat/library-path.hpp"
#include <utility>
#include <cstdio>
#include <QByteArray>
#include <QByteArrayView>
#include <QString>
#include <QStringDecoder>
#include <QFile>
#include <QDebug>

namespace {

void chomp(QString& str)
{
    if (!str.isEmpty() && str.back() == '\n')
    {
        str.chop(1);
        if (!str.isEmpty() && str.back() == '\r')
            str.chop(1);
    }
}

auto do_scanf(QLatin1StringView s, unsigned(&tmp)[8])
{
    unsigned fuzz[3];
    return std::sscanf(s.constData(),
                       "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                       fuzz + 2,
                       fuzz + 0,
                       tmp + 3, tmp + 2, tmp + 1, tmp + 0,
                       tmp + 7, tmp + 6, tmp + 5, tmp + 4,
                       fuzz + 1);
};

}

bool getGameData(int id, unsigned char* table, QString& gamename)
{
    for (int i = 0; i < 8; i++)
        table[i] = 0;

    if (id != 0)
        qDebug() << "csv: lookup game id" << id;

    static const QString csv_path(OPENTRACK_BASE_PATH +
                                  OPENTRACK_DOC_PATH "settings/facetracknoir supported games.csv");
    auto id_str = QString::number(id);

    QFile file(csv_path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "csv: can't open game list for freetrack protocol!";
        return false;
    }
    QStringDecoder decoder{QStringConverter::Encoding::Utf8};
    QStringList gameLine; gameLine.reserve(8);
    unsigned lineno = 1;
    // TODO QIODevice::readLineInto() is Qt 6.9 - sh 20250515
    char buf[256];
    qint64 sz;

    while ((sz = file.readLine(buf, sizeof buf)) > 0)
    {
        QString line = decoder.decode(QByteArrayView{buf, sz});

        if (line.isEmpty())
            break;
        chomp(line);
        if (line.isEmpty())
            continue;

        gameLine = line.split(';', Qt::SplitBehaviorFlags::KeepEmptyParts);

        // 0 No.
        // 1 Game Name
        // 2 Game Protocol
        // 3 Supported since version
        // 4 Verified
        // 5 By
        // 6 International ID
        // 7 FaceTrackNoIR ID

        if (gameLine.size() == 8)
        {
            if (gameLine.at(6).compare(id_str, Qt::CaseInsensitive) == 0)
            {
                const QString& proto = gameLine[3];
                QString& name = gameLine[1];
                const QByteArray id_cstr = gameLine[7].toLatin1();
                unsigned tmp[8] {};

                if (proto == QStringLiteral("V160") || id_cstr.length() != 22)
                    (void)0;
                else if (id_cstr.length() != 22 || do_scanf(QLatin1StringView(id_cstr), tmp) != 11)
                    qDebug() << "scanf failed" << lineno;
                else
                {
                    using uchar = unsigned char;
                    for (int i = 0; i < 8; i++)
                        table[i] = uchar(tmp[i]);
                }
                gamename = std::move(name);
                return true;
            }
        }
        else
            qDebug() << "csv: wrong column count on line" << lineno;

        lineno++;
    }

    if (id)
        qDebug() << "unknown game connected" << id;

    return false;
}
