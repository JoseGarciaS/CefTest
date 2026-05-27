#include "include/cef_app.h"
#if defined(_WIN32)
#include <windows.h>
#endif

int main(int argc, char** argv) {
#if defined(_WIN32)
    CefMainArgs main_args(GetModuleHandle(NULL));
#else
    CefMainArgs main_args(argc, argv);
#endif
    return CefExecuteProcess(main_args, nullptr, nullptr);
}

