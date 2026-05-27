#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>

using CefRunFn = int (*)(int, char **, const char *, const char *, const char *);

int main(int argc, char **argv) {
    const char *build_dir = "/home/Amongus/Repos/CefTest/native/build/linux";
    const char *libpath = (argc > 1) ? argv[1] : "./libCefNative.so";

    void *h = dlopen(libpath, RTLD_NOW | RTLD_LOCAL);
    if (!h) {
        std::fprintf(stderr, "dlopen('%s') failed: %s\n", libpath, dlerror());
        return 2;
    }

    auto fn = (CefRunFn)dlsym(h, "CefNative_RunHelloWorld");
    if (!fn) {
        std::fprintf(stderr, "dlsym(CefNative_RunHelloWorld) failed: %s\n", dlerror());
        return 3;
    }

    char *args[2];
    args[0] = (char *)"cef_test_runner";
    args[1] = nullptr;

    const char *resources = build_dir;
    const char *locales = "/home/Amongus/Repos/CefTest/native/build/linux/locales";
    const char *cache = "/home/Amongus/Repos/CefTest/native/build/linux/cef_cache";

    return fn(1, args, resources, locales, cache);
}
