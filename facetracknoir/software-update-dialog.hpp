#pragma once

#include <QtNetwork>
#include <QDialog>
#include <QSettings>
#include <QString>
#include <QRegExp>
#include <functional>
#include "ui_software-update.h"
#include "opentrack/options.hpp"

extern "C" const char* opentrack_version;

class update_dialog : public QDialog
{
    Q_OBJECT
public:
    struct query
    {
        query(QWidget* parent) : parent(parent), qnam(parent) {}

        QWidget* parent;
        QNetworkAccessManager qnam;
        QByteArray buf;
        QNetworkReply* r;
        void on_finished()
        {
            if (r->error() != QNetworkReply::NoError)
            {
                qDebug() << "update error" << r->errorString();
                return;
            }
            QString str(buf);
            QRegExp re("OPENTRACK_VERSION([a-zA-Z0-9_.-]+)");
            int idx = re.indexIn(str);
            if (idx != -1)
            {
                str = re.cap(1);
                QSettings s(OPENTRACK_ORG);
                QString quiet_version = s.value("quiet-update-version").toString();

                if (!str.isEmpty() && str != opentrack_version && str != quiet_version)
                {
                    qDebug() << "update version" << str;
                    update_dialog dlg(parent, *this, str);
                    dlg.show();
                    dlg.raise();
                    dlg.exec();
                    if (dlg.ui.disable_reminder->isChecked())
                        s.setValue("quiet-update-version", str);
                }
            }
            buf.clear();
            r->deleteLater();
        }
        void on_ready()
        {
            buf.append(r->readAll());
        }
        void maybe_show_dialog()
        {
            static auto uri = QStringLiteral("http://www.trackhat.org/#!opentrackversion/c1oxn");
            r = qnam.get(QNetworkRequest(uri));

            QObject::connect(r, &QNetworkReply::finished, [&]() { on_finished(); });
            QObject::connect(r, &QNetworkReply::readyRead, [&]() { on_ready(); });
        }
    };
private:
    Ui::UpdateDialog ui;
    query& q;
private slots:
    void close(QAbstractButton*)
    {
        QDialog::close();
    }
public:
    update_dialog(QWidget* parent, query& q, const QString& new_version) : QDialog(parent), q(q)
    {
        ui.setupUi(this);
        ui.ver_current->setText(const_cast<const char*>(opentrack_version));
        ui.ver_new->setTextFormat(Qt::RichText);
        ui.ver_new->setText("<a href='http://www.trackhat.org/#!trackhat-opentrack/c1jzc'>" + new_version + "</a>");
        ui.ver_new->setOpenExternalLinks(true);
        connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close(QAbstractButton*)));
    }
};
