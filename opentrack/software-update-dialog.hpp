#pragma once

#include <QtNetwork>
#include <QDialog>
#include "ui_software-update.h"

extern "C" const char* const opentrack_version;

struct update_query final
{
    explicit update_query(QWidget* parent) : parent(parent), qnam(parent) {}

    QWidget* parent;
    QNetworkReply* r = nullptr;
    QNetworkAccessManager qnam;
    QByteArray buf;
    bool abort = false;

    void on_finished();
    void on_ready() { buf.append(r->readAll()); }
    void maybe_show_dialog();
};

class update_dialog : public QDialog
{
    Q_OBJECT
    friend struct update_query;
private:
    Ui::UpdateDialog ui;
    update_query& q;
private slots:
    void close(QAbstractButton*) { QDialog::close(); }
public:
    update_dialog(QWidget* parent, update_query& q, const QString& new_version);
};

