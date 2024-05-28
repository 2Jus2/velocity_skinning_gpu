# Our Implementation

Files that we have modified are located in scenes/sources/squashy_skinning

## skinning.cpp
| Lines Changed    | Implementation details |
| -------- | ------- |
| 130-131  | To calculate the new linear and angular acceleration    |

## velocity_tracker.cpp
| Lines Changed    | Implementation details |
| -------- | ------- |
| 87-91  | To calculate the average rotation acceleration    |
| 99  | Sets the last speed as the current rotation speed    |

## velocity_tracker.hpp
| Lines Changed    | Implementation details |
| -------- | ------- |
| 87-91  | To calculate the average rotation acceleration    |
| 99  | Sets the last speed as the current rotation speed    |


# GPU Velocity Skinning Code

- The shader is in the file velocity_skinning_shader


## Compilation on Windows system with Visual Studio 

- Use CMakeLists.txt with Visual Studio
- Precompiled version of GLFW3 is provided (precompiled/glfw3_win)

=> You need to copy assets/ and scenes/ directories in the executable directory

## Compilation in command line using the Makefile (Linux/MacOS only)

$ make

$ ./scene


## Setup compilation in command line using CMake (Linux/MacOs)

This step create the build directory, call CMake to generate the Makefile, compile and execute the code. Note that the creation of the build directory and CMake call has to be done only once on a given system.

The following command assume you have opened a command line in the directory vcl/

### Create a build directory

$ mkdir build

$ cd build

### Execute CMake, compile

$ cmake ..

$ make

$ cd ..

### Execute

$ build/pgm



