cmake_minimum_required(VERSION 3.10)
project(SmartHomeController)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(CURL REQUIRED)
find_package(jsoncpp REQUIRED)

# 包含目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 添加可执行文件
add_executable(smart_home
    src/main.cpp
    src/OllamaClient.cpp
    src/DeviceController.cpp
)

# 链接库
target_link_libraries(smart_home
    ${CURL_LIBRARIES}
    jsoncpp
)
