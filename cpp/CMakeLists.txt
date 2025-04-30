cmake_minimum_required(VERSION 3.10)
project(GeminiClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CURL 라이브러리 찾기
find_package(CURL REQUIRED)

# 소스 파일 및 헤더 파일 추가
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/**/*.hpp")

# 실행 파일 생성
add_executable(${PROJECT_NAME} main.cpp ${SOURCES} ${HEADERS})

# CURL 라이브러리 링크
target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)

# include 디렉토리 추가
target_include_directories(${PROJECT_NAME} PRIVATE include)