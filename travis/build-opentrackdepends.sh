#!/bin/bash
pwd=`pwd`
git clone --recurse-submodules -j8 https://github.com/opentrack/opentrack-depends
mkdir opentrack-depends/aruco/build
cd opentrack-depends/aruco/build
cmake ..
make
cd $pwd
