Import-Module "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"

Enter-VsDevShell -VsInstallPath "C:\Program Files\Microsoft Visual Studio\2022\Community" -DevCmdArguments '-arch=x64'

cd $PSScriptRoot

cl /LD /O2 rdtsc.cpp /link /OUT:rdtsc.dll
