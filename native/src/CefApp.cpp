#include "CefApp.hpp"
#include "CefClient.hpp"

void MyWindowDelegate::OnWindowCreated(CefRefPtr<CefWindow> window)
{
    window->AddChildView(browser_view_);

#if defined(__APPLE__)
    window->Hide();
#endif
    window->Show();
}

void MyWindowDelegate::OnWindowDestroyed(CefRefPtr<CefWindow> window)
{
    browser_view_ = nullptr;
}

bool MyWindowDelegate::CanClose(CefRefPtr<CefWindow> window)
{
    CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
    if (browser)
        return browser->GetHost()->TryCloseBrowser();
    return true;
}

CefSize MyWindowDelegate::GetPreferredSize(CefRefPtr<CefView> view)
{
    return CefSize(800, 600);
}

void MyApp::OnContextInitialized()
{
    CefRefPtr<MyClient> client(new MyClient());
    CefBrowserSettings browser_settings;

    CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
        client, "https://example.com", browser_settings, nullptr, nullptr, nullptr);

    CefWindow::CreateTopLevelWindow(new MyWindowDelegate(browser_view));
}