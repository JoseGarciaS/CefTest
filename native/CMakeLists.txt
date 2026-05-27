cmake_minimum_required(VERSION 3.20)
project(CefTest)

set(CMAKE_CXX_STANDARD 20)

if (WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(UNIX AND NOT APPLE)
    set(CMAKE_BUILD_RPATH "$ORIGIN")
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

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

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM_HANDLER_SRCS src/simple_handler_linux.cc)
    set(CEF_LIB_TARGET "$<$<CONFIG:Debug>:${CEF_LIB_DEBUG}>$<$<NOT:$<CONFIG:Debug>>:${CEF_LIB_RELEASE}>")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_HANDLER_SRCS src/simple_handler_win.cc)
    set(CEF_LIB_TARGET "$<$<CONFIG:Debug>:${CEF_LIB_DEBUG}>$<$<NOT:$<CONFIG:Debug>>:${CEF_LIB_RELEASE}>")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(PLATFORM_HANDLER_SRCS src/simple_handler_mac.mm)
    set(CEF_LIB_TARGET "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework")
endif()



# Shared library
add_library(CefNative SHARED
    src/CefNative.cpp
    src/simple_handler.cc
    ${PLATFORM_HANDLER_SRCS}
)

target_include_directories(CefNative PRIVATE
    ${CEF_ROOT}
    src
)

# On Windows, prevent min/max macro collisions with std::min/std::max
if (WIN32)
    target_compile_definitions(CefNative PRIVATE NOMINMAX)
endif()

target_link_libraries(CefNative PRIVATE
    libcef_dll_wrapper
    ${CEF_STANDARD_LIBS}
    ${CEF_LIB_TARGET}
)

SET_LIBRARY_TARGET_PROPERTIES(CefNative)

# Subprocess helper executable used when the host is not a native exe
add_executable(cef_subprocess src/cef_subprocess_main.cc)

target_include_directories(cef_subprocess PRIVATE
    ${CEF_ROOT}
    src
)

if (WIN32)
    target_compile_definitions(cef_subprocess PRIVATE NOMINMAX)
endif()

target_link_libraries(cef_subprocess PRIVATE
    libcef_dll_wrapper
    ${CEF_STANDARD_LIBS}
    ${CEF_LIB_TARGET}
)

add_custom_command(TARGET cef_subprocess POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:cef_subprocess> ${CMAKE_CURRENT_BINARY_DIR}
)
