#include "imgur.h"

#include <curl/curl.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <QDebug>

// Callback function for writing received data to memory
size_t Imgur::WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    std::vector<char>* mem = (std::vector<char>*)userp;
    
    size_t current_size = mem->size();
    mem->resize(current_size + realsize);
    memcpy(&((*mem)[current_size]), contents, realsize);
    
    return realsize;
}

// Function to download an image into memory
std::vector<char> Imgur::downloadImage(const std::string& url) {
    std::vector<char> buffer;
    if (!m_initialized) return buffer;

    CURL* curl = curl_easy_init();
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&buffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            buffer.clear();
        }
        
        curl_easy_cleanup(curl);
    }
    
    return buffer;
}

// Upload to Imgur using raw method
bool Imgur::uploadRaw(const std::vector<char>& imageData, std::string& response) {
    if (!m_initialized) return false;
    
    CURL* curl = curl_easy_init();
    bool success = false;
    std::vector<char> responseBuffer;
    
    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, (std::string("Authorization: Client-ID ") + this->CLIENT_ID).c_str());
        headers = curl_slist_append(headers, "Content-Type: image/jpeg"); // Adjust content type as needed
        
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.imgur.com/3/image");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, imageData.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, imageData.size());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&responseBuffer);
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            success = true;
            response = extractImgurLink(std::string(responseBuffer.begin(), responseBuffer.end()));
        } else {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        if (response.length() <= 1 || response.length() >= 256) {
            return false;
        }
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    
    return success;
}

// Function to extract the image URL from the Imgur JSON response
std::string Imgur::extractImgurLink(const std::string& jsonResponse) {
    qDebug() << "RESPONSE: " << jsonResponse.c_str();
    // Look for the "link" field in the JSON response
    const std::string linkMarker = "\"link\":\"";
    size_t linkPos = jsonResponse.find(linkMarker);
    if (linkPos == std::string::npos) {
        return "";
    }
    
    // Find the start and end of the URL
    size_t linkStart = linkPos + linkMarker.length();
    size_t linkEnd = jsonResponse.find("\"", linkStart);
    if (linkEnd == std::string::npos) {
        return "";
    }
    
    // Extract the URL
    return jsonResponse.substr(linkStart, linkEnd - linkStart);
}

bool Imgur::downloadAndUpload(const std::string& imageUrl, std::string& response) {
    std::vector<char> imageData = downloadImage(imageUrl);
    if (imageData.empty()) {
        return false;
    }
    return uploadRaw(imageData, response);
}