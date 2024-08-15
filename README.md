# What is this for?

First of all, so that when working with old projects, you don't have to waste time on completely porting them to modern toolchain without a pressing reason, but have the same infrastructure to work with, the same one that was originally developed.

This can really be useful when you are a reverse-engineer, you have a task to find function signatures and offsets for the original version of the game, from which you managed to get the source code of approximately the same revision, then you can build a version with debug information enabled (in the form of .pdb files), then you can import it and make your work dozens of times easier and faster.

At the same time, when porting to newer toolchain, there is a high chance that due to gradual internal changes in the compiler code, the final result in the form of how the compiler will generate assembly will differ accordingly.

# Where did you get this from?

Officially, Microsoft has not sold or distributed Visual Studio 6.0 (1998) for a long time, so the status of this version is recognized by most as abandonware. On the Internet, various users have long been storing both .iso with content from the original Visual Studio 6.0 (1998) installation disks on their websites, as well as portable builds in the form of one of these. This particular build was packed by a user nicknamed `Psycho-A`, who is one of the significant contributors to the modding community of a game called VTMB (Vampire: The Masquerade – Bloodlines).

# Example build with GitHub Actions:

```yaml
name: CI

on:
  push:
    paths-ignore:
     - 'README.md'
  pull_request:
    paths-ignore:
     - 'README.md'
  workflow_dispatch:
  schedule:
    - cron: '0 0 1 * *' # Monthly

jobs:
  win32:
    name: Win32
    runs-on: windows-latest
    steps:
    - uses: actions/checkout@v4
      with:
        fetch-depth: 0
    - name: Install Visual C++ 6.0
      run: |
        git clone --depth 1 https://github.com/SmileyAG/MSVC60
    - name: Build
      shell: cmd
      run: |
        call MSVC60/vc_studio/main/VC98/Bin/VCVars32.bat
        msdev cl_dll/cl_dll.dsp /make "cl_dll - Win32 Release"
        msdev cl_dll/cl_dll.dsp /make "cl_dll - Win32 Debug"
        msdev dlls/hl.dsp /make "hl - Win32 Release"
        msdev dlls/hl.dsp /make "hl - Win32 Debug"
    - name: Upload artifacts
      uses: actions/upload-artifact@v4
      with:
        name: Win32
        path: |
          cl_dll/Release/client.dll
          cl_dll/Release/client.pdb
          cl_dll/Debug/client.dll
          cl_dll/Debug/client.pdb
          dlls/Releasehl/hl.dll
          dlls/Releasehl/hl.pdb
          dlls/debughl/hl.dll
          dlls/debughl/hl.pdb
```
