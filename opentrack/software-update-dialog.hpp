#pragma once

#include <QtNetwork>
#include <QDialog>
#include <QTimer>
#include "ui_software-update.h"

extern "C" const char* const opentrack_version;

class update_query final : public QObject
{
    Q_OBJECT
public:
    explicit update_query(QWidget* parent) : QObject{parent} {}

    QNetworkReply* r = nullptr;
    QNetworkAccessManager qnam{this};
    QByteArray buf;
    QTimer t{this};

    void on_finished();
    void on_ready() { buf.append(r->readAll()); }
    void maybe_show_dialog();
};

class update_dialog : QDialog
{
    Q_OBJECT
    friend class update_query;
private:
    Ui::UpdateDialog ui;
    update_query& q;
    update_dialog(QWidget* parent, update_query& q, const QString& new_version);
};
