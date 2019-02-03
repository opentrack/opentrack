#include "globals.hpp"
#include "compat/base-path.hpp"
#include "defs.hpp"

#include <QFile>
#include <QDir>
#include <QStandardPaths>
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
    const QString pathname = ini_pathname();

    ini.mtx.lock();

    if (pathname.isEmpty())
    {
        ini.qsettings.emplace();
        ini.pathname = pathname;
    }

    if (ini.pathname != pathname)
    {
        ini.qsettings.emplace(pathname, QSettings::IniFormat);
        ini.pathname = pathname;
    }

    return ini;
}

ini_ctx& global_settings()
{
    static ini_ctx ini;

    ini.mtx.lock();

    if (ini.pathname.isEmpty())
    {
        if (!is_portable_installation())
            // Windows registry or xdg on Linux
            ini.qsettings.emplace(OPENTRACK_ORG);
        else
        {
            static const QString pathname = OPENTRACK_BASE_PATH + QStringLiteral("/globals.ini");
            // file in executable's directory
            ini.qsettings.emplace(pathname, QSettings::IniFormat);
            ini.pathname = pathname;
        }

        ini.pathname = "placeholder";
    }

    return ini;
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
        const QString ret = settings.value(OPENTRACK_PROFILE_FILENAME_KEY, OPENTRACK_DEFAULT_PROFILE).toString();
        if (ret.size() == 0)
            return QStringLiteral(OPENTRACK_DEFAULT_PROFILE);
        return ret;
    });
}

QString ini_pathname()
{
    const auto dir = ini_directory();
    if (dir.isEmpty())
        return {};
    return dir + "/" + ini_filename();
}

QString ini_combine(const QString& filename)
{
    return ini_directory() + QStringLiteral("/") + filename;
}

QStringList ini_list()
{
    const auto dirname = ini_directory();
    if (dirname == "")
        return QStringList();
    QDir settings_dir(dirname);
    QStringList list = settings_dir.entryList( QStringList { "*.ini" } , QDir::Files, QDir::Name );
    std::sort(list.begin(), list.end());
    return list;
}

void mark_ini_modified(bool value)
{
    auto& ini = cur_settings();
    ini.modifiedp = value;
    ini.mtx.unlock();
}

static QString ini_directory_()
{
    QString dir;

    if (detail::is_portable_installation())
    {
        dir = OPENTRACK_BASE_PATH;

        static const QString subdir = "ini";

        if (!QDir(dir).mkpath(subdir))
            return {};

        return dir + '/' + subdir;
    }
    else
    {
        dir = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).value(0, QString());
        if (dir.isEmpty())
            return QString();
        if (!QDir(dir).mkpath(OPENTRACK_ORG))
            return QString();

        dir += '/';
        dir += OPENTRACK_ORG;
    }

    return dir;
}

QString ini_directory()
{
    static const QString dir = ini_directory_();
    return dir;
}

} // ns options::globals
