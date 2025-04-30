#include <iostream>
#include "gemini_test.hpp"

int main() {
    try {
        GeminiClient client;
        std::string prompt = "Explain how AI works in a few words";
        
        std::cout << "Sending prompt: " << prompt << std::endl;
        std::string response = client.generateContent(prompt);
        std::cout << "Response: " << response << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}