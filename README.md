# CefNative — Manual Build & Run Guide

> A cross-platform Chromium Embedded Framework (CEF) wrapper distributed as a NuGet package. The native shared library (`libCefProcess` / `CefProcess.dll`) is exposed to .NET via P/Invoke, with a standalone subprocess helper required by CEF's multi-process architecture.

---

## Table of Contents

1. [Project Architecture](#1-project-architecture)
2. [Native Source Files Explained](#2-native-source-files-explained)
3. [Prerequisites](#3-prerequisites)
4. [Step-by-Step Manual Build](#4-step-by-step-manual-build)
   - [Linux](#41-linux)
   - [Windows](#42-windows)
   - [macOS](#43-macos)
5. [Staging the Runtime](#5-staging-the-runtime)
6. [Packing the NuGet Package Locally](#6-packing-the-nuget-package-locally)
7. [Creating and Running a Consumer App](#7-creating-and-running-a-consumer-app)
8. [Troubleshooting](#8-troubleshooting)

> **Prefer automation?** If you have access to GitHub Actions, the entire build, staging, packing, and publishing pipeline is already defined in `.github/workflows/build.yml`. Push to `main` (or open a PR) and the workflow handles everything across all three platforms. The manual steps below are for local development.

---

## 1. Project Architecture

```
CefNative/
├── native/
│   ├── CMakeLists.txt          # CMake build definition for all native targets
│   ├── src/
│   │   ├── CefNative.hpp       # Public C API export declaration
│   │   ├── CefNative.cpp       # Entry point: CefNative_Run(); CEF init & message loop
│   │   ├── CefApp.hpp          # MyApp + MyWindowDelegate declarations
│   │   ├── CefApp.cpp          # Window creation, browser view setup
│   │   ├── CefClient.hpp       # MyClient declaration (lifecycle + load events)
│   │   ├── CefClient.cpp       # Browser lifecycle, PDF generation on page load
│   │   └── CefSubprocessHelper.cpp  # Standalone subprocess entry point (main)
│   └── stage/                  # Populated after staging; consumed by dotnet pack
│       └── runtimes/
│           ├── linux-x64/native/
│           ├── win-x64/native/
│           └── osx-x64/native.zip
└── dotnet/
    └── CefNative/
        ├── CefNative.csproj    # NuGet package definition
        ├── CefNative.cs        # P/Invoke + DllImportResolver
        └── CefNative.targets   # MSBuild targets: copy/extract native files on build & publish
```

Two native binaries are produced:

| Binary                                                             | Purpose                                                            |
| ------------------------------------------------------------------ | ------------------------------------------------------------------ |
| `libCefProcess.so` / `.dylib` / `CefProcess.dll`                   | Main browser-process library, loaded by the .NET host via P/Invoke |
| `CefSubprocess` / `CefSubprocess.exe` / `CefSubprocess Helper.app` | Renderer and GPU sub-process helper, spawned by CEF automatically  |

---

## 2. Native Source Files Explained

| File(s)                           | Purpose                                                                                                                                                                                   |
| --------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `CefNative.hpp` / `CefNative.cpp` | Declares and implements the single exported C function `CefNative_Run`. Handles CEF initialisation, settings, and the message loop. This is the only entry point the .NET side calls.     |
| `CefApp.hpp` / `CefApp.cpp`       | Defines the application and window delegate. Sets up the browser view and top-level window once CEF's browser process is ready.                                                           |
| `CefClient.hpp` / `CefClient.cpp` | Handles browser lifecycle events (creation, close) and the load handler. On page load it prints to PDF and shuts the browser down — making the current build a PDF-generation smoke test. |
| `CefSubprocessHelper.cpp`         | Standalone `main()` for the renderer/GPU sub-process executable. Loads the CEF framework (macOS) and calls `CefExecuteProcess`. CEF spawns this automatically.                            |

---

## 3. Prerequisites

### All Platforms

- **CMake** ≥ 3.15
- **Ninja** (recommended) or the platform default generator
- **.NET SDK** 10.0

### Linux

```bash
sudo apt update && sudo apt install -y cmake ninja-build libgtk-3-dev libnss3 libasound2t64 libxss1
```

### Windows

- Visual Studio 2022 (with C++ workload) or Build Tools for Visual Studio

### macOS

```bash
brew install cmake ninja wget
```

Xcode command-line tools must be installed (`xcode-select --install`).

---

## 4. Step-by-Step Manual Build

### CEF Version

All commands below use CEF `148.0.9`. Replace the version string if you upgrade.

---

### 4.1 Linux

```bash
# 1. Download and extract CEF
wget -q "https://cef-builds.spotifycdn.com/cef_binary_148.0.9+g0d9d52a+chromium-148.0.7778.180_linux64_minimal.tar.bz2" \
  -O cef_linux.tar.bz2
mkdir -p /tmp/cef/linux
tar -xjf cef_linux.tar.bz2 -C /tmp/cef/linux --strip-components=1

# 2. Configure
cmake -B native/build/linux -S native -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCEF_ROOT=/tmp/cef/linux

# 3. Build
cmake --build native/build/linux --target CefSubprocess CefProcess --parallel
```

Outputs: `native/build/linux/libCefProcess.so`, `native/build/linux/CefSubprocess`

---

### 4.2 Windows

Run in a **Developer PowerShell for VS 2022** (or any shell with `cl.exe` on PATH):

```powershell
# 1. Download and extract CEF
Invoke-WebRequest "https://cef-builds.spotifycdn.com/cef_binary_148.0.9+g0d9d52a+chromium-148.0.7778.180_windows64_minimal.tar.bz2" `
  -OutFile cef_windows.tar.bz2
New-Item -ItemType Directory -Force C:\cef\windows | Out-Null
tar -xjf cef_windows.tar.bz2 -C C:\cef\windows --strip-components=1

# 2. Configure (uses default MSBuild/VS generator)
cmake -B native/build/windows -S native -DCEF_ROOT=C:\cef\windows

# 3. Build
cmake --build native/build/windows --target CefSubprocess CefProcess --config Release --parallel
```

Outputs: `native/build/windows/Release/CefProcess.dll`, `native/build/windows/Release/CefSubprocess.exe`

---

### 4.3 macOS

```bash
# 1. Download and extract CEF
wget -q "https://cef-builds.spotifycdn.com/cef_binary_148.0.9+g0d9d52a+chromium-148.0.7778.180_macosx64_minimal.tar.bz2" \
  -O cef_macos.tar.bz2
mkdir -p /tmp/cef/macos
tar -xjf cef_macos.tar.bz2 -C /tmp/cef/macos --strip-components=1

# 2. Configure
cmake -B native/build/macos -S native -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCEF_ROOT=/tmp/cef/macos

# 3. Build
cmake --build native/build/macos --target CefSubprocess CefProcess --parallel
```

Outputs: `native/build/macos/libCefProcess.dylib`, `native/build/macos/Frameworks/` (contains the CEF framework and `CefSubprocess Helper.app`)

---

## 5. Staging the Runtime

The NuGet `.csproj` globs files from `native/stage/runtimes/<rid>/native/`. You must populate this before packing.

### Linux

```bash
mkdir -p native/stage/runtimes/linux-x64/native
cp -a /tmp/cef/linux/Release/. native/stage/runtimes/linux-x64/native/
cp -a /tmp/cef/linux/Resources/. native/stage/runtimes/linux-x64/native/
cp native/build/linux/libCefProcess.so native/stage/runtimes/linux-x64/native/
cp native/build/linux/CefSubprocess native/stage/runtimes/linux-x64/native/
```

### Windows

```powershell
New-Item -ItemType Directory -Force native\stage\runtimes\win-x64\native | Out-Null
Copy-Item -Recurse -Force C:\cef\windows\Release\* native\stage\runtimes\win-x64\native\
Copy-Item -Recurse -Force C:\cef\windows\Resources\* native\stage\runtimes\win-x64\native\
Copy-Item -Force native\build\windows\Release\CefProcess.dll native\stage\runtimes\win-x64\native\
Copy-Item -Force native\build\windows\Release\CefSubprocess.exe native\stage\runtimes\win-x64\native\
```

### macOS

macOS uses a zip to preserve framework symlinks. Standard file copy will silently break them.

```bash
mkdir -p native/stage/runtimes/osx-x64/native
cp native/build/macos/libCefProcess.dylib native/stage/runtimes/osx-x64/native/
cp -a native/build/macos/Frameworks native/stage/runtimes/osx-x64/native/

cd native/stage/runtimes/osx-x64
ditto -c -k --sequesterRsrc native native.zip
rm -rf native
```

> **Important:** Use `ditto`, not `zip`. `zip` does not preserve symlinks inside `.framework` bundles, which will cause `dyld` to fail at runtime. Do not use `--keepParent` as it adds an unwanted nesting level.

---

## 6. Packing the NuGet Package Locally

With all three platforms staged (or just the one you need for local testing):

```bash
cd dotnet/CefNative
dotnet pack -c Release -o /tmp/local-feed /p:Version=0.0.1-local
```

This produces `/tmp/local-feed/CefNative.0.0.1-local.nupkg`.

---

## 7. Creating and Running a Consumer App

```bash
# Create a minimal console app
dotnet new console -n MyCefApp -o /tmp/MyCefApp
cd /tmp/MyCefApp

# Point dotnet restore at your local feed
dotnet nuget add source /tmp/local-feed --name local

# Add the package
dotnet add package CefNative --version 0.0.1-local

# Replace Program.cs
echo 'using CefNative; return Cef.Run();' > Program.cs
```

### Linux / Windows — `dotnet run`

```bash
# Linux
dotnet run -r linux-x64

# Windows
dotnet run -r win-x64
```

> **Windows only:** `[STAThread]` is required. CEF must run on the main STA thread. If you use a custom `Program.cs`, annotate `Main` accordingly.

### macOS — publish first

`dotnet run` does not trigger the `ditto` extraction targets. You must publish:

```bash
dotnet publish -r osx-x64 --self-contained -o publish

cd publish

# Ad-hoc codesign (required for dyld to load the framework)
codesign --force --sign - \
  "Frameworks/Chromium Embedded Framework.framework/Chromium Embedded Framework"
codesign --force --sign - \
  "Frameworks/CefSubprocess Helper.app/Contents/MacOS/CefSubprocess Helper"

./MyCefApp
```

On success the app will open a window loading `https://example.com`, print the page to `output.pdf`, and exit.

---

## 8. Troubleshooting

| Symptom                                                           | Likely Cause                                                        | Fix                                                                                                           |
| ----------------------------------------------------------------- | ------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------- |
| `Failed to open/find the framework` (macOS)                       | `Frameworks/` not next to `libCefProcess.dylib`, or symlinks broken | Re-stage using `ditto`; verify `Frameworks/` is adjacent to the dylib                                         |
| `CEF initialization failed`                                       | Missing resources, wrong `resources_dir_path` or `locales_dir_path` | Confirm `icudtl.dat`, `chrome_100_percent.pak`, and `locales/` exist in the directory pointed to by `lib_dir` |
| Blank window / renderer crash immediately                         | Subprocess not found or wrong path                                  | Check `browser_subprocess_path` in settings; confirm the `CefSubprocess` binary exists and is executable      |
| `Check that CEF is running on the main thread` on Windows startup | CEF invoked from an MTA thread                                      | Add `[STAThread]` to `Main`                                                                                   |
