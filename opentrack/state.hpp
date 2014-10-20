#include <vector>
#include "opentrack/options.hpp"
using namespace options;
#include "opentrack/plugin-support.h"
#include "opentrack/main-settings.hpp"
#include "opentrack/mappings.hpp"

struct Modules {
    Modules() :
        module_list(dylib::enum_libraries()),
        filter_modules(filter(dylib::Filter)),
        tracker_modules(filter(dylib::Tracker)),
        protocol_modules(filter(dylib::Protocol))
    {}
    QList<ptr<dylib>>& filters() { return filter_modules; }
    QList<ptr<dylib>>& trackers() { return tracker_modules; }
    QList<ptr<dylib>>& protocols() { return protocol_modules; }
private:
    QList<ptr<dylib>> module_list;
    QList<ptr<dylib>> filter_modules;
    QList<ptr<dylib>> tracker_modules;
    QList<ptr<dylib>> protocol_modules;
    
    QList<ptr<dylib>> filter(dylib::Type t)
    {
        QList<ptr<dylib>> ret;
        for (auto x : module_list)
            if (x->type == t)
                ret.push_back(x);
        return ret;
    }
};

struct Work;

struct State {
    State() :
        b(bundle("opentrack-ui")),
        s(b),
        pose(std::vector<axis_opts*>{&s.a_x, &s.a_y, &s.a_z, &s.a_yaw, &s.a_pitch, &s.a_roll})
    {}
    Modules modules;
    SelectedLibraries libs;
    pbundle b;
    main_settings s;
    Mappings pose;
    ptr<Work> work;
};
