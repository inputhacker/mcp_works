#ifndef GEMINI_IMAGE_GENERATOR_HPP
#define GEMINI_IMAGE_GENERATOR_HPP

#include <string>

class GeminiImageGenerator {
public:
    explicit GeminiImageGenerator(const std::string& api_key);

    // @brief 이미지를 생성해 파일로 저장
    // @param prompt 프롬프트 텍스트
    // @param output_file 저장할 파일명
    // @return true = 성공, false = 실패
    bool generateImageToFile(const std::string& prompt, const std::string& output_file);

private:
    std::string api_key_;
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

    // base64 디코딩 함수 (간단 버전)
    static bool base64DecodeToFile(const std::string& base64_data, const std::string& filename);
};

#endif // GEMINI_IMAGE_GENERATOR_HPP
