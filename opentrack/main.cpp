#include "gui/init.hpp"
#include "main-window.hpp"

#if defined _WIN32
#   include <windows.h>
#endif

#ifdef __clang__
#   pragma GCC diagnostic ignored "-Wmain"
#endif

int main(int argc, char** argv)
{
    return run_application(argc, argv, [] { return std::make_unique<main_window>(); });
}

#if defined _MSC_VER || defined __MINGW32__
// -mwindows demands a WinMain; providing our own keeps Qt's entry-point
// shim (which assumes an msvcrt-flavored CRT) out of the link. This
// matters for UCRT-based mingw toolchains.
extern "C" int __argc;
extern "C" char** __argv;

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int /* nCmdShow */)
{
    return main(__argc, __argv);
}
#endif
