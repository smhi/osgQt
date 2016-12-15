### Introduction

Welcome to the OpenSceneGraph (OSG).

For up-to-date information on the project, in-depth details on how to
compile and run libraries and examples, see the documentation on the
OpenSceneGraph website:

    http://www.openscenegraph.org/index.php/documentation

For support subscribe to our public mailing list or forum, details at:

    http://www.openscenegraph.org/index.php/support

For the impatient, we've included quick build instructions below, these
are are broken down is three parts:

  1) General notes on building the OpenSceneGraph
  2) OSX release notes
  3) iOS release notes

If details below are not sufficient then head over to the openscenegraph.org
to the Documentation/GettingStarted and Documentation/PlatformSpecifics sections for
more indepth instructions.

Robert Osfield.
Project Lead.
5th September 2016.

--

### Section 1. How to build the osgQt

The osgQt uses the CMake build system to generate a
platform-specific build environment.  CMake reads the CMakeLists.txt
files that you'll find throughout the osgQt directories,
checks for installed dependenciesand then generates the appropriate
build system.

If you don't already have CMake installed on your system you can grab
it from http://www.cmake.org, use version 2.4.6 or later.  Details on the
OpenSceneGraph's CMake build can be found at:

    http://www.openscenegraph.org/projects/osg/wiki/Build/CMake
    
NOTE: You must have at least OpenSceneGraph 3.5.6 installed in your system to build osgQt.

You build osgQt with cmake -DBUILD_OSG_EXAMPLES=1 -DCMAKE_INSTALL_PREFIX=<install directory> .

