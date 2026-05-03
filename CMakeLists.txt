cmake_minimum_required(VERSION 3.15)
project(DeepSeekPlugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(DeepSeekPlugin SHARED
    src/dllmain.cpp
    src/Config.cpp
    src/HttpClient.cpp
    src/DeepSeekItem.cpp
    src/DeepSeekPlugin.cpp
)

target_include_directories(DeepSeekPlugin PRIVATE include src)

target_link_libraries(DeepSeekPlugin PRIVATE
    winhttp
    comctl32
)

set_target_properties(DeepSeekPlugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out"
)

# Disable CRT warnings about unsafe functions
target_compile_definitions(DeepSeekPlugin PRIVATE _CRT_SECURE_NO_WARNINGS)
