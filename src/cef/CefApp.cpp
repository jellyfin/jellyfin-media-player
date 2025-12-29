#include "CefApp.h"
#include "include/cef_command_line.h"
#include "include/cef_scheme.h"
#include <QDebug>

// Static member definitions
std::string JellyfinCefApp::settings_json_;
std::string JellyfinCefApp::nativeshell_script_;
JellyfinCefApp::ScheduleWorkCallback JellyfinCefApp::schedule_work_callback_;
std::atomic<int64_t> JellyfinCefApp::last_activity_time_{0};

static int64_t now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

void JellyfinCefApp::MarkActivity() {
    last_activity_time_.store(now_ms(), std::memory_order_relaxed);
}

int64_t JellyfinCefApp::GetIdleTimeMs() {
    int64_t last = last_activity_time_.load(std::memory_order_relaxed);
    if (last == 0) return 0;  // No activity yet, treat as active
    return now_ms() - last;
}

void JellyfinCefApp::SetSettingsJson(const std::string& json) {
    settings_json_ = json;
}

void JellyfinCefApp::SetNativeShellScript(const std::string& script) {
    nativeshell_script_ = script;
}

void JellyfinCefApp::SetScheduleWorkCallback(ScheduleWorkCallback callback) {
    schedule_work_callback_ = std::move(callback);
}

JellyfinCefApp::ScheduleWorkCallback& JellyfinCefApp::GetScheduleWorkCallback() {
    return schedule_work_callback_;
}

void JellyfinCefApp::OnScheduleMessagePumpWork(int64_t delay_ms) {
    if (schedule_work_callback_) {
        schedule_work_callback_(delay_ms);
    }
}

void JellyfinCefApp::OnBeforeCommandLineProcessing(
    const CefString& /*process_type*/,
    CefRefPtr<CefCommandLine> command_line) {
    // Enable GPU acceleration
    command_line->AppendSwitch("enable-gpu-rasterization");
    command_line->AppendSwitch("ignore-gpu-blocklist");

    // Disable Google services
    command_line->AppendSwitch("disable-background-networking");
    command_line->AppendSwitch("disable-client-side-phishing-detection");
    command_line->AppendSwitch("disable-extensions");
    command_line->AppendSwitch("disable-component-update");
    command_line->AppendSwitch("disable-sync");
    command_line->AppendSwitch("disable-translate");
    command_line->AppendSwitch("disable-breakpad");
    command_line->AppendSwitchWithValue("disable-features",
        "PushMessaging,BackgroundSync,SafeBrowsing,Translate,OptimizationHints,"
        "MediaRouter,DialMediaRouteProvider,AutofillServerCommunication");
}

void JellyfinCefApp::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) {
    // Register qrc:// scheme with standard scheme flags to enable storage access
    registrar->AddCustomScheme("qrc",
        CEF_SCHEME_OPTION_STANDARD |
        CEF_SCHEME_OPTION_LOCAL |
        CEF_SCHEME_OPTION_CORS_ENABLED);
}

void JellyfinCefApp::OnContextInitialized() {
    qDebug() << "[CEF] Context initialized";
}

void JellyfinCefApp::OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line) {
    // Pass settings to renderer subprocess via command line
    if (!settings_json_.empty()) {
        command_line->AppendSwitchWithValue("jmp-settings", settings_json_);
    }
    // Pass nativeshell script to renderer subprocess via command line
    if (!nativeshell_script_.empty()) {
        command_line->AppendSwitchWithValue("jmp-nativeshell", nativeshell_script_);
    }
}

void JellyfinCefApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                       CefRefPtr<CefFrame> frame,
                                       CefRefPtr<CefV8Context> context) {
    // Create V8 handler for webchannel send function
    CefRefPtr<NativeV8Handler> handler = new NativeV8Handler(browser);

    // Create the send function
    CefRefPtr<CefV8Value> sendFunc = CefV8Value::CreateFunction("_cefWebChannelSend", handler);

    // Register the function on window object
    context->GetGlobal()->SetValue("_cefWebChannelSend", sendFunc, V8_PROPERTY_ATTRIBUTE_READONLY);

    // Get settings and script from command line (passed from browser process)
    std::string settingsJson;
    std::string nativeshellScript;
    CefRefPtr<CefCommandLine> cmdLine = CefCommandLine::GetGlobalCommandLine();
    if (cmdLine) {
        if (cmdLine->HasSwitch("jmp-settings")) {
            settingsJson = cmdLine->GetSwitchValue("jmp-settings").ToString();
        }
        if (cmdLine->HasSwitch("jmp-nativeshell")) {
            nativeshellScript = cmdLine->GetSwitchValue("jmp-nativeshell").ToString();
        }
    }

    // Build the initialization script - sets up transport shim
    std::string initScript = R"JS(
(function() {
    window.qt = {
        webChannelTransport: {
            send: function(msg) {
                if (typeof msg === 'object') {
                    msg = JSON.stringify(msg);
                }
                window._cefWebChannelSend(msg);
            }
        }
    };
)JS";

    // If we have settings from Qt, use them; otherwise use minimal stub
    if (!settingsJson.empty()) {
        initScript += "    window.jmpInfo = JSON.parse(window.atob(\"" + settingsJson + "\"));\n";
    } else {
        initScript += R"JS(
    if (!window.jmpInfo) {
        window.jmpInfo = {
            settings: { main: {}, audio: {}, video: {} },
            settingsDescriptions: {},
            settingsUpdate: [],
            settingsDescriptionsUpdate: []
        };
    }
)JS";
    }

    initScript += "})();\n";

    frame->ExecuteJavaScript(initScript, frame->GetURL(), 0);

}

bool JellyfinCefApp::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> /*browser*/,
    CefRefPtr<CefFrame> /*frame*/,
    CefProcessId /*source_process*/,
    CefRefPtr<CefProcessMessage> /*message*/) {
    // Browser-side message handling is done in CefHandler
    return false;
}

bool NativeV8Handler::Execute(const CefString& name,
                               CefRefPtr<CefV8Value> /*object*/,
                               const CefV8ValueList& arguments,
                               CefRefPtr<CefV8Value>& /*retval*/,
                               CefString& /*exception*/) {
    if (name == "_cefWebChannelSend") {
        if (arguments.size() >= 1 && arguments[0]->IsString()) {
            std::string jsonStr = arguments[0]->GetStringValue().ToString();
            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("webChannelMessage");
            msg->GetArgumentList()->SetString(0, jsonStr);
            browser_->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);
        }
        return true;
    }
    return false;
}
