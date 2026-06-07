#include "include/cef_client.h"

class MyClient : public CefClient, public CefLifeSpanHandler, public CefLoadHandler
{
    // CefClient methods.
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

    // CefLifeSpanHandler methods.
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    bool DoClose(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;

    // Member accessors.
    CefRefPtr<CefBrowser> GetBrower() const { return browser_; }
    bool IsClosing() const { return is_closing_; }

private:
    CefRefPtr<CefBrowser> browser_;
    int browser_id_ = -1; // invalid value
    int browser_count_ = 0;
    bool is_closing_ = false;

    IMPLEMENT_REFCOUNTING(MyClient);
};
