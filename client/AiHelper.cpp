#include "AiHelper.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>

static size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* out)
{
    out->append(ptr, size * nmemb);
    return size * nmemb;
}

AiHelper::AiHelper(QObject* parent) : QObject(parent) {}

void AiHelper::ask(const QString& code)
{
    const std::string API_KEY  = "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";
    const std::string endpoint = "https://openrouter.ai/api/v1/chat/completions";

    nlohmann::json body = {
        {"model", "nvidia/nemotron-3-super-120b-a12b:free"},
        {"messages", {{
            {"role", "user"},
            {"content", "Review this C++ code and suggest improvements:\n\n"
                        + code.toStdString()}
        }}}
    };

    std::string responseStr;

    CURL* curl = curl_easy_init();
    if (!curl) {
        emit replyReady("Error: could not start curl.");
        return;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string auth = "Authorization: Bearer " + API_KEY;
    headers = curl_slist_append(headers, auth.c_str());

    std::string bodyStr = body.dump();
    curl_easy_setopt(curl, CURLOPT_URL,            endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,     headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,     bodyStr.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &responseStr);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        emit replyReady("Request failed: " + QString(curl_easy_strerror(res)));
        return;
    }

    try {
        auto resp  = nlohmann::json::parse(responseStr);
        std::string reply = resp["choices"][0]["message"]["content"];
        emit replyReady(QString::fromStdString(reply));
    }
    catch (...) {
        emit replyReady("Could not read AI response.");
    }
}