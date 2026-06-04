#pragma once

#if defined(_WIN32)
#define CEF_NATIVE_EXPORT __declspec(dllexport)
#else
#define CEF_NATIVE_EXPORT __attribute__((visibility("default")))
#endif

extern "C"
{
    CEF_NATIVE_EXPORT int CefNative_Run(
        int argc,
        char **argv);
}