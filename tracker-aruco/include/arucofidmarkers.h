/*****************************
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
********************************/

#ifndef ArucoFiducicalMarkerDetector_H
#define ArucoFiducicalMarkerDetector_H
#include <opencv2/core/core.hpp>
#include "exports.h"
#include "marker.h"
#include "board.h"
namespace aruco {

class ARUCO_EXPORTS FiducidalMarkers {
public:
    /**
    * \brief Creates an ar marker with the id specified using a modified version of the hamming code.
    * There are two type of markers: a) These of 10 bits b) these of 3 bits. The latter are employed for applications
    * that need few marker but they must be small.  The two type of markers are distinguished by their ids. While the first type
    * of markers have ids in the interval [0-1023], the second type ids in the interval [2000-2006].
    *
    *
    * 10 bits markers
    * -----------------------
    * There are a total of 5 rows of 5 cols. Each row encodes a total of 2 bits, so there are 2^10 bits:(0-1023).
    *
    * The least significative bytes are first (from left-up to to right-bottom)
    *
    * Example: the id = 110 (decimal) is be represented in binary as : 00 01 10 11 10.
    *
    * Then, it will generate the following marker:
    *
    * -# 1st row encodes 00: 1 0 0 0 0 : hex 0x10
    * -# 2nd row encodes 01: 1 0 1 1 1 : hex 0x17
    * -# 3nd row encodes 10: 0 1 0 0 1 : hex 0x09
    * -# 4th row encodes 11: 0 1 1 1 0 : hex 0x0e
    * -# 5th row encodes 10: 0 1 0 0 1 : hex 0x09
    *
    * Note that : The first bit, is the inverse of the hamming parity. This avoids the 0 0 0 0 0 to be valid
    * These marker are detected by the function  getFiduciadlMarker_Aruco_Type1
    */
    static cv::Mat createMarkerImage(int id,int size) throw (cv::Exception);

    /** Detection of fiducidal aruco markers (10 bits)
     * @param in input image with the patch that contains the possible marker
     * @param nRotations number of 90deg rotations in clockwise direction needed to set the marker in correct position
     * @return -1 if the image passed is a not a valid marker, and its id in case it really is a marker
     */
    static int detect(const cv::Mat &in,int &nRotations);

    /**Similar to createMarkerImage. Instead of returning a visible image, returns a 8UC1 matrix of 0s and 1s with the marker info
     */
    static cv::Mat getMarkerMat(int id) throw (cv::Exception);


    /**Creates a printable image of a board
     * @param gridSize grid layout (numer of sqaures in x and Y)
     * @param MarkerSize size of markers sides in pixels
     * @param MarkerDistance distance between the markers
      * @param TInfo output 
     * @param excludedIds set of ids excluded from the board
     */
    static  cv::Mat createBoardImage( cv::Size  gridSize,int MarkerSize,int MarkerDistance,  BoardConfiguration& TInfo ,vector<int> *excludedIds=NULL ) throw (cv::Exception);


    /**Creates a printable image of a board in chessboard_like manner
     * @param gridSize grid layout (numer of sqaures in x and Y)
     * @param MarkerSize size of markers sides in pixels
      * @param TInfo output 
     * @param setDataCentered indicates if the center is set at the center of the board. Otherwise it is the left-upper corner
     * 
     */
    static  cv::Mat  createBoardImage_ChessBoard( cv::Size gridSize,int MarkerSize, BoardConfiguration& TInfo ,bool setDataCentered=true ,vector<int> *excludedIds=NULL) throw (cv::Exception);

    /**Creates a printable image of a board in a frame fashion 
     * @param gridSize grid layout (numer of sqaures in x and Y)
     * @param MarkerSize size of markers sides in pixels
     * @param MarkerDistance distance between the markers
      * @param TInfo output 
     * @param setDataCentered indicates if the center is set at the center of the board. Otherwise it is the left-upper corner
     * 
     */
    static  cv::Mat  createBoardImage_Frame( cv::Size gridSize,int MarkerSize,int MarkerDistance,  BoardConfiguration& TInfo ,bool setDataCentered=true,vector<int> *excludedIds=NULL ) throw (cv::Exception);

private:
  
    static vector<int> getListOfValidMarkersIds_random(int nMarkers,vector<int> *excluded) throw (cv::Exception);
    static  cv::Mat rotate(const cv::Mat & in);
    static  int hammDistMarker(cv::Mat  bits);
    static  int analyzeMarkerImage(cv::Mat &grey,int &nRotations);
    static  bool correctHammMarker(cv::Mat &bits);
};

}

#endif
