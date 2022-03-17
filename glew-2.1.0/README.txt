The static library is in lib.  This is the Release version.
Due to the naming convention, to build with the Debug version
requires changes to the CMakeLists.txt file so that the filename
ending glew32sd.lib is used.  Normally we shouldn't need 
to build with the debug version, except in certain cases if we
needed to debug an issue within the library itself.