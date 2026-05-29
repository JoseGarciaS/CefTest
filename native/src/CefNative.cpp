#include "CefNative.h"

#include <filesystem>
#include <cstdio>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <unistd.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

#include "simple_handler.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"

namespace
{

    std::string GetExecutablePath()
    {
#if defined(_WIN32)
        std::wstring wide_path;
        DWORD size = MAX_PATH;
        while (true)
        {
            wide_path.resize(size);
            DWORD len = GetModuleFileNameW(nullptr, wide_path.data(), size);
            if (len == 0)
            {
                return {};
            }
            if (len < size)
            {
                wide_path.resize(len);
                break;
            }
            size *= 2;
        }

        int utf8_size =
            WideCharToMultiByte(CP_UTF8, 0, wide_path.c_str(), -1, nullptr, 0, nullptr,
                                nullptr);
        if (utf8_size <= 0)
        {
            return {};
        }

        std::string utf8_path(utf8_size - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide_path.c_str(), -1, utf8_path.data(),
                            utf8_size, nullptr, nullptr);
        return utf8_path;
#elif defined(__APPLE__)
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        if (size == 0)
        {
            return {};
        }

        std::string path(size, '\0');
        if (_NSGetExecutablePath(path.data(), &size) != 0)
        {
            return {};
        }

        return std::string(path.c_str());
#else
        std::vector<char> buffer(4096, '\0');
        ssize_t len = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
        if (len <= 0)
        {
            return {};
        }

        buffer[static_cast<size_t>(len)] = '\0';
        return std::string(buffer.data());
#endif
    }

    std::string GetExecutableDir()
    {
        const std::string exe_path = GetExecutablePath();
        if (exe_path.empty())
        {
            return {};
        }
        return std::filesystem::path(exe_path).parent_path().string();
    }

    std::string DefaultPath(const char *value, const std::string &fallback)
    {
        if (value && value[0] != '\0')
        {
            return value;
        }
        return fallback;
    }

#if defined(__APPLE__)
    std::string ResolveMacSubprocessPath(const std::string &base_dir)
    {
        const std::filesystem::path base_path(base_dir);
        const std::string helper_bundle = "cef_subprocess Helper.app";
        const std::string helper_binary = "cef_subprocess Helper";

        const std::filesystem::path flat_candidate =
            base_path / helper_bundle / "Contents" / "MacOS" / helper_binary;
        if (std::filesystem::exists(flat_candidate))
        {
            return flat_candidate.string();
        }

        const std::filesystem::path framework_candidate =
            base_path.parent_path() / "Frameworks" / helper_bundle / "Contents" / "MacOS" / helper_binary;
        if (std::filesystem::exists(framework_candidate))
        {
            return framework_candidate.string();
        }

        return {};
    }

    std::string ResolveMacResourcesPath(const std::string &base_dir)
    {
        const std::filesystem::path base_path(base_dir);
        const std::filesystem::path contents_dir = base_path.parent_path();
        const std::filesystem::path bundle_resources = contents_dir / "Resources";
        if (std::filesystem::exists(bundle_resources))
        {
            return bundle_resources.string();
        }
        return base_dir;
    }
#endif

    void SetSettingPath(cef_string_t *target, const std::string &value)
    {
        if (!value.empty())
        {
            CefString target_value(target);
            target_value = value;
        }
    }

    std::string BuildHelloUrl()
    {
        const std::string html =
            "<!doctype html><html><body style='font-family:sans-serif'>"
            "<h1>Hello World</h1>"
            "</body></html>";
        return "data:text/html," + CefURIEncode(html, false).ToString();
    }

#if defined(__linux__)
    bool EnsureLibcefLoaded()
    {
        void *handle = dlopen("libcef.so", RTLD_NOW | RTLD_GLOBAL);
        if (!handle)
        {
            const char *error = dlerror();
            std::fprintf(stderr, "Failed to load libcef.so: %s\n",
                         error ? error : "unknown error");
            return false;
        }
        return true;
    }
#endif

    class SimpleWindowDelegate : public CefWindowDelegate
    {
    public:
        explicit SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view)
            : browser_view_(browser_view) {}

        void OnWindowCreated(CefRefPtr<CefWindow> window) override
        {
            window->AddChildView(browser_view_);
            window->Show();
        }

        void OnWindowDestroyed(CefRefPtr<CefWindow> window) override
        {
            browser_view_ = nullptr;
        }

        bool CanClose(CefRefPtr<CefWindow> window) override
        {
            CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
            if (browser)
            {
                return browser->GetHost()->TryCloseBrowser();
            }
            return true;
        }

        CefSize GetPreferredSize(CefRefPtr<CefView> view) override
        {
            return CefSize(800, 600);
        }

    private:
        CefRefPtr<CefBrowserView> browser_view_;
        IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
    };

    class HelloApp : public CefApp, public CefBrowserProcessHandler
    {
    public:
        HelloApp() = default;

        CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override
        {
            return this;
        }

        void OnContextInitialized() override
        {
            CEF_REQUIRE_UI_THREAD();

            CefRefPtr<SimpleHandler> handler(new SimpleHandler(false));
            CefBrowserSettings browser_settings;

            CefRefPtr<CefBrowserView> browser_view =
                CefBrowserView::CreateBrowserView(handler, BuildHelloUrl(),
                                                  browser_settings, nullptr, nullptr,
                                                  nullptr);

            CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(browser_view));
        }

    private:
        IMPLEMENT_REFCOUNTING(HelloApp);
    };

    class ProcessApp : public CefApp
    {
    public:
        ProcessApp() = default;

    private:
        IMPLEMENT_REFCOUNTING(ProcessApp);
    };

} // namespace

extern "C"
{
    CEF_NATIVE_EXPORT int CefNative_RunHelloWorldWithSubprocessPath(
        int argc,
        char **argv,
        const char *resources_dir,
        const char *locales_dir,
        const char *cache_dir,
        const char *browser_subprocess_path)
    {
        @autoreleasepool
        {
        CefRefPtr<ProcessApp> process_app(new ProcessApp());

#if defined(__linux__)
        if (!EnsureLibcefLoaded())
        {
            return -1;
        }
#endif

#if defined(_WIN32)
        CefMainArgs main_args(GetModuleHandle(nullptr));
#else
        CefMainArgs main_args(argc, argv);
#endif

        int exit_code = CefExecuteProcess(main_args, process_app.get(), nullptr);
        if (exit_code >= 0)
        {
            return exit_code;
        }

        CefRefPtr<HelloApp> app(new HelloApp());

        const std::string exe_dir = GetExecutableDir();
#if defined(__APPLE__)
        const std::string resources_base = ResolveMacResourcesPath(exe_dir);
        const std::string resources_path = DefaultPath(resources_dir, resources_base);
#else
        const std::string resources_path = DefaultPath(resources_dir, exe_dir);
#endif
#if defined(__APPLE__)
        const std::string locales_path =
            DefaultPath(locales_dir, resources_path.empty() ? "" : resources_path + "/locales");
#else
        const std::string locales_path =
            DefaultPath(locales_dir, exe_dir.empty() ? "" : exe_dir + "/locales");
#endif
        const std::string cache_path =
            DefaultPath(cache_dir, exe_dir.empty() ? "" : exe_dir + "/cef_cache");

        CefSettings settings;
#if !defined(CEF_USE_SANDBOX)
        settings.no_sandbox = true;
#endif
        SetSettingPath(&settings.resources_dir_path, resources_path);
        SetSettingPath(&settings.locales_dir_path, locales_path);
        SetSettingPath(&settings.root_cache_path, cache_path);

        // Configure browser subprocess path. Prefer explicit param, otherwise
        // resolve to a helper next to the resources dir or executable.
        if (browser_subprocess_path && browser_subprocess_path[0] != '\0')
        {
            SetSettingPath(&settings.browser_subprocess_path, browser_subprocess_path);
        }
        else
        {
#if defined(__APPLE__)
            const std::string helper_path = ResolveMacSubprocessPath(exe_dir.empty() ? resources_path : exe_dir);
            if (!helper_path.empty())
            {
                SetSettingPath(&settings.browser_subprocess_path, helper_path);
            }
#else
            std::string helper_name = "cef_subprocess";
#if defined(_WIN32)
            helper_name += ".exe";
#endif
            const std::string base_dir = resources_path.empty() ? exe_dir : resources_path;
            if (!base_dir.empty())
            {
                std::string candidate = base_dir + "/" + helper_name;
                SetSettingPath(&settings.browser_subprocess_path, candidate);
            }
#endif
        }

        if (!CefInitialize(main_args, settings, app.get(), nullptr))
        {
            return CefGetExitCode();
        }

        CefRunMessageLoop();
        CefShutdown();
        return 0;
        }
    }

    CEF_NATIVE_EXPORT int CefNative_RunHelloWorld(
        int argc,
        char **argv,
        const char *resources_dir,
        const char *locales_dir,
        const char *cache_dir)
    {
        return CefNative_RunHelloWorldWithSubprocessPath(argc, argv, resources_dir,
                                                         locales_dir, cache_dir, nullptr);
    }
}
