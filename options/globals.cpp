#include "globals.hpp"
#include "compat/base-path.hpp"
#include "defs.hpp"

#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

namespace options::globals::detail {

bool is_portable_installation()
{
#if defined _WIN32
    // must keep consistent between invocations
    static const bool ret = QFile::exists(OPENTRACK_BASE_PATH + "/portable.txt");
    return ret;
#endif
    return false;
}

saver_::~saver_()
{
    if (--ctx.refcount == 0 && ctx.modifiedp)
    {
        ctx.modifiedp = false;
        auto& settings = *ctx.qsettings;
        settings.sync();
        if (settings.status() != QSettings::NoError)
            qDebug() << "error with .ini file" << settings.fileName() << settings.status();
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
    static QString ini_pathname;

    ini.mtx.lock();

    if (pathname.isEmpty())
    {
        ini.qsettings.emplace();
        ini.pathname = pathname;
    }

    if (pathname != ini_pathname)
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

    return ini;
}

} // ns options::globals::detail

namespace options::globals
{

QString ini_filename()
{
    return with_global_settings_object([&](QSettings& settings) {
        const QString ret = settings.value(OPENTRACK_CONFIG_FILENAME_KEY, OPENTRACK_DEFAULT_CONFIG).toString();
        if (ret.size() == 0)
            return QStringLiteral(OPENTRACK_DEFAULT_CONFIG);
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

void mark_ini_modified()
{
    using namespace detail;
    auto& ini = cur_settings();
    ini.modifiedp = true;
    ini.mtx.unlock();
}

QString ini_directory()
{
    QString dir;

    if (detail::is_portable_installation())
    {
        dir = OPENTRACK_BASE_PATH;

        static const QString subdir = "ini";

        if (!QDir(dir).mkpath(subdir))
            return QString();

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

} // ns options::globals
