#ifndef IMGUR_H
#define IMGUR_H

#include <string>
#include <vector>
#include <curl/curl.h>
#include <QDebug>

class Imgur {

private:
    static constexpr const char* CLIENT_ID = "70c050548a48f8e";
    bool m_initialized;

    static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp);
    std::string extractImgurLink(const std::string& jsonResponse);
    
public:
    std::vector<char> downloadImage(const std::string& url);
    bool uploadRaw(const std::vector<char>& imageData, std::string& response);
    bool downloadAndUpload(const std::string& imageUrl, std::string& response);
    Imgur() {
        // Initialize libcurl globally (should be done only once)
        m_initialized = (curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK);
        if (!m_initialized) {
            qDebug() << "Failed to initialize libcurl";
        }
    }

    ~Imgur() {
        if (m_initialized) {
            // Clean up global libcurl resources
            curl_global_cleanup();
        }
    }
};

#endif