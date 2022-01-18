************
Building instructions
************

Prerequisites
=============

 1. [CMake](https://cmake.org) version 3.1 or higher.
 2. [Git](https://git-scm.com/).
 3. C++ compiler with C++11 support

The following C++11 compilers have been known to work:

 * Visual Studio 2015 or later
 * GCC 4.8 or later
 * Clang 3.3 or later

Linux build steps
=================

The following commands will checkout the project source code and create a
directory called 'build' where the compiler output will be placed.
CMake is then used for generating build files and compiling the VCA binaries.

    $ git clone https://github.com/cd-athena/VCA.git
    $ cd vca
    $ mkdir build
    $ cd build
    $ cmake -G "Unix Makefiles" ../ && ccmake ../
    $ make

This will create VCA binaries in the vca/build/source/apps/ folder.

Windows build steps
===================

The following commands will checkout the project source code and create a
directory called 'build' where the compiler output will be placed.
CMake is then used for generating build files and creating the Visual Studio
solution.

    $ git clone https://github.com/cd-athena/VCA.git
    $ cd vca
    $ mkdir build
    $ cd build
    $ cmake -G "Visual Studio 15 Win64" ../ && cmake-gui ../

The -G argument should be adjusted to match your version of Visual Studio and
the target architecture.
This will generate a Visual Studio solution called vca.sln in the build folder.
After building the solution, the vca binaries will be found
in the vca/build/source/app/ folder.
