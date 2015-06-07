#ifdef _WIN32
#   include <stdlib.h>
#endif

#include "ui.h"
#include "opentrack/options.hpp"
using namespace options;
#include <QApplication>
#include <QCommandLineParser>
#include <QStyleFactory>
#include <QStringList>
#include <QMessageBox>
#include <memory>
#include <cstring>

#ifdef _WIN32
// workaround QTBUG-38598, allow for launching from another directory
static void add_program_library_path()
{
    {
        char* p = _pgmptr;
        {
            char path[MAX_PATH];
            strcpy(path, p);
            char* ptr = strrchr(path, '\\');
            if (ptr)
            {
                *ptr = '\0';
                QCoreApplication::addLibraryPath(path);
            }
        }
    }
}
#endif

int main(int argc, char** argv)
{
#ifdef _WIN32
    add_program_library_path();
#endif
    // workaround QTBUG-38598
    QCoreApplication::addLibraryPath(".");

    // qt5 designer-made controls look like shit on 'doze -sh 20140921
#ifdef _WIN32
    {
        const QStringList preferred { "fusion", "windowsvista", "jazzbands'-marijuana", "macintosh", "windowsxp" };
        for (const auto& style_name : preferred)
        {
            QStyle* s = QStyleFactory::create(style_name);
            if (s)
            {
                QApplication::setStyle(s);
                break;
            }
        }
    }
#endif

    QApplication::setAttribute(Qt::AA_X11InitThreads, true);
    QApplication app(argc, argv);

	QCommandLineParser p;
	p.setApplicationDescription("opentrack - Head tracking software for MS Windows, Linux, and Apple OSX");
	p.addHelpOption();
	QCommandLineOption autostartOption(QStringList() << "a" << "autostart", "Load <profile> and start tracking", "profile");
	p.addOption(autostartOption);
	p.process(app);

	QString profile = p.value(autostartOption);
    
    bool use_profile = profile.endsWith(".ini") && QFileInfo(profile).exists() && QFileInfo(profile).isFile();
    if (!profile.isEmpty() && !use_profile)
        QMessageBox::warning(nullptr, "Can't load profile", "Profile " + profile + " specified but can't be opened!",
                             QMessageBox::Ok, QMessageBox::NoButton);
    
	if (use_profile)
        MainWindow::set_profile(profile);
    
    MainWindow w;
    
    if (use_profile)
        w.startTracker();
		
    w.show();
    app.exec();

    return 0;
}
