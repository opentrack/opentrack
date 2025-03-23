#include "ftnoir_protocol_shm.h"
#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <qcombobox.h>
#include <qdebug.h>
#include <qdir.h>
#include <qradiobutton.h>

#include "api/plugin-api.hpp"
#include "options/tie.hpp"


FTControls::FTControls()
{
    ui.setupUi(this);

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &FTControls::doOK);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &FTControls::doCancel);
}


void FTControls::doOK()
{
    s.b->save();
    close();
}

void FTControls::doCancel()
{
    s.b->reload();
    close();
}
