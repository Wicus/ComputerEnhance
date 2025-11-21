Import-Module "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"

Enter-VsDevShell -VsInstallPath "C:\Program Files\Microsoft Visual Studio\2022\Community" -DevCmdArguments '-arch=x64'

cd $PSScriptRoot

cl /Zi /Od /TC rdtsc.cpp rdtsc_harness.cpp /Fe:rdtsc_harness.exe
