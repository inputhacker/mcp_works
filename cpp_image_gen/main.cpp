#include "GeminiImageGenerator.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    std::string api_key = std::getenv("GEMINI_API_KEY");

    GeminiImageGenerator generator(api_key);

    std::string prompt = "Hi, can you create a 3d rendered image of a pig with wings and a top hat flying over a happy futuristic scifi city with lots of greenery?";

    if (argc > 1) {
        prompt = argv[1];
    }

    std::string output_file = "gemini-native-image.png";

    if (generator.generateImageToFile(prompt, output_file)) {
        std::cout << "이미지 생성 및 저장 완료: " << output_file << std::endl;
    } else {
        std::cout << "이미지 생성 실패!" << std::endl;
    }
    return 0;
}
