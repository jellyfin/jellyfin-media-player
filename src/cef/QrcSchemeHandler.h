#pragma once

#include "include/cef_resource_handler.h"
#include "include/cef_scheme.h"
#include <QByteArray>
#include <string>

// Factory for creating QRC resource handlers
class QrcSchemeHandlerFactory : public CefSchemeHandlerFactory {
public:
    QrcSchemeHandlerFactory() = default;

    CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         const CefString& scheme_name,
                                         CefRefPtr<CefRequest> request) override;

    IMPLEMENT_REFCOUNTING(QrcSchemeHandlerFactory);
};

// Resource handler for Qt resource files (qrc://)
class QrcResourceHandler : public CefResourceHandler {
public:
    explicit QrcResourceHandler(const std::string& path);

    bool Open(CefRefPtr<CefRequest> request,
              bool& handle_request,
              CefRefPtr<CefCallback> callback) override;

    void GetResponseHeaders(CefRefPtr<CefResponse> response,
                           int64_t& response_length,
                           CefString& redirectUrl) override;

    bool Read(void* data_out,
              int bytes_to_read,
              int& bytes_read,
              CefRefPtr<CefResourceReadCallback> callback) override;

    void Cancel() override;

    IMPLEMENT_REFCOUNTING(QrcResourceHandler);

private:
    std::string getMimeType(const std::string& path);

    std::string m_path;
    QByteArray m_data;
    size_t m_offset = 0;
    std::string m_mimeType;
    bool m_loaded = false;
};
