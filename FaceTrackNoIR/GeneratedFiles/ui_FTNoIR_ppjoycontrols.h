/********************************************************************************
** Form generated from reading UI file 'FTNoIR_ppjoycontrols.ui'
**
** Created: Fri 3. Sep 13:27:15 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FTNOIR_PPJOYCONTROLS_H
#define UI_FTNOIR_PPJOYCONTROLS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_UICPPJoyControls
{
public:
    QVBoxLayout *_vertical_layout;
    QHBoxLayout *hboxLayout;
    QLabel *textLabel2;
    QComboBox *cbxSelectPPJoyNumber;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *btnOK;
    QPushButton *btnCancel;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWidget *UICPPJoyControls)
    {
        if (UICPPJoyControls->objectName().isEmpty())
            UICPPJoyControls->setObjectName(QString::fromUtf8("UICPPJoyControls"));
        UICPPJoyControls->resize(346, 180);
        QIcon icon;
        icon.addFile(QString::fromUtf8("images/FaceTrackNoIR.ico"), QSize(), QIcon::Normal, QIcon::Off);
        UICPPJoyControls->setWindowIcon(icon);
        UICPPJoyControls->setLayoutDirection(Qt::LeftToRight);
        UICPPJoyControls->setAutoFillBackground(false);
        _vertical_layout = new QVBoxLayout(UICPPJoyControls);
        _vertical_layout->setObjectName(QString::fromUtf8("_vertical_layout"));
        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        textLabel2 = new QLabel(UICPPJoyControls);
        textLabel2->setObjectName(QString::fromUtf8("textLabel2"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(textLabel2->sizePolicy().hasHeightForWidth());
        textLabel2->setSizePolicy(sizePolicy);
        textLabel2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        textLabel2->setWordWrap(false);

        hboxLayout->addWidget(textLabel2);

        cbxSelectPPJoyNumber = new QComboBox(UICPPJoyControls);
        cbxSelectPPJoyNumber->setObjectName(QString::fromUtf8("cbxSelectPPJoyNumber"));
        cbxSelectPPJoyNumber->setInsertPolicy(QComboBox::InsertAlphabetically);

        hboxLayout->addWidget(cbxSelectPPJoyNumber);


        _vertical_layout->addLayout(hboxLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        _vertical_layout->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setSizeConstraint(QLayout::SetDefaultConstraint);
        btnOK = new QPushButton(UICPPJoyControls);
        btnOK->setObjectName(QString::fromUtf8("btnOK"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(btnOK->sizePolicy().hasHeightForWidth());
        btnOK->setSizePolicy(sizePolicy1);
        btnOK->setMinimumSize(QSize(100, 0));
        btnOK->setMaximumSize(QSize(100, 16777215));

        horizontalLayout_2->addWidget(btnOK);

        btnCancel = new QPushButton(UICPPJoyControls);
        btnCancel->setObjectName(QString::fromUtf8("btnCancel"));
        sizePolicy1.setHeightForWidth(btnCancel->sizePolicy().hasHeightForWidth());
        btnCancel->setSizePolicy(sizePolicy1);
        btnCancel->setMinimumSize(QSize(100, 0));
        btnCancel->setMaximumSize(QSize(100, 16777215));

        horizontalLayout_2->addWidget(btnCancel);


        horizontalLayout->addLayout(horizontalLayout_2);

        horizontalSpacer = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        _vertical_layout->addLayout(horizontalLayout);


        retranslateUi(UICPPJoyControls);

        QMetaObject::connectSlotsByName(UICPPJoyControls);
    } // setupUi

    void retranslateUi(QWidget *UICPPJoyControls)
    {
        UICPPJoyControls->setWindowTitle(QApplication::translate("UICPPJoyControls", "PPJoy settings FaceTrackNoIR", 0, QApplication::UnicodeUTF8));
        textLabel2->setText(QApplication::translate("UICPPJoyControls", "Virtual Joystick number:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        cbxSelectPPJoyNumber->setToolTip(QApplication::translate("UICPPJoyControls", "Select Number", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnOK->setText(QApplication::translate("UICPPJoyControls", "OK", 0, QApplication::UnicodeUTF8));
        btnCancel->setText(QApplication::translate("UICPPJoyControls", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class UICPPJoyControls: public Ui_UICPPJoyControls {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FTNOIR_PPJOYCONTROLS_H
