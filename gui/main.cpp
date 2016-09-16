#ifdef _WIN32
#   include "opentrack-library-path.h"
#   include <stdlib.h>
#   include <vector>
#   include <QCoreApplication>
#   include <QFile>
#   include <QString>
#endif

#include "migration/migration.hpp"
#include "main-window.hpp"
#include "options/options.hpp"
using namespace options;
#include <QApplication>
#include <QCommandLineParser>
#include <QStyleFactory>
#include <QStringList>
#include <QMessageBox>
#include <QDebug>
#include <memory>
#include <cstring>

void set_qt_style()
{
#if defined(_WIN32) || defined(__APPLE__)
    // our layouts on OSX make some control wrongly sized -sh 20160908
    {
        const QStringList preferred { "fusion", "windowsvista", "macintosh" };
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
}

#ifdef _WIN32

void add_win32_path()
{
    // see https://software.intel.com/en-us/articles/limitation-to-the-length-of-the-system-path-variable
    static char env_path[4096] { '\0', };
    {
        QString lib_path = OPENTRACK_BASE_PATH;
        lib_path.replace("/", "\\");
        const QByteArray lib_path_ = QFile::encodeName(lib_path);

        QString mod_path = OPENTRACK_BASE_PATH + QString(OPENTRACK_LIBRARY_PATH);
        mod_path.replace("/", "\\");
        const QByteArray mod_path_ = QFile::encodeName(mod_path);

        std::vector<const char*> contents
        {
            "PATH=",
            lib_path_.constData(),
            ";",
            mod_path_.constData(),
            ";",
            getenv("PATH"),
        };

        bool ok = true;

        for (const char* ptr : contents)
        {
            strcat_s(env_path, sizeof(env_path), ptr);

            if (ptr == nullptr || ptr[0] == '\0' || env_path[0] == '\0')
            {
                qDebug() << "bad path element, debug info:"
                         << (ptr == nullptr ? "<null>" : ptr)
                         << (ptr != nullptr && ptr[0] == '\0')
                         << (env_path[0] == '\0');
                ok = false;
                break;
            }
        }

        if (ok)
        {
            const int error = _putenv(env_path);

            if (error)
                qDebug() << "can't _putenv win32 path";
        }
        else
            qDebug() << "can't set win32 path";
    }
}
// workaround QTBUG-38598, allow for launching from another directory
static void add_program_library_path()
{
    // Windows 10 allows for paths longer than MAX_PATH via fsutil and friends, shit
    const char* p = _pgmptr;
    char path[4096+1];

    strncpy(path, p, sizeof(path)-1);
    path[sizeof(path)-1] = '\0';

    char* ptr = strrchr(path, '\\');
    if (ptr)
    {
        *ptr = '\0';
        QCoreApplication::setLibraryPaths({ path });
    }
}
#endif

int
#ifdef _MSC_VER
WINAPI
#endif
main(int argc, char** argv)
{
#ifdef _WIN32
    add_program_library_path();
#elif !defined(__linux)
    // workaround QTBUG-38598
    QCoreApplication::addLibraryPath(".");
#endif

#if QT_VERSION >= 0x050600 // flag introduced in QT 5.6. It is non-essential so might as well allow compilation on older systems.
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication::setAttribute(Qt::AA_X11InitThreads, true);
    QApplication app(argc, argv);

    set_qt_style();
    MainWindow::set_working_directory();

#ifdef _WIN32
    add_win32_path();
#endif

    {
        mem<MainWindow> w = std::make_shared<MainWindow>();

        if (!w->is_tray_enabled())
        {
            w->setHidden(false);
            w->show();
        }
        else
        {
            w->setVisible(false);
            w->setHidden(true);
        }

        app.setQuitOnLastWindowClosed(false);
        app.exec();

        qDebug() << "exit: window";
    }

    qDebug() << "exit: main()";

    // msvc crashes again in some destructor
#if defined(_MSC_VER)
    qDebug() << "exit: terminating";
    TerminateProcess(GetCurrentProcess(), 0);
#endif

    return 0;
}

#if defined(Q_CREATOR_RUN)
#   pragma clang diagnostic ignored "-Wmain"
#endif

#ifdef _MSC_VER
int WINAPI
WinMain (struct HINSTANCE__*, struct HINSTANCE__*, char*, int)
{
  return main (__argc, __argv);
}

#endif
