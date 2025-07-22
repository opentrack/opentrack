#pragma once
#include "export.hpp"
#include "input/key.hpp"
#include "input/key-opts.hpp"
#include <functional>
#include <tuple>

namespace opentrack::gameinput {

using fun = std::function<void(bool)>;
using t_key = std::tuple<key_opts, fun, bool>;

struct Mods
{
    bool ctrl  : 1 = false;
    bool alt   : 1 = false;
    bool shift : 1 = false;
};

class OTR_INPUT_EXPORT Worker
{
public:
    Worker(const Worker&) = delete;
    Worker& operator=(const Worker&) = delete;
    Worker(Worker&&) = delete;
    Worker& operator=(Worker&&) = delete;

    Worker();
    virtual ~Worker() noexcept;

    virtual void poll(const std::function<void(const Key&)>& fn, Mods mods) = 0;
};

extern "C" [[nodiscard]] OTR_INPUT_EXPORT Worker* make_gameinput_worker();

} // namespace opentrack::gameinput
