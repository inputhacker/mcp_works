cmake_minimum_required(VERSION 3.14)
project(GeminiImageGeneratorSample)

set(CMAKE_CXX_STANDARD 14)

# find curl
find_package(CURL REQUIRED)
# find jsoncpp
find_package(jsoncpp REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${CURL_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
)

add_executable(GeminiImageGenerator
    main.cpp
    src/GeminiImageGenerator.cpp
)

target_link_libraries(GeminiImageGenerator
    PRIVATE
        CURL::libcurl
        jsoncpp_lib           # 또는 jsoncpp::jsoncpp (환경에 따라 다름, 오류시 바꿔보세요)
)
