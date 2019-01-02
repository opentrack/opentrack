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
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include <QApplication>
#include <QStyleFactory>
#include <QLocale>
#include <QTranslator>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QString>
#include <QOperatingSystemVersion>

#include <QDebug>

#include <cfloat>
#include <cfenv>

#ifdef __MINGW32__
extern "C" __declspec(dllimport) unsigned __cdecl _controlfp(unsigned, unsigned);
#endif

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
    fesetenv(FE_DFL_DISABLE_SSE_DENORMS_ENV);
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

#include "compat/spinlock.hpp"
#include <string>

#ifdef _WIN32
#   include <windows.h>
#endif

static void qdebug_to_console(QtMsgType, const QMessageLogContext& ctx, const QString &msg)
{
    static std::atomic_flag lock = ATOMIC_FLAG_INIT;
    const auto bytes{msg.toUtf8()};

    constexpr bool is_win32 =
#ifdef _WIN32
        true;
#else
        false;
#endif


    if constexpr (is_win32)
    {
        if (IsDebuggerPresent())
        {
            spinlock_guard l(lock);

            OutputDebugStringA(bytes.constData());
            OutputDebugStringA("\n");
        }
        else
        {
            std::fflush(stderr);
            const char* const s = bytes.constData();
            {
                spinlock_guard l(lock);

                if (ctx.function)
                    std::fprintf(stderr, "[%s:%d] %s: %s\n", ctx.file, ctx.line, ctx.function, s);
                else if (ctx.file)
                    std::fprintf(stderr, "[%s:%d]: %s\n", ctx.file, ctx.line, s);
                else
                    std::fprintf(stderr, "%s\n", s);
            }
            std::fflush(stderr);
        }
    }
}

#ifdef _WIN32

static void add_win32_path()
{
    // see https://software.intel.com/en-us/articles/limitation-to-the-length-of-the-system-path-variable
    static char env_path[4096] { '\0', };
    {
        QString lib_path = OPENTRACK_BASE_PATH;
        lib_path.replace("/", "\\");
        const QByteArray lib_path_ = QFile::encodeName(lib_path);

        QString mod_path = OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH;
        mod_path.replace("/", "\\");
        const QByteArray mod_path_ = QFile::encodeName(mod_path);

        const char* contents[] {
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
            if (ptr)
                strcat_s(env_path, sizeof(env_path), ptr);

            if (!ptr || ptr[0] == '\0' || env_path[0] == '\0')
            {
                qDebug() << "bad path element"
                         << (ptr == nullptr ? "<null>" : ptr);
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

int otr_main(int argc, char** argv, std::function<QWidget*()> const& make_main_window)
{
    set_fp_mask();

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QCoreApplication::setAttribute(Qt::AA_X11InitThreads);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);

#ifdef _WIN32
    add_win32_path();
    attach_parent_console();
#endif

    (void)qInstallMessageHandler(qdebug_to_console);

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

    int ret = run_window(std::unique_ptr<QWidget>(make_main_window()));

#if 0
    // msvc crashes in Qt plugin system's dtor
    // Note: QLibrary::PreventUnloadHint seems to workaround it
#if defined _MSC_VER
    TerminateProcess(GetCurrentProcess(), 0);
#endif
#endif

    return ret;
}

