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
#ifndef _Aruco_BoardDetector_H
#define _Aruco_BoardDetector_H
#include <opencv2/opencv.hpp>
#include "exports.h"
#include "board.h"
#include "cameraparameters.h"
#include "markerdetector.h"
using namespace std;

namespace aruco
{

/**\brief This class detects AR boards
 * Version 1.2
 * There are two modes for board detection.
 * First, the old way. (You first detect markers with MarkerDetector and then call to detect in this class.
 * 
 * Second: New mode, marker detection is included in the class
 * \code
 
  CameraParameters CP;
  CP.readFromFile(path_cp)
  BoardConfiguration BC;
  BC.readFromFile(path_bc);
  BoardDetector BD;
  BD.setParams(BC,CP); //or only BD.setParams(BC)
  //capture image
  cv::Mat im;
  capture_image(im);
  
  float prob=BD.detect(im);
  if (prob>0.3) 
	CvDrawingUtils::draw3DAxis(im,BD.getDetectedBoard(),CP);
 
 \endcode
 * 
*/
class ARUCO_EXPORTS  BoardDetector
{
public:
  /** See discussion in @see enableRotateXAxis.
   * Do not change unless you know what you are doing
   */
    BoardDetector(bool  setYPerperdicular=true);
    
    
    /**
     * Use if you plan to let this class to perform marker detection too
     */
    void setParams(const BoardConfiguration &bc,const CameraParameters &cp, float markerSizeMeters=-1);
    void setParams(const BoardConfiguration &bc);
    /**
     * Detect markers, and then, look for the board indicated in setParams()
     * @return value indicating  the  likelihood of having found the marker
     */
    float  detect(const cv::Mat &im)throw (cv::Exception);
    /**Returns a reference to the board detected
     */
    Board & getDetectedBoard(){return _boardDetected;}
    /**Returns a reference to the internal marker detector
     */
    MarkerDetector &getMarkerDetector(){return _mdetector;}
    /**Returns the vector of markers detected
     */
    vector<Marker> &getDetectedMarkers(){return _vmarkers;}
    
    
    //ALTERNATIVE DETECTION METHOD, BASED ON MARKERS PREVIOUSLY DETECTED
    
    /** Given the markers detected, determines if there is the board passed
    * @param detectedMarkers result provided by aruco::ArMarkerDetector
    * @param BConf the board you want to see if is present
    * @param Bdetected output information of the detected board
    * @param camMatrix camera matrix with intrinsics
    * @param distCoeff camera distorsion coeff
    * @param camMatrix intrinsic camera information.
    * @param distCoeff camera distorsion coefficient. If set Mat() if is assumed no camera distorion
    * @param markerSizeMeters size of the marker sides expressed in meters
    * @return value indicating  the  likelihood of having found the marker
    */
    float detect(const vector<Marker> &detectedMarkers,const  BoardConfiguration &BConf, Board &Bdetected, cv::Mat camMatrix=cv::Mat(),cv::Mat distCoeff=cv::Mat(), float markerSizeMeters=-1 )throw (cv::Exception);
    float detect(const vector<Marker> &detectedMarkers,const  BoardConfiguration &BConf, Board &Bdetected,const CameraParameters &cp, float markerSizeMeters=-1 )throw (cv::Exception);


    /**
     * By default, the Y axis is set to point up. However this is not the default
     * operation mode of opencv, which produces the Z axis pointing up instead. 
     * So, to achieve this change, we have to rotate the X axis.
     */
    void setYPerperdicular(bool enable){_setYPerperdicular=enable;}
    
    
    
    
private:
    void rotateXAxis(cv::Mat &rotation);
    bool _setYPerperdicular;
    
    //-- Functionality to detect markers inside
    bool _areParamsSet;
    BoardConfiguration _bconf;
    Board _boardDetected;
    float _markerSize;
    CameraParameters _camParams;
    MarkerDetector _mdetector;//internal markerdetector
    vector<Marker> _vmarkers;//markers detected in the call to : float  detect(const cv::Mat &im);
    
};

};
#endif

