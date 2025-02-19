cmake_minimum_required(VERSION 3.22)
project(ProgramDesign)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin) # 可执行文件存放目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib) # 库文件存放目录

# 包含头文件路径
include_directories(${CMAKE_SOURCE_DIR}/include)

# 手动列出所有源文件
set(SRC_FILES
    ${CMAKE_SOURCE_DIR}/src/common.cpp
    ${CMAKE_SOURCE_DIR}/src/compressor.cpp
    # ${CMAKE_SOURCE_DIR}/src/decompressor.cpp
    ${CMAKE_SOURCE_DIR}/src/ui.cpp
)

# 添加动态库
add_library(ProgramLib SHARED ${SRC_FILES})

# 添加可执行文件
add_executable(ProgramDesign ${CMAKE_SOURCE_DIR}/main.cpp)

# 链接库文件到可执行文件
target_link_libraries(ProgramDesign PRIVATE ProgramLib)

# 指定头文件路径（适用于外部项目引用此库时）
target_include_directories(ProgramLib PUBLIC ${CMAKE_SOURCE_DIR}/include)