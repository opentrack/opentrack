#include "init.hpp"

/* Copyright (c) 2013-2017 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#if defined(Q_CREATOR_RUN)
#   pragma clang diagnostic ignored "-Wmain"
#endif

#include "migration/migration.hpp"
#include "options/options.hpp"
using namespace options;
#include "compat/library-path.hpp"

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
#include <QSysInfo>

#include <QDebug>

#if /* denormal control */ \
    /* GNU */   defined __x86_64__  || defined __SSE2__ || \
    /* MSVC */  defined _M_AMD64    || (defined _M_IX86_FP && _M_IX86_FP >= 2)
#   include <xmmintrin.h>
#   include <pmmintrin.h>
#   include <cfloat>

#define OTR_HAS_DENORM_CONTROL
void set_fp_mask()
{
    //_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
    _MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
    _MM_SET_EXCEPTION_MASK(_MM_MASK_MASK);
}
#endif

void set_qt_style()
{
#if defined _WIN32
    if (QSysInfo::WindowsVersion == QSysInfo::WV_XP)
        return;
#endif

#if defined _WIN32 || defined __APPLE__
    // our layouts on OSX make some control wrongly sized -sh 20160908
    {
        const char* const preferred[] { "fusion", "windowsvista", "macintosh" };
        for (const char* style_name : preferred)
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

void qdebug_to_console(QtMsgType, const QMessageLogContext& ctx, const QString &msg)
{
    const unsigned short* const str_ = msg.utf16();
    auto str = reinterpret_cast<const wchar_t* const>(str_);
    static_assert(sizeof(*str_) == sizeof(*str), "");

    std::fflush(stderr);
    if (ctx.function)
        std::fprintf(stderr, "[%s:%d%s]: %ls\n", ctx.file, ctx.line, ctx.function, str);
    else if (ctx.file)
        std::fprintf(stderr, "[%s:%d]: %ls\n", ctx.file, ctx.line, str);
    else
        std::fprintf(stderr, "%ls\n", str);
    std::fflush(stderr);
}

void add_win32_path()
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

#include <windows.h>

void attach_parent_console()
{
    std::fflush(stdin);
    std::fflush(stderr);

    (void)qInstallMessageHandler(qdebug_to_console);

    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        _wfreopen(L"CON", L"w", stdout);
        _wfreopen(L"CON", L"w", stderr);
        _wfreopen(L"CON", L"r", stdin);
        freopen("CON", "w", stdout);
        freopen("CON", "w", stderr);
        freopen("CON", "w", stderr);

        // skip prompt in cmd.exe window
        fprintf(stderr, "\n");
        fflush(stderr);
    }
}

#endif

int run_window(QApplication& app, std::unique_ptr<QWidget> main_window)
{
    if (!main_window->isEnabled())
    {
        qDebug() << "exit before window created";
        return 2;
    }

    app.setQuitOnLastWindowClosed(true);
    int status = app.exec();

    return status;
}

OTR_GUI_EXPORT int otr_main(int argc, char** argv, std::function<QWidget*()> const& make_main_window)
{
#if defined OTR_HAS_DENORM_CONTROL
    set_fp_mask();
#endif

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_X11InitThreads, true);

    QApplication app(argc, argv);

#ifdef _WIN32
    add_win32_path();
    attach_parent_console();
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

        const bool no_i18n = group::with_global_settings_object([](QSettings& s) {
            return s.value("disable-translation", false).toBool();
        });

        if (forced_locale || !no_i18n)
        {
            (void) t.load(QLocale(), "", "", OPENTRACK_BASE_PATH + "/" OPENTRACK_I18N_PATH, ".qm");
            (void) QCoreApplication::installTranslator(&t);
        }
    }

    int ret = run_window(app, std::unique_ptr<QWidget>(make_main_window()));

#if 0
    // msvc crashes in Qt plugin system's dtor
    // Note: QLibrary::PreventUnloadHint seems to workaround it
#if defined _MSC_VER
    TerminateProcess(GetCurrentProcess(), 0);
#endif
#endif

    return ret;
}

