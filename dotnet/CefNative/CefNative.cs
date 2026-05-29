using System.Diagnostics;
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
    private const int RtldNow = 0x2;
    private const int RtldGlobal = 0x8;

    [DllImport("libdl.dylib")]
    private static extern IntPtr dlopen(string path, int mode);

    static Cef()
    {
        NativeLibrary.SetDllImportResolver(typeof(Cef).Assembly, ResolveLibrary);
    }

    [DllImport(LibName, EntryPoint = "CefNative_RunHelloWorldWithSubprocessPath")]
    private static extern int _RunWithSubprocess(
        int argc,
        IntPtr argv,
        string resourcesDir,
        string localesDir,
        string cacheDir,
        string browserSubprocessPath);

    private static IntPtr BuildArgv(string[] argv, out IntPtr[] allocatedStrings)
    {
        allocatedStrings = new IntPtr[argv.Length + 1];

        for (int index = 0; index < argv.Length; index++)
        {
            allocatedStrings[index] = Marshal.StringToHGlobalAnsi(argv[index]);
        }

        allocatedStrings[argv.Length] = IntPtr.Zero;

        IntPtr argvBuffer = Marshal.AllocHGlobal(allocatedStrings.Length * IntPtr.Size);
        for (int index = 0; index < allocatedStrings.Length; index++)
        {
            Marshal.WriteIntPtr(argvBuffer, index * IntPtr.Size, allocatedStrings[index]);
        }

        return argvBuffer;
    }

    private static void FreeArgv(IntPtr argvBuffer, IntPtr[] allocatedStrings)
    {
        foreach (IntPtr allocatedString in allocatedStrings)
        {
            if (allocatedString != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(allocatedString);
            }
        }

        if (argvBuffer != IntPtr.Zero)
        {
            Marshal.FreeHGlobal(argvBuffer);
        }
    }

    private static IntPtr ResolveLibrary(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
{
    if (libraryName != LibName)
        return IntPtr.Zero;

    if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
    {
        var executablePath = Process.GetCurrentProcess().MainModule?.FileName;
        var executableDir = !string.IsNullOrWhiteSpace(executablePath)
            ? Path.GetDirectoryName(executablePath)
            : null;

        if (!string.IsNullOrEmpty(executableDir))
        {
            var frameworkBinary = Path.GetFullPath(Path.Combine(
                executableDir,
                "..",
                "Frameworks",
                "Chromium Embedded Framework.framework",
                "Chromium Embedded Framework"));

            if (File.Exists(frameworkBinary))
            {
                dlopen(frameworkBinary, RtldNow | RtldGlobal);
            }

            var bundleLibraryPath = Path.Combine(executableDir, "libCefNative.dylib");
            if (File.Exists(bundleLibraryPath))
            {
                return NativeLibrary.Load(bundleLibraryPath);
            }
        }
    }

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
        IntPtr argvBuffer = BuildArgv(argv, out IntPtr[] allocatedStrings);

        try
        {
            return _RunWithSubprocess(argv.Length, argvBuffer, resourcesDir, localesDir, cacheDir, subprocessPath);
        }
        finally
        {
            FreeArgv(argvBuffer, allocatedStrings);
        }
}
}