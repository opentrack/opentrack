#include "software-update-dialog.hpp"

update_dialog::update_dialog(QWidget* parent, update_query& q, const QString& new_version)
    : QDialog(parent), q(q)
{
    ui.setupUi(this);
    ui.ver_current->setText(const_cast<const char*>(opentrack_version));
    ui.ver_new->setTextFormat(Qt::RichText);
    ui.ver_new->setText("<a href='https://www.trackhat.org/trackhat-opentrack'>" + new_version + "</a>");
    ui.ver_new->setOpenExternalLinks(true);
    connect(ui.buttonBox, &QDialogButtonBox::clicked, this, &update_dialog::close);
}

void update_query::on_finished()
{
    if (abort)
        return;
    if (r->error() != QNetworkReply::NoError)
    {
        qDebug() << "update error" << r->errorString();
        return;
    }
    QString str(buf);
    QRegExp re("SOFTWARE-UPDATE-V2: ([a-zA-Z0-9_.-+]+)");
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

void update_query::maybe_show_dialog()
{
    QEventLoop ev;
    QTimer t{&ev}; t.setSingleShot(true);
    t.start(1000 * 10);
    QObject::connect(&t, &QTimer::timeout, [this, &ev] {
      abort = true;
      ev.quit();
    });

    r = qnam.get(QNetworkRequest(QStringLiteral("https://www.trackhat.org/thotversion")));

    QObject::connect(r, &QIODevice::readyRead, [this] { on_ready(); });
    QObject::connect(r, &QNetworkReply::finished, [&]() {
      on_finished();
      ev.quit();
    });
    ev.exec();
}
