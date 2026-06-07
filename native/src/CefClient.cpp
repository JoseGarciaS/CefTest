#include "CefClient.hpp"
#include "include/cef_app.h"
#include <iostream>

void MyClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
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

class PdfCallback : public CefPdfPrintCallback
{
public:
    PdfCallback(CefRefPtr<CefBrowser> browser) : browser_(browser) {};

    void OnPdfPrintFinished(const CefString &path, bool ok) override
    {
        std::cout << (ok ? "PDF saved: " : "PDF failed: ") << path.ToString() << std::endl;
        browser_->GetHost()->CloseBrowser(true);
    }

private:
    CefRefPtr<CefBrowser> browser_;
    IMPLEMENT_REFCOUNTING(PdfCallback);
};

void MyClient::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         int httpStatusCode)
{
    if (!frame->IsMain())
        return;
    CefPdfPrintSettings pdf_settings;
    browser->GetHost()->PrintToPDF("output.pdf", pdf_settings, new PdfCallback(browser));
}