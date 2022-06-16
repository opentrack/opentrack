#include "globals.hpp"
#include "compat/base-path.hpp"
#include "defs.hpp"
#include "opentrack-org.hxx"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

namespace options::globals::detail {

ini_ctx::ini_ctx() = default;

static bool is_portable_installation()
{
#if defined _WIN32
    // must keep consistent between invocations
    static const bool ret = QFile::exists(OPENTRACK_BASE_PATH + "/portable.txt");
    return ret;
#else
    return false;
#endif
}

saver_::~saver_()
{
    if (--ctx.refcount == 0 && ctx.modifiedp)
    {
        auto& settings = *ctx.qsettings;
        settings.sync();
        if (settings.status() != QSettings::NoError)
            qDebug() << "error with .ini file" << settings.fileName() << settings.status();
        ctx.modifiedp = false;
    }
    ctx.mtx.unlock();
}

saver_::saver_(ini_ctx& ini) : ctx { ini }
{
    ctx.refcount++;
}

ini_ctx& cur_settings()
{
    static ini_ctx ini;
    const QString filename = ini_filename();

    ini.mtx.lock();

    if (ini.pathname != filename)
    {
        ini.qsettings.emplace(ini_combine(filename), QSettings::IniFormat);
        ini.pathname = filename;
    }

    return ini;
}

ini_ctx& global_settings()
{
    static ini_ctx& ret = progn(
        static ini_ctx ini;

        if (!is_portable_installation())
            // Windows registry or xdg on Linux
            ini.qsettings.emplace(OPENTRACK_ORG);
        else
            // file in executable's directory
            ini.qsettings.emplace(OPENTRACK_BASE_PATH + QStringLiteral("/globals.ini"),
                                  QSettings::IniFormat);

        ini.pathname = QStringLiteral(".");
        return (ini_ctx&)ini;
    );
    ret.mtx.lock();
    return ret;
}

void mark_ini_modified(bool value)
{
    auto& ini = cur_settings();
    ini.modifiedp = value;
    ini.mtx.unlock();
}

} // ns options::globals::detail

namespace options::globals
{

using namespace detail;

bool is_ini_modified()
{
    ini_ctx& ini = cur_settings();
    bool ret = ini.modifiedp;
    ini.mtx.unlock();
    return ret;
}

QString ini_filename()
{
    return with_global_settings_object([&](QSettings& settings) {
        static_assert(sizeof(OPENTRACK_DEFAULT_PROFILE) > 1);
        static_assert(sizeof(OPENTRACK_PROFILE_FILENAME_KEY) > 1);

        const QString ret = settings.value(QStringLiteral(OPENTRACK_PROFILE_FILENAME_KEY),
                                           QStringLiteral(OPENTRACK_DEFAULT_PROFILE)).toString();
        if (ret.isEmpty())
            return QStringLiteral(OPENTRACK_DEFAULT_PROFILE);
        return ret;
    });
}

QString ini_pathname()
{
    return ini_combine(ini_filename());
}

QString ini_combine(const QString& filename)
{
    return QStringLiteral("%1/%2").arg(ini_directory(), filename);
}

QStringList ini_list()
{
    static QMutex mtx;
    static QStringList list;
    QMutexLocker l{&mtx};

    const QString dirname = ini_directory();

    {
        static QDateTime last_time = {};
        auto time = QFileInfo{dirname}.lastModified();
        if (time == last_time)
            return list;
        last_time = time;
    }

    QDir settings_dir(dirname);

    using f = QDir::Filter;
    list = settings_dir.entryList({ QStringLiteral("*.ini") }, f::Files | f::Readable, QDir::Name);
    std::sort(list.begin(), list.end());
    return list;
}

void mark_global_ini_modified(bool value)
{
    auto& ini = global_settings();
    ini.modifiedp = value;
    ini.mtx.unlock();
}

static QString ini_directory_()
{
    if (detail::is_portable_installation())
    {
fail:   constexpr const char* subdir = "ini";
        QString dir = OPENTRACK_BASE_PATH;
        if (dir.isEmpty())
            dir = '.';
        (void)QDir(dir).mkpath(subdir);
        return QStringLiteral("%1/%2").arg(dir, subdir);
    }
    else
    {
        QString dir = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).value(0, QString());
        if (dir.isEmpty())
            goto fail;
        const QString fmt = QStringLiteral("%1/%2");
#if !defined _WIN32 && !defined __APPLE__
        if (!QFile::exists(fmt.arg(dir, OPENTRACK_ORG)))
        {
            dir = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).value(0, QString());
            if (dir.isEmpty())
                goto fail;
        }
#endif
        (void)QDir(dir).mkpath(OPENTRACK_ORG);
        return fmt.arg(dir, OPENTRACK_ORG);
    }
}

QString ini_directory()
{
    static const QString dir = ini_directory_();
    return dir;
}

} // ns options::globals
