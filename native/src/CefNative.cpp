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
                // Load the CEF framework library at runtime instead of linking directly
                // as required by the macOS sandbox implementation.
                // CefScopedLibraryLoader library_loader;
                // if (!library_loader.LoadInMain())
                //         return 1;

                std::cout << lib_dir << std::endl;
                std::string fw_path = (std::filesystem::path(lib_dir) /
                                       "Frameworks/Chromium Embedded Framework.framework/Chromium Embedded Framework")
                                          .string();
                std::cerr << "lib_dir: " << lib_dir << std::endl;
                std::cerr << "fw_path: " << fw_path << std::endl;
                void *fw_handle = dlopen(fw_path.c_str(), RTLD_GLOBAL | RTLD_NOW);
                if (!fw_handle)
                {
                        std::cerr << "dlopen failed: " << dlerror() << std::endl;
                        return 1;
                }
                std::cerr << "Framework loaded OK" << std::endl;

                std::cerr << "Building CefMainArgs" << std::endl;
                CefMainArgs main_args2(argc, argv);
                std::cerr << "Creating MyApp" << std::endl;
                CefRefPtr<MyApp> app2(new MyApp);
                std::cerr << "Building settings" << std::endl;
                CefSettings settings2;
#endif

                std::cout << "main_args" << std::endl;
#if defined(_WIN32)
                CefMainArgs main_args(GetModuleHandle(nullptr));
#else
                CefMainArgs main_args(argc, argv);
#endif

                // Structure for passing command-line arguments.
                // The definition of this structure is platform-specific.
                std::cout << "app" << std::endl;
                // Implementation of the CefApp interface.
                CefRefPtr<MyApp> app(new MyApp);
                std::cout << "settings + sandbox" << std::endl;
                // Populate this structure to customize CEF behavior.
                CefSettings settings;
                settings.no_sandbox = true;
                std::cout << "lib_path" << std::endl;
                const std::string lib_path = std::filesystem::path(lib_dir).string();
                std::cout << lib_path << std::endl;
                std::cout << "subprocess_path" << std::endl;
#if defined(_WIN32)
                const std::string subprocess_path = (std::filesystem::path(lib_path) / "CefSubprocess.exe").string();
#elif defined(__APPLE__)
                const std::string subprocess_path = (std::filesystem::path(lib_path) / "Frameworks/CefSubprocess Helper.app/Contents/MacOS/CefSubprocess Helper").string();
#else
                const std::string subprocess_path = (std::filesystem::path(lib_path) / "CefSubprocess").string();
#endif
                std::cout << subprocess_path << std::endl;
#if defined(__APPLE__)
                // std::string fw_path = lib_path + "/CefSubprocess Helper.app/Contents/Frameworks";
                // std::string res_path = fw_path + "/Chromium Embedded Framework.framework/Resources";
                // CefString(&settings.framework_dir_path).FromString(fw_path);
                // CefString(&settings.resources_dir_path).FromString(res_path);

#else
                CefString(&settings.resources_dir_path).FromString(lib_path);
                CefString(&settings.locales_dir_path).FromString((std::filesystem::path(lib_path) / "locales").string());
#endif
                std::cout << "browser_subprocess" << std::endl;
                CefString(&settings.browser_subprocess_path).FromString(subprocess_path);
                std::cout << "log " << std::endl;
                CefString(&settings.log_file).FromString((std::filesystem::path(lib_path) / "cef_debug.log").string());
                std::cout << "cache" << std::endl;
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