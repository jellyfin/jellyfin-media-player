#include "QrcSchemeHandler.h"
#include <QFile>
#include <QDebug>
#include <algorithm>
#include <cstring>

CefRefPtr<CefResourceHandler> QrcSchemeHandlerFactory::Create(
    CefRefPtr<CefBrowser> browser [[maybe_unused]],
    CefRefPtr<CefFrame> frame [[maybe_unused]],
    const CefString& scheme_name [[maybe_unused]],
    CefRefPtr<CefRequest> request) {

    std::string url = request->GetURL().ToString();

    // Extract path from qrc://host/path or qrc:///path
    // For qrc://, we treat everything after :// as the path (host is part of path)
    size_t schemeEnd = url.find("://");
    if (schemeEnd == std::string::npos)
        return nullptr;

    std::string pathPart = url.substr(schemeEnd + 3);

    // Ensure leading slash
    if (pathPart.empty() || pathPart[0] != '/') {
        pathPart = "/" + pathPart;
    }

    return new QrcResourceHandler(pathPart);
}

QrcResourceHandler::QrcResourceHandler(const std::string& path)
    : m_path(path) {
    m_mimeType = getMimeType(path);
}

bool QrcResourceHandler::Open(CefRefPtr<CefRequest> request [[maybe_unused]],
                               bool& handle_request,
                               CefRefPtr<CefCallback> callback [[maybe_unused]]) {
    handle_request = true;

    // Convert path to Qt resource format (:/path)
    QString qrcPath = QString::fromStdString(":" + m_path);

    QFile file(qrcPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open Qt resource:" << qrcPath;
        m_loaded = false;
        return false;
    }

    m_data = file.readAll();
    m_offset = 0;
    m_loaded = true;

    qDebug() << "Loaded Qt resource:" << qrcPath << "(" << m_data.size() << "bytes)";

    return true;
}

void QrcResourceHandler::GetResponseHeaders(CefRefPtr<CefResponse> response,
                                            int64_t& response_length,
                                            CefString& redirectUrl [[maybe_unused]]) {
    if (m_loaded) {
        response->SetStatus(200);
        response->SetStatusText("OK");
        response->SetMimeType(m_mimeType);
        response_length = m_data.size();
    } else {
        response->SetStatus(404);
        response->SetStatusText("Not Found");
        response_length = 0;
    }
}

bool QrcResourceHandler::Read(void* data_out,
                               int bytes_to_read,
                               int& bytes_read,
                               CefRefPtr<CefResourceReadCallback> callback [[maybe_unused]]) {
    if (!m_loaded || m_offset >= static_cast<size_t>(m_data.size())) {
        bytes_read = 0;
        return false;
    }

    size_t available = m_data.size() - m_offset;
    size_t to_read = std::min(static_cast<size_t>(bytes_to_read), available);

    if (to_read == 0) {
        bytes_read = 0;
        return false;
    }

    // Copy data to output buffer
    std::memcpy(data_out, m_data.constData() + m_offset, to_read);
    m_offset += to_read;
    bytes_read = static_cast<int>(to_read);

    return true;
}

void QrcResourceHandler::Cancel() {
    m_offset = m_data.size();
}

std::string QrcResourceHandler::getMimeType(const std::string& path) {
    // Find extension
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos)
        return "application/octet-stream";

    std::string ext = path.substr(dotPos);

    // Convert to lowercase for comparison
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    // Map common extensions to MIME types
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".js") return "application/javascript";
    if (ext == ".css") return "text/css";
    if (ext == ".json") return "application/json";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif") return "image/gif";
    if (ext == ".svg") return "image/svg+xml";
    if (ext == ".ico") return "image/x-icon";
    if (ext == ".woff") return "font/woff";
    if (ext == ".woff2") return "font/woff2";
    if (ext == ".ttf") return "font/ttf";
    if (ext == ".eot") return "application/vnd.ms-fontobject";
    if (ext == ".xml") return "application/xml";
    if (ext == ".txt") return "text/plain";
    if (ext == ".pdf") return "application/pdf";
    if (ext == ".zip") return "application/zip";
    if (ext == ".mp4") return "video/mp4";
    if (ext == ".webm") return "video/webm";
    if (ext == ".mp3") return "audio/mpeg";
    if (ext == ".ogg") return "audio/ogg";
    if (ext == ".wav") return "audio/wav";

    return "application/octet-stream";
}
