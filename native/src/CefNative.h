#pragma once

#if defined(_WIN32)
#define CEF_NATIVE_EXPORT __declspec(dllexport)
#else
#define CEF_NATIVE_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
  CEF_NATIVE_EXPORT int CefNative_RunHelloWorld(
      int argc,
      char** argv,
      const char* resources_dir,
      const char* locales_dir,
      const char* cache_dir);
  // New function accepts an explicit browser subprocess path. If nullptr,
  // CefNative will attempt to resolve a helper next to the resources or exe.
  CEF_NATIVE_EXPORT int CefNative_RunHelloWorldWithSubprocessPath(
      int argc,
      char** argv,
      const char* resources_dir,
      const char* locales_dir,
      const char* cache_dir,
      const char* browser_subprocess_path);
}