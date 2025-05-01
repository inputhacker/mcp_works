#include "GeminiImageGenerator.hpp"
#include <curl/curl.h>
#include <json/json.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>

// 생성자 초기화
GeminiImageGenerator::GeminiImageGenerator(const std::string& api_key)
: api_key_(api_key) {}

size_t GeminiImageGenerator::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    ((std::string*)userp)->append((char*)contents, total);
    return total;
}

// 간단한 base64 디코더 (오류검사 생략)
bool GeminiImageGenerator::base64DecodeToFile(const std::string& b64, const std::string& filename) {
    // base64 디코드표
    static constexpr unsigned char kDecTable[256] = {
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,
        64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,
        64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64
    };

    std::vector<unsigned char> bin;
    int val = 0, valb = -8;
    for (unsigned char c : b64) {
        if (kDecTable[c] == 64) break;
        val = (val << 6) + kDecTable[c];
        valb += 6;
        if (valb >= 0) {
            bin.push_back(char((val >> valb) & 0xff));
            valb -= 8;
        }
    }

    std::ofstream fout(filename, std::ios::binary);
    if (!fout) return false;
    fout.write(reinterpret_cast<const char*>(bin.data()), bin.size());
    fout.close();
    return true;
}


// 실제 HTTP 요청 및 처리 로직
bool GeminiImageGenerator::generateImageToFile(const std::string& prompt, const std::string& output_file) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL 초기화 실패" << std::endl;
        return false;
    }

    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash-exp-image-generation:generateContent?key=" + api_key_;

    // JSON payload 구성
    Json::Value part;
    part["text"] = prompt;
    Json::Value parts(Json::arrayValue);
    parts.append(part);

    Json::Value content;
    content["parts"] = parts;
    Json::Value contents(Json::arrayValue);
    contents.append(content);

    Json::Value genConfig;
    genConfig["responseModalities"] = Json::arrayValue;
    genConfig["responseModalities"].append("TEXT");
    genConfig["responseModalities"].append("IMAGE");

    Json::Value root;
    root["contents"] = contents;
    root["generationConfig"] = genConfig;
    Json::StreamWriterBuilder builder;
    std::string postFields = Json::writeString(builder, root);

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string responseStr;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL 요청 실패: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    //std::cout << "responseStr=" << responseStr << std::endl;

    // 응답에서 base64 추출 (grep, cut 역할)
    Json::CharReaderBuilder rbuilder;
    Json::Value jroot;
    std::string errs;
    std::istringstream s(responseStr);

    if (!Json::parseFromStream(rbuilder, s, &jroot, &errs)) {
        std::cerr << "JSON 파싱 실패: " << errs << std::endl;
        return false;
    }

    // 응답 구조는 다음과 같음(예상)
    // {
    //   "candidates": [
    //     {
    //       "content": {
    //         "parts": [
    //             {"text":"..."}, {"inline_data":{... "data": "<base64>"}}
    //         ]
    //       },
    //       ...
    //     }
    //   ]
    // }
    std::string img_b64;
    try {
        const Json::Value& candidates = jroot["candidates"];
        if (!candidates.isArray() || candidates.empty()) throw std::runtime_error("no candidates");

        const Json::Value& parts = candidates[0]["content"]["parts"];
        for (const auto& part : parts) {
            if (part.isMember("inlineData") &&
                part["inlineData"].isMember("data")) {
                img_b64 = part["inlineData"]["data"].asString();
                break;
            }
        }
        if (img_b64.empty()) {
            std::cerr << "이미지 base64 데이터가 없습니다." << std::endl;
            return false;
        }
    } catch (...) {
        std::cerr << "응답 구조 오류(이미지 base64 추출 실패)" << std::endl;
        return false;
    }

    return base64DecodeToFile(img_b64, output_file);
}
