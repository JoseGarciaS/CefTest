cmake_minimum_required(VERSION 3.15)
project(CefTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Linux needs RPATH so it can find libcef.so next to the executable
if(UNIX AND NOT APPLE)
    set(CMAKE_BUILD_RPATH "$ORIGIN")
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

# Platform-specific CEF path, can be overridden with -DCEF_ROOT=...
if(NOT CEF_ROOT)
    message(FATAL_ERROR "CEF_ROOT not set. Use -DCEF_ROOT=/path/to/cef")
endif()

# Use CEF's own CMake integration instead of add_subdirectory
list(APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake")
find_package(CEF REQUIRED)
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)

# Platform-specific CEF library to link against
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CEF_LIB_TARGET "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework")
else()
    set(CEF_LIB_TARGET "$<$<CONFIG:Debug>:${CEF_LIB_DEBUG}>$<$<NOT:$<CONFIG:Debug>>:${CEF_LIB_RELEASE}>")
endif()

# Main browser process executable
add_executable(CefTest2
    src/CefMain.cpp
    src/CefApp.cpp
    src/CefClient.cpp
    src/CefApp.hpp
    src/CefClient.hpp
)

target_include_directories(CefTest2 PRIVATE
    ${CEF_ROOT}
    ${CEF_ROOT}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(CefTest2
    libcef_dll_wrapper
    ${CEF_STANDARD_LIBS}
    ${CEF_LIB_TARGET}
)

if (WIN32)
    target_compile_definitions(CefTest2 PRIVATE NOMINMAX)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_target_properties(CefTest2 PROPERTIES
        MACOSX_RPATH TRUE
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@loader_path/../Frameworks"
    )
endif()

# ------------------------------------------------------------------
# Subprocess executable
add_executable(SubCefTest2
    src/CefSubprocessHelper.cpp
)

target_include_directories(SubCefTest2 PRIVATE
    ${CEF_ROOT}
    ${CEF_ROOT}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(SubCefTest2
    libcef_dll_wrapper
    ${CEF_STANDARD_LIBS}
    ${CEF_LIB_TARGET}
)

if (WIN32)
    target_compile_definitions(SubCefTest2 PRIVATE NOMINMAX)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_target_properties(SubCefTest2 PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "SubCefTest2 Helper"
        OUTPUT_NAME "SubCefTest2 Helper"
        MACOSX_RPATH TRUE
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@loader_path/../../../Frameworks"
    )
endif()

