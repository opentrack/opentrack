#include "opentrack-guts.h"
#include "opentrack.h"

#if defined(__APPLE__)
#   define SONAME "dylib"
#elif defined(_WIN32)
#   define SONAME "dll"
#else
#   define SONAME "so"
#endif

#include <iostream>

#ifdef _MSC_VER
#   define LIB_PREFIX ""
#else
#   define LIB_PREFIX "lib"
#endif

static Metadata* get_metadata(DynamicLibrary* lib, QString& longName, QIcon& icon)
{
    Metadata* meta;
    if (!lib->Metadata || ((meta = lib->Metadata()), !meta))
        return NULL;
    meta->getFullName(&longName);
    meta->getIcon(&icon);
    return meta;
}

static QList<opentrack_meta> list_files(QString filter)
{
    QList<opentrack_meta> ret;
    QStringList filenames = QDir((qApp->applicationDirPath())).entryList(
                QStringList() << (LIB_PREFIX + filter + ("*." SONAME)),
                QDir::Files, QDir::Name );
    for ( int i = 0; i < filenames.size(); i++) {
        QIcon icon;
        QString long_name;
        QString str = filenames.at(i);
        DynamicLibrary* lib = new DynamicLibrary(str);
        qDebug() << "Loading" << str;
        std::cout.flush();
        Metadata* meta;
        if (!(meta = get_metadata(lib, long_name, icon)))
        {
            delete lib;
            continue;
        }
        /* TODO perhaps return full name and somesuch */
        delete meta;
        QString prefix(LIB_PREFIX + filter);
        QString suffix("." SONAME);
        if (str.size() > prefix.size() + suffix.size() && str.startsWith(prefix) && str.endsWith(suffix))
        {
            auto str2 = str.mid(prefix.size(), str.size() - prefix.size() - suffix.size());
            opentrack_meta item(str2, lib);
            ret.push_back(item);
        }
    }

    return ret;
}

opentrack_ctx::opentrack_ctx(int argc, char** argv, void* window_parent) :
    app(argc, argv),
    meta_list(list_files("opentrack-tracker-")),
    fake_frame(window_parent)
{
    const int count = meta_list.size();
    list = new char*[count + 1];
    for (int i = 0; i < count; i++)
    {
        QByteArray tmp = meta_list.at(i).path.toUtf8();
        int len = tmp.size();
        auto foo = new char[len+1];
        for (int j = 0; j < len; j++)
            foo[j] = tmp.at(j);
        foo[len] = '\0';
        list[i] = foo;
    }
    list[count] = NULL;
}

opentrack_ctx::~opentrack_ctx()
{
    for (int i = 0; list[i]; i++)
    {
        delete[] list[i];
    }
    delete[] list;
}

extern "C"
{

OPENTRACK_EXPORT const char** opentrack_enum_trackers(opentrack ctx)
{
    return const_cast<const char**>(ctx->list);
}

OPENTRACK_EXPORT opentrack  opentrack_make_ctx(int argc, char** argv, void* window_parent)
{
    return new opentrack_ctx(argc, argv, window_parent);
}

OPENTRACK_EXPORT void  opentrack_finalize_ctx(opentrack foo)
{
    delete foo;
}

}

