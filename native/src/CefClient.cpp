#include "CefClient.hpp"
#include "include/cef_thread.h"
#include "include/cef_app.h"

void MyClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    // Must be executed on the UI thread.
    // REQUIRE_UI_THREAD();

    if (!browser_)
    {
        // Keep a reference to the main browser.
        browser_ = browser;
        browser_id_ = browser->GetIdentifier();
    }

    // Keep track of how many browsers currently exist.
    browser_count_++;
}

bool MyClient::DoClose(CefRefPtr<CefBrowser> browser)
{
    if (browser->GetIdentifier() == browser_id_)
        is_closing_ = true;
    return false;
}

void MyClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    if (browser->GetIdentifier() == browser_id_)
        browser_ = nullptr;

    if (--browser_count_ == 0)
        CefQuitMessageLoop();
}