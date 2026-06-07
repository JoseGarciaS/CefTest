#include "include/cef_app.h"

#if defined(__APPLE__)
#include "include/wrapper/cef_library_loader.h"
#endif

// Program entry-point function.
int main(int argc, char *argv[])
{

#if defined(__APPLE__)
    // Load the CEF framework library at runtime.
    CefScopedLibraryLoader library_loader;
    if (!library_loader.LoadInHelper())
        return 1;
#endif

    // Structure for passing command-line arguments.
    // The definition of this structure is platform-specific.

#if defined(_WIN32)
    CefMainArgs main_args(GetModuleHandle(nullptr));
#else
    CefMainArgs main_args(argc, argv);
#endif

    // Execute the sub-process logic. This will block until the sub-process should exit.
    return CefExecuteProcess(main_args, nullptr, nullptr);
}
