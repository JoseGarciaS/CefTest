using System.Runtime.InteropServices;
using System.Reflection;
using System;
using System.IO;

namespace CefNative;

public static class Cef
{
    private const string LibName = "CefProcess";

    static Cef()
    {
        NativeLibrary.SetDllImportResolver(typeof(Cef).Assembly, ResolveLibrary);
    }

    [DllImport(LibName, EntryPoint = "CefNative_Run")]
    private static extern int _Run(int argc, IntPtr argv);
    private static IntPtr ResolveLibrary(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
    {
        if (libraryName != LibName)
            return IntPtr.Zero;

        var libFileName = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
            ? "CefProcess.dll"
            : RuntimeInformation.IsOSPlatform(OSPlatform.OSX)
                ? "libCefProcess.dylib"
                : "libCefProcess.so";

        var fullPath = Path.Combine(AppContext.BaseDirectory, libFileName);
        return File.Exists(fullPath) ? NativeLibrary.Load(fullPath) : IntPtr.Zero;
    }

    public static int Run()
    {
        return _Run(0, IntPtr.Zero);
    }
}

