#include "include/cef_app.h"

#if defined(__APPLE__)
#include "include/wrapper/cef_library_loader.h"
#include "include/cef_sandbox_mac.h"
#endif

// Program entry-point function.
int main(int argc, char *argv[])
{

#if defined(__APPLE__)
    // Initialize the macOS sandbox for this helper process.
    CefScopedSandboxContext sandbox_context;
    if (!sandbox_context.Initialize(argc, argv))
        return 1;

    // Load the CEF framework library at runtime instead of linking directly
    // as required by the macOS sandbox implementation.
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
