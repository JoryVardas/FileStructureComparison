# FileStructureComparison

This is a mult-platform application for comparing the structure and contents of a one path against another. This tool was created to help compare large numbers of files to assist with the usage of [Archiver](https://github.com/JoryVardas/Archiver).

The application takes a reference and a base path, it then iterates though all the files and directories in the reference path. For each directory it checks that the base path has a corresponding directory. For each file it checks that the base path has a corresponding file, and then checks that the contents of the two files are identical.
Once the application finishes iterating though the reference path a message "All items processed" is displayed, after which a list of any non existant paths and/or files which were not identical are displayed.

It is important to note that the base path can contain files and directories which are not in the reference path and this is by design.

## Usage

```
FileStructureComparison --reference <path1> --base <path2> [--size <num>]
```

`-r, --reference <path1>` : This is the path that is being compared against.
`-b, --base <path1>` : This is the path that is being compared.
`-s, --size <num>` : This specifies the size of the buffers used to compare files, whatever value is used two buffers must be constructed. If this parameter is not provided a default of 1073741824 (1GB) is used.

:warning: On Windows paths are limited to 260 characters including the NULL terminator and drive specifier by default. It is not uncommon to have nested folders and file names that exceed this limit, which can cause unexpected behaviour with this application. For this reason it is recommended that either extended length paths or long paths be used when running on Windows. Extended lenght paths are paths prefixed with **\\\\?\\**, or **\\\\?\\UNC\\** for network locations. Long paths have to be enabled on a system wide level. For more information check out [this link](https://learn.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation) 