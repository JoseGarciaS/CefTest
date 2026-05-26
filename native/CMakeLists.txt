cmake_minimum_required(VERSION 3.20)
project(CefTest)

set(CMAKE_CXX_STANDARD 17)

if(NOT CEF_ROOT)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(CEF_ROOT "/tmp/cef/linux")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(CEF_ROOT "C:/cef/windows")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(CEF_ROOT "/tmp/cef/macos")
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake")

find_package(CEF REQUIRED)

add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)