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

#if defined _MSC_VER

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int /* nCmdShow */)
{
    return main(__argc, __argv);
}
#endif
