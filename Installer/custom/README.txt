Place in .\bin the Qt DLLs. Currently, these are used:
Qt6Core.dll
Qt6Gui.dll
Qt6Network.dll
Qt6Widgets.dll
Also, place the plugins directory. To prune the directory:
1. Delete all subdirectories except
    imageformats\
    platforms\
    styles\
2. Delete all *.pdb
3. Delete all *d.dll
The cleaned plugins directory is less than 3 MiB.
