Place in .\lib\clang\[clang_version]\include the MSVC headers.
Note: At the time of writing, clang_version = 3.8.0

Place in .\bin the Qt DLLs. Currently, these are used:
icudt54.dll
icuin54.dll
icuuc54.dll
Qt5Core.dll
Qt5Gui.dll
Qt5Network.dll
Qt5Svg.dll
Qt5Widgets.dll
Also, place the plugins directory. Note that, for release builds, files matching
the following patterns do not need to be distributed: *d.dll *d.pdb
The cleaned plugins directory is slightly under 10 MiB.
