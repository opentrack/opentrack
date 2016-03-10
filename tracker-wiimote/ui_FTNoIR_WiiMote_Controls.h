/********************************************************************************
** Form generated from reading UI file 'FTNoIR_WiiMote_Controls.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FTNOIR_WIIMOTE_CONTROLS_H
#define UI_FTNOIR_WIIMOTE_CONTROLS_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_UICPTClientControls
{
public:
    QGridLayout *gridLayout_9;
    QTabWidget *tabWidget;
    QWidget *tab_2;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QLabel *label_6;
    QSpinBox *res_y_spin;
    QLabel *label_4;
    QLabel *label_36;
    QSpinBox *fov;
    QCheckBox *dynamic_pose;
    QLabel *label_5;
    QSpinBox *init_phase_timeout;
    QComboBox *camdevice_combo;
    QLabel *label_41;
    QSpinBox *res_x_spin;
    QLabel *label_2;
    QLabel *label_9;
    QPushButton *camera_settings;
    QWidget *tab_4;
    QGridLayout *gridLayout_3;
    QTabWidget *model_tabs;
    QWidget *tab_5;
    QGridLayout *gridLayout_6;
    QGroupBox *groupBox_8;
    QSpinBox *clip_tlength_spin;
    QSpinBox *clip_bheight_spin;
    QLabel *label_44;
    QLabel *label_50;
    QSpinBox *clip_blength_spin;
    QSpinBox *clip_theight_spin;
    QLabel *label_51;
    QLabel *label_45;
    QWidget *tab_6;
    QVBoxLayout *verticalLayout_14;
    QGroupBox *groupBox_9;
    QLabel *label_46;
    QLabel *label_48;
    QSpinBox *cap_length_spin;
    QLabel *label_47;
    QSpinBox *cap_width_spin;
    QLabel *label_49;
    QSpinBox *cap_height_spin;
    QWidget *tab_7;
    QGridLayout *gridLayout;
    QGroupBox *groupBox_7;
    QGridLayout *gridLayout_5;
    QLabel *label_57;
    QSpinBox *m2y_spin;
    QLabel *label_63;
    QSpinBox *m1x_spin;
    QSpinBox *m1y_spin;
    QSpinBox *m2z_spin;
    QSpinBox *m2x_spin;
    QLabel *label_56;
    QSpinBox *m1z_spin;
    QLabel *label_70;
    QLabel *label_67;
    QLabel *label_64;
    QLabel *label_60;
    QLabel *label_69;
    QLabel *label_58;
    QGroupBox *groupBox_10;
    QGridLayout *gridLayout_4;
    QFrame *frame_2;
    QGridLayout *gridLayout_11;
    QLabel *label_61;
    QSpinBox *tx_spin;
    QLabel *label_62;
    QSpinBox *ty_spin;
    QLabel *label_66;
    QSpinBox *tz_spin;
    QFrame *frame;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_59;
    QPushButton *tcalib_button;
    QWidget *tab_3;
    QGridLayout *gridLayout_8;
    QLabel *label_10;
    QGroupBox *groupBox_5;
    QGridLayout *gridLayout_10;
    QLabel *label_3;
    QDialogButtonBox *buttonBox;
    QLabel *label_38;
    QLabel *pointinfo_label;
    QLabel *caminfo_label;

    void setupUi(QWidget *UICPTClientControls)
    {
        if (UICPTClientControls->objectName().isEmpty())
            UICPTClientControls->setObjectName(QStringLiteral("UICPTClientControls"));
        UICPTClientControls->setWindowModality(Qt::NonModal);
        UICPTClientControls->resize(553, 588);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(UICPTClientControls->sizePolicy().hasHeightForWidth());
        UICPTClientControls->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QStringLiteral(":/Resources/Logo_IR.png"), QSize(), QIcon::Normal, QIcon::Off);
        UICPTClientControls->setWindowIcon(icon);
        UICPTClientControls->setLayoutDirection(Qt::LeftToRight);
        UICPTClientControls->setAutoFillBackground(false);
        gridLayout_9 = new QGridLayout(UICPTClientControls);
        gridLayout_9->setObjectName(QStringLiteral("gridLayout_9"));
        gridLayout_9->setSizeConstraint(QLayout::SetFixedSize);
        tabWidget = new QTabWidget(UICPTClientControls);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy1);
        tabWidget->setMinimumSize(QSize(0, 0));
        tabWidget->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        verticalLayout = new QVBoxLayout(tab_2);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(tab_2);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout_2->addWidget(label_6, 5, 0, 1, 1);

        res_y_spin = new QSpinBox(groupBox);
        res_y_spin->setObjectName(QStringLiteral("res_y_spin"));
        res_y_spin->setEnabled(false);
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(res_y_spin->sizePolicy().hasHeightForWidth());
        res_y_spin->setSizePolicy(sizePolicy2);
        res_y_spin->setMaximum(2000);
        res_y_spin->setSingleStep(10);

        gridLayout_2->addWidget(res_y_spin, 2, 1, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(label_4, 3, 0, 1, 1);

        label_36 = new QLabel(groupBox);
        label_36->setObjectName(QStringLiteral("label_36"));
        sizePolicy3.setHeightForWidth(label_36->sizePolicy().hasHeightForWidth());
        label_36->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(label_36, 1, 0, 1, 1);

        fov = new QSpinBox(groupBox);
        fov->setObjectName(QStringLiteral("fov"));
        fov->setEnabled(false);
        fov->setMinimum(10);
        fov->setMaximum(90);

        gridLayout_2->addWidget(fov, 3, 1, 1, 1);

        dynamic_pose = new QCheckBox(groupBox);
        dynamic_pose->setObjectName(QStringLiteral("dynamic_pose"));
        dynamic_pose->setEnabled(true);

        gridLayout_2->addWidget(dynamic_pose, 4, 1, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));
        sizePolicy3.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(label_5, 4, 0, 1, 1);

        init_phase_timeout = new QSpinBox(groupBox);
        init_phase_timeout->setObjectName(QStringLiteral("init_phase_timeout"));
        init_phase_timeout->setEnabled(true);
        init_phase_timeout->setMinimum(1);
        init_phase_timeout->setMaximum(10000);

        gridLayout_2->addWidget(init_phase_timeout, 5, 1, 1, 1);

        camdevice_combo = new QComboBox(groupBox);
        camdevice_combo->setObjectName(QStringLiteral("camdevice_combo"));
        camdevice_combo->setEnabled(false);
        sizePolicy2.setHeightForWidth(camdevice_combo->sizePolicy().hasHeightForWidth());
        camdevice_combo->setSizePolicy(sizePolicy2);
        camdevice_combo->setCurrentText(QStringLiteral(""));
        camdevice_combo->setMinimumContentsLength(10);

        gridLayout_2->addWidget(camdevice_combo, 0, 1, 1, 1);

        label_41 = new QLabel(groupBox);
        label_41->setObjectName(QStringLiteral("label_41"));
        sizePolicy3.setHeightForWidth(label_41->sizePolicy().hasHeightForWidth());
        label_41->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(label_41, 2, 0, 1, 1);

        res_x_spin = new QSpinBox(groupBox);
        res_x_spin->setObjectName(QStringLiteral("res_x_spin"));
        res_x_spin->setEnabled(false);
        sizePolicy2.setHeightForWidth(res_x_spin->sizePolicy().hasHeightForWidth());
        res_x_spin->setSizePolicy(sizePolicy2);
        res_x_spin->setMaximum(2000);
        res_x_spin->setSingleStep(10);

        gridLayout_2->addWidget(res_x_spin, 1, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));
        sizePolicy3.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy3);

        gridLayout_2->addWidget(label_2, 0, 0, 1, 1);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QStringLiteral("label_9"));

        gridLayout_2->addWidget(label_9, 6, 0, 1, 1);

        camera_settings = new QPushButton(groupBox);
        camera_settings->setObjectName(QStringLiteral("camera_settings"));
        camera_settings->setEnabled(false);
        QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Maximum);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(camera_settings->sizePolicy().hasHeightForWidth());
        camera_settings->setSizePolicy(sizePolicy4);

        gridLayout_2->addWidget(camera_settings, 6, 1, 1, 1);


        verticalLayout->addWidget(groupBox);

        tabWidget->addTab(tab_2, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QStringLiteral("tab_4"));
        gridLayout_3 = new QGridLayout(tab_4);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        model_tabs = new QTabWidget(tab_4);
        model_tabs->setObjectName(QStringLiteral("model_tabs"));
        QSizePolicy sizePolicy5(QSizePolicy::Preferred, QSizePolicy::Minimum);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(model_tabs->sizePolicy().hasHeightForWidth());
        model_tabs->setSizePolicy(sizePolicy5);
        model_tabs->setTabShape(QTabWidget::Rounded);
        model_tabs->setUsesScrollButtons(false);
        model_tabs->setDocumentMode(false);
        model_tabs->setTabsClosable(false);
        tab_5 = new QWidget();
        tab_5->setObjectName(QStringLiteral("tab_5"));
        gridLayout_6 = new QGridLayout(tab_5);
        gridLayout_6->setObjectName(QStringLiteral("gridLayout_6"));
        groupBox_8 = new QGroupBox(tab_5);
        groupBox_8->setObjectName(QStringLiteral("groupBox_8"));
        sizePolicy5.setHeightForWidth(groupBox_8->sizePolicy().hasHeightForWidth());
        groupBox_8->setSizePolicy(sizePolicy5);
        groupBox_8->setMinimumSize(QSize(331, 208));
        clip_tlength_spin = new QSpinBox(groupBox_8);
        clip_tlength_spin->setObjectName(QStringLiteral("clip_tlength_spin"));
        clip_tlength_spin->setGeometry(QRect(70, 35, 100, 22));
        clip_tlength_spin->setMinimum(-65535);
        clip_tlength_spin->setMaximum(65535);
        clip_bheight_spin = new QSpinBox(groupBox_8);
        clip_bheight_spin->setObjectName(QStringLiteral("clip_bheight_spin"));
        clip_bheight_spin->setGeometry(QRect(150, 130, 100, 22));
        clip_bheight_spin->setMinimum(-65535);
        clip_bheight_spin->setMaximum(65535);
        label_44 = new QLabel(groupBox_8);
        label_44->setObjectName(QStringLiteral("label_44"));
        label_44->setGeometry(QRect(65, 55, 71, 111));
        label_44->setPixmap(QPixmap(QString::fromUtf8(":/Resources/clip_side.png")));
        label_50 = new QLabel(groupBox_8);
        label_50->setObjectName(QStringLiteral("label_50"));
        label_50->setGeometry(QRect(20, 40, 46, 13));
        clip_blength_spin = new QSpinBox(groupBox_8);
        clip_blength_spin->setObjectName(QStringLiteral("clip_blength_spin"));
        clip_blength_spin->setGeometry(QRect(50, 160, 100, 22));
        clip_blength_spin->setMinimum(-65535);
        clip_blength_spin->setMaximum(65535);
        clip_theight_spin = new QSpinBox(groupBox_8);
        clip_theight_spin->setObjectName(QStringLiteral("clip_theight_spin"));
        clip_theight_spin->setGeometry(QRect(150, 70, 100, 22));
        clip_theight_spin->setMinimum(-65535);
        clip_theight_spin->setMaximum(65535);
        label_51 = new QLabel(groupBox_8);
        label_51->setObjectName(QStringLiteral("label_51"));
        label_51->setGeometry(QRect(290, 40, 46, 13));
        label_45 = new QLabel(groupBox_8);
        label_45->setObjectName(QStringLiteral("label_45"));
        label_45->setGeometry(QRect(300, 70, 21, 111));
        label_45->setPixmap(QPixmap(QString::fromUtf8(":/Resources/clip_front.png")));

        gridLayout_6->addWidget(groupBox_8, 0, 0, 1, 1);

        model_tabs->addTab(tab_5, QString());
        tab_6 = new QWidget();
        tab_6->setObjectName(QStringLiteral("tab_6"));
        verticalLayout_14 = new QVBoxLayout(tab_6);
        verticalLayout_14->setObjectName(QStringLiteral("verticalLayout_14"));
        groupBox_9 = new QGroupBox(tab_6);
        groupBox_9->setObjectName(QStringLiteral("groupBox_9"));
        groupBox_9->setMinimumSize(QSize(331, 208));
        label_46 = new QLabel(groupBox_9);
        label_46->setObjectName(QStringLiteral("label_46"));
        label_46->setGeometry(QRect(100, 60, 111, 81));
        label_46->setPixmap(QPixmap(QString::fromUtf8(":/Resources/cap_side.png")));
        label_48 = new QLabel(groupBox_9);
        label_48->setObjectName(QStringLiteral("label_48"));
        label_48->setGeometry(QRect(20, 40, 46, 13));
        cap_length_spin = new QSpinBox(groupBox_9);
        cap_length_spin->setObjectName(QStringLiteral("cap_length_spin"));
        cap_length_spin->setGeometry(QRect(90, 40, 101, 22));
        cap_length_spin->setMinimum(-65535);
        cap_length_spin->setMaximum(65535);
        label_47 = new QLabel(groupBox_9);
        label_47->setObjectName(QStringLiteral("label_47"));
        label_47->setGeometry(QRect(220, 100, 81, 81));
        label_47->setPixmap(QPixmap(QString::fromUtf8(":/Resources/cap_front.png")));
        cap_width_spin = new QSpinBox(groupBox_9);
        cap_width_spin->setObjectName(QStringLiteral("cap_width_spin"));
        cap_width_spin->setGeometry(QRect(240, 70, 81, 22));
        cap_width_spin->setMinimum(-65535);
        cap_width_spin->setMaximum(65535);
        label_49 = new QLabel(groupBox_9);
        label_49->setObjectName(QStringLiteral("label_49"));
        label_49->setGeometry(QRect(240, 40, 46, 13));
        cap_height_spin = new QSpinBox(groupBox_9);
        cap_height_spin->setObjectName(QStringLiteral("cap_height_spin"));
        cap_height_spin->setGeometry(QRect(20, 90, 81, 22));
        cap_height_spin->setMinimum(-65535);
        cap_height_spin->setMaximum(65535);

        verticalLayout_14->addWidget(groupBox_9);

        model_tabs->addTab(tab_6, QString());
        tab_7 = new QWidget();
        tab_7->setObjectName(QStringLiteral("tab_7"));
        gridLayout = new QGridLayout(tab_7);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        groupBox_7 = new QGroupBox(tab_7);
        groupBox_7->setObjectName(QStringLiteral("groupBox_7"));
        gridLayout_5 = new QGridLayout(groupBox_7);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        label_57 = new QLabel(groupBox_7);
        label_57->setObjectName(QStringLiteral("label_57"));
        QSizePolicy sizePolicy6(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(label_57->sizePolicy().hasHeightForWidth());
        label_57->setSizePolicy(sizePolicy6);

        gridLayout_5->addWidget(label_57, 3, 1, 1, 1);

        m2y_spin = new QSpinBox(groupBox_7);
        m2y_spin->setObjectName(QStringLiteral("m2y_spin"));
        m2y_spin->setMinimum(-65535);
        m2y_spin->setMaximum(65535);

        gridLayout_5->addWidget(m2y_spin, 2, 5, 1, 1);

        label_63 = new QLabel(groupBox_7);
        label_63->setObjectName(QStringLiteral("label_63"));
        sizePolicy6.setHeightForWidth(label_63->sizePolicy().hasHeightForWidth());
        label_63->setSizePolicy(sizePolicy6);

        gridLayout_5->addWidget(label_63, 1, 1, 1, 1);

        m1x_spin = new QSpinBox(groupBox_7);
        m1x_spin->setObjectName(QStringLiteral("m1x_spin"));
        m1x_spin->setMinimum(-65535);
        m1x_spin->setMaximum(65535);

        gridLayout_5->addWidget(m1x_spin, 1, 2, 1, 1);

        m1y_spin = new QSpinBox(groupBox_7);
        m1y_spin->setObjectName(QStringLiteral("m1y_spin"));
        m1y_spin->setMinimum(-65535);
        m1y_spin->setMaximum(65535);

        gridLayout_5->addWidget(m1y_spin, 2, 2, 1, 1);

        m2z_spin = new QSpinBox(groupBox_7);
        m2z_spin->setObjectName(QStringLiteral("m2z_spin"));
        m2z_spin->setMinimum(-65535);
        m2z_spin->setMaximum(65535);

        gridLayout_5->addWidget(m2z_spin, 3, 5, 1, 1);

        m2x_spin = new QSpinBox(groupBox_7);
        m2x_spin->setObjectName(QStringLiteral("m2x_spin"));
        m2x_spin->setMinimum(-65535);
        m2x_spin->setMaximum(65535);

        gridLayout_5->addWidget(m2x_spin, 1, 5, 1, 1);

        label_56 = new QLabel(groupBox_7);
        label_56->setObjectName(QStringLiteral("label_56"));
        sizePolicy.setHeightForWidth(label_56->sizePolicy().hasHeightForWidth());
        label_56->setSizePolicy(sizePolicy);

        gridLayout_5->addWidget(label_56, 0, 0, 1, 6);

        m1z_spin = new QSpinBox(groupBox_7);
        m1z_spin->setObjectName(QStringLiteral("m1z_spin"));
        m1z_spin->setMinimum(-65535);
        m1z_spin->setMaximum(65535);

        gridLayout_5->addWidget(m1z_spin, 3, 2, 1, 1);

        label_70 = new QLabel(groupBox_7);
        label_70->setObjectName(QStringLiteral("label_70"));
        sizePolicy6.setHeightForWidth(label_70->sizePolicy().hasHeightForWidth());
        label_70->setSizePolicy(sizePolicy6);

        gridLayout_5->addWidget(label_70, 2, 4, 1, 1);

        label_67 = new QLabel(groupBox_7);
        label_67->setObjectName(QStringLiteral("label_67"));
        sizePolicy6.setHeightForWidth(label_67->sizePolicy().hasHeightForWidth());
        label_67->setSizePolicy(sizePolicy6);

        gridLayout_5->addWidget(label_67, 1, 4, 1, 1);

        label_64 = new QLabel(groupBox_7);
        label_64->setObjectName(QStringLiteral("label_64"));
        sizePolicy6.setHeightForWidth(label_64->sizePolicy().hasHeightForWidth());
        label_64->setSizePolicy(sizePolicy6);

        gridLayout_5->addWidget(label_64, 1, 3, 1, 1);

        label_60 = new QLabel(groupBox_7);
        label_60->setObjectName(QStringLiteral("label_60"));
        sizePolicy6.setHeightForWidth(label_60->sizePolicy().hasHeightForWidth());
        label_60->setSizePolicy(sizePolicy6);

        gridLayout_5->addWidget(label_60, 1, 0, 1, 1);

        label_69 = new QLabel(groupBox_7);
        label_69->setObjectName(QStringLiteral("label_69"));
        sizePolicy6.setHeightForWidth(label_69->sizePolicy().hasHeightForWidth());
        label_69->setSizePolicy(sizePolicy6);

        gridLayout_5->addWidget(label_69, 3, 4, 1, 1);

        label_58 = new QLabel(groupBox_7);
        label_58->setObjectName(QStringLiteral("label_58"));
        sizePolicy6.setHeightForWidth(label_58->sizePolicy().hasHeightForWidth());
        label_58->setSizePolicy(sizePolicy6);

        gridLayout_5->addWidget(label_58, 2, 1, 1, 1);


        gridLayout->addWidget(groupBox_7, 0, 0, 1, 1);

        model_tabs->addTab(tab_7, QString());

        gridLayout_3->addWidget(model_tabs, 0, 0, 1, 1);

        groupBox_10 = new QGroupBox(tab_4);
        groupBox_10->setObjectName(QStringLiteral("groupBox_10"));
        QSizePolicy sizePolicy7(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy7.setHorizontalStretch(0);
        sizePolicy7.setVerticalStretch(0);
        sizePolicy7.setHeightForWidth(groupBox_10->sizePolicy().hasHeightForWidth());
        groupBox_10->setSizePolicy(sizePolicy7);
        gridLayout_4 = new QGridLayout(groupBox_10);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        frame_2 = new QFrame(groupBox_10);
        frame_2->setObjectName(QStringLiteral("frame_2"));
        frame_2->setFrameShape(QFrame::NoFrame);
        frame_2->setFrameShadow(QFrame::Raised);
        gridLayout_11 = new QGridLayout(frame_2);
        gridLayout_11->setObjectName(QStringLiteral("gridLayout_11"));
        label_61 = new QLabel(frame_2);
        label_61->setObjectName(QStringLiteral("label_61"));
        sizePolicy6.setHeightForWidth(label_61->sizePolicy().hasHeightForWidth());
        label_61->setSizePolicy(sizePolicy6);

        gridLayout_11->addWidget(label_61, 0, 0, 1, 1);

        tx_spin = new QSpinBox(frame_2);
        tx_spin->setObjectName(QStringLiteral("tx_spin"));
        tx_spin->setMinimum(-65535);
        tx_spin->setMaximum(65536);

        gridLayout_11->addWidget(tx_spin, 0, 1, 1, 1);

        label_62 = new QLabel(frame_2);
        label_62->setObjectName(QStringLiteral("label_62"));
        sizePolicy6.setHeightForWidth(label_62->sizePolicy().hasHeightForWidth());
        label_62->setSizePolicy(sizePolicy6);

        gridLayout_11->addWidget(label_62, 1, 0, 1, 1);

        ty_spin = new QSpinBox(frame_2);
        ty_spin->setObjectName(QStringLiteral("ty_spin"));
        ty_spin->setMinimum(-65535);
        ty_spin->setMaximum(65536);

        gridLayout_11->addWidget(ty_spin, 1, 1, 1, 1);

        label_66 = new QLabel(frame_2);
        label_66->setObjectName(QStringLiteral("label_66"));
        sizePolicy6.setHeightForWidth(label_66->sizePolicy().hasHeightForWidth());
        label_66->setSizePolicy(sizePolicy6);

        gridLayout_11->addWidget(label_66, 2, 0, 1, 1);

        tz_spin = new QSpinBox(frame_2);
        tz_spin->setObjectName(QStringLiteral("tz_spin"));
        tz_spin->setMinimum(-65535);
        tz_spin->setMaximum(65536);

        gridLayout_11->addWidget(tz_spin, 2, 1, 1, 1);


        gridLayout_4->addWidget(frame_2, 0, 0, 1, 1);

        frame = new QFrame(groupBox_10);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setFrameShape(QFrame::NoFrame);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout_2 = new QVBoxLayout(frame);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        label_59 = new QLabel(frame);
        label_59->setObjectName(QStringLiteral("label_59"));
        label_59->setOpenExternalLinks(true);

        verticalLayout_2->addWidget(label_59);

        tcalib_button = new QPushButton(frame);
        tcalib_button->setObjectName(QStringLiteral("tcalib_button"));
        tcalib_button->setEnabled(false);
        tcalib_button->setCheckable(true);

        verticalLayout_2->addWidget(tcalib_button);


        gridLayout_4->addWidget(frame, 0, 1, 1, 1);


        gridLayout_3->addWidget(groupBox_10, 1, 0, 1, 1);

        tabWidget->addTab(tab_4, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QStringLiteral("tab_3"));
        gridLayout_8 = new QGridLayout(tab_3);
        gridLayout_8->setObjectName(QStringLiteral("gridLayout_8"));
        label_10 = new QLabel(tab_3);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        label_10->setOpenExternalLinks(true);

        gridLayout_8->addWidget(label_10, 0, 0, 1, 1);

        tabWidget->addTab(tab_3, QString());

        gridLayout_9->addWidget(tabWidget, 0, 0, 1, 1);

        groupBox_5 = new QGroupBox(UICPTClientControls);
        groupBox_5->setObjectName(QStringLiteral("groupBox_5"));
        gridLayout_10 = new QGridLayout(groupBox_5);
        gridLayout_10->setObjectName(QStringLiteral("gridLayout_10"));
        label_3 = new QLabel(groupBox_5);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout_10->addWidget(label_3, 1, 0, 1, 1);

        buttonBox = new QDialogButtonBox(groupBox_5);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout_10->addWidget(buttonBox, 2, 1, 1, 1);

        label_38 = new QLabel(groupBox_5);
        label_38->setObjectName(QStringLiteral("label_38"));

        gridLayout_10->addWidget(label_38, 0, 0, 1, 1);

        pointinfo_label = new QLabel(groupBox_5);
        pointinfo_label->setObjectName(QStringLiteral("pointinfo_label"));
        pointinfo_label->setMinimumSize(QSize(50, 0));

        gridLayout_10->addWidget(pointinfo_label, 1, 1, 1, 1);

        caminfo_label = new QLabel(groupBox_5);
        caminfo_label->setObjectName(QStringLiteral("caminfo_label"));
        sizePolicy.setHeightForWidth(caminfo_label->sizePolicy().hasHeightForWidth());
        caminfo_label->setSizePolicy(sizePolicy);
        caminfo_label->setMinimumSize(QSize(120, 0));

        gridLayout_10->addWidget(caminfo_label, 0, 1, 1, 1);


        gridLayout_9->addWidget(groupBox_5, 1, 0, 1, 1);

#ifndef QT_NO_SHORTCUT
        label_2->setBuddy(camdevice_combo);
#endif // QT_NO_SHORTCUT
        QWidget::setTabOrder(tabWidget, camdevice_combo);
        QWidget::setTabOrder(camdevice_combo, res_x_spin);
        QWidget::setTabOrder(res_x_spin, res_y_spin);
        QWidget::setTabOrder(res_y_spin, fov);
        QWidget::setTabOrder(fov, model_tabs);
        QWidget::setTabOrder(model_tabs, clip_tlength_spin);
        QWidget::setTabOrder(clip_tlength_spin, clip_theight_spin);
        QWidget::setTabOrder(clip_theight_spin, clip_bheight_spin);
        QWidget::setTabOrder(clip_bheight_spin, clip_blength_spin);
        QWidget::setTabOrder(clip_blength_spin, cap_length_spin);
        QWidget::setTabOrder(cap_length_spin, cap_width_spin);
        QWidget::setTabOrder(cap_width_spin, cap_height_spin);
        QWidget::setTabOrder(cap_height_spin, m1x_spin);
        QWidget::setTabOrder(m1x_spin, m1y_spin);
        QWidget::setTabOrder(m1y_spin, m1z_spin);
        QWidget::setTabOrder(m1z_spin, m2x_spin);
        QWidget::setTabOrder(m2x_spin, m2y_spin);
        QWidget::setTabOrder(m2y_spin, m2z_spin);
        QWidget::setTabOrder(m2z_spin, tx_spin);
        QWidget::setTabOrder(tx_spin, ty_spin);
        QWidget::setTabOrder(ty_spin, tz_spin);

        retranslateUi(UICPTClientControls);

        tabWidget->setCurrentIndex(2);
        model_tabs->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(UICPTClientControls);
    } // setupUi

    void retranslateUi(QWidget *UICPTClientControls)
    {
        UICPTClientControls->setWindowTitle(QApplication::translate("UICPTClientControls", "WiiMoteTracker Settings", 0));
        groupBox->setTitle(QApplication::translate("UICPTClientControls", "Camera settings", 0));
        label_6->setText(QApplication::translate("UICPTClientControls", "Dynamic pose timeout", 0));
#ifndef QT_NO_TOOLTIP
        res_y_spin->setToolTip(QApplication::translate("UICPTClientControls", "Desired capture height", 0));
#endif // QT_NO_TOOLTIP
        res_y_spin->setSuffix(QApplication::translate("UICPTClientControls", " px", 0));
        label_4->setText(QApplication::translate("UICPTClientControls", "Diagonal field of view", 0));
        label_36->setText(QApplication::translate("UICPTClientControls", "Width", 0));
        fov->setSuffix(QApplication::translate("UICPTClientControls", "\302\260", 0));
        fov->setPrefix(QString());
        dynamic_pose->setText(QString());
        label_5->setText(QApplication::translate("UICPTClientControls", "Dynamic pose resolution", 0));
        init_phase_timeout->setSuffix(QApplication::translate("UICPTClientControls", " ms", 0));
        label_41->setText(QApplication::translate("UICPTClientControls", "Height", 0));
#ifndef QT_NO_TOOLTIP
        res_x_spin->setToolTip(QApplication::translate("UICPTClientControls", "Desired capture width", 0));
#endif // QT_NO_TOOLTIP
        res_x_spin->setSuffix(QApplication::translate("UICPTClientControls", " px", 0));
        label_2->setText(QApplication::translate("UICPTClientControls", "Device", 0));
        label_9->setText(QApplication::translate("UICPTClientControls", "Camera settings dialog", 0));
        camera_settings->setText(QApplication::translate("UICPTClientControls", "Open", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("UICPTClientControls", "Camera", 0));
        groupBox_8->setTitle(QApplication::translate("UICPTClientControls", "Model Dimensions", 0));
        clip_tlength_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        clip_bheight_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        label_44->setText(QString());
        label_50->setText(QApplication::translate("UICPTClientControls", "Side", 0));
        clip_blength_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        clip_theight_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        label_51->setText(QApplication::translate("UICPTClientControls", "Front", 0));
        label_45->setText(QString());
        model_tabs->setTabText(model_tabs->indexOf(tab_5), QApplication::translate("UICPTClientControls", "Clip", 0));
        groupBox_9->setTitle(QApplication::translate("UICPTClientControls", "Model Dimensions", 0));
        label_46->setText(QString());
        label_48->setText(QApplication::translate("UICPTClientControls", "Side", 0));
        cap_length_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        label_47->setText(QString());
        cap_width_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        label_49->setText(QApplication::translate("UICPTClientControls", "Front", 0));
        cap_height_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        model_tabs->setTabText(model_tabs->indexOf(tab_6), QApplication::translate("UICPTClientControls", "Cap", 0));
        groupBox_7->setTitle(QApplication::translate("UICPTClientControls", "Model Dimensions", 0));
        label_57->setText(QApplication::translate("UICPTClientControls", "z:", 0));
        m2y_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        label_63->setText(QApplication::translate("UICPTClientControls", "x:", 0));
        m1x_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        m1y_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        m2z_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        m2x_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        label_56->setText(QApplication::translate("UICPTClientControls", "<html><head/><body><p>Location of the two remaining model points<br/>with respect to the reference point in default pose</p><p>Use any units you want, not necessarily centimeters.</p></body></html>", 0));
        m1z_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        label_70->setText(QApplication::translate("UICPTClientControls", "y:", 0));
        label_67->setText(QApplication::translate("UICPTClientControls", "x:", 0));
        label_64->setText(QApplication::translate("UICPTClientControls", "<html><head/><body><p><span style=\" font-size:16pt;\">P</span><span style=\" font-size:16pt; vertical-align:sub;\">3</span></p></body></html>", 0));
        label_60->setText(QApplication::translate("UICPTClientControls", "<html><head/><body><p><span style=\" font-size:16pt;\">P</span><span style=\" font-size:16pt; vertical-align:sub;\">2</span></p></body></html>", 0));
        label_69->setText(QApplication::translate("UICPTClientControls", "z:", 0));
        label_58->setText(QApplication::translate("UICPTClientControls", "y:", 0));
        model_tabs->setTabText(model_tabs->indexOf(tab_7), QApplication::translate("UICPTClientControls", "Custom", 0));
        groupBox_10->setTitle(QApplication::translate("UICPTClientControls", "Model position", 0));
        label_61->setText(QApplication::translate("UICPTClientControls", "x:", 0));
        tx_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        label_62->setText(QApplication::translate("UICPTClientControls", "y:", 0));
        ty_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        label_66->setText(QApplication::translate("UICPTClientControls", "z:", 0));
        tz_spin->setSuffix(QApplication::translate("UICPTClientControls", " mm", 0));
        label_59->setText(QApplication::translate("UICPTClientControls", "<html><head/><body><p><a href=\"https://github.com/opentrack/opentrack/wiki/model-calibration-for-PT-and-Aruco-trackers\"><span style=\" text-decoration: underline; color:#0000ff;\">Instructions on the opentrack wiki</span></a></p></body></html>", 0));
        tcalib_button->setText(QApplication::translate("UICPTClientControls", "Toggle calibration", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QApplication::translate("UICPTClientControls", "Model", 0));
        label_10->setText(QApplication::translate("UICPTClientControls", "<html><head/><body><p><span style=\" font-size:8pt; font-weight:600;\">FTNoIR WiiMote Tracker Plugin, based hid-wiimote<br/>Version 1.0</span></p><p><span style=\" font-size:8pt; font-weight:600;\">by fred41</span><span style=\" font-size:8pt;\"><br/><br/></span></p><p><span style=\" font-size:8pt;\">Requirements:</span></p><p><span style=\" font-size:8pt;\">kernel &gt;= 3.1, bluez &gt;= 4.101, WiiMote Controller</span></p><p><span style=\" font-size:8pt;\"><br/></span></p><p><span style=\" font-size:8pt;\">Device acces:</span></p><p><span style=\" font-size:8pt;\">Make a group 'player' and add your gaming user.</span></p><p><span style=\" font-size:8pt;\">insert the following lines in </span><span style=\" font-size:8pt; font-weight:600;\">/etc/udev/rules.d/99-input.rules</span><span style=\" font-size:8pt;\">:</span></p><p><span style=\" font-size:8pt;\">ATTRS{name}==&quot;Nintendo Wii Remote IR&quot;, SYMLINK+=&quot;input/wii_ir&quot;, GROUP=&quot;player&quot;</span></p><p><span style=\" font-size:8pt;\">Fo"
                        "r 'libevdev' protocol try this additional line:</span></p><p><span style=\" font-size:8pt;\">KERNEL==&quot;uinput&quot;, SYMLINK+=&quot;input/uinput&quot;, GROUP=&quot;player&quot;</span></p></body></html>", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QApplication::translate("UICPTClientControls", "About", 0));
        groupBox_5->setTitle(QApplication::translate("UICPTClientControls", "Status", 0));
        label_3->setText(QApplication::translate("UICPTClientControls", "Extracted Points:", 0));
        label_38->setText(QApplication::translate("UICPTClientControls", "Camera Info:", 0));
        pointinfo_label->setText(QString());
        caminfo_label->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class UICPTClientControls: public Ui_UICPTClientControls {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FTNOIR_WIIMOTE_CONTROLS_H
