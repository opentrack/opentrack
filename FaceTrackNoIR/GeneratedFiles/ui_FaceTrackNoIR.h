/********************************************************************************
** Form generated from reading UI file 'FaceTrackNoIR.ui'
**
** Created: Fri 3. Sep 13:27:15 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FACETRACKNOIR_H
#define UI_FACETRACKNOIR_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FaceTrackNoIRClass
{
public:
    QAction *actionOpen;
    QAction *actionExit;
    QAction *actionPlayPause;
    QAction *actionNext;
    QAction *actionPreviews;
    QAction *actionVolumeUp;
    QAction *actionVolumeDown;
    QAction *actionVideoWidget;
    QAction *actionHeadPoseWidget;
    QAction *actionAbout;
    QAction *actionMute;
    QAction *actionSave;
    QAction *actionSave_As;
    QAction *actionKeyboard_Shortcuts;
    QAction *actionPreferences;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_2;
    QWidget *widgetTop;
    QHBoxLayout *horizontalLayout_5;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *horizontalSpacer_2;
    QFrame *logoInstitute;
    QHBoxLayout *horizontalLayout;
    QWidget *Leftwidget;
    QVBoxLayout *verticalLayout;
    QWidget *spacerwidget;
    QWidget *headPoseWidget;
    QLineEdit *headRotYLine;
    QLineEdit *headRotXLine;
    QLineEdit *headRotZLine;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *label_9;
    QLineEdit *headZLine;
    QLineEdit *headYLine;
    QLineEdit *headXLine;
    QFrame *video_frame;
    QWidget *widget;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_3;
    QGroupBox *groupTrackerSource;
    QComboBox *iconcomboTrackerSource;
    QPushButton *btnShowEngineControls;
    QPushButton *btnStartTracker;
    QPushButton *btnStopTracker;
    QGroupBox *groupGameProtocol;
    QComboBox *iconcomboBox;
    QPushButton *btnShowServerControls;
    QGridLayout *gridLayout_2;
    QLabel *lblSensYaw_3;
    QSlider *slideSmoothing;
    QSpinBox *spinSmoothing;
    QCheckBox *chkUseEWMA;
    QLabel *lblSensYaw_5;
    QLabel *lblSensYaw_4;
    QSlider *slideNeutralZone;
    QSpinBox *spinNeutralZone;
    QLabel *lblSensYaw_6;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout_11;
    QSpacerItem *horizontalSpacer_9;
    QWidget *bubbleSmallWidget_2;
    QVBoxLayout *verticalLayout_6;
    QLabel *cameraName;
    QHBoxLayout *horizontalLayout_8;
    QSpacerItem *horizontalSpacer_17;
    QWidget *bubbleBigWidget;
    QHBoxLayout *horizontalLayout_2;
    QGridLayout *gridLayout;
    QLabel *lblSensitivity;
    QLabel *lblSensYaw;
    QSlider *sensYaw;
    QSpinBox *spinSensYaw;
    QLabel *lblSensX;
    QSlider *sensX;
    QSpinBox *spinSensX;
    QSpinBox *spinSensY;
    QLabel *lblSensPitch;
    QSlider *sensPitch;
    QSpinBox *spinSensPitch;
    QLabel *lblSensY;
    QSlider *sensY;
    QLabel *lblSensRoll;
    QSlider *sensRoll;
    QSpinBox *spinSensRoll;
    QLabel *lblSensZ;
    QSlider *sensZ;
    QSpinBox *spinSensZ;
    QCheckBox *chkInvertYaw;
    QLabel *lblInvert1;
    QCheckBox *chkInvertPitch;
    QCheckBox *chkInvertRoll;
    QCheckBox *chkInvertX;
    QLabel *lblInvert1_2;
    QCheckBox *chkInvertY;
    QCheckBox *chkInvertZ;
    QSlider *redYaw;
    QSlider *redPitch;
    QSlider *redRoll;
    QSpinBox *spinRedYaw;
    QSpinBox *spinRedPitch;
    QSpinBox *spinRedRoll;
    QLabel *lblSensitivity_2;
    QSlider *redX;
    QSlider *redY;
    QSlider *redZ;
    QSpinBox *spinRedX;
    QSpinBox *spinRedY;
    QSpinBox *spinRedZ;
    QLabel *lblSensitivity_3;
    QLabel *lblSensitivity_4;
    QSpacerItem *horizontalSpacer_13;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuAbout;
    QMenu *menuView;
    QMenu *menuOptions;

    void setupUi(QMainWindow *FaceTrackNoIRClass)
    {
        if (FaceTrackNoIRClass->objectName().isEmpty())
            FaceTrackNoIRClass->setObjectName(QString::fromUtf8("FaceTrackNoIRClass"));
        FaceTrackNoIRClass->setWindowModality(Qt::NonModal);
        FaceTrackNoIRClass->resize(970, 521);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(FaceTrackNoIRClass->sizePolicy().hasHeightForWidth());
        FaceTrackNoIRClass->setSizePolicy(sizePolicy);
        FaceTrackNoIRClass->setContextMenuPolicy(Qt::DefaultContextMenu);
        QIcon icon;
        icon.addFile(QString::fromUtf8("UIElements/FaceTrackNoIR.ico"), QSize(), QIcon::Normal, QIcon::Off);
        FaceTrackNoIRClass->setWindowIcon(icon);
        FaceTrackNoIRClass->setStyleSheet(QString::fromUtf8("/* Customize any plain widget that is a child of a QMainWindow. */\n"
"QMainWindow > .QWidget {\n"
"	background-color: rgb(100, 100, 100);\n"
"}\n"
"\n"
"/* Set the selection colors for all widgets. */\n"
"QWidget {\n"
"    selection-color: black;\n"
"    selection-background-color: Silver;\n"
"    color: black;\n"
"}\n"
"\n"
"/* Specials for individual widget(s) */\n"
"QWidget#widget {\n"
"	background-color: #484848;\n"
"}\n"
"\n"
"QWidget#Leftwidget {\n"
"	background-color: #595959;\n"
"	border-right: 1px solid #000;\n"
"}\n"
"\n"
"QWidget#widgetTop {\n"
"	background-color: #595959;\n"
"	border-bottom: 1px solid #000;\n"
"}\n"
"\n"
"/* Make text in message boxes selectable. */\n"
"QMessageBox {\n"
"    /* LinksAccessibleByMouse | TextSelectableByMouse */\n"
"    messagebox-text-interaction-flags: 5;\n"
"}\n"
"   \n"
"/* Make the entire row selected in item views. */\n"
"QAbstractItemView {\n"
"    show-decoration-selected: 1;\n"
"}\n"
"\n"
"/* Nice WindowsXP-style password character for password line edits. "
                        "*/\n"
"QLineEdit[echoMode=\"2\"] {\n"
"    lineedit-password-character: 9679;\n"
"}\n"
"\n"
"/* Customize tooltips. */\n"
"QToolTip {\n"
"	background-color: rgb(170, 255, 127);\n"
"	opacity: 200;\n"
"}\n"
"\n"
"/* Customize push buttons and comboboxes. Our read-only combobox\n"
"   is very similar to a push button, so they share the same border image. */\n"
"\n"
"QPushButton {\n"
"    min-width: 4em;\n"
"}\n"
"\n"
"QPushButton:disabled {\n"
"	color: rgb(128, 128, 128);\n"
"}\n"
"\n"
"QGroupBox {\n"
"	color: rgb(255, 255, 255);\n"
"}"));
        FaceTrackNoIRClass->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        FaceTrackNoIRClass->setToolButtonStyle(Qt::ToolButtonIconOnly);
        FaceTrackNoIRClass->setAnimated(true);
        actionOpen = new QAction(FaceTrackNoIRClass);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        actionOpen->setIconVisibleInMenu(false);
        actionExit = new QAction(FaceTrackNoIRClass);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionExit->setMenuRole(QAction::QuitRole);
        actionExit->setIconVisibleInMenu(false);
        actionPlayPause = new QAction(FaceTrackNoIRClass);
        actionPlayPause->setObjectName(QString::fromUtf8("actionPlayPause"));
        actionPlayPause->setCheckable(true);
        actionPlayPause->setIconVisibleInMenu(false);
        actionNext = new QAction(FaceTrackNoIRClass);
        actionNext->setObjectName(QString::fromUtf8("actionNext"));
        actionNext->setIconVisibleInMenu(false);
        actionPreviews = new QAction(FaceTrackNoIRClass);
        actionPreviews->setObjectName(QString::fromUtf8("actionPreviews"));
        actionPreviews->setIconVisibleInMenu(false);
        actionVolumeUp = new QAction(FaceTrackNoIRClass);
        actionVolumeUp->setObjectName(QString::fromUtf8("actionVolumeUp"));
        actionVolumeUp->setIconVisibleInMenu(false);
        actionVolumeDown = new QAction(FaceTrackNoIRClass);
        actionVolumeDown->setObjectName(QString::fromUtf8("actionVolumeDown"));
        actionVolumeDown->setIconVisibleInMenu(false);
        actionVideoWidget = new QAction(FaceTrackNoIRClass);
        actionVideoWidget->setObjectName(QString::fromUtf8("actionVideoWidget"));
        actionVideoWidget->setCheckable(true);
        actionVideoWidget->setChecked(true);
        actionHeadPoseWidget = new QAction(FaceTrackNoIRClass);
        actionHeadPoseWidget->setObjectName(QString::fromUtf8("actionHeadPoseWidget"));
        actionHeadPoseWidget->setCheckable(true);
        actionAbout = new QAction(FaceTrackNoIRClass);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        actionMute = new QAction(FaceTrackNoIRClass);
        actionMute->setObjectName(QString::fromUtf8("actionMute"));
        actionMute->setCheckable(true);
        actionSave = new QAction(FaceTrackNoIRClass);
        actionSave->setObjectName(QString::fromUtf8("actionSave"));
        actionSave_As = new QAction(FaceTrackNoIRClass);
        actionSave_As->setObjectName(QString::fromUtf8("actionSave_As"));
        actionKeyboard_Shortcuts = new QAction(FaceTrackNoIRClass);
        actionKeyboard_Shortcuts->setObjectName(QString::fromUtf8("actionKeyboard_Shortcuts"));
        actionPreferences = new QAction(FaceTrackNoIRClass);
        actionPreferences->setObjectName(QString::fromUtf8("actionPreferences"));
        centralWidget = new QWidget(FaceTrackNoIRClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        centralWidget->setMinimumSize(QSize(800, 500));
        centralWidget->setStyleSheet(QString::fromUtf8(""));
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        widgetTop = new QWidget(centralWidget);
        widgetTop->setObjectName(QString::fromUtf8("widgetTop"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(widgetTop->sizePolicy().hasHeightForWidth());
        widgetTop->setSizePolicy(sizePolicy1);
        widgetTop->setMinimumSize(QSize(166, 90));
        widgetTop->setStyleSheet(QString::fromUtf8(""));
        horizontalLayout_5 = new QHBoxLayout(widgetTop);
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        horizontalLayout_5->setContentsMargins(-1, 0, 0, 0);
        horizontalSpacer = new QSpacerItem(177, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer);

        horizontalSpacer_2 = new QSpacerItem(186, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_2);

        logoInstitute = new QFrame(widgetTop);
        logoInstitute->setObjectName(QString::fromUtf8("logoInstitute"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(logoInstitute->sizePolicy().hasHeightForWidth());
        logoInstitute->setSizePolicy(sizePolicy2);
        logoInstitute->setMinimumSize(QSize(90, 90));
        logoInstitute->setStyleSheet(QString::fromUtf8("QFrame#logoInstitute {\n"
"	background:#595959 url(UIElements/logoFaceTrackNoIR.png) no-repeat;\n"
"border:none;\n"
"}"));
        logoInstitute->setFrameShape(QFrame::StyledPanel);
        logoInstitute->setFrameShadow(QFrame::Raised);

        horizontalLayout_5->addWidget(logoInstitute);


        verticalLayout_2->addWidget(widgetTop);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        Leftwidget = new QWidget(centralWidget);
        Leftwidget->setObjectName(QString::fromUtf8("Leftwidget"));
        Leftwidget->setMaximumSize(QSize(1000, 16777215));
        Leftwidget->setStyleSheet(QString::fromUtf8(""));
        verticalLayout = new QVBoxLayout(Leftwidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        spacerwidget = new QWidget(Leftwidget);
        spacerwidget->setObjectName(QString::fromUtf8("spacerwidget"));
        spacerwidget->setMinimumSize(QSize(250, 300));
        spacerwidget->setStyleSheet(QString::fromUtf8(""));

        verticalLayout->addWidget(spacerwidget);

        headPoseWidget = new QWidget(Leftwidget);
        headPoseWidget->setObjectName(QString::fromUtf8("headPoseWidget"));
        headPoseWidget->setMinimumSize(QSize(0, 100));
        headPoseWidget->setMaximumSize(QSize(16777215, 100));
        headPoseWidget->setStyleSheet(QString::fromUtf8(""));
        headRotYLine = new QLineEdit(headPoseWidget);
        headRotYLine->setObjectName(QString::fromUtf8("headRotYLine"));
        headRotYLine->setGeometry(QRect(151, 40, 80, 20));
        headRotYLine->setStyleSheet(QString::fromUtf8("border:1px solid #ccc;\n"
"background:white;\n"
"color:#000;"));
        headRotXLine = new QLineEdit(headPoseWidget);
        headRotXLine->setObjectName(QString::fromUtf8("headRotXLine"));
        headRotXLine->setGeometry(QRect(151, 10, 81, 20));
        headRotXLine->setStyleSheet(QString::fromUtf8("border:1px solid #ccc;\n"
"background:white;\n"
"color:#000;"));
        headRotZLine = new QLineEdit(headPoseWidget);
        headRotZLine->setObjectName(QString::fromUtf8("headRotZLine"));
        headRotZLine->setGeometry(QRect(150, 70, 81, 20));
        headRotZLine->setStyleSheet(QString::fromUtf8("border:1px solid #ccc;\n"
"background:white;\n"
"color:#000;"));
        label_4 = new QLabel(headPoseWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(10, 10, 21, 16));
        label_4->setStyleSheet(QString::fromUtf8("border:none;\n"
"color:white"));
        label_5 = new QLabel(headPoseWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(10, 40, 21, 16));
        label_5->setStyleSheet(QString::fromUtf8("border:none;\n"
"color:white;"));
        label_6 = new QLabel(headPoseWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(10, 70, 21, 16));
        label_6->setStyleSheet(QString::fromUtf8("color:white;\n"
"border:none;"));
        label_7 = new QLabel(headPoseWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(120, 70, 21, 16));
        label_7->setStyleSheet(QString::fromUtf8("border:none;\n"
"color:white;"));
        label_8 = new QLabel(headPoseWidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(120, 40, 21, 16));
        label_8->setStyleSheet(QString::fromUtf8("color:white;\n"
"border:none;"));
        label_9 = new QLabel(headPoseWidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(120, 10, 21, 16));
        label_9->setStyleSheet(QString::fromUtf8("border:none;\n"
"color:white;"));
        headZLine = new QLineEdit(headPoseWidget);
        headZLine->setObjectName(QString::fromUtf8("headZLine"));
        headZLine->setGeometry(QRect(30, 70, 81, 20));
        headZLine->setStyleSheet(QString::fromUtf8("border:1px solid #ccc;\n"
"background:white;\n"
"color:#000;"));
        headYLine = new QLineEdit(headPoseWidget);
        headYLine->setObjectName(QString::fromUtf8("headYLine"));
        headYLine->setGeometry(QRect(30, 40, 81, 20));
        headYLine->setStyleSheet(QString::fromUtf8("border:1px solid #ccc;\n"
"background:white;\n"
"color:#000;"));
        headXLine = new QLineEdit(headPoseWidget);
        headXLine->setObjectName(QString::fromUtf8("headXLine"));
        headXLine->setGeometry(QRect(30, 10, 81, 20));
        headXLine->setStyleSheet(QString::fromUtf8("border:1px solid #ccc;\n"
"background:white;\n"
"color:#000;"));

        verticalLayout->addWidget(headPoseWidget);

        video_frame = new QFrame(Leftwidget);
        video_frame->setObjectName(QString::fromUtf8("video_frame"));
        video_frame->setMinimumSize(QSize(250, 180));
        video_frame->setMaximumSize(QSize(250, 180));
        video_frame->setStyleSheet(QString::fromUtf8(""));
        video_frame->setFrameShape(QFrame::StyledPanel);
        video_frame->setFrameShadow(QFrame::Raised);

        verticalLayout->addWidget(video_frame);


        horizontalLayout->addWidget(Leftwidget);

        widget = new QWidget(centralWidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setEnabled(true);
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(widget->sizePolicy().hasHeightForWidth());
        widget->setSizePolicy(sizePolicy3);
        widget->setMinimumSize(QSize(400, 300));
        QPalette palette;
        QBrush brush(QColor(0, 0, 0, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
        QBrush brush1(QColor(72, 72, 72, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Button, brush1);
        palette.setBrush(QPalette::Active, QPalette::Text, brush);
        palette.setBrush(QPalette::Active, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Active, QPalette::Base, brush1);
        palette.setBrush(QPalette::Active, QPalette::Window, brush1);
        QBrush brush2(QColor(192, 192, 192, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Highlight, brush2);
        palette.setBrush(QPalette::Active, QPalette::HighlightedText, brush);
        palette.setBrush(QPalette::Active, QPalette::AlternateBase, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Button, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Highlight, brush2);
        palette.setBrush(QPalette::Inactive, QPalette::HighlightedText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Text, brush);
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Highlight, brush2);
        palette.setBrush(QPalette::Disabled, QPalette::HighlightedText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, brush1);
        widget->setPalette(palette);
        widget->setAutoFillBackground(false);
        widget->setStyleSheet(QString::fromUtf8(""));
        widget->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        verticalLayout_4 = new QVBoxLayout(widget);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        groupTrackerSource = new QGroupBox(widget);
        groupTrackerSource->setObjectName(QString::fromUtf8("groupTrackerSource"));
        groupTrackerSource->setMinimumSize(QSize(200, 120));
        groupTrackerSource->setStyleSheet(QString::fromUtf8(""));
        iconcomboTrackerSource = new QComboBox(groupTrackerSource);
        iconcomboTrackerSource->setObjectName(QString::fromUtf8("iconcomboTrackerSource"));
        iconcomboTrackerSource->setGeometry(QRect(10, 20, 161, 22));
        iconcomboTrackerSource->setStyleSheet(QString::fromUtf8(""));
        iconcomboTrackerSource->setMaxVisibleItems(3);
        btnShowEngineControls = new QPushButton(groupTrackerSource);
        btnShowEngineControls->setObjectName(QString::fromUtf8("btnShowEngineControls"));
        btnShowEngineControls->setEnabled(false);
        btnShowEngineControls->setGeometry(QRect(10, 80, 161, 23));
        btnShowEngineControls->setStyleSheet(QString::fromUtf8(""));
        btnStartTracker = new QPushButton(groupTrackerSource);
        btnStartTracker->setObjectName(QString::fromUtf8("btnStartTracker"));
        btnStartTracker->setGeometry(QRect(10, 50, 75, 23));
        btnStartTracker->setStyleSheet(QString::fromUtf8(""));
        btnStopTracker = new QPushButton(groupTrackerSource);
        btnStopTracker->setObjectName(QString::fromUtf8("btnStopTracker"));
        btnStopTracker->setEnabled(false);
        btnStopTracker->setGeometry(QRect(96, 50, 75, 23));
        btnStopTracker->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_3->addWidget(groupTrackerSource);

        groupGameProtocol = new QGroupBox(widget);
        groupGameProtocol->setObjectName(QString::fromUtf8("groupGameProtocol"));
        groupGameProtocol->setMinimumSize(QSize(200, 100));
        groupGameProtocol->setStyleSheet(QString::fromUtf8(""));
        iconcomboBox = new QComboBox(groupGameProtocol);
        iconcomboBox->setObjectName(QString::fromUtf8("iconcomboBox"));
        iconcomboBox->setGeometry(QRect(10, 20, 151, 22));
        iconcomboBox->setStyleSheet(QString::fromUtf8(""));
        iconcomboBox->setMaxVisibleItems(5);
        btnShowServerControls = new QPushButton(groupGameProtocol);
        btnShowServerControls->setObjectName(QString::fromUtf8("btnShowServerControls"));
        btnShowServerControls->setEnabled(true);
        btnShowServerControls->setGeometry(QRect(10, 80, 161, 23));
        btnShowServerControls->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_3->addWidget(groupGameProtocol);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setSpacing(6);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        lblSensYaw_3 = new QLabel(widget);
        lblSensYaw_3->setObjectName(QString::fromUtf8("lblSensYaw_3"));
        lblSensYaw_3->setMinimumSize(QSize(25, 0));
        lblSensYaw_3->setMaximumSize(QSize(150, 16777215));
        lblSensYaw_3->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout_2->addWidget(lblSensYaw_3, 0, 0, 1, 2);

        slideSmoothing = new QSlider(widget);
        slideSmoothing->setObjectName(QString::fromUtf8("slideSmoothing"));
        slideSmoothing->setMinimumSize(QSize(50, 15));
        slideSmoothing->setMinimum(1);
        slideSmoothing->setMaximum(120);
        slideSmoothing->setPageStep(10);
        slideSmoothing->setValue(10);
        slideSmoothing->setOrientation(Qt::Horizontal);
        slideSmoothing->setTickPosition(QSlider::NoTicks);

        gridLayout_2->addWidget(slideSmoothing, 2, 0, 1, 1);

        spinSmoothing = new QSpinBox(widget);
        spinSmoothing->setObjectName(QString::fromUtf8("spinSmoothing"));
        spinSmoothing->setMinimumSize(QSize(50, 22));
        spinSmoothing->setMaximum(120);
        spinSmoothing->setValue(10);

        gridLayout_2->addWidget(spinSmoothing, 2, 1, 1, 1);

        chkUseEWMA = new QCheckBox(widget);
        chkUseEWMA->setObjectName(QString::fromUtf8("chkUseEWMA"));

        gridLayout_2->addWidget(chkUseEWMA, 3, 1, 1, 1);

        lblSensYaw_5 = new QLabel(widget);
        lblSensYaw_5->setObjectName(QString::fromUtf8("lblSensYaw_5"));
        lblSensYaw_5->setMinimumSize(QSize(25, 0));
        lblSensYaw_5->setMaximumSize(QSize(150, 16777215));
        lblSensYaw_5->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout_2->addWidget(lblSensYaw_5, 3, 0, 1, 1);

        lblSensYaw_4 = new QLabel(widget);
        lblSensYaw_4->setObjectName(QString::fromUtf8("lblSensYaw_4"));
        lblSensYaw_4->setMinimumSize(QSize(25, 0));
        lblSensYaw_4->setMaximumSize(QSize(150, 16777215));
        lblSensYaw_4->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout_2->addWidget(lblSensYaw_4, 1, 0, 1, 1);

        slideNeutralZone = new QSlider(widget);
        slideNeutralZone->setObjectName(QString::fromUtf8("slideNeutralZone"));
        slideNeutralZone->setMinimumSize(QSize(50, 15));
        slideNeutralZone->setMinimum(1);
        slideNeutralZone->setMaximum(45);
        slideNeutralZone->setPageStep(5);
        slideNeutralZone->setValue(5);
        slideNeutralZone->setOrientation(Qt::Horizontal);
        slideNeutralZone->setTickPosition(QSlider::NoTicks);

        gridLayout_2->addWidget(slideNeutralZone, 5, 0, 1, 1);

        spinNeutralZone = new QSpinBox(widget);
        spinNeutralZone->setObjectName(QString::fromUtf8("spinNeutralZone"));
        spinNeutralZone->setMinimumSize(QSize(50, 22));
        spinNeutralZone->setMaximum(45);
        spinNeutralZone->setSingleStep(5);
        spinNeutralZone->setValue(5);

        gridLayout_2->addWidget(spinNeutralZone, 5, 1, 1, 1);

        lblSensYaw_6 = new QLabel(widget);
        lblSensYaw_6->setObjectName(QString::fromUtf8("lblSensYaw_6"));
        lblSensYaw_6->setMinimumSize(QSize(25, 0));
        lblSensYaw_6->setMaximumSize(QSize(150, 16777215));
        lblSensYaw_6->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout_2->addWidget(lblSensYaw_6, 4, 0, 1, 1);


        horizontalLayout_3->addLayout(gridLayout_2);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);


        verticalLayout_4->addLayout(horizontalLayout_3);

        verticalSpacer = new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);

        horizontalLayout_11 = new QHBoxLayout();
        horizontalLayout_11->setSpacing(6);
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        horizontalSpacer_9 = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_11->addItem(horizontalSpacer_9);

        bubbleSmallWidget_2 = new QWidget(widget);
        bubbleSmallWidget_2->setObjectName(QString::fromUtf8("bubbleSmallWidget_2"));
        bubbleSmallWidget_2->setMinimumSize(QSize(667, 67));
        bubbleSmallWidget_2->setMaximumSize(QSize(667, 67));
        bubbleSmallWidget_2->setAcceptDrops(true);
        bubbleSmallWidget_2->setStyleSheet(QString::fromUtf8("background:  url(\"UIElements/bubble_1_small.png\") no-repeat;\n"
"opacity:100;"));
        verticalLayout_6 = new QVBoxLayout(bubbleSmallWidget_2);
        verticalLayout_6->setSpacing(0);
        verticalLayout_6->setContentsMargins(11, 11, 11, 11);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        verticalLayout_6->setContentsMargins(15, 10, 10, 10);
        cameraName = new QLabel(bubbleSmallWidget_2);
        cameraName->setObjectName(QString::fromUtf8("cameraName"));
        cameraName->setMinimumSize(QSize(0, 30));
        cameraName->setMaximumSize(QSize(16777215, 30));
        cameraName->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        verticalLayout_6->addWidget(cameraName);


        horizontalLayout_11->addWidget(bubbleSmallWidget_2);


        verticalLayout_4->addLayout(horizontalLayout_11);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setSpacing(6);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        horizontalSpacer_17 = new QSpacerItem(15, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_17);

        bubbleBigWidget = new QWidget(widget);
        bubbleBigWidget->setObjectName(QString::fromUtf8("bubbleBigWidget"));
        bubbleBigWidget->setMinimumSize(QSize(667, 115));
        bubbleBigWidget->setMaximumSize(QSize(667, 115));
        bubbleBigWidget->setAcceptDrops(true);
        bubbleBigWidget->setStyleSheet(QString::fromUtf8("background: url(\"UIElements/bubble_2_big.png\") no-repeat;\n"
"opacity:100;"));
        horizontalLayout_2 = new QHBoxLayout(bubbleBigWidget);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        lblSensitivity = new QLabel(bubbleBigWidget);
        lblSensitivity->setObjectName(QString::fromUtf8("lblSensitivity"));
        lblSensitivity->setMinimumSize(QSize(110, 0));
        lblSensitivity->setMaximumSize(QSize(150, 16777215));
        lblSensitivity->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout->addWidget(lblSensitivity, 0, 0, 1, 2);

        lblSensYaw = new QLabel(bubbleBigWidget);
        lblSensYaw->setObjectName(QString::fromUtf8("lblSensYaw"));
        lblSensYaw->setMinimumSize(QSize(25, 0));
        lblSensYaw->setMaximumSize(QSize(150, 16777215));
        lblSensYaw->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout->addWidget(lblSensYaw, 1, 0, 1, 1);

        sensYaw = new QSlider(bubbleBigWidget);
        sensYaw->setObjectName(QString::fromUtf8("sensYaw"));
        sensYaw->setMinimumSize(QSize(50, 15));
        sensYaw->setMaximum(500);
        sensYaw->setPageStep(10);
        sensYaw->setValue(100);
        sensYaw->setOrientation(Qt::Horizontal);
        sensYaw->setTickPosition(QSlider::NoTicks);

        gridLayout->addWidget(sensYaw, 1, 1, 1, 1);

        spinSensYaw = new QSpinBox(bubbleBigWidget);
        spinSensYaw->setObjectName(QString::fromUtf8("spinSensYaw"));
        spinSensYaw->setMinimumSize(QSize(50, 22));
        spinSensYaw->setMaximum(500);
        spinSensYaw->setSingleStep(10);
        spinSensYaw->setValue(100);

        gridLayout->addWidget(spinSensYaw, 1, 2, 2, 1);

        lblSensX = new QLabel(bubbleBigWidget);
        lblSensX->setObjectName(QString::fromUtf8("lblSensX"));
        lblSensX->setMinimumSize(QSize(25, 0));
        lblSensX->setMaximumSize(QSize(150, 16777215));
        lblSensX->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout->addWidget(lblSensX, 1, 6, 1, 1);

        sensX = new QSlider(bubbleBigWidget);
        sensX->setObjectName(QString::fromUtf8("sensX"));
        sensX->setMinimumSize(QSize(50, 15));
        sensX->setMaximum(500);
        sensX->setPageStep(10);
        sensX->setValue(100);
        sensX->setOrientation(Qt::Horizontal);
        sensX->setTickPosition(QSlider::NoTicks);

        gridLayout->addWidget(sensX, 1, 7, 1, 1);

        spinSensX = new QSpinBox(bubbleBigWidget);
        spinSensX->setObjectName(QString::fromUtf8("spinSensX"));
        spinSensX->setMinimumSize(QSize(50, 22));
        spinSensX->setMaximum(500);
        spinSensX->setSingleStep(10);
        spinSensX->setValue(100);

        gridLayout->addWidget(spinSensX, 1, 8, 1, 1);

        spinSensY = new QSpinBox(bubbleBigWidget);
        spinSensY->setObjectName(QString::fromUtf8("spinSensY"));
        spinSensY->setMinimumSize(QSize(50, 22));
        spinSensY->setMaximum(500);
        spinSensY->setSingleStep(10);
        spinSensY->setValue(100);

        gridLayout->addWidget(spinSensY, 2, 8, 2, 1);

        lblSensPitch = new QLabel(bubbleBigWidget);
        lblSensPitch->setObjectName(QString::fromUtf8("lblSensPitch"));
        lblSensPitch->setMinimumSize(QSize(25, 0));
        lblSensPitch->setMaximumSize(QSize(150, 16777215));
        lblSensPitch->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout->addWidget(lblSensPitch, 3, 0, 1, 1);

        sensPitch = new QSlider(bubbleBigWidget);
        sensPitch->setObjectName(QString::fromUtf8("sensPitch"));
        sensPitch->setMinimumSize(QSize(50, 15));
        sensPitch->setMaximum(500);
        sensPitch->setPageStep(10);
        sensPitch->setValue(100);
        sensPitch->setOrientation(Qt::Horizontal);
        sensPitch->setTickPosition(QSlider::NoTicks);

        gridLayout->addWidget(sensPitch, 3, 1, 1, 1);

        spinSensPitch = new QSpinBox(bubbleBigWidget);
        spinSensPitch->setObjectName(QString::fromUtf8("spinSensPitch"));
        spinSensPitch->setMinimumSize(QSize(50, 22));
        spinSensPitch->setMaximum(500);
        spinSensPitch->setSingleStep(10);
        spinSensPitch->setValue(100);

        gridLayout->addWidget(spinSensPitch, 3, 2, 1, 1);

        lblSensY = new QLabel(bubbleBigWidget);
        lblSensY->setObjectName(QString::fromUtf8("lblSensY"));
        lblSensY->setMinimumSize(QSize(25, 0));
        lblSensY->setMaximumSize(QSize(150, 16777215));
        lblSensY->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout->addWidget(lblSensY, 3, 6, 1, 1);

        sensY = new QSlider(bubbleBigWidget);
        sensY->setObjectName(QString::fromUtf8("sensY"));
        sensY->setMinimumSize(QSize(50, 15));
        sensY->setMaximum(500);
        sensY->setPageStep(10);
        sensY->setValue(100);
        sensY->setOrientation(Qt::Horizontal);
        sensY->setTickPosition(QSlider::NoTicks);

        gridLayout->addWidget(sensY, 3, 7, 1, 1);

        lblSensRoll = new QLabel(bubbleBigWidget);
        lblSensRoll->setObjectName(QString::fromUtf8("lblSensRoll"));
        lblSensRoll->setMinimumSize(QSize(25, 0));
        lblSensRoll->setMaximumSize(QSize(150, 16777215));
        lblSensRoll->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout->addWidget(lblSensRoll, 4, 0, 1, 1);

        sensRoll = new QSlider(bubbleBigWidget);
        sensRoll->setObjectName(QString::fromUtf8("sensRoll"));
        sensRoll->setMinimumSize(QSize(50, 15));
        sensRoll->setMaximum(500);
        sensRoll->setPageStep(10);
        sensRoll->setValue(100);
        sensRoll->setOrientation(Qt::Horizontal);
        sensRoll->setTickPosition(QSlider::NoTicks);

        gridLayout->addWidget(sensRoll, 4, 1, 1, 1);

        spinSensRoll = new QSpinBox(bubbleBigWidget);
        spinSensRoll->setObjectName(QString::fromUtf8("spinSensRoll"));
        spinSensRoll->setMinimumSize(QSize(50, 22));
        spinSensRoll->setMaximum(500);
        spinSensRoll->setSingleStep(10);
        spinSensRoll->setValue(100);

        gridLayout->addWidget(spinSensRoll, 4, 2, 1, 1);

        lblSensZ = new QLabel(bubbleBigWidget);
        lblSensZ->setObjectName(QString::fromUtf8("lblSensZ"));
        lblSensZ->setMinimumSize(QSize(25, 0));
        lblSensZ->setMaximumSize(QSize(150, 16777215));
        lblSensZ->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout->addWidget(lblSensZ, 4, 6, 1, 1);

        sensZ = new QSlider(bubbleBigWidget);
        sensZ->setObjectName(QString::fromUtf8("sensZ"));
        sensZ->setMinimumSize(QSize(50, 15));
        sensZ->setMaximum(500);
        sensZ->setPageStep(10);
        sensZ->setValue(100);
        sensZ->setOrientation(Qt::Horizontal);
        sensZ->setTickPosition(QSlider::NoTicks);

        gridLayout->addWidget(sensZ, 4, 7, 1, 1);

        spinSensZ = new QSpinBox(bubbleBigWidget);
        spinSensZ->setObjectName(QString::fromUtf8("spinSensZ"));
        spinSensZ->setMinimumSize(QSize(50, 22));
        spinSensZ->setMaximum(500);
        spinSensZ->setSingleStep(10);
        spinSensZ->setValue(100);

        gridLayout->addWidget(spinSensZ, 4, 8, 1, 1);

        chkInvertYaw = new QCheckBox(bubbleBigWidget);
        chkInvertYaw->setObjectName(QString::fromUtf8("chkInvertYaw"));

        gridLayout->addWidget(chkInvertYaw, 1, 5, 1, 1);

        lblInvert1 = new QLabel(bubbleBigWidget);
        lblInvert1->setObjectName(QString::fromUtf8("lblInvert1"));
        lblInvert1->setMinimumSize(QSize(0, 0));
        lblInvert1->setMaximumSize(QSize(30, 16777215));
        lblInvert1->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout->addWidget(lblInvert1, 0, 5, 1, 1);

        chkInvertPitch = new QCheckBox(bubbleBigWidget);
        chkInvertPitch->setObjectName(QString::fromUtf8("chkInvertPitch"));

        gridLayout->addWidget(chkInvertPitch, 3, 5, 1, 1);

        chkInvertRoll = new QCheckBox(bubbleBigWidget);
        chkInvertRoll->setObjectName(QString::fromUtf8("chkInvertRoll"));

        gridLayout->addWidget(chkInvertRoll, 4, 5, 1, 1);

        chkInvertX = new QCheckBox(bubbleBigWidget);
        chkInvertX->setObjectName(QString::fromUtf8("chkInvertX"));

        gridLayout->addWidget(chkInvertX, 1, 11, 1, 1);

        lblInvert1_2 = new QLabel(bubbleBigWidget);
        lblInvert1_2->setObjectName(QString::fromUtf8("lblInvert1_2"));
        lblInvert1_2->setMinimumSize(QSize(0, 0));
        lblInvert1_2->setMaximumSize(QSize(30, 16777215));
        lblInvert1_2->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout->addWidget(lblInvert1_2, 0, 11, 1, 1);

        chkInvertY = new QCheckBox(bubbleBigWidget);
        chkInvertY->setObjectName(QString::fromUtf8("chkInvertY"));

        gridLayout->addWidget(chkInvertY, 3, 11, 1, 1);

        chkInvertZ = new QCheckBox(bubbleBigWidget);
        chkInvertZ->setObjectName(QString::fromUtf8("chkInvertZ"));

        gridLayout->addWidget(chkInvertZ, 4, 11, 1, 1);

        redYaw = new QSlider(bubbleBigWidget);
        redYaw->setObjectName(QString::fromUtf8("redYaw"));
        redYaw->setMinimumSize(QSize(50, 15));
        redYaw->setMaximum(100);
        redYaw->setPageStep(10);
        redYaw->setValue(70);
        redYaw->setOrientation(Qt::Horizontal);
        redYaw->setTickPosition(QSlider::NoTicks);

        gridLayout->addWidget(redYaw, 1, 3, 1, 1);

        redPitch = new QSlider(bubbleBigWidget);
        redPitch->setObjectName(QString::fromUtf8("redPitch"));
        redPitch->setMinimumSize(QSize(50, 15));
        redPitch->setMaximum(100);
        redPitch->setPageStep(10);
        redPitch->setValue(70);
        redPitch->setOrientation(Qt::Horizontal);
        redPitch->setTickPosition(QSlider::NoTicks);

        gridLayout->addWidget(redPitch, 3, 3, 1, 1);

        redRoll = new QSlider(bubbleBigWidget);
        redRoll->setObjectName(QString::fromUtf8("redRoll"));
        redRoll->setMinimumSize(QSize(50, 15));
        redRoll->setMaximum(100);
        redRoll->setPageStep(10);
        redRoll->setValue(70);
        redRoll->setOrientation(Qt::Horizontal);
        redRoll->setTickPosition(QSlider::NoTicks);

        gridLayout->addWidget(redRoll, 4, 3, 1, 1);

        spinRedYaw = new QSpinBox(bubbleBigWidget);
        spinRedYaw->setObjectName(QString::fromUtf8("spinRedYaw"));
        spinRedYaw->setMinimumSize(QSize(50, 22));
        spinRedYaw->setMaximum(100);
        spinRedYaw->setSingleStep(5);
        spinRedYaw->setValue(70);

        gridLayout->addWidget(spinRedYaw, 1, 4, 1, 1);

        spinRedPitch = new QSpinBox(bubbleBigWidget);
        spinRedPitch->setObjectName(QString::fromUtf8("spinRedPitch"));
        spinRedPitch->setMinimumSize(QSize(50, 22));
        spinRedPitch->setMaximum(100);
        spinRedPitch->setSingleStep(5);
        spinRedPitch->setValue(70);

        gridLayout->addWidget(spinRedPitch, 3, 4, 1, 1);

        spinRedRoll = new QSpinBox(bubbleBigWidget);
        spinRedRoll->setObjectName(QString::fromUtf8("spinRedRoll"));
        spinRedRoll->setMinimumSize(QSize(50, 22));
        spinRedRoll->setMaximum(100);
        spinRedRoll->setSingleStep(5);
        spinRedRoll->setValue(70);

        gridLayout->addWidget(spinRedRoll, 4, 4, 1, 1);

        lblSensitivity_2 = new QLabel(bubbleBigWidget);
        lblSensitivity_2->setObjectName(QString::fromUtf8("lblSensitivity_2"));
        lblSensitivity_2->setMinimumSize(QSize(110, 0));
        lblSensitivity_2->setMaximumSize(QSize(150, 16777215));
        lblSensitivity_2->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout->addWidget(lblSensitivity_2, 0, 3, 1, 1);

        redX = new QSlider(bubbleBigWidget);
        redX->setObjectName(QString::fromUtf8("redX"));
        redX->setMinimumSize(QSize(50, 15));
        redX->setMaximum(100);
        redX->setPageStep(10);
        redX->setValue(70);
        redX->setOrientation(Qt::Horizontal);
        redX->setTickPosition(QSlider::NoTicks);

        gridLayout->addWidget(redX, 1, 9, 1, 1);

        redY = new QSlider(bubbleBigWidget);
        redY->setObjectName(QString::fromUtf8("redY"));
        redY->setMinimumSize(QSize(50, 15));
        redY->setMaximum(100);
        redY->setPageStep(10);
        redY->setValue(70);
        redY->setOrientation(Qt::Horizontal);
        redY->setTickPosition(QSlider::NoTicks);

        gridLayout->addWidget(redY, 3, 9, 1, 1);

        redZ = new QSlider(bubbleBigWidget);
        redZ->setObjectName(QString::fromUtf8("redZ"));
        redZ->setMinimumSize(QSize(50, 15));
        redZ->setMaximum(100);
        redZ->setPageStep(10);
        redZ->setValue(70);
        redZ->setOrientation(Qt::Horizontal);
        redZ->setTickPosition(QSlider::NoTicks);

        gridLayout->addWidget(redZ, 4, 9, 1, 1);

        spinRedX = new QSpinBox(bubbleBigWidget);
        spinRedX->setObjectName(QString::fromUtf8("spinRedX"));
        spinRedX->setMinimumSize(QSize(50, 22));
        spinRedX->setMaximum(100);
        spinRedX->setSingleStep(5);
        spinRedX->setValue(70);

        gridLayout->addWidget(spinRedX, 1, 10, 1, 1);

        spinRedY = new QSpinBox(bubbleBigWidget);
        spinRedY->setObjectName(QString::fromUtf8("spinRedY"));
        spinRedY->setMinimumSize(QSize(50, 22));
        spinRedY->setMaximum(100);
        spinRedY->setSingleStep(5);
        spinRedY->setValue(70);

        gridLayout->addWidget(spinRedY, 3, 10, 1, 1);

        spinRedZ = new QSpinBox(bubbleBigWidget);
        spinRedZ->setObjectName(QString::fromUtf8("spinRedZ"));
        spinRedZ->setMinimumSize(QSize(50, 22));
        spinRedZ->setMaximum(100);
        spinRedZ->setSingleStep(5);
        spinRedZ->setValue(70);

        gridLayout->addWidget(spinRedZ, 4, 10, 1, 1);

        lblSensitivity_3 = new QLabel(bubbleBigWidget);
        lblSensitivity_3->setObjectName(QString::fromUtf8("lblSensitivity_3"));
        lblSensitivity_3->setMinimumSize(QSize(110, 0));
        lblSensitivity_3->setMaximumSize(QSize(150, 16777215));
        lblSensitivity_3->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout->addWidget(lblSensitivity_3, 0, 9, 1, 1);

        lblSensitivity_4 = new QLabel(bubbleBigWidget);
        lblSensitivity_4->setObjectName(QString::fromUtf8("lblSensitivity_4"));
        lblSensitivity_4->setMinimumSize(QSize(110, 0));
        lblSensitivity_4->setMaximumSize(QSize(150, 16777215));
        lblSensitivity_4->setStyleSheet(QString::fromUtf8("color:#ccc;\n"
"background:none;"));

        gridLayout->addWidget(lblSensitivity_4, 0, 7, 1, 1);


        horizontalLayout_2->addLayout(gridLayout);

        horizontalSpacer_13 = new QSpacerItem(30, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_13);


        horizontalLayout_8->addWidget(bubbleBigWidget);


        verticalLayout_4->addLayout(horizontalLayout_8);


        horizontalLayout->addWidget(widget);


        verticalLayout_2->addLayout(horizontalLayout);

        FaceTrackNoIRClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(FaceTrackNoIRClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 970, 21));
        menuBar->setStyleSheet(QString::fromUtf8("b"));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuAbout = new QMenu(menuBar);
        menuAbout->setObjectName(QString::fromUtf8("menuAbout"));
        menuAbout->setAutoFillBackground(true);
        menuView = new QMenu(menuBar);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        menuOptions = new QMenu(menuBar);
        menuOptions->setObjectName(QString::fromUtf8("menuOptions"));
        FaceTrackNoIRClass->setMenuBar(menuBar);
        QWidget::setTabOrder(iconcomboTrackerSource, btnStartTracker);
        QWidget::setTabOrder(btnStartTracker, btnStopTracker);
        QWidget::setTabOrder(btnStopTracker, btnShowEngineControls);
        QWidget::setTabOrder(btnShowEngineControls, iconcomboBox);
        QWidget::setTabOrder(iconcomboBox, sensYaw);
        QWidget::setTabOrder(sensYaw, spinSensYaw);
        QWidget::setTabOrder(spinSensYaw, sensPitch);
        QWidget::setTabOrder(sensPitch, spinSensPitch);
        QWidget::setTabOrder(spinSensPitch, sensRoll);
        QWidget::setTabOrder(sensRoll, spinSensRoll);
        QWidget::setTabOrder(spinSensRoll, sensX);
        QWidget::setTabOrder(sensX, spinSensX);
        QWidget::setTabOrder(spinSensX, sensY);
        QWidget::setTabOrder(sensY, spinSensY);
        QWidget::setTabOrder(spinSensY, sensZ);
        QWidget::setTabOrder(sensZ, spinSensZ);
        QWidget::setTabOrder(spinSensZ, slideSmoothing);
        QWidget::setTabOrder(slideSmoothing, spinSmoothing);
        QWidget::setTabOrder(spinSmoothing, headXLine);
        QWidget::setTabOrder(headXLine, headRotXLine);
        QWidget::setTabOrder(headRotXLine, headYLine);
        QWidget::setTabOrder(headYLine, headRotYLine);
        QWidget::setTabOrder(headRotYLine, headZLine);
        QWidget::setTabOrder(headZLine, headRotZLine);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuBar->addAction(menuOptions->menuAction());
        menuBar->addAction(menuAbout->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSave);
        menuFile->addAction(actionSave_As);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menuAbout->addAction(actionAbout);
        menuView->addAction(actionVideoWidget);
        menuView->addAction(actionHeadPoseWidget);
        menuOptions->addAction(actionPreferences);
        menuOptions->addAction(actionKeyboard_Shortcuts);

        retranslateUi(FaceTrackNoIRClass);
        QObject::connect(sensYaw, SIGNAL(valueChanged(int)), spinSensYaw, SLOT(setValue(int)));
        QObject::connect(spinSensYaw, SIGNAL(valueChanged(int)), sensYaw, SLOT(setValue(int)));
        QObject::connect(sensPitch, SIGNAL(valueChanged(int)), spinSensPitch, SLOT(setValue(int)));
        QObject::connect(spinSensPitch, SIGNAL(valueChanged(int)), sensPitch, SLOT(setValue(int)));
        QObject::connect(sensRoll, SIGNAL(valueChanged(int)), spinSensRoll, SLOT(setValue(int)));
        QObject::connect(spinSensRoll, SIGNAL(valueChanged(int)), sensRoll, SLOT(setValue(int)));
        QObject::connect(sensX, SIGNAL(valueChanged(int)), spinSensX, SLOT(setValue(int)));
        QObject::connect(spinSensX, SIGNAL(valueChanged(int)), sensX, SLOT(setValue(int)));
        QObject::connect(sensY, SIGNAL(valueChanged(int)), spinSensY, SLOT(setValue(int)));
        QObject::connect(spinSensY, SIGNAL(valueChanged(int)), sensY, SLOT(setValue(int)));
        QObject::connect(sensZ, SIGNAL(valueChanged(int)), spinSensZ, SLOT(setValue(int)));
        QObject::connect(spinSensZ, SIGNAL(valueChanged(int)), sensZ, SLOT(setValue(int)));
        QObject::connect(slideSmoothing, SIGNAL(valueChanged(int)), spinSmoothing, SLOT(setValue(int)));
        QObject::connect(spinSmoothing, SIGNAL(valueChanged(int)), slideSmoothing, SLOT(setValue(int)));
        QObject::connect(redYaw, SIGNAL(valueChanged(int)), spinRedYaw, SLOT(setValue(int)));
        QObject::connect(spinRedYaw, SIGNAL(valueChanged(int)), redYaw, SLOT(setValue(int)));
        QObject::connect(redPitch, SIGNAL(valueChanged(int)), spinRedPitch, SLOT(setValue(int)));
        QObject::connect(spinRedPitch, SIGNAL(valueChanged(int)), redPitch, SLOT(setValue(int)));
        QObject::connect(redRoll, SIGNAL(valueChanged(int)), spinRedRoll, SLOT(setValue(int)));
        QObject::connect(spinRedRoll, SIGNAL(valueChanged(int)), redRoll, SLOT(setValue(int)));
        QObject::connect(redX, SIGNAL(valueChanged(int)), spinRedX, SLOT(setValue(int)));
        QObject::connect(spinRedX, SIGNAL(valueChanged(int)), redX, SLOT(setValue(int)));
        QObject::connect(redY, SIGNAL(valueChanged(int)), spinRedY, SLOT(setValue(int)));
        QObject::connect(spinRedY, SIGNAL(valueChanged(int)), redY, SLOT(setValue(int)));
        QObject::connect(redZ, SIGNAL(valueChanged(int)), spinRedZ, SLOT(setValue(int)));
        QObject::connect(spinRedZ, SIGNAL(valueChanged(int)), redZ, SLOT(setValue(int)));
        QObject::connect(slideNeutralZone, SIGNAL(valueChanged(int)), spinNeutralZone, SLOT(setValue(int)));
        QObject::connect(spinNeutralZone, SIGNAL(valueChanged(int)), slideNeutralZone, SLOT(setValue(int)));

        iconcomboTrackerSource->setCurrentIndex(-1);
        iconcomboBox->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(FaceTrackNoIRClass);
    } // setupUi

    void retranslateUi(QMainWindow *FaceTrackNoIRClass)
    {
        FaceTrackNoIRClass->setWindowTitle(QApplication::translate("FaceTrackNoIRClass", "FaceTrackNoIR", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        FaceTrackNoIRClass->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        actionOpen->setText(QApplication::translate("FaceTrackNoIRClass", "&Open", 0, QApplication::UnicodeUTF8));
        actionOpen->setShortcut(QApplication::translate("FaceTrackNoIRClass", "Ctrl+O", 0, QApplication::UnicodeUTF8));
        actionExit->setText(QApplication::translate("FaceTrackNoIRClass", "&Exit", 0, QApplication::UnicodeUTF8));
        actionExit->setShortcut(QApplication::translate("FaceTrackNoIRClass", "Ctrl+Q", 0, QApplication::UnicodeUTF8));
        actionPlayPause->setText(QApplication::translate("FaceTrackNoIRClass", "Play / Pause", 0, QApplication::UnicodeUTF8));
        actionNext->setText(QApplication::translate("FaceTrackNoIRClass", "Next", 0, QApplication::UnicodeUTF8));
        actionNext->setShortcut(QApplication::translate("FaceTrackNoIRClass", "Ctrl+.", 0, QApplication::UnicodeUTF8));
        actionPreviews->setText(QApplication::translate("FaceTrackNoIRClass", "Previews", 0, QApplication::UnicodeUTF8));
        actionPreviews->setShortcut(QApplication::translate("FaceTrackNoIRClass", "Ctrl+,", 0, QApplication::UnicodeUTF8));
        actionVolumeUp->setText(QApplication::translate("FaceTrackNoIRClass", "Volume Up", 0, QApplication::UnicodeUTF8));
        actionVolumeUp->setShortcut(QApplication::translate("FaceTrackNoIRClass", "Ctrl++", 0, QApplication::UnicodeUTF8));
        actionVolumeDown->setText(QApplication::translate("FaceTrackNoIRClass", "Volume Down", 0, QApplication::UnicodeUTF8));
        actionVolumeDown->setShortcut(QApplication::translate("FaceTrackNoIRClass", "Ctrl+-", 0, QApplication::UnicodeUTF8));
        actionVideoWidget->setText(QApplication::translate("FaceTrackNoIRClass", "Video Widget", 0, QApplication::UnicodeUTF8));
        actionVideoWidget->setShortcut(QApplication::translate("FaceTrackNoIRClass", "Ctrl+V", 0, QApplication::UnicodeUTF8));
        actionHeadPoseWidget->setText(QApplication::translate("FaceTrackNoIRClass", "HeadPose Widget", 0, QApplication::UnicodeUTF8));
        actionHeadPoseWidget->setShortcut(QApplication::translate("FaceTrackNoIRClass", "Ctrl+W", 0, QApplication::UnicodeUTF8));
        actionAbout->setText(QApplication::translate("FaceTrackNoIRClass", "About FaceTrackNoIR", 0, QApplication::UnicodeUTF8));
        actionAbout->setShortcut(QApplication::translate("FaceTrackNoIRClass", "Ctrl+A", 0, QApplication::UnicodeUTF8));
        actionMute->setText(QApplication::translate("FaceTrackNoIRClass", "Mute", 0, QApplication::UnicodeUTF8));
        actionMute->setShortcut(QApplication::translate("FaceTrackNoIRClass", "Ctrl+M", 0, QApplication::UnicodeUTF8));
        actionSave->setText(QApplication::translate("FaceTrackNoIRClass", "Save", 0, QApplication::UnicodeUTF8));
        actionSave_As->setText(QApplication::translate("FaceTrackNoIRClass", "Save As", 0, QApplication::UnicodeUTF8));
        actionKeyboard_Shortcuts->setText(QApplication::translate("FaceTrackNoIRClass", "Keyboard Shortcuts", 0, QApplication::UnicodeUTF8));
        actionPreferences->setText(QApplication::translate("FaceTrackNoIRClass", "Preferences", 0, QApplication::UnicodeUTF8));
        headRotYLine->setText(QApplication::translate("FaceTrackNoIRClass", "N/A", 0, QApplication::UnicodeUTF8));
        headRotXLine->setText(QApplication::translate("FaceTrackNoIRClass", "N/A", 0, QApplication::UnicodeUTF8));
        headRotZLine->setText(QApplication::translate("FaceTrackNoIRClass", "N/A", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("FaceTrackNoIRClass", "X", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("FaceTrackNoIRClass", "Y", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("FaceTrackNoIRClass", "Z", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("FaceTrackNoIRClass", "rotZ", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("FaceTrackNoIRClass", "rotY", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("FaceTrackNoIRClass", "rotX", 0, QApplication::UnicodeUTF8));
        headZLine->setText(QApplication::translate("FaceTrackNoIRClass", "N/A", 0, QApplication::UnicodeUTF8));
        headYLine->setText(QApplication::translate("FaceTrackNoIRClass", "N/A", 0, QApplication::UnicodeUTF8));
        headXLine->setText(QApplication::translate("FaceTrackNoIRClass", "N/A", 0, QApplication::UnicodeUTF8));
        groupTrackerSource->setTitle(QApplication::translate("FaceTrackNoIRClass", "Tracker Source", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        btnShowEngineControls->setToolTip(QApplication::translate("FaceTrackNoIRClass", "Change tracker settings", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnShowEngineControls->setText(QApplication::translate("FaceTrackNoIRClass", "Settings", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        btnStartTracker->setToolTip(QApplication::translate("FaceTrackNoIRClass", "Start the Tracker", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnStartTracker->setText(QApplication::translate("FaceTrackNoIRClass", "Start", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        btnStopTracker->setToolTip(QApplication::translate("FaceTrackNoIRClass", "Stop the Tracker", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnStopTracker->setText(QApplication::translate("FaceTrackNoIRClass", "Stop", 0, QApplication::UnicodeUTF8));
        groupGameProtocol->setTitle(QApplication::translate("FaceTrackNoIRClass", "Game protocol", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        btnShowServerControls->setToolTip(QApplication::translate("FaceTrackNoIRClass", "Change tracker settings", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnShowServerControls->setText(QApplication::translate("FaceTrackNoIRClass", "Settings", 0, QApplication::UnicodeUTF8));
        lblSensYaw_3->setText(QApplication::translate("FaceTrackNoIRClass", "Global settings", 0, QApplication::UnicodeUTF8));
        chkUseEWMA->setText(QString());
        lblSensYaw_5->setText(QApplication::translate("FaceTrackNoIRClass", "Use EWMA filtering:", 0, QApplication::UnicodeUTF8));
        lblSensYaw_4->setText(QApplication::translate("FaceTrackNoIRClass", "Smoothing (samples):", 0, QApplication::UnicodeUTF8));
        lblSensYaw_6->setText(QApplication::translate("FaceTrackNoIRClass", "Neutral Zone:", 0, QApplication::UnicodeUTF8));
        cameraName->setText(QApplication::translate("FaceTrackNoIRClass", "Camera Name", 0, QApplication::UnicodeUTF8));
        lblSensitivity->setText(QApplication::translate("FaceTrackNoIRClass", "Sensitivity (100 = x1)", 0, QApplication::UnicodeUTF8));
        lblSensYaw->setText(QApplication::translate("FaceTrackNoIRClass", "Yaw", 0, QApplication::UnicodeUTF8));
        lblSensX->setText(QApplication::translate("FaceTrackNoIRClass", "X", 0, QApplication::UnicodeUTF8));
        lblSensPitch->setText(QApplication::translate("FaceTrackNoIRClass", "Pitch", 0, QApplication::UnicodeUTF8));
        lblSensY->setText(QApplication::translate("FaceTrackNoIRClass", "Y", 0, QApplication::UnicodeUTF8));
        lblSensRoll->setText(QApplication::translate("FaceTrackNoIRClass", "Roll", 0, QApplication::UnicodeUTF8));
        lblSensZ->setText(QApplication::translate("FaceTrackNoIRClass", "Z", 0, QApplication::UnicodeUTF8));
        chkInvertYaw->setText(QString());
        lblInvert1->setText(QApplication::translate("FaceTrackNoIRClass", "Invert", 0, QApplication::UnicodeUTF8));
        chkInvertPitch->setText(QString());
        chkInvertRoll->setText(QString());
        chkInvertX->setText(QString());
        lblInvert1_2->setText(QApplication::translate("FaceTrackNoIRClass", "Invert", 0, QApplication::UnicodeUTF8));
        chkInvertY->setText(QString());
        chkInvertZ->setText(QString());
        lblSensitivity_2->setText(QApplication::translate("FaceTrackNoIRClass", "Red.factor (100 = 1)", 0, QApplication::UnicodeUTF8));
        lblSensitivity_3->setText(QApplication::translate("FaceTrackNoIRClass", "Red.factor (100 = 1)", 0, QApplication::UnicodeUTF8));
        lblSensitivity_4->setText(QApplication::translate("FaceTrackNoIRClass", "Sensitivity (100 = x1)", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("FaceTrackNoIRClass", "File", 0, QApplication::UnicodeUTF8));
        menuAbout->setTitle(QApplication::translate("FaceTrackNoIRClass", "About", 0, QApplication::UnicodeUTF8));
        menuView->setTitle(QApplication::translate("FaceTrackNoIRClass", "View", 0, QApplication::UnicodeUTF8));
        menuOptions->setTitle(QApplication::translate("FaceTrackNoIRClass", "Options", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class FaceTrackNoIRClass: public Ui_FaceTrackNoIRClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FACETRACKNOIR_H
