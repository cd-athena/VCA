= Mandatory Prerequisites =

* GCC, MSVC (9, 10, 11, 12), Xcode or Intel C/C++
* CMake 2.8.8 or later http://www.cmake.org
* On linux, ccmake is helpful, usually a package named cmake-curses-gui 

Note: MSVC12 requires cmake 2.8.11 or later


= Optional Prerequisites =

1. VisualLeakDetector (Windows Only)

   Download from https://vld.codeplex.com/releases and install. May need
   to re-login in order for it to be in your %PATH%.  Cmake will find it
   and enable leak detection in debug builds without any additional work.

   If VisualLeakDetector is not installed, cmake will complain a bit, but
   it is completely harmless.


= Build Instructions Linux =

1. Use cmake to generate Makefiles: cmake ../source
2. Build vca:                       make

  Or use our shell script which runs cmake then opens the curses GUI to
  configure build options

1. cd build/linux ; ./make-Makefiles.bash
2. make


= Build Instructions Windows =

We recommend you use one of the make-solutions.bat files in the appropriate
build/ sub-folder for your preferred compiler.  They will open the cmake-gui
to configure build options, click configure until no more red options remain,
then click generate and exit.  There should now be an vca.sln file in the
same folder, open this in Visual Studio and build it.
