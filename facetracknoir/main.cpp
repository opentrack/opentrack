#ifdef _WIN32
#   include <stdlib.h>
#endif

#include "wizard.h"
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
            char path[MAX_PATH+1];
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

    {
        QSettings s(OPENTRACK_ORG);
        if (!s.contains("wizard-run-once"))
        {
            s.setValue("wizard-run-once", true);
            auto w = std::make_shared<Wizard>();
            w->show();
            app.exec();
        }
    }

    auto w = std::make_shared<MainWindow>();

    w->show();
    app.exec();

    // on MSVC crashes in atexit
#ifdef _MSC_VER
    TerminateProcess(GetCurrentProcess(), 0);
#endif
    return 0;
}
