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
# if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
#     set(CEF_LIB_TARGET "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework")
# else()
#     set(CEF_LIB_TARGET "$<$<CONFIG:Debug>:${CEF_LIB_DEBUG}>$<$<NOT:$<CONFIG:Debug>>:${CEF_LIB_RELEASE}>")
# endif()

set(CEF_LIB_TARGET "$<$<CONFIG:Debug>:${CEF_LIB_DEBUG}>$<$<NOT:$<CONFIG:Debug>>:${CEF_LIB_RELEASE}>")

# Main browser process executable
add_library(CefProcess SHARED
    src/CefNative.cpp
    src/CefNative.hpp
    src/CefApp.cpp
    src/CefClient.cpp
    src/CefApp.hpp
    src/CefClient.hpp
)

target_include_directories(CefProcess PRIVATE
    ${CEF_ROOT}
    ${CEF_ROOT}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(CefProcess
    libcef_dll_wrapper
    ${CEF_STANDARD_LIBS}
    $<$<NOT:$<PLATFORM_ID:Darwin>>:${CEF_LIB_TARGET}>
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(CefProcess
        "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework/Chromium Embedded Framework"
    )
    set_target_properties(CefProcess PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@loader_path/Frameworks"
    )
endif()

if (WIN32)
    target_compile_definitions(CefProcess PRIVATE NOMINMAX)
endif()

# ------------------------------------------------------------------
# Subprocess executable
add_executable(CefSubprocess
    src/CefSubprocessHelper.cpp
)

target_include_directories(CefSubprocess PRIVATE
    ${CEF_ROOT}
    ${CEF_ROOT}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(CefSubprocess
    libcef_dll_wrapper
    ${CEF_STANDARD_LIBS}
    $<$<NOT:$<PLATFORM_ID:Darwin>>:${CEF_LIB_TARGET}>
)

if (WIN32)
    target_compile_definitions(CefSubprocess PRIVATE NOMINMAX)
endif()


if(UNIX AND NOT APPLE)
    target_link_options(CefProcess PRIVATE -static-libstdc++ -static-libgcc)
    target_link_options(CefSubprocess PRIVATE -static-libstdc++ -static-libgcc)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

    set_target_properties(CefSubprocess PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "CefSubprocess Helper"
        MACOSX_BUNDLE_BUNDLE_IDENTIFIER "com.test.ceftest.helper"
        OUTPUT_NAME "CefSubprocess Helper"
    )

    set(MACOS_FRAMEWORKS "${CMAKE_CURRENT_BINARY_DIR}/Frameworks")
    add_dependencies(CefProcess CefSubprocess)

    add_custom_command(TARGET CefProcess POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "${MACOS_FRAMEWORKS}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework"
            "${MACOS_FRAMEWORKS}/Chromium Embedded Framework.framework"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_BINARY_DIR}/CefSubprocess Helper.app"
            "${MACOS_FRAMEWORKS}/CefSubprocess Helper.app"
        VERBATIM
    )
endif()