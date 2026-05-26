cmake_minimum_required(VERSION 3.20)
project(CefTest)

set(CMAKE_CXX_STANDARD 17)

# Point to the CEF distribution for the current platform
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CEF_ROOT "/home/Amongus/Repos/CEF/linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CEF_ROOT "C:/Repos/CEF/windows")  # adjust later
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CEF_ROOT "/home/Amongus/Repos/CEF/macos")  # adjust later
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

find_package(CEF REQUIRED)

add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)