#include "facetracknoir/facetracknoir.h"
#include "facetracknoir/curve-config.h"
CurveConfigurationDialog::CurveConfigurationDialog(FaceTrackNoIR *ftnoir, QWidget *parent) :
    QWidget( parent, Qt::Dialog ), mainApp(ftnoir)
{
    ui.setupUi( this );

    // rest of mapping settings taken care of by options::value<t>
    mainApp->load_mappings();

    {
        struct {
            QFunctionConfigurator* qfc;
            Axis axis;
            bool altp;
        } qfcs[] =
        {
            { ui.rxconfig, Yaw, false },
            { ui.ryconfig, Pitch, false},
            { ui.rzconfig, Roll, false },
            { ui.txconfig, TX, false },
            { ui.tyconfig, TY, false },
            { ui.tzconfig, TZ, false },

            { ui.rxconfig_alt, Yaw, true },
            { ui.ryconfig_alt, Pitch, true},
            { ui.rzconfig_alt, Roll, true },
            { ui.txconfig_alt, TX, true },
            { ui.tyconfig_alt, TY, true },
            { ui.tzconfig_alt, TZ, true },
            { nullptr, Yaw, false }
        };

        for (int i = 0; qfcs[i].qfc; i++)
        {
            const bool altp = qfcs[i].altp;
            THeadPoseDOF& axis = mainApp->axis(qfcs[i].axis);
            FunctionConfig* conf = altp ? &axis.curveAlt : &axis.curve;
            const auto& name = qfcs[i].altp ? axis.name2 : axis.name1;

            qfcs[i].qfc->setConfig(conf, name);
        }
    }

    setFont(qApp->font());
    QPoint offsetpos(120, 30);
    this->move(parent->pos() + offsetpos);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(mainApp->s.a_x.altp, ui.tx_altp);
    tie_setting(mainApp->s.a_y.altp, ui.ty_altp);
    tie_setting(mainApp->s.a_z.altp, ui.tz_altp);
    tie_setting(mainApp->s.a_yaw.altp, ui.rx_altp);
    tie_setting(mainApp->s.a_pitch.altp, ui.ry_altp);
    tie_setting(mainApp->s.a_roll.altp, ui.rz_altp);

    tie_setting(mainApp->s.tcomp_p, ui.tcomp_enable);
    tie_setting(mainApp->s.tcomp_tz, ui.tcomp_rz);

    tie_setting(mainApp->s.a_x.zero, ui.pos_tx);
    tie_setting(mainApp->s.a_y.zero, ui.pos_ty);
    tie_setting(mainApp->s.a_z.zero, ui.pos_tz);
    tie_setting(mainApp->s.a_yaw.zero, ui.pos_rx);
    tie_setting(mainApp->s.a_pitch.zero, ui.pos_ry);
    tie_setting(mainApp->s.a_roll.zero, ui.pos_rz);

    tie_setting(mainApp->s.a_yaw.invert, ui.invert_yaw);
    tie_setting(mainApp->s.a_pitch.invert, ui.invert_pitch);
    tie_setting(mainApp->s.a_roll.invert, ui.invert_roll);
    tie_setting(mainApp->s.a_x.invert, ui.invert_x);
    tie_setting(mainApp->s.a_y.invert, ui.invert_y);
    tie_setting(mainApp->s.a_z.invert, ui.invert_z);

    tie_setting(mainApp->s.a_yaw.src, ui.src_yaw);
    tie_setting(mainApp->s.a_pitch.src, ui.src_pitch);
    tie_setting(mainApp->s.a_roll.src, ui.src_roll);
    tie_setting(mainApp->s.a_x.src, ui.src_x);
    tie_setting(mainApp->s.a_y.src, ui.src_y);
    tie_setting(mainApp->s.a_z.src, ui.src_z);
}

void CurveConfigurationDialog::doOK() {
    save();
    this->close();
}

void CurveConfigurationDialog::doCancel() {
    mainApp->load_mappings();
    this->close();
}

void CurveConfigurationDialog::save() {
    mainApp->save_mappings();
}
