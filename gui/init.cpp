/* Copyright (c) 2013-2017 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "init.hpp"
#include "migration/migration.hpp"
#include "options/options.hpp"
using namespace options;
#include "compat/library-path.hpp"
#include "compat/arch.hpp"

#include <memory>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include <QApplication>
#include <QStyleFactory>
#include <QLocale>
#include <QTranslator>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QString>
#include <QOperatingSystemVersion>
#include <QMutex>

#include <QDebug>

#include <cfloat>
#include <cfenv>

#ifdef __MINGW32__
extern "C" __declspec(dllimport) unsigned __cdecl _controlfp(unsigned, unsigned);
#endif

using std::intptr_t;

static void set_fp_mask()
{
#if defined OTR_ARCH_DENORM_DAZ
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#elif defined OTR_ARCH_DENORM_FTZ
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#endif

#ifdef OTR_ARCH_FPU_MASK
    _MM_SET_EXCEPTION_MASK(_MM_MASK_MASK);
#endif

#ifdef __APPLE__
#if defined __i386__ || defined __x86_64__
    fesetenv(FE_DFL_DISABLE_SSE_DENORMS_ENV);
#endif
#endif

#ifdef _WIN32
#   ifdef __clang__
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wreserved-id-macro"
#   endif
#   ifndef _DN_FLUSH
#       define _DN_FLUSH 0x01000000
#   endif
#   ifndef _MCW_DN
#       define _MCW_DN 0x03000000
#   endif
#   ifdef __clang__
#       pragma clang diagnostic pop
#   endif
    _controlfp(_DN_FLUSH, _MCW_DN);
#endif
}

#ifdef OTR_X11_THREADS
#include <X11/Xlib.h>
static void enable_x11_threads()
{
    (void)XInitThreads();
}
#endif

static void set_qt_style()
{
#if defined _WIN32 || defined __APPLE__
    // our layouts on OSX make some control wrongly sized -sh 20160908
    {
        const char* const preferred[] {
#ifdef __APPLE__
            "macintosh", "fusion", "windowsvista", "windows",
#else
            "fusion", "windowsvista", "windows", "windowsxp",
#endif
        };
        for (const char* style_name : preferred)
            if (QStyle* s = QStyleFactory::create(style_name); s != nullptr)
            {
                QApplication::setStyle(s);
                break;
            }
    }
#endif
}

#include <string>

#ifdef _WIN32
#   include <windows.h>
#   include <malloc.h>
#else
#   include <alloca.h>
#endif

static void qdebug_to_console(QtMsgType loglevel, const QMessageLogContext& ctx, const QString &msg)
{
    const char* level;

    switch (loglevel)
    {
    default:
    case QtDebugMsg: level = "DEBUG"; break;
    case QtWarningMsg: level = "WARN"; break;
    case QtCriticalMsg: level = "CRIT"; break;
    case QtFatalMsg: level = "FATAL"; break;
    case QtInfoMsg: level = "INFO"; break;
    }

#ifdef _WIN32
    static_assert(sizeof(wchar_t) == sizeof(decltype(*msg.utf16())));

    if (IsDebuggerPresent())
    {
        static QMutex lock;
        QMutexLocker l(&lock);

        const wchar_t* const bytes = (const wchar_t*)msg.utf16();

        OutputDebugStringW(bytes);
        OutputDebugStringW(L"\n");
    }
    else
#endif
    {
#if defined _WIN32
        const wchar_t* const bytes = (const wchar_t*)msg.utf16();
#else
        unsigned len = (unsigned)msg.size()+1;
        wchar_t* const bytes = (wchar_t*)alloca(len * sizeof(wchar_t));
        bytes[len-1] = 0;
        (void)msg.toWCharArray(bytes);
#endif
        if (ctx.file)
            std::fprintf(stderr, "%s [%s:%d]: %ls\n", level, ctx.file, ctx.line, bytes);
        else
            std::fprintf(stderr, "%s %ls\n", level, bytes);
        std::fflush(stderr);
    }
}

#ifdef _WIN32

static void apply_dark_windows_theme_if_needed()
{
    // On Windows apply dark theme if requested by user settings
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    if (settings.value("AppsUseLightTheme") == 0) {
        qApp->setStyle(QStyleFactory::create("Dark"));
        QPalette darkPalette;
        QColor darkColor = QColor(45, 45, 45);
        QColor disabledColor = QColor(127, 127, 127);
        darkPalette.setColor(QPalette::Window, darkColor);
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(18, 18, 18));
        darkPalette.setColor(QPalette::AlternateBase, darkColor);
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
        darkPalette.setColor(QPalette::Button, darkColor);
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);

        qApp->setPalette(darkPalette);

        qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    }
}

static void add_win32_path()
{
    // see https://web.archive.org/web/20180924055536/https://software.intel.com/en-us/articles/limitation-to-the-length-of-the-system-path-variable
    {
        QString lib_path = OPENTRACK_BASE_PATH;
        lib_path.replace("/", "\\");
        QString mod_path = OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH;
        mod_path.replace("/", "\\");

        const QString orig_path = qgetenv("PATH");

        QString env_path; env_path.reserve(4096);

#if 0
        qDebug() << "orig" << orig_path;
        qDebug() << "libpath" << lib_path;
        qDebug() << "modpath" << mod_path;
#endif

        if (lib_path.isEmpty())
            qDebug() << "env: empty lib_path!";
        else
        {
            if (!QFile(lib_path).exists())
                qDebug() << "env: lib_path doesn't exist, this shouldn't happen!";
            env_path += lib_path;
            env_path += ';';
        }
        if (mod_path.isEmpty())
            qDebug() << "env: can't add mod_path to env PATH";
        else
        {
            if (!QFile(mod_path).exists())
                qDebug() << "env: mod_path doesn't exist, did you install it correctly?";
            env_path += mod_path;
            env_path += ';';
        }

        if (orig_path.isEmpty())
            qDebug() << "env: empty PATH";
        else
            env_path += orig_path;

#if 0
        qDebug() << "data" << env_path.constData();
#endif

        // better length limit than putenv() and SetEnvironmentVariableA
        bool ret = SetEnvironmentVariableW(L"PATH", (const wchar_t*)env_path.constData());

        if (!ret)
            qDebug() << "_putenv() failed with" << (void*)(intptr_t)GetLastError();
    }
}

static void attach_parent_console()
{
    if (GetConsoleWindow() != nullptr)
        return;

    fflush(stdin);
    fflush(stderr);

    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        _wfreopen(L"CON", L"w", stdout);
        _wfreopen(L"CON", L"w", stderr);
        _wfreopen(L"CON", L"r", stdin);
        freopen("CON", "w", stdout);
        freopen("CON", "w", stderr);
        freopen("CON", "r", stdin);

        // skip prompt in cmd.exe window
        fprintf(stderr, "\n");
        fflush(stderr);
    }
}

#endif

static int run_window(std::unique_ptr<QWidget> main_window)
{
    if (!main_window->isEnabled())
    {
        qDebug() << "opentrack: exit before window created";
        return 2;
    }

    QApplication::setQuitOnLastWindowClosed(true);
    int status = QApplication::exec();

    return status;
}

int otr_main(int argc, char** argv, std::function<std::unique_ptr<QWidget>()> const& make_main_window)
{
#ifdef _WIN32
    (void)setvbuf(stderr, nullptr, _IONBF, 0);
#else
    (void)setvbuf(stderr, nullptr, _IOLBF, 256);
#endif

    set_fp_mask();

#ifdef OTR_X11_THREADS
    enable_x11_threads();
#endif

    QApplication app(argc, argv);

#ifdef _WIN32
    attach_parent_console();
#endif
    (void)qInstallMessageHandler(qdebug_to_console);
#ifdef _WIN32
    apply_dark_windows_theme_if_needed();
    add_win32_path();
#endif

    QDir::setCurrent(OPENTRACK_BASE_PATH);

    set_qt_style();
    QTranslator t;

    {
        const char* forced_locale = getenv("OTR_FORCE_LANG");

        if (forced_locale)
        {
            QLocale::setDefault(QLocale(forced_locale)); // force i18n for testing
            qDebug() << "locale:" << forced_locale;
        }

        using namespace options::globals;

        const bool no_i18n = with_global_settings_object([](QSettings& s) {
            return s.value("disable-translation", false).toBool();
        });

        if (forced_locale || !no_i18n)
        {
            (void) t.load(QLocale(), "", "", OPENTRACK_BASE_PATH + "/" OPENTRACK_I18N_PATH, ".qm");
            (void) QCoreApplication::installTranslator(&t);
        }
    }

    int ret = run_window(make_main_window());

#if 0
    // msvc crashes in Qt plugin system's dtor
    // Note: QLibrary::PreventUnloadHint seems to workaround it
#if defined _MSC_VER
    TerminateProcess(GetCurrentProcess(), 0);
#endif
#endif

    return ret;
}
