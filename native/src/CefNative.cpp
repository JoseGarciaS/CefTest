#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include <filesystem>
#include "CefNative.hpp"
#include "CefApp.hpp"
#include <iostream>

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
                // Load the CEF framework library at runtime.
                std::string fw_path = (std::filesystem::path(lib_dir) /
                                       "Frameworks/Chromium Embedded Framework.framework/Chromium Embedded Framework")
                                          .string();
                if (!cef_load_library(fw_path.c_str()))
                {
                        std::cerr << "Failed to open/find the framework." << std::endl;
                        return 1;
                }
#endif

#if defined(_WIN32)
                CefMainArgs main_args(GetModuleHandle(nullptr));
#else
                CefMainArgs main_args(argc, argv);
#endif

                // The definition of this structure is platform-specific.
                // Implementation of the CefApp interface.
                CefRefPtr<MyApp> app(new MyApp);

                // Populate this structure to customize CEF behavior.
                CefSettings settings;
                settings.no_sandbox = true;
                const std::string lib_path = std::filesystem::path(lib_dir).string();
#if defined(_WIN32)
                const std::string subprocess_path = (std::filesystem::path(lib_path) / "CefSubprocess.exe").string();
#elif defined(__APPLE__)
                const std::string subprocess_path = (std::filesystem::path(lib_path) / "Frameworks/CefSubprocess Helper.app/Contents/MacOS/CefSubprocess Helper").string();
#else
                const std::string subprocess_path = (std::filesystem::path(lib_path) / "CefSubprocess").string();
#endif

#if defined(__APPLE__)
                std::string fw_path_dir = (std::filesystem::path(lib_dir) /
                                           "Frameworks/Chromium Embedded Framework.framework")
                                              .string();
                std::string res_path_dir = fw_path_dir + "/Resources";
                CefString(&settings.framework_dir_path).FromString(fw_path_dir);
                CefString(&settings.resources_dir_path).FromString(res_path_dir);

#else
                CefString(&settings.resources_dir_path).FromString(lib_path);
                CefString(&settings.locales_dir_path).FromString((std::filesystem::path(lib_path) / "locales").string());

#endif
                CefString(&settings.browser_subprocess_path).FromString(subprocess_path);
                CefString(&settings.log_file).FromString((std::filesystem::path(lib_path) / "cef_debug.log").string());
                CefString(&settings.cache_path).FromString((std::filesystem::path(lib_path) / "cache").string());

                // Initialize CEF in the main process.
                std::cout << "CEF initialization" << std::endl;
                if (!CefInitialize(main_args, settings, app.get(), nullptr))
                {
                        std::cout << "CEF initialization failed" << std::endl;
                        return 1;
                }
                std::cout << "CEF initialized correctly" << std::endl;

                // Run the CEF message loop. This will block until CefQuitMessageLoop() is called.
                CefRunMessageLoop();

                // Shut down CEF.
                CefShutdown();

                return 0;
        }
}