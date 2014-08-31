#include "ftnoir_protocol_vjoy_sf.h"
#include "facetracknoir/plugin-support.h"

VJoySFControls::VJoySFControls() : QWidget()
{
    ui.setupUi( this );
    connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
    connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
    connect(ui.sbvJoyID, SIGNAL(valueChanged(int)), this, SLOT(checkJoystickID(int)));

    tie_setting(s.intvJoyID, ui.sbvJoyID);

    checkJoystickID(s.intvJoyID);
}

void VJoySFControls::doOK() {
    save();
    this->close();
}

void VJoySFControls::doCancel() {
    revert();
    this->close();
}

void VJoySFControls::save() {
    s.b->save();
}

void VJoySFControls::revert() {
    s.b->revert();
}

void VJoySFControls::checkJoystickID(int i) {

    QString qsMessage, qsColor;

    VjdStat status = GetVJDStatus(i);
    switch (status)
    {
        case VJD_STAT_OWN:
            qsMessage = "";
            qsColor = "black";
            break;
        case VJD_STAT_FREE:
            qsMessage = tr("Free");
            qsColor = "green";
            break;
        case VJD_STAT_BUSY:
            qsMessage = tr("Busy");
            qsColor = "red";
            break;
        case VJD_STAT_MISS:
            qsMessage = tr("Unavailable");
            qsColor = "red";
            break;
        default:
            return;
    };

    ui.lblvJoyStatus->setText(qsMessage);
    ui.lblvJoyStatus->setStyleSheet(QString("QLabel { color: %1}").arg(qsColor));
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocolDialog* CALLING_CONVENTION GetDialog( )
{
    return new VJoySFControls;
}
