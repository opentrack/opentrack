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
#ifndef _ARUCO_MarkerDetector_H
#define _ARUCO_MarkerDetector_H
#include <opencv2/core/core.hpp>
#include <cstdio>
#include <iostream>
#include "cameraparameters.h"
#include "exports.h"
#include "marker.h"

namespace aruco
{
using namespace std;
/**\brief Main class for marker detection
 *
 */
class ARUCO_EXPORTS  MarkerDetector
{
  //Represent a candidate to be a maker
  class MarkerCandidate: public Marker{
  public:
    MarkerCandidate(){}
    MarkerCandidate(const Marker &M): Marker(M){}
    MarkerCandidate(const  MarkerCandidate &M): Marker(M){
      contour=M.contour;
      idx=M.idx;
    }
    MarkerCandidate & operator=(const  MarkerCandidate &M){
      *(Marker*)this = M;
      contour=M.contour;
      idx=M.idx;
      return *this;
    }

    vector<cv::Point> contour;//all the points of its contour
    int idx;//index position in the global contour list
  };
public:

    /**
     * See
     */
    MarkerDetector();

    /**
     */
    ~MarkerDetector();

    /**Detects the markers in the image passed
     *
     * If you provide information about the camera parameters and the size of the marker, then, the extrinsics of the markers are detected
     *
     * @param input input color image
     * @param detectedMarkers output vector with the markers detected
     * @param camMatrix intrinsic camera information.
     * @param distCoeff camera distorsion coefficient. If set Mat() if is assumed no camera distorion
     * @param markerSizeMeters size of the marker sides expressed in meters
     * @param setYPerperdicular If set the Y axis will be perpendicular to the surface. Otherwise, it will be the Z axis
     */
    void detect(const cv::Mat &input,std::vector<Marker> &detectedMarkers,cv::Mat camMatrix=cv::Mat(),cv::Mat distCoeff=cv::Mat(),float markerSizeMeters=-1,bool setYPerperdicular=true) noexcept;
    /**Detects the markers in the image passed
     *
     * If you provide information about the camera parameters and the size of the marker, then, the extrinsics of the markers are detected
     *
     * @param input input color image
     * @param detectedMarkers output vector with the markers detected
     * @param camParams Camera parameters
     * @param markerSizeMeters size of the marker sides expressed in meters
     * @param setYPerperdicular If set the Y axis will be perpendicular to the surface. Otherwise, it will be the Z axis
     */
    void detect(const cv::Mat &input,std::vector<Marker> &detectedMarkers, CameraParameters camParams,float markerSizeMeters=-1,bool setYPerperdicular=true) noexcept;

    /**This set the type of thresholding methods available
     */

    enum ThresholdMethods {FIXED_THRES,ADPT_THRES,CANNY};



    /**Sets the threshold method
     */
    void setThresholdMethod(ThresholdMethods m) {
        _thresMethod=m;
    }
    /**Returns the current threshold method
     */
    ThresholdMethods getThresholdMethod()const {
        return _thresMethod;
    }
    /**
     * Set the parameters of the threshold method
     * We are currently using the Adptive threshold ee opencv doc of adaptiveThreshold for more info
     *   @param param1: blockSize of the pixel neighborhood that is used to calculate a threshold value for the pixel
     *   @param param2: The constant subtracted from the mean or weighted mean
     */
    void setThresholdParams(double param1,double param2) {
        _thresParam1=param1;
        _thresParam2=param2;
    }
    /**
     * Set the parameters of the threshold method
     * We are currently using the Adptive threshold ee opencv doc of adaptiveThreshold for more info
     *   param1: blockSize of the pixel neighborhood that is used to calculate a threshold value for the pixel
     *   param2: The constant subtracted from the mean or weighted mean
     */
    void getThresholdParams(double &param1,double &param2)const {
        param1=_thresParam1;
        param2=_thresParam2;
    }


    /**Returns a reference to the internal image thresholded. It is for visualization purposes and to adjust manually
     * the parameters
     */
    const cv::Mat & getThresholdedImage() {
        return thres;
    }
    /**Methods for corner refinement
     */
    enum CornerRefinementMethod {NONE,HARRIS,SUBPIX,LINES};
    /**
     */
    void setCornerRefinementMethod(CornerRefinementMethod method) {
        _cornerMethod=method;
    }
    /**
     */
    CornerRefinementMethod getCornerRefinementMethod()const {
        return _cornerMethod;
    }
    /**Specifies the min and max sizes of the markers as a fraction of the image size. By size we mean the maximum
     * of cols and rows.
     * @param min size of the contour to consider a possible marker as valid (0,1]
     * @param max size of the contour to consider a possible marker as valid [0,1)
     *
     */
    void setMinMaxSize(float min=0.03,float max=0.5)noexcept;

    /**reads the min and max sizes employed
     * @param min output size of the contour to consider a possible marker as valid (0,1]
     * @param max output size of the contour to consider a possible marker as valid [0,1)
     *
     */
    void getMinMaxSize(float &min,float &max){min=_minSize;max=_maxSize;}

    /**Enables/Disables erosion process that is REQUIRED for chessboard like boards.
     * By default, this property is enabled
     */
    void enableErosion(bool enable){_doErosion=enable;}

    /**
     * Specifies a value to indicate the required speed for the internal processes. If you need maximum speed (at the cost of a lower detection rate),
     * use the value 3, If you rather a more precise and slow detection, set it to 0.
     *
     * Actually, the main differences are that in highspeed mode, we employ setCornerRefinementMethod(NONE) and internally, we use a small canonical
     * image to detect the marker. In low speed mode, we use setCornerRefinementMethod(HARRIS) and a bigger size for the canonical marker image
     */
    void setDesiredSpeed(int val);
    /**
     */
    int getDesiredSpeed()const {
        return _speed;
    }

    /**
     * Allows to specify the function that identifies a marker. Therefore, you can create your own type of markers different from these
     * employed by default in the library.
     * The marker function must have the following structure:
     *
     * int myMarkerIdentifier(const cv::Mat &in,int &nRotations);
     *
     * The marker function receives the image 'in' with the region that migh contain one of your markers. These are the rectangular regions with black
     *  in the image.
     *
     * As output your marker function must indicate the following information. First, the output parameter nRotations must indicate how many times the marker
     * must be rotated clockwise 90 deg  to be in its ideal position. (The way you would see it when you print it). This is employed to know
     * always which is the corner that acts as reference system. Second, the function must return -1 if the image does not contains one of your markers, and its id otherwise.
     *
     */
    void setMakerDetectorFunction(int (* markerdetector_func)(const cv::Mat &in,int &nRotations) ) {
        markerIdDetector_ptrfunc=markerdetector_func;
    }

    /** Use an smaller version of the input image for marker detection.
     * If your marker is small enough, you can employ an smaller image to perform the detection without noticeable reduction in the precision.
     * Internally, we are performing a pyrdown operation
     *
     * @param level number of times the image size is divided by 2. Internally, we are performing a pyrdown.
     */
    void pyrDown(unsigned int level){pyrdown_level=level;}

    ///-------------------------------------------------
    /// Methods you may not need
    /// Thesde methods do the hard work. They have been set public in case you want to do customizations
    ///-------------------------------------------------

    /**
     * Thesholds the passed image with the specified method.
     */
    void thresHold(int method,const cv::Mat &grey,cv::Mat &thresImg,double param1=-1,double param2=-1)noexcept;
    /**
    * Detection of candidates to be markers, i.e., rectangles.
    * This function returns in candidates all the rectangles found in a thresolded image
    */
    void detectRectangles(const cv::Mat &thresImg,vector<std::vector<cv::Point2f> > & candidates);

    /**Returns a list candidates to be markers (rectangles), for which no valid id was found after calling detectRectangles
     */
    const vector<std::vector<cv::Point2f> > &getCandidates() {
        return _candidates;
    }

    /**Given the iput image with markers, creates an output image with it in the canonical position
     * @param in input image
     * @param out image with the marker
     * @param size of out
     * @param points 4 corners of the marker in the image in
     * @return true if the operation succeed
     */
    bool warp(cv::Mat &in,cv::Mat &out,cv::Size size, std::vector<cv::Point2f> points)noexcept;



    /** Refine MarkerCandidate Corner using LINES method
     * @param candidate candidate to refine corners
     */
    void refineCandidateLines(MarkerCandidate &candidate);


    /**DEPRECATED!!! Use the member function in CameraParameters
     *
     * Given the intrinsic camera parameters returns the GL_PROJECTION matrix for opengl.
     * PLease NOTE that when using OpenGL, it is assumed no camera distorsion! So, if it is not true, you should have
     * undistor image
     *
     * @param CamMatrix  arameters of the camera specified.
     * @param orgImgSize size of the original image
     * @param size of the image/window where to render (can be different from the real camera image). Please not that it must be related to CamMatrix
     * @param proj_matrix output projection matrix to give to opengl
     * @param gnear,gfar: visible rendering range
     * @param invert: indicates if the output projection matrix has to yield a horizontally inverted image because image data has not been stored in the order of glDrawPixels: bottom-to-top.
     */
    static void glGetProjectionMatrix( CameraParameters &  CamMatrix,cv::Size orgImgSize, cv::Size size,double proj_matrix[16],double gnear,double gfar,bool invert=false   )noexcept;

private:

    bool _enableCylinderWarp;
    bool warp_cylinder ( cv::Mat &in,cv::Mat &out,cv::Size size, MarkerCandidate& mc ) noexcept;
    /**
    * Detection of candidates to be markers, i.e., rectangles.
    * This function returns in candidates all the rectangles found in a thresolded image
    */
    void detectRectangles(const cv::Mat &thresImg,vector<MarkerCandidate> & candidates);
public:
    std::vector<std::vector<cv::Point>> contours;
    //Current threshold method
    ThresholdMethods _thresMethod;
    //Threshold parameters
    double _thresParam1,_thresParam2;
    //Current corner method
    CornerRefinementMethod _cornerMethod;
    //minimum and maximum size of a contour lenght
    float _minSize,_maxSize;
    //Speed control
    int _speed;
    int _markerWarpSize;
    bool _doErosion;
    //vectr of candidates to be markers. This is a vector with a set of rectangles that have no valid id
    vector<std::vector<cv::Point2f> > _candidates;
    //level of image reduction
    int pyrdown_level;
    //Images
    cv::Mat grey,thres,thres2,reduced;
    //pointer to the function that analizes a rectangular region so as to detect its internal marker
    int (* markerIdDetector_ptrfunc)(const cv::Mat &in,int &nRotations);
private:
    /**
     */
    bool isInto(cv::Mat &contour,std::vector<cv::Point2f> &b);
    /**
     */
    int perimeter(std::vector<cv::Point2f> &a);


//     //GL routines
//
//     static void argConvGLcpara2( double cparam[3][4], int width, int height, double gnear, double gfar, double m[16], bool invert )noexcept;
//     static int  arParamDecompMat( double source[3][4], double cpara[3][4], double trans[3][4] )noexcept;
//     static double norm( double a, double b, double c );
//     static double dot(  double a1, double a2, double a3,
//                         double b1, double b2, double b3 );
//

    //detection of the
    void findBestCornerInRegion_harris(const cv::Mat  & grey,vector<cv::Point2f> &  Corners,int blockSize);


    // auxiliar functions to perform LINES refinement
    void interpolate2Dline( const vector< cv::Point > &inPoints, cv::Point3f &outLine);
    cv::Point2f getCrossPoint(const cv::Point3f& line1, const cv::Point3f& line2);


    /**Given a vector vinout with elements and a boolean vector indicating the lements from it to remove,
     * this function remove the elements
     * @param vinout
     * @param toRemove
     */
    template<typename T>
    void removeElements(vector<T> & vinout,const vector<bool> &toRemove)
    {
       //remove the invalid ones by setting the valid in the positions left by the invalids
      size_t indexValid=0;
      for (size_t i=0;i<toRemove.size();i++) {
        if (!toRemove[i]) {
            if (indexValid!=i) vinout[indexValid]=vinout[i];
            indexValid++;
        }
      }
      vinout.resize(indexValid);
    }

    //graphical debug
    void drawApproxCurve(cv::Mat &in,std::vector<cv::Point>  &approxCurve ,cv::Scalar color);
    void drawContour(cv::Mat &in,std::vector<cv::Point>  &contour,cv::Scalar  );
    void drawAllContours(cv::Mat input, std::vector<std::vector<cv::Point> > &contours);
    void draw(cv::Mat out,const std::vector<Marker> &markers );

};




}
#endif
