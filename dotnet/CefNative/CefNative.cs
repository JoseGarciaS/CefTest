using System.Reflection;
using System.Runtime.InteropServices;

namespace CefNative;

public class CefOptions
{
    public string? CacheDir { get; set; }
}

public static class Cef
{
    private const string LibName = "CefNative";

    static Cef()
    {
        NativeLibrary.SetDllImportResolver(typeof(Cef).Assembly, ResolveLibrary);
    }

    [DllImport(LibName, EntryPoint = "CefNative_RunHelloWorldWithSubprocessPath")]
    private static extern int _RunWithSubprocess(
        int argc,
        string[] argv,
        string resourcesDir,
        string localesDir,
        string cacheDir,
        string browserSubprocessPath);

    private static IntPtr ResolveLibrary(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
    {
        if (libraryName != LibName)
            return IntPtr.Zero;

        var nativeDir = GetRuntimeNativeFolder();
        var libFileName = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
            ? "CefNative.dll"
            : RuntimeInformation.IsOSPlatform(OSPlatform.OSX)
                ? "libCefNative.dylib"
                : "libCefNative.so";

        return NativeLibrary.Load(Path.Combine(nativeDir, libFileName));
    }

    private static string GetRuntimeNativeFolder()
    {
        var rid = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
            ? "win-x64"
            : RuntimeInformation.IsOSPlatform(OSPlatform.OSX)
                ? "osx-x64"
                : "linux-x64";
        return Path.Combine(AppContext.BaseDirectory, "runtimes", rid, "native");
    }

    public static int Run(CefOptions? options = null)
    {
        options ??= new CefOptions();

        var nativeDir = GetRuntimeNativeFolder();
        var cacheDir = options.CacheDir ?? Path.Combine(AppContext.BaseDirectory, "cef_cache");
        var subprocessPath = Path.Combine(nativeDir, RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
            ? "cef_subprocess.exe"
            : "cef_subprocess");

        string[] argv = [AppContext.BaseDirectory];

        return _RunWithSubprocess(argv.Length, argv, nativeDir, nativeDir, cacheDir, subprocessPath);
    }
}