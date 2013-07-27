/**

Copyright 2011 Rafael Muñoz Salinas. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Rafael Muñoz Salinas ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Rafael Muñoz Salinas OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Rafael Muñoz Salinas.

 
 
 \mainpage ArUco: Augmented Reality library from the University of Cordoba


ArUco is a minimal C++ library for detection of Augmented Reality markers based on OpenCv exclusively.  

It is an educational project to show student how to detect augmented reality markers and it is provided under BSD license.


\section INTRODUCTION INTRODUCTION

The library relies on the use of coded markers. Each marker has an unique code indicated by the black and white colors in it. The libary detect borders, and analyzes into the rectangular regions which of them are likely to be markers. Then, a decoding is performed and if the code is valid, it is considered that the rectangle is a marker.

The codification included into the marker is a slighly modified version of the Hamming Code. It has a total a 25 bits didived in 5 rows of 5 bits each. So, we have 5 words of 5 bits. Each word, contains only 2 bits of real information, the rest is for  and error detection/correction (error correction is yet to be done). As a conclusion, a marker contains 10 bits of real information wich allows 1024 different markers.


\section BOARDS BOARDS

Aruco allows the possibility to employ board. Boards are markers composed by an array of markers arranged in a known order. The advantages of using boards instead of simple markers are:
 - More robusteness. The misdetection of several markers of the board is not a problem as long as a minimum set of them are detected.
 - More precision. Since there are a larger number of corners, camera pose estimation becomes more precise.


\section APPLICATIONS APPLICATIONS

The library comes with five applications that will help you to learn how to use the library:
 - aruco_create_marker: which creates marker and saves it in a jpg file you can print.
 - aruco_simple : simple test aplication that detects the markers in a image 
 - aruco_test: this is the main application for detection. It reads images either from the camera of from a video and detect markers. Additionally, if you provide the intrinsics of the camera(obtained by OpenCv calibration) and the size of the marker in meters, the library calculates the marker intrinsics so that you can easily create your AR applications.
 - aruco_test_gl: shows how to use the library AR applications using OpenGL for rendering
 - aruco_create_board: application that helps you to create a board
 - aruco_simple_board: simple test aplication that detects a board of markers in a image 
 - aruco_test_board: application that detects boards
 - aruco_test_board_gl: application that detects boards and uses OpenGL to draw

\section LIBRARY LIBRARY DESCRIPTION:

The ArUco library contents are divided in two main directories. The src directory, which contains the library itself. And the utils directory which contains the applications.

The library main classes are: 
   - aruco::CameraParameters: represent the information of the camera that captures the images. Here you must set the calibration info.
   - aruco::Marker: which represent a marker detected in the image
   - aruco::MarkerDetector: that is in charge of deteting the markers in a image Detection is done by simple calling the member funcion ArMarkerDetector::detect(). Additionally, the classes contain members to create the required matrices for rendering using OpenGL. See aruco_test_gl for details
   - aruco::BoardConfiguration: A board is an array of markers in a known order. BoardConfiguracion is the class that defines a board by indicating the id of its markers. In addition, it has informacion about the distance between the markers so that extrinsica camera computations can be done.
   - aruco::Board: This class defines a board detected in a image. The board has the extrinsic camera parameters as public atributes. In addition, it has a method that allows obtain the matrix for getting its position in OpenGL (see aruco_test_board_gl for details).
   - aruco::BoardDetector : This is the class in charge of detecting a board in a image. You must pass to it the set of markers detected by ArMarkerDetector and the BoardConfiguracion of the board you want to detect. This class will do the rest for you, even calculating the camera extrinsics.


\section COMPILING COMPILING THE LIBRARY:
\subsection Linux
Go to the aruco library and do
\verbatim
>mkdir build
>cd build
>cmake ..
>make
>make install (optional)
\endverbatim

NOTE ON OPENGL: The library supports eaily the integration with OpenGL. In order to compile with support for OpenGL, you just have  installed in your system the develop packages for GL and glut (or freeglut).

\subsection WINDOWS

The library has been compiled using MinGW and codeblocks. Below I describe the best way to compile it that I know. If you know better, please let me know.
  - step 1) codeblocks
    -# Download codeblocks. I recommend to download the version 10.5 with mingw included (codeblocks-10.05mingw-setup.exe)
    -# Install and set the PATH variable so that the codeblock/mingw/bin directory is included. In my case c:/codeblocks/mingw/bin. This will allow cmake to find the compiler.
    -# The codeblock program will not find the mingw path by deafult. So, run codeblocks and go to setting->Compuiler debugger and set the correct path to the MinGW dir.
  - step 2) cmake
     -# Download and install the last version of cmake.
  - step 3) OpenCv
      -# Download the source code and compile it using cmake and codeblocks. Note: install the library in C:\ if you want it to be easily detected by cmake afterwards
  - step 4) aruco
     -# Download and decompress.
     -# Open cmake gui application and set the path to the main library directory and also set a path where the project is going to be built. 
     -# Generate the codeblock project.
     -# Open the project with codeblock and compile then, install. The programs will be probably generated into the bin directory

OpenGL: by default, the mingw version installed has not the glut library. So, the opengl programs are not compiled.  If you want to compile with OpenGL support, you must install glut, or prefereably freeglut.
Thus, 
  - Download the library (http://www.martinpayne.me.uk/software/development/GLUT/freeglut-MinGW.zip) for mingw. 
  - Decompress in a directory X. 
  - Then, rerun cmake setting the variable GLU_PATH to that directory (>cmake .. -DGLUT_PATH="C:\X")
  - Finally, recompile and test. Indeed, you should move the freeglut.dll to the directory with the binaries or to any other place in the PATH.


CONCLUSION: Move to Linux, things are simpler :P

 
\section Testing 

For testing the applications, the library provides videos and the corresponding camera parameters of these videos. Into the directories you will find information on how to run the examples.
 
\section Final Notes

 - REQUIREMENTS: OpenCv >= 2.1.0. and OpenGL for (aruco_test_gl and aruco_test_board_gl)
 - CONTACT: Rafael Munoz-Salinas: rmsalinas@uco.es
 - This libary is free software and come with no guaratee!
 
*/

#include "markerdetector.h"
#include "boarddetector.h"
#include "cvdrawingutils.h"

