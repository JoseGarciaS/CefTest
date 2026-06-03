#pragma once
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"

class MyWindowDelegate : public CefWindowDelegate
{
public:
    explicit MyWindowDelegate(CefRefPtr<CefBrowserView> browser_view)
        : browser_view_(browser_view) {}

    void OnWindowCreated(CefRefPtr<CefWindow> window) override;
    void OnWindowDestroyed(CefRefPtr<CefWindow> window) override;
    bool CanClose(CefRefPtr<CefWindow> window) override;
    CefSize GetPreferredSize(CefRefPtr<CefView> view) override;

private:
    CefRefPtr<CefBrowserView> browser_view_;
    IMPLEMENT_REFCOUNTING(MyWindowDelegate);
};

class MyApp : public CefApp, public CefBrowserProcessHandler
{
public:
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override
    {
        return this;
    }
    void OnContextInitialized() override;

private:
    IMPLEMENT_REFCOUNTING(MyApp);
};