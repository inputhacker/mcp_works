#include <iostream>
#include <string>
#include <cstdlib>
#include <curl/curl.h>
#include <memory>

// 응답 데이터를 저장하기 위한 콜백 함수
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

class GeminiClient {
public:
    GeminiClient() {
        // 환경 변수에서 API 키 가져오기
        const char* api_key = std::getenv("GEMINI_API_KEY");
        if (!api_key) {
            throw std::runtime_error("GEMINI_API_KEY environment variable not set");
        }
        api_key_ = api_key;
        
        // CURL 초기화
        curl_ = curl_easy_init();
        if (!curl_) {
            throw std::runtime_error("Failed to initialize CURL");
        }
    }

    ~GeminiClient() {
        if (curl_) {
            curl_easy_cleanup(curl_);
        }
    }

    std::string generateContent(const std::string& prompt) {
        std::string response;
        
        // API 엔드포인트 URL 구성
        std::string url = "https://generativelanguage.googleapis.com/v1beta/models/"
                         "gemini-2.0-flash:generateContent?key=" + api_key_;

        // JSON 요청 본문 구성
        std::string json_body = R"({
            "contents": [
                {
                    "parts": [
                        {
                            "text": ")" + prompt + R"("
                        }
                    ]
                }
            ]
        })";

        // HTTP 헤더 설정
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // CURL 옵션 설정
        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, json_body.c_str());
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

        // 요청 실행
        CURLcode res = curl_easy_perform(curl_);
        
        // 헤더 메모리 해제
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            throw std::runtime_error(std::string("curl_easy_perform() failed: ") + 
                                   curl_easy_strerror(res));
        }

        return response;
    }

private:
    CURL* curl_;
    std::string api_key_;
};
