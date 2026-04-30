#include "AiHelper.h"
#include <curl/curl.h>
#include "config.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

static size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* out) {
    int totalSize = size * nmemb;
    out->append(ptr, totalSize);
    return totalSize;
}

AiHelper::AiHelper(QObject* parent) : QObject(parent) {}

void AiHelper::ask(const QString& code) {
    std::string endpoint = "https://openrouter.ai/api/v1/chat/completions";

    nlohmann::json body;
    body["model"] = "poolside/laguna-xs.2:free";
    
    std::string promptText = "Review this C++ code and suggest improvements:\n\n";
    promptText += code.toStdString();
    nlohmann::json messageArray = nlohmann::json::array();

    nlohmann::json sysObj;
    sysObj["role"]    = "system";
    sysObj["content"] = "You are a concise C++ tutor for beginners. "
                    "Keep answers short and direct.";
    messageArray.push_back(sysObj);

    nlohmann::json messageObj;
    messageObj["role"]    = "user";
    messageObj["content"] = promptText;
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
    headers = curl_slist_append(headers, "HTTP-Referer: http://localhost");

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

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        QString errorMsg = "Request failed: ";
        errorMsg += QString(curl_easy_strerror(res));
        emit replyReady(errorMsg);
        return;
    }

    try {
        nlohmann::json resp = nlohmann::json::parse(responseStr);

        if (resp.contains("error")) {
            std::string apiError = resp["error"]["message"];
            emit replyReady(QString::fromStdString("API Error: " + apiError));
            return;
        }

        if (resp.contains("choices") && resp["choices"].size() > 0) {
            std::string finalReply = resp["choices"][0]["message"]["content"];
            QString qtReply = QString::fromStdString(finalReply);
            emit replyReady(qtReply);
        } else {
            QString rawStr = QString::fromStdString("Unexpected structure: " + responseStr);
            emit replyReady(rawStr);
        }
    } catch (...) {
        QString crashStr = QString::fromStdString("Could not parse JSON. Raw: " + responseStr);
        emit replyReady(crashStr);
    }
}