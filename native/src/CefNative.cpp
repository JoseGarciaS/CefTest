#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include <filesystem>
#include "CefNative.hpp"
#include "CefApp.hpp"

#if defined(__APPLE__)
#include "include/wrapper/cef_library_loader.h"
#endif

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

// Program entry-point function.
extern "C"
{
        CEF_NATIVE_EXPORT int CefNative_Run(int argc, char **argv, const char *lib_dir)
        {

#if defined(__APPLE__)
                // Load the CEF framework library at runtime instead of linking directly
                // as required by the macOS sandbox implementation.
                CefScopedLibraryLoader library_loader;
                if (!library_loader.LoadInMain())
                        return 1;
#endif

#if defined(_WIN32)
                CefMainArgs main_args(GetModuleHandle(nullptr));
#else
                CefMainArgs main_args(argc, argv);
#endif

                // Structure for passing command-line arguments.
                // The definition of this structure is platform-specific.

                // Implementation of the CefApp interface.
                CefRefPtr<MyApp> app(new MyApp);

                // Populate this structure to customize CEF behavior.
                CefSettings settings;
                settings.no_sandbox = true;

                const std::string lib_path(lib_dir);

#if defined(_WIN32)
                const std::string subprocess_path = lib_path + "/CefSubprocess.exe";
#elif defined(__APPLE__)
                const std::string subprocess_path = lib_path + "/CefSubprocess Helper.app/Contents/MacOS/CefSubprocess Helper";
#else
                const std::string subprocess_path = lib_path + "/CefSubprocess";
#endif

                CefString(&settings.browser_subprocess_path).FromString(subprocess_path);

                // Initialize CEF in the main process.
                CefInitialize(main_args, settings, app.get(), nullptr);

                // Run the CEF message loop. This will block until CefQuitMessageLoop() is called.
                CefRunMessageLoop();

                // Shut down CEF.
                CefShutdown();

                return 0;
        }
}