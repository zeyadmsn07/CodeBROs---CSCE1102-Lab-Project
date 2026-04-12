#include "AiHelper.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream> //incase we need it for debugging.

static size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* out)
{
    int totalSize = size * nmemb;
    out->append(ptr, totalSize);
    return totalSize;
}

AiHelper::AiHelper(QObject* parent) : QObject(parent) {
}

void AiHelper::ask(const QString& code)
{
    std::string API_KEY  = "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";
    std::string endpoint = "https://openrouter.ai/api/v1/chat/completions";

    nlohmann::json body;
    body["model"] = "meta-llama/llama-3.3-70b-instruct:free";

    nlohmann::json messageObj;
    messageObj["role"] = "user";
    
    std::string promptText = "Review this C++ code and suggest improvements:\n\n";
    promptText += code.toStdString(); 
    
    messageObj["content"] = promptText;

    nlohmann::json messageArray = nlohmann::json::array();
    messageArray.push_back(messageObj);
    
    body["messages"] = messageArray;

    std::string responseStr = "";

    CURL* curl = curl_easy_init();
    if (curl == NULL) {
        emit replyReady("Error: could not start curl.");
        return;
    }

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    std::string authString = "Authorization: Bearer ";
    authString = authString + API_KEY;
    headers = curl_slist_append(headers, authString.c_str());

    std::string bodyStr = body.dump();
    
    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyStr.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);  // clean up memory
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        QString errorMsg = "Request failed: ";
        errorMsg += QString(curl_easy_strerror(res));
        emit replyReady(errorMsg);
        return;
    }

    try {
        nlohmann::json resp = nlohmann::json::parse(responseStr);
        std::string finalReply = resp["choices"][0]["message"]["content"];
        
        QString qtReply = QString::fromStdString(finalReply);
        emit replyReady(qtReply);
    }
    catch (...) {
        emit replyReady("Could not read AI response.");
    }
}