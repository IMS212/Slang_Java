cmake_minimum_required(VERSION 4.0)
project(Slang_Java)

set(CMAKE_CXX_STANDARD 20 )

include_directories(native/src/main/public)

add_executable(Slang_Java
        native/src/main/cpp/slangJava.cpp
        native/src/main/public/slangJava.h
)

target_include_directories(Slang_Java PUBLIC "/home/ims/slang/include")
FIND_PACKAGE(JNI REQUIRED)