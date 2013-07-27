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
#ifndef _Aruco_CameraParameters_H
#define  _Aruco_CameraParameters_H
#include "exports.h"
#include <opencv2/opencv.hpp>
#include <string>
using namespace std;
namespace aruco
{
/**\brief Parameters of the camera
 */

class ARUCO_EXPORTS  CameraParameters
{
public:

    // 3x3 matrix (fx 0 cx, 0 fy cy, 0 0 1)
    cv::Mat  CameraMatrix;
    //4x1 matrix (k1,k2,p1,p2)
    cv::Mat  Distorsion;
    //size of the image
    cv::Size CamSize;

    /**Empty constructor
     */
    CameraParameters() ;
    /**Creates the object from the info passed
     * @param cameraMatrix 3x3 matrix (fx 0 cx, 0 fy cy, 0 0 1)
     * @param distorsionCoeff 4x1 matrix (k1,k2,p1,p2)
     * @param size image size
     */
    CameraParameters(cv::Mat cameraMatrix,cv::Mat distorsionCoeff,cv::Size size) throw(cv::Exception);
    /**Sets the parameters
     * @param cameraMatrix 3x3 matrix (fx 0 cx, 0 fy cy, 0 0 1)
     * @param distorsionCoeff 4x1 matrix (k1,k2,p1,p2)
     * @param size image size
     */
    void setParams(cv::Mat cameraMatrix,cv::Mat distorsionCoeff,cv::Size size) throw(cv::Exception);
    /**Copy constructor
     */
    CameraParameters(const CameraParameters &CI) ;

    /**Indicates whether this object is valid
     */
    bool isValid()const {
        return CameraMatrix.rows!=0 && CameraMatrix.cols!=0  && Distorsion.rows!=0 && Distorsion.cols!=0 && CamSize.width!=-1 && CamSize.height!=-1;
    }
    /**Assign operator
    */
    CameraParameters & operator=(const CameraParameters &CI);
    /**Reads the camera parameters from a file generated using saveToFile.
     */
    void readFromFile(string path)throw(cv::Exception);
    /**Saves this to a file
     */
    void saveToFile(string path,bool inXML=true)throw(cv::Exception);

    /**Reads from a YAML file generated with the opencv2.2 calibration utility
     */
    void readFromXMLFile(string filePath)throw(cv::Exception);

    /**Adjust the parameters to the size of the image indicated
     */
    void resize(cv::Size size)throw(cv::Exception);

    /**Returns the location of the camera in the reference system given by the rotation and translation vectors passed
     * NOT TESTED
    */
    static cv::Point3f getCameraLocation(cv::Mat Rvec,cv::Mat Tvec);

    /**Given the intrinsic camera parameters returns the GL_PROJECTION matrix for opengl.
    * PLease NOTE that when using OpenGL, it is assumed no camera distorsion! So, if it is not true, you should have
    * undistor image
    *
    * @param orgImgSize size of the original image
    * @param size of the image/window where to render (can be different from the real camera image). Please not that it must be related to CamMatrix
    * @param proj_matrix output projection matrix to give to opengl
    * @param gnear,gfar: visible rendering range
    * @param invert: indicates if the output projection matrix has to yield a horizontally inverted image because image data has not been stored in the order of glDrawPixels: bottom-to-top.
    */
    void glGetProjectionMatrix( cv::Size orgImgSize, cv::Size size,double proj_matrix[16],double gnear,double gfar,bool invert=false   )throw(cv::Exception);
    
    /**
     * setup camera for an Ogre project.
     * 	Use:
     * ...
     * Ogre::Matrix4 PM(proj_matrix[0], proj_matrix[1], ... , proj_matrix[15]);
     * yourCamera->setCustomProjectionMatrix(true, PM);
     * yourCamera->setCustomViewMatrix(true, Ogre::Matrix4::IDENTITY);
     * ...
     * As in OpenGL, it assumes no camera distorsion
     */
    void OgreGetProjectionMatrix( cv::Size orgImgSize, cv::Size size,double proj_matrix[16],double gnear,double gfar,bool invert=false   )throw(cv::Exception);
    

private:
    //GL routines

    static void argConvGLcpara2( double cparam[3][4], int width, int height, double gnear, double gfar, double m[16], bool invert )throw(cv::Exception);
    static int  arParamDecompMat( double source[3][4], double cpara[3][4], double trans[3][4] )throw(cv::Exception);
    static double norm( double a, double b, double c );
    static double dot(  double a1, double a2, double a3,
                        double b1, double b2, double b3 );


};

}
#endif


