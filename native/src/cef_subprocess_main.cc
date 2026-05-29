#include "include/cef_app.h"
#if defined(__APPLE__)
#include "include/wrapper/cef_library_loader.h"
#endif
#if defined(_WIN32)
#include <windows.h>
#endif

int main(int argc, char** argv) {
#if defined(__APPLE__)
    CefScopedLibraryLoader library_loader;
    if (!library_loader.LoadInHelper()) {
        return 1;
    }
#endif

#if defined(_WIN32)
    CefMainArgs main_args(GetModuleHandle(NULL));
#else
    CefMainArgs main_args(argc, argv);
#endif
    return CefExecuteProcess(main_args, nullptr, nullptr);
}

