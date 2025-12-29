#pragma once

#include "include/cef_app.h"
#include "include/cef_v8.h"
#include <atomic>
#include <chrono>
#include <functional>

// V8 handler for native function calls from JavaScript
class NativeV8Handler : public CefV8Handler {
public:
    explicit NativeV8Handler(CefRefPtr<CefBrowser> browser) : browser_(browser) {}

    bool Execute(const CefString& name,
                 CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments,
                 CefRefPtr<CefV8Value>& retval,
                 CefString& exception) override;

private:
    CefRefPtr<CefBrowser> browser_;
    IMPLEMENT_REFCOUNTING(NativeV8Handler);
};

class JellyfinCefApp : public CefApp,
                       public CefBrowserProcessHandler,
                       public CefRenderProcessHandler {
public:
    JellyfinCefApp() = default;

    // Set settings JSON to be passed to renderer processes
    static void SetSettingsJson(const std::string& json);

    // Set nativeshell script to be injected in renderer processes
    static void SetNativeShellScript(const std::string& script);

    // Callback for external message pump scheduling
    using ScheduleWorkCallback = std::function<void(int64_t delay_ms)>;
    static void SetScheduleWorkCallback(ScheduleWorkCallback callback);
    static ScheduleWorkCallback& GetScheduleWorkCallback();

    // Activity tracking for adaptive message pump
    static void MarkActivity();
    static int64_t GetIdleTimeMs();

    // CefApp methods
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return this;
    }

    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return this;
    }

    void OnBeforeCommandLineProcessing(const CefString& process_type,
                                       CefRefPtr<CefCommandLine> command_line) override;

    void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) override;

    // CefBrowserProcessHandler methods
    void OnContextInitialized() override;
    void OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line) override;
    void OnScheduleMessagePumpWork(int64_t delay_ms) override;

    // CefRenderProcessHandler methods
    void OnContextCreated(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         CefRefPtr<CefV8Context> context) override;

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefProcessId source_process,
                                  CefRefPtr<CefProcessMessage> message) override;

private:
    static std::string settings_json_;
    static std::string nativeshell_script_;
    static ScheduleWorkCallback schedule_work_callback_;
    static std::atomic<int64_t> last_activity_time_;
    IMPLEMENT_REFCOUNTING(JellyfinCefApp);
};
