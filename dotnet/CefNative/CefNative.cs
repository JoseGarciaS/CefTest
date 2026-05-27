using System.Runtime.InteropServices;

namespace CefNative;

public static class Cef
{
    private const string LibName = "CefNative";

    [DllImport(LibName, EntryPoint = "CefNative_RunHelloWorld")]
    public static extern int RunHelloWorld(
        int argc,
        string[] argv,
        string resourcesDir,
        string localesDir,
        string cacheDir);

    [DllImport(LibName, EntryPoint = "CefNative_RunHelloWorldWithSubprocessPath")]
    public static extern int RunHelloWorldWithSubprocessPath(
        int argc,
        string[] argv,
        string resourcesDir,
        string localesDir,
        string cacheDir,
        string browserSubprocessPath);
}