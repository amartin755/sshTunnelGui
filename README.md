# Stechuhr

A tiny utility for tracking of working time.

## Build instructions
Stechuhr has been successfully compiled on fedora linux (x64) with gcc and on windows with MSVC.
Theoretically it should also be compilable with other C++17 compilers, but this was never tested.

### Dependencies
* cmake >= V3.18
* C++17 compiler
* QT6 development files (Fedora: qt6-base-devel, qt6-qttools-devel)

### How to build
#### Setup

    cd stechuhr
    cmake -B build

For debugging add to cmake call:

    -DCMAKE_BUILD_TYPE=Debug

#### Build

    cmake --build build

