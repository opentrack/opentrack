#include "new_file_dialog.h"

new_file_dialog::new_file_dialog(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(ok_clicked()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(cancel_clicked()));
    setFixedSize(size());
}

bool new_file_dialog::is_ok(QString& name_)
{
    name_ = name;
    return ok;
}

void new_file_dialog::cancel_clicked() { close(); }

void new_file_dialog::ok_clicked()
{
    QString text = ui.lineEdit->text();
    text = text.replace('/', "");
    text = text.replace('\\', "");
    if (text != "" && !text.endsWith(".ini"))
        text += ".ini";
    if (text == "" || text == ".ini" || QFile(options::globals::ini_directory() + "/" + text).exists())
    {
        QMessageBox::warning(this,
                             tr("File exists"),
                             tr("This file already exists. Pick another name."),
                             QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }
    ok = true;
    close();
    name = text;
}
