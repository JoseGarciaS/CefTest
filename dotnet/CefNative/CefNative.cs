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
        [In, MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPStr)] string[] argv,
        string resourcesDir,
        string localesDir,
        string cacheDir,
        string browserSubprocessPath);

    private static IntPtr ResolveLibrary(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
{
    if (libraryName != LibName)
        return IntPtr.Zero;

    var libFileName = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
        ? "CefNative.dll"
        : RuntimeInformation.IsOSPlatform(OSPlatform.OSX)
            ? "libCefNative.dylib"
            : "libCefNative.so";

    var fullPath = Path.Combine(AppContext.BaseDirectory, libFileName);
    return File.Exists(fullPath) ? NativeLibrary.Load(fullPath) : IntPtr.Zero;
}

    public static int Run(CefOptions? options = null)
{
    options ??= new CefOptions();

    var baseDir = AppContext.BaseDirectory;
    var resourcesDir = baseDir;
    var localesDir = baseDir;
    var cacheDir = options.CacheDir ?? Path.Combine(baseDir, "cef_cache");
    var subprocessPath = Path.Combine(baseDir, RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
        ? "cef_subprocess.exe"
        : "cef_subprocess");

    if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
    {
        var contentsDir = Directory.GetParent(baseDir)?.FullName ?? baseDir;
        var bundleResourcesDir = Path.Combine(contentsDir, "Resources");
        if (Directory.Exists(bundleResourcesDir))
        {
            resourcesDir = bundleResourcesDir;
            var bundleLocalesDir = Path.Combine(bundleResourcesDir, "locales");
            if (Directory.Exists(bundleLocalesDir))
            {
                localesDir = bundleLocalesDir;
            }
        }

        var bundleHelperPath = Path.Combine(
            contentsDir,
            "Frameworks",
            "cef_subprocess Helper.app",
            "Contents",
            "MacOS",
            "cef_subprocess Helper");

        if (File.Exists(bundleHelperPath))
        {
            subprocessPath = bundleHelperPath;
        }
        else
        {
            var flatHelperPath = Path.Combine(baseDir, "cef_subprocess Helper.app", "Contents", "MacOS", "cef_subprocess Helper");
            if (File.Exists(flatHelperPath))
            {
                subprocessPath = flatHelperPath;
            }
        }
    }

        string[] argv = Environment.GetCommandLineArgs();

    return _RunWithSubprocess(argv.Length, argv, resourcesDir, localesDir, cacheDir, subprocessPath);
}
}