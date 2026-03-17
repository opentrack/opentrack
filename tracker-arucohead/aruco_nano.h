/** This is the ArucoNano library. A header only library that includes what 99% of users need in a single header file.
    We compress all our knowledge in less than 200 lines of code that you can simply drop into your project.

   The library detects markers of dictionaries ARUCO_MIP_36h12 (https://sourceforge.net/projects/aruco/files/aruco_mip_36h12_dict.zip/download)
    and AprilTag 36h11.


 Simply add this file to your project to start enjoying Aruco.  Example of Usage:

  #include <opencv2/highgui.hpp>
  #include "aruco_nano.h"
  int main(){
    auto image=cv::imread("/path/to/image");
    auto markers=aruconano::MarkerDetector::detect(image);
    for(const auto &m:markers)
       m.draw(image);
     cv::imwrite("/path/to/out.png",image);

    //now, compute R and T vectors
    cv::Mat camMatrix,distCoeff;
    float markerSize=0.05;//5cm
    //read CamMatrix and DistCoeffs from calibration FILE ....
     for(const auto &m:markers)
       auto r_t=m.estimatePose(camMatrix,distCoeff,markerSize);
  }

  If you use this file in your research, you must cite:

  1."Speeded up detection of squared fiducial markers", Francisco J.Romero-Ramirez, Rafael Muñoz-Salinas, Rafael Medina-Carnicer, Image and Vision Computing, vol 76, pages 38-47, year 2018
  2."Generation of fiducial marker dictionaries using mixed integer linear programming",S. Garrido-Jurado, R. Muñoz Salinas, F.J. Madrid-Cuevas, R. Medina-Carnicer, Pattern Recognition:51, 481-491,2016


  You can freely use the code in your commercial products.



 *ChangeLog:
 * version 5: adds support for  AprilTag 36h11. Use MarkerDetector::detect(img,10,MarkerDetector::APRILTAG_36h11);
 * version 6: increased adaptive thres window. No more obfuscation

Copyright 2025 Rafael Munoz-Salinas

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef _ArucoNANO_H_
#define _ArucoNANO_H_
#define ArucoNanoVersion 6
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <bitset>

namespace aruconano {
/**
 * @brief The Marker class is a marker detectable by the library
 * It is a vector where each corner is a corner of the detected marker
 */
class Marker : public std::vector<cv::Point2f>
{
public:
    // id of  the marker
    int id=-1;
    //draws it in a image
    inline void draw(cv::Mat &image,const cv::Scalar color=cv::Scalar(0,0,255))const;
    //given the camera params, returns the Rvec,Tvec indicating the pose of the marker wrt the camera, i.e., moves points
    //from the center of the marker to the camera center
    //This is just a call to cv::solvePnp using the  cv::SOLVEPNP_IPPE  method
    inline std::pair<cv::Mat,cv::Mat> estimatePose(cv::Mat cameraMatrix,cv::Mat distCoeffs,double markerSize=1.0f)const;
};

/** @brief The MarkerDetector class is detecting the markers in the image passed */
class MarkerDetector{
public:

    enum Dict:int{ARUCO_MIP_36h12=0,APRILTAG_36h11=1};
    //receives the input image and returns the detected markers. The second parameter allows to do multiple detection attempts for each
    // marker candidate. Introduces an small overhead, that is sometimes worth. If not, set this to one
    static inline std::vector<Marker> detect(const cv::Mat &img,unsigned int maxAttemptsPerCandidate=10,Dict dict=ARUCO_MIP_36h12){
        return _detect( img,maxAttemptsPerCandidate,dict);
    }
private:
    //obfuscate start
    static inline std::vector<Marker> _detect(const cv::Mat &img,unsigned int maxAttemptsPerCandidate=10,Dict dict=ARUCO_MIP_36h12);
    static inline  Marker sort( const  Marker &marker);
    static inline  float  getSubpixelValue(const cv::Mat &im_grey,const cv::Point2f &p);
    static inline  int    getMarkerId(const cv::Mat &bits,int &nrotations,const std::vector<uint64_t> &dict);
    static inline  int    perimeter(const std::vector<cv::Point2f>& a);
};

namespace _private {


struct Homographer{
    Homographer(const std::vector<cv::Point2f> & out ){
        std::vector<cv::Point2f>  in={cv::Point2f(0,0),cv::Point2f(1,0),cv::Point2f(1,1),cv::Point2f(0,1)};
        H=cv::getPerspectiveTransform(in, out);
    }
    cv::Point2f operator()(const cv::Point2f &p){
        double *m=H.ptr<double>(0);
        double a=m[0]*p.x+m[1]*p.y+m[2];
        double b=m[3]*p.x+m[4]*p.y+m[5];
        double c=m[6]*p.x+m[7]*p.y+m[8];
        return cv::Point2f(a/c,b/c);
    }
    cv::Mat H;
};
std::vector<uint64_t> Dict_codes;

}


std::vector<Marker>  MarkerDetector::_detect(const cv::Mat &img, unsigned int maxAttemptsPerCandidate, Dict dict){
     if(maxAttemptsPerCandidate==0) maxAttemptsPerCandidate=1;
    cv::Mat bwimage,thresImage;
    std::vector<Marker> DetectedMarkers;
    std::vector<uint64_t> Dict_codes;

    if(dict==ARUCO_MIP_36h12)
        Dict_codes={0xd2b63a09dUL,0x6001134e5UL,0x1206fbe72UL,0xff8ad6cb4UL,0x85da9bc49UL,0xb461afe9cUL,0x6db51fe13UL,0x5248c541fUL,0x8f34503UL,0x8ea462eceUL,0xeac2be76dUL,0x1af615c44UL,0xb48a49f27UL,0x2e4e1283bUL,0x78b1f2fa8UL,0x27d34f57eUL,0x89222fff1UL,0x4c1669406UL,0xbf49b3511UL,0xdc191cd5dUL,0x11d7c3f85UL,0x16a130e35UL,0xe29f27effUL,0x428d8ae0cUL,0x90d548477UL,0x2319cbc93UL,0xc3b0c3dfcUL,0x424bccc9UL,0x2a081d630UL,0x762743d96UL,0xd0645bf19UL,0xf38d7fd60UL,0xc6cbf9a10UL,0x3c1be7c65UL,0x276f75e63UL,0x4490a3f63UL,0xda60acd52UL,0x3cc68df59UL,0xab46f9daeUL,0x88d533d78UL,0xb6d62ec21UL,0xb3c02b646UL,0x22e56d408UL,0xac5f5770aUL,0xaaa993f66UL,0x4caa07c8dUL,0x5c9b4f7b0UL,0xaa9ef0e05UL,0x705c5750UL,0xac81f545eUL,0x735b91e74UL,0x8cc35cee4UL,0xe44694d04UL,0xb5e121de0UL,0x261017d0fUL,0xf1d439eb5UL,0xa1a33ac96UL,0x174c62c02UL,0x1ee27f716UL,0x8b1c5ece9UL,0x6a05b0c6aUL,0xd0568dfcUL,0x192d25e5fUL,0x1adbeccc8UL,0xcfec87f00UL,0xd0b9dde7aUL,0x88dcef81eUL,0x445681cb9UL,0xdbb2ffc83UL,0xa48d96df1UL,0xb72cc2e7dUL,0xc295b53fUL,0xf49832704UL,0x9968edc29UL,0x9e4e1af85UL,0x8683e2d1bUL,0x810b45c04UL,0x6ac44bfe2UL,0x645346615UL,0x3990bd598UL,0x1c9ed0f6aUL,0xc26729d65UL,0x83993f795UL,0x3ac05ac5dUL,0x357adff3bUL,0xd5c05565UL,0x2f547ef44UL,0x86c115041UL,0x640fd9e5fUL,0xce08bbcf7UL,0x109bb343eUL,0xc21435c92UL,0x35b4dfce4UL,0x459752cf2UL,0xec915b82cUL,0x51881eed0UL,0x2dda7dc97UL,0x2e0142144UL,0x42e890f99UL,0x9a8856527UL,0x8e80d9d80UL,0x891cbcf34UL,0x25dd82410UL,0x239551d34UL,0x8fe8f0c70UL,0x94106a970UL,0x82609b40cUL,0xfc9caf36UL,0x688181d11UL,0x718613c08UL,0xf1ab7629UL,0xa357bfc18UL,0x4c03b7a46UL,0x204dedce6UL,0xad6300d37UL,0x84cc4cd09UL,0x42160e5c4UL,0x87d2adfa8UL,0x7850e7749UL,0x4e750fc7cUL,0xbf2e5dfdaUL,0xd88324da5UL,0x234b52f80UL,0x378204514UL,0xabdf2ad53UL,0x365e78ef9UL,0x49caa6ca2UL,0x3c39ddf3UL,0xc68c5385dUL,0x5bfcbbf67UL,0x623241e21UL,0xabc90d5ccUL,0x388c6fe85UL,0xda0e2d62dUL,0x10855dfe9UL,0x4d46efd6bUL,0x76ea12d61UL,0x9db377d3dUL,0xeed0efa71UL,0xe6ec3ae2fUL,0x441faee83UL,0xba19c8ff5UL,0x313035eabUL,0x6ce8f7625UL,0x880dab58dUL,0x8d3409e0dUL,0x2be92ee21UL,0xd60302c6cUL,0x469ffc724UL,0x87eebeed3UL,0x42587ef7aUL,0x7a8cc4e52UL,0x76a437650UL,0x999e41ef4UL,0x7d0969e42UL,0xc02baf46bUL,0x9259f3e47UL,0x2116a1dc0UL,0x9f2de4d84UL,0xeffac29UL,0x7b371ff8cUL,0x668339da9UL,0xd010aee3fUL,0x1cd00b4c0UL,0x95070fc3bUL,0xf84c9a770UL,0x38f863d76UL,0x3646ff045UL,0xce1b96412UL,0x7a5d45da8UL,0x14e00ef6cUL,0x5e95abfd8UL,0xb2e9cb729UL,0x36c47dd7UL,0xb8ee97c6bUL,0xe9e8f657UL,0xd4ad2ef1aUL,0x8811c7f32UL,0x47bde7c31UL,0x3adadfb64UL,0x6e5b28574UL,0x33e67cd91UL,0x2ab9fdd2dUL,0x8afa67f2bUL,0xe6a28fc5eUL,0x72049cdbdUL,0xae65dac12UL,0x1251a4526UL,0x1089ab841UL,0xe2f096ee0UL,0xb0caee573UL,0xfd6677e86UL,0x444b3f518UL,0xbe8b3a56aUL,0x680a75cfcUL,0xac02baea8UL,0x97d815e1cUL,0x1d4386e08UL,0x1a14f5b0eUL,0xe658a8d81UL,0xa3868efa7UL,0x3668a9673UL,0xe8fc53d85UL,0x2e2b7edd5UL,0x8b2470f13UL,0xf69795f32UL,0x4589ffc8eUL,0x2e2080c9cUL,0x64265f7dUL,0x3d714dd10UL,0x1692c6ef1UL,0x3e67f2f49UL,0x5041dad63UL,0x1a1503415UL,0x64c18c742UL,0xa72eec35UL,0x1f0f9dc60UL,0xa9559bc67UL,0xf32911d0dUL,0x21c0d4ffcUL,0xe01cef5b0UL,0x4e23a3520UL,0xaa4f04e49UL,0xe1c4fcc43UL,0x208e8f6e8UL,0x8486774a5UL,0x9e98c7558UL,0x2c59fb7dcUL,0x9446a4613UL,0x8292dcc2eUL,0x4d61631UL,0xd05527809UL,0xa0163852dUL,0x8f657f639UL,0xcca6c3e37UL,0xcb136bc7aUL,0xfc5a83e53UL,0x9aa44fc30UL,0xbdec1bd3cUL,0xe020b9f7cUL,0x4b8f35fb0UL,0xb8165f637UL,0x33dc88d69UL,0x10a2f7e4dUL,0xc8cb5ff53UL,0xde259ff6bUL,0x46d070dd4UL,0x32d3b9741UL,0x7075f1c04UL,0x4d58dbea0UL};
    else
        Dict_codes={0xd5d628584UL,0xd97f18b49UL,0xdd280910eUL,0xe479e9c98UL,0xebcbca822UL,0xf31dab3acUL,0x56a5d085UL,0x10652e1d4UL,0x22b1dfeadUL,0x265ad0472UL,0x34fe91b86UL,0x3ff962cd5UL,0x43a25329aUL,0x474b4385fUL,0x4e9d243e9UL,0x5246149aeUL,0x5997f5538UL,0x683bb6c4cUL,0x6be4a7211UL,0x7e3158eeaUL,0x81da494afUL,0x858339a74UL,0x8cd51a5feUL,0x9f21cc2d7UL,0xa2cabc89cUL,0xadc58d9ebUL,0xb16e7dfb0UL,0xb8c05eb3aUL,0xd25ef139dUL,0xd607e1962UL,0xe4aba3076UL,0x2dde6a3daUL,0x43d40c678UL,0x5620be351UL,0x64c47fa65UL,0x686d7002aUL,0x6c16605efUL,0x6fbf50bb4UL,0x8d06d39dcUL,0x9f53856b5UL,0xadf746dc9UL,0xbc9b084ddUL,0xd290aa77bUL,0xd9e28b305UL,0xe4dd5c454UL,0xfad2fe6f2UL,0x181a8151aUL,0x26be42c2eUL,0x2e10237b8UL,0x405cd5491UL,0x7742eab1cUL,0x85e6ac230UL,0x8d388cdbaUL,0x9f853ea93UL,0xc41ea2445UL,0xcf1973594UL,0x14a34a333UL,0x31eacd15bUL,0x6c79d2dabUL,0x73cbb3935UL,0x89c155bd3UL,0x8d6a46198UL,0x91133675dUL,0xa708d89fbUL,0xae5ab9585UL,0xb9558a6d4UL,0xb98743ab2UL,0xd6cec68daUL,0x1506bcaefUL,0x4becd217aUL,0x4f95c273fUL,0x658b649ddUL,0xa76c4b1b7UL,0xecf621f56UL,0x1c8a56a57UL,0x3628e92baUL,0x53706c0e2UL,0x5e6b3d231UL,0x7809cfa94UL,0xe97eead6fUL,0x5af40604aUL,0x7492988adUL,0xed5994712UL,0x5eceaf9edUL,0x7c1632815UL,0xc1a0095b4UL,0xe9e25d52bUL,0x3a6705419UL,0xa8333012fUL,0x4ce5704d0UL,0x508e60a95UL,0x877476120UL,0xa864e950dUL,0xea45cfce7UL,0x19da047e8UL,0x24d4d5937UL,0x6e079cc9bUL,0x99f2e11d7UL,0x33aa50429UL,0x499ff26c7UL,0x50f1d3251UL,0x66e7754efUL,0x96ad633ceUL,0x9a5653993UL,0xaca30566cUL,0xc298a790aUL,0x8be44b65dUL,0xdc68f354bUL,0x16f7f919bUL,0x4dde0e826UL,0xd548cbd9fUL,0xe0439ceeeUL,0xfd8b1fd16UL,0x76521bb7bUL,0xd92375742UL,0xcab16d40cUL,0x730c9dd72UL,0xad9ba39c2UL,0xb14493f87UL,0x52b15651fUL,0x185409cadUL,0x77ae2c68dUL,0x94f5af4b5UL,0xa13bad55UL,0x61ea437cdUL,0xa022399e2UL,0x203b163d1UL,0x7bba8f40eUL,0x95bc9442dUL,0x41c0b5358UL,0x8e9c6cc81UL,0xeb549670UL,0x9da3a0b51UL,0xd832a67a1UL,0xdcd4350bcUL,0x4aa05fdd2UL,0x60c7bb44eUL,0x4b358b96cUL,0x67299b45UL,0xb9c89b5faUL,0x6975acaeaUL,0x62b8f7afaUL,0x33567c3d7UL,0xbac139950UL,0xa5927c62aUL,0x5c916e6a4UL,0x260ecb7d5UL,0x29b7bbd9aUL,0x903205f26UL,0xae72270a4UL,0x3d2ec51a7UL,0x82ea55324UL,0x11a6f3427UL,0x1ca1c4576UL,0xa40c81aefUL,0xbddccd730UL,0xe617561eUL,0x969317b0fUL,0x67f781364UL,0x610912f96UL,0xb2549fdfcUL,0x6e5aaa6bUL,0xb6c475339UL,0xc56836a4dUL,0x844e351ebUL,0x4647f83b4UL,0x908a04f5UL,0x7f51034c9UL,0xaee537fcaUL,0x5e92494baUL,0xd445808f4UL,0x28d68b563UL,0x4d25374bUL,0x2bc065f65UL,0x96dc3ea0cUL,0x4b2ade817UL,0x7c3fd502UL,0xe768b5cafUL,0x17605cf6cUL,0x182741ee4UL,0x62846097cUL,0x72b5ebf80UL,0x263da6e13UL,0xfa841bcb5UL,0x7e45e8c69UL,0x653c81fa0UL,0x7443b5e70UL,0xa5234afdUL,0x74756f24eUL,0x157ebf02aUL,0x82ef46939UL,0x80d420264UL,0x2aeed3e98UL,0xb0a1dd4f8UL,0xb5436be13UL,0x7b7b4b13bUL,0x1ce80d6d3UL,0x16c08427dUL,0xee54462ddUL,0x1f7644cceUL,0x9c7b5cc92UL,0xe369138f8UL,0x5d5a66e91UL,0x485d62f49UL,0xe6e819e94UL,0xb1f340eb5UL,0x9d198ce2UL,0xd60717437UL,0x196b856cUL,0xf0a6173a5UL,0x12c0e1ec6UL,0x62b82d5cfUL,0xad154c067UL,0xce3778832UL,0x6b0a7b864UL,0x4c7686694UL,0x5058ff3ecUL,0xd5e21ea23UL,0x9ff4a76eeUL,0x9dd981019UL,0x1bad4d30aUL,0xc601896d1UL,0x973439b48UL,0x1ce7431a8UL,0x57a8021d6UL,0xf9dba96e6UL,0x83a2e4e7cUL,0x8ea585380UL,0xaf6c0e744UL,0x875b73babUL,0xda34ca901UL,0x2ab9727efUL,0xd39f21b9aUL,0x8a10b742fUL,0x5f8952dbaUL,0xf8da71ab0UL,0xc25f9df96UL,0x6f8a5d94UL,0xe42e63e1aUL,0xb78409d1bUL,0x792229addUL,0x5acf8c455UL,0x2fc29a9b0UL,0xea486237bUL,0xb0c9685a0UL,0x1ad748a47UL,0x3b4712d5UL,0xf29216d30UL,0x8dad65e49UL,0xa2cf09ddUL,0xb5f174c6UL,0xe54f57743UL,0xb9cf54d78UL,0x4a312a88aUL,0x27babc962UL,0xb86897111UL,0xf2ff6c116UL,0x82274bd8aUL,0x97023505eUL,0x52d46edd1UL,0x585c1f538UL,0xbddd00e43UL,0x5590b74dfUL,0x729404a1fUL,0x65320855eUL,0xd3d4b6956UL,0x7ae374f14UL,0x2d7a60e06UL,0x315cd9b5eUL,0xfd36b4eacUL,0xf1df7642bUL,0x55db27726UL,0x8f15ebc19UL,0x992f8c531UL,0x62dea2a40UL,0x928275cabUL,0x69c263cb9UL,0xa774cca9eUL,0x266b2110eUL,0x1b14acbb8UL,0x624b8a71bUL,0x1c539406bUL,0x3086d529bUL,0x111dd66eUL,0x98cd630bfUL,0x8b9d1ffdcUL,0x72b2f61e7UL,0x9ed9d672bUL,0x96cdd15f3UL,0x6366c2504UL,0x6ca9df73aUL,0xa066d60f0UL,0xe7a4b8addUL,0x8264647efUL,0xaa195bf81UL,0x9a3db8244UL,0x14d2df6aUL,0xb63265b7UL,0x2f010de73UL,0x97e774986UL,0x248affc29UL,0xfb57dcd11UL,0xb1a7e4d9UL,0x4bfa2d07dUL,0x54e5cdf96UL,0x4c15c1c86UL,0xcd9c61166UL,0x499380b2aUL,0x540308d09UL,0x8b63fe66fUL,0xc81aeb35eUL,0x86fe0bd5cUL,0xce2480c2aUL,0x1ab29ee60UL,0x8048daa15UL,0xdbfeb2d39UL,0x567c9858cUL,0x2b6edc5bcUL,0x2078fca82UL,0xadacc22aaUL,0xb92486f49UL,0x51fac5964UL,0x691ee6420UL,0xf63b3e129UL,0x39be7e572UL,0xda2ce6c74UL,0x20cf17a5cUL,0xee55f9b6eUL,0xfb8572726UL,0xb2c2de548UL,0xcaa9bce92UL,0xae9182db3UL,0x74b6e5bd1UL,0x137b252afUL,0x51f686881UL,0xd672f6c02UL,0x654146ce4UL,0xf944bc825UL,0xe8327f809UL,0x76a73fd59UL,0xf79da4cb4UL,0x956f8099bUL,0x7b5f2655cUL,0xd06b114a6UL,0xd0697ca50UL,0x27c390797UL,0xbc61ed9b2UL,0xcc12dd19bUL,0xeb7818d2cUL,0x92fcecdaUL,0x89ded4ea1UL,0x256a0ba34UL,0xb6948e627UL,0x1ef6b1054UL,0x8639294a2UL,0xeda3780a4UL,0x39ee2af1dUL,0xcd257edc5UL,0x2d9d6bc22UL,0x121d3b47dUL,0x37e23f8adUL,0x119f31cf6UL,0x2c97f4f09UL,0xd502abfe0UL,0x10bc3ca77UL,0x53d7190efUL,0x90c3e62a6UL,0x7e9ebf675UL,0x979ce23d1UL,0x27f0c98e9UL,0xeafb4ae59UL,0x7ca7fe2bdUL,0x1490ca8f6UL,0x9123387baUL,0xb3bc73888UL,0x3ea87e325UL,0x4888964aaUL,0xa0188a6b9UL,0xcd383c666UL,0x40029a3fdUL,0xe1c00ac5cUL,0x39e6f2b6eUL,0xde664f622UL,0xe979a75e8UL,0x7c6b4c86cUL,0xfd492e071UL,0x8fbb35118UL,0x40b4a09b7UL,0xaf80bd6daUL,0x70e0b2521UL,0x2f5c54d93UL,0x3f4a118d5UL,0x9c1897b9UL,0x79776eacUL,0x84b00b17UL,0x3a95ad90eUL,0x28c544095UL,0x39d457c05UL,0x7a3791a78UL,0xbb770e22eUL,0x9a822bd6cUL,0x68a4b1fedUL,0xa5fd27b3bUL,0xc3995b79UL,0xd1519dff1UL,0x8e7eee359UL,0xcd3ca50b1UL,0xb73b8b793UL,0x57aca1c43UL,0xec2655277UL,0x785a2c1b3UL,0x75a07985aUL,0xa4b01eb69UL,0xa18a11347UL,0xdb1f28ca3UL,0x877ec3e25UL,0x31f6341b8UL,0x1363a3a4cUL,0x75d8b9baUL,0x7ae0792a9UL,0xa83a21651UL,0x7f08f9fb5UL,0xd0cf73a9UL,0xb04dcc98eUL,0xf65c7b0f8UL,0x65ddaf69aUL,0x2cf9b86b3UL,0x14cb51e25UL,0xf48027b5bUL,0xec26ea8bUL,0x44bafd45cUL,0xb12c7c0c4UL,0x959fd9d82UL,0xc77c9725aUL,0x48a22d462UL,0x8398e8072UL,0xec89b05ceUL,0xbb682d4c9UL,0xe5a86d2ffUL,0x358f01134UL,0x8556ddcf6UL,0x67584b6e2UL,0x11609439fUL,0x8488816eUL,0xaaf1a2c46UL,0xf879898cfUL,0x8bbe5e2f7UL,0x101eee363UL,0x690f69377UL,0xf5bd93cd9UL,0xcea4c2bf6UL,0x9550be706UL,0x2c5b38a60UL,0xe72033547UL,0x4458b0629UL,0xee8d9ed41UL,0xd2f918d72UL,0x78dc39fd3UL,0x8212636f6UL,0x7450a72a7UL,0xc4f0cf4c6UL,0x367bcddcdUL,0xc1caf8cc6UL,0xa7f5b853dUL,0x9d536818bUL,0x535e021b0UL,0xa7eb8729eUL,0x422a67b49UL,0x929e928a6UL,0x48e8aefccUL,0xa9897393cUL,0x5eb81d37eUL,0x1e80287b7UL,0x34770d903UL,0x2eef86728UL,0x59266ccb6UL,0x110bba61UL,0x1dfd284efUL,0x447439d1bUL,0xfece0e599UL,0x9309f3703UL,0x80764d1ddUL,0x353f1e6a0UL,0x2c1c12dccUL,0xc1d21b9d7UL,0x457ee453eUL,0xd66faf540UL,0x44831e652UL,0xcfd49a848UL,0x9312d4133UL,0x3f097d3eeUL,0x8c9ebef7aUL,0xa99e29e88UL,0xe9fab22cUL,0x4e748f4fbUL,0xecdee4288UL,0xabce5f1d0UL,0xc42f6876cUL,0x7ed402ea0UL,0xe5c4242c3UL,0xd5b2c31aeUL,0x286863be6UL,0x160444d94UL,0x5f0f5808eUL,0xae3d44b2aUL,0x9f5c5d109UL,0x8ad9316d7UL,0x3422ba064UL,0x2fed11d56UL,0xbea6e3e04UL,0x4b029eecUL,0x6deed7435UL,0x3718ce17cUL,0x55857f5e2UL,0x2edac7b62UL,0x85d6c512UL,0xd6ca88e0fUL,0x2b7e1fc69UL,0xa699d5c1bUL,0xf05ad74deUL,0x4cf5fb56dUL,0x5725e07e1UL,0x72f18a2deUL,0x1cec52609UL,0x48534243cUL,0x2523a4d69UL,0x35c1b80d1UL,0xa4d7338a7UL,0xdb1af012UL,0xe61a9475dUL,0x5df03f91UL,0x97ae260bbUL,0x32d627fefUL,0xb640f73c2UL,0x45a1ac9c6UL,0x6a2202de1UL,0x57d3e25f2UL,0x5aa9f986eUL,0xcc859d8aUL,0xe3ec6cca8UL,0x54e95e1aeUL,0x446887b06UL,0x7516732beUL,0x3817ac8f5UL,0x3e26d938cUL,0xaa81bc235UL,0xdf387ca1bUL,0xf3a3b3f2UL,0xb4bf69677UL,0xae21868edUL,0x81e1d2d9dUL,0xa0a9ea14cUL,0x8eee297a9UL,0x4740c0559UL,0xe8b141837UL,0xac69e0a3dUL,0x9ed83a1e1UL,0x5edb55ecbUL,0x7340fe81UL,0x50dfbc6bfUL,0x4f583508aUL,0xcb1fb78bcUL,0x4025ced2fUL,0x39791ebecUL,0x53ee388f1UL,0x7d6c0bd23UL,0x93a995fbeUL,0x8a41728deUL,0x2fe70e053UL,0xab3db443aUL,0x1364edb05UL,0x47b6eeed6UL,0x12e71af01UL,0x52ff83587UL,0x3a1575dd8UL,0x3feaa3564UL,0xeacf78ba7UL,0x872b94f8UL,0xda8ddf9a2UL,0x9aa920d2bUL,0x1f350ed36UL,0x18a5e861fUL,0x2c35b89c3UL,0x3347ac48aUL,0x7f23e022eUL,0x2459068fbUL,0xe83be4b73UL};

    //first, convert to bw
    if(img.channels()==3)
        cv::cvtColor(img,bwimage,cv::COLOR_BGR2GRAY);
    else bwimage=img;
     /////////////////// Adaptive Threshold to detect border
    cv::adaptiveThreshold(bwimage, thresImage, 255.,cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, 13, 7);
    /////////////////// compute marker candidates by detecting contours
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Point> approxCurve;
    cv::RNG rand;
    cv::findContours(thresImage, contours, cv::noArray(), cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
    cv::Mat bits(8,8,CV_8UC1);
    ///////////////// for each contour, approx to a rectangle and check bits inside
    for (unsigned int i = 0; i < contours.size(); i++)
    {
        // check it is a possible element by first checking that is is large enough
        if (50 > int(contours[i].size())  ) continue;
        // can approximate to a convex rect?
        cv::approxPolyDP(contours[i], approxCurve, double(contours[i].size()) * 0.05, true);
        if (approxCurve.size() != 4 || !cv::isContourConvex(approxCurve)) continue;
        // add the points
        Marker marker;
        for (int j = 0; j < 4; j++)
            marker.push_back( cv::Point2f( approxCurve[j].x,approxCurve[j].y));
        //sort corner in clockwise direction
        marker=sort(marker);
        ////// extract the code. Obtain the intensities of the bits using  homography
        for(int i=0;i<int(maxAttemptsPerCandidate) && marker.id==-1;i++){
            //if not first attempt, we may wanna produce small random alteration of the corners
            auto marker2=marker;
            if( i!=0) for(int c=0;c<4;c++) {marker2[c].x+=rand.gaussian(0.75);marker2[c].y+=rand.gaussian(0.75);}//if not first, alter corner location
            int pixelSum=0;
            _private::Homographer hom(marker2);
            for(int r=0;r<bits.rows;r++){
                for(int c=0;c<bits.cols;c++){
                    auto pixelValue=uchar(0.5+getSubpixelValue(bwimage,hom(cv::Point2f(  float(c+0.5) / float(bits.cols) ,  float(r+0.5) / float(bits.rows)  ))));
                    bits.at<uchar>(r,c)=pixelValue;
                    pixelSum+=pixelValue;
                }
            }
            //threshold by the average value
            double mean=double(pixelSum)/double(bits.cols*bits.rows);
            cv::threshold(bits,bits,mean,255,cv::THRESH_BINARY);

            //now, analyze the inner code to see it if is a marker. If so, rotate to have the points properly sorted
            int nrotations=0;
            marker.id=getMarkerId(bits,nrotations,Dict_codes);
            if(marker.id==-1) continue;//not a marker
            std::rotate(marker.begin(),marker.begin() + 4 - nrotations,marker.end());
        }
        if(marker.id!=-1) DetectedMarkers.push_back(marker);
    }


    //////  remove duplicates
    // sort by id and within same id set the largest first
    std::sort(DetectedMarkers.begin(), DetectedMarkers.end(),[](const Marker &a,const Marker &b){
        if( a.id<b.id) return true;
        else if( a.id==b.id) return perimeter(a)>perimeter(b);
        else return false;
    });
    // Using std::unique remove duplicates
    auto ip = std::unique(DetectedMarkers.begin(), DetectedMarkers.end(),[](const Marker &a,const Marker &b){return a.id==b.id;});
    DetectedMarkers.resize(std::distance(DetectedMarkers.begin(), ip));
    ////// finally subpixel corner refinement
    if(DetectedMarkers.size()>0){
        int halfwsize= 4*float(bwimage.cols)/float(bwimage.cols) +0.5 ;
        std::vector<cv::Point2f> Corners;
        for (const auto &m:DetectedMarkers)
            Corners.insert(Corners.end(), m.begin(),m.end());
        cv::cornerSubPix(bwimage, Corners, cv::Size(halfwsize,halfwsize), cv::Size(-1, -1),cv::TermCriteria( cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS, 12, 0.005));
        // copy back to the markers
        for (unsigned int i = 0; i < DetectedMarkers.size(); i++)
            for (int c = 0; c < 4; c++) DetectedMarkers[i][c] = Corners[i * 4 + c];
    }
    return DetectedMarkers;//DONE


}
int  MarkerDetector::perimeter(const std::vector<cv::Point2f>& a)
{
    int sum = 0;
    for (size_t i = 0; i < a.size(); i++)
        sum+=cv::norm( a[i]-a[(i + 1) % a.size()]);
    return sum;
}
int MarkerDetector:: getMarkerId(const cv::Mat &bits, int &nrotations, const std::vector<uint64_t> &dict){

    //converts the inner code to a uint64_t value
    auto   touulong=[](const cv::Mat& code)
    {
        std::bitset<64> bits;
        int bidx = 0;
        for (int y = code.rows - 1; y >= 0; y--)
            for (int x = code.cols - 1; x >= 0; x--)
                bits[bidx++] = code.at<uchar>(y, x);
        return bits.to_ullong();
    };
    auto rotate=[](const cv::Mat& in)
    {
        cv::Mat out(in.size(),in.type());
        for (int i = 0; i < in.rows; i++)
            for (int j = 0; j < in.cols; j++)
                out.at<uchar>(i, j) = in.at<uchar>(in.cols - j - 1, i);
        return out;
    };
    //first check that outer is all black
    for(int x=0;x<bits.cols;x++){
        if( bits.at<uchar>(0,x)!=0)return -1;
        if( bits.at<uchar>(bits.rows-1,x)!=0)return -1;
        if( bits.at<uchar>(x,0)!=0)return -1;
        if( bits.at<uchar>(x,bits.cols-1)!=0)return -1;
    }
    //now, get the inner bits wo the black border
    cv::Mat bit_inner(bits.cols-2,bits.rows-2,CV_8UC1);
    for(int r=0;r<bit_inner.rows;r++)
        for(int c=0;c<bit_inner.cols;c++)
            bit_inner.at<uchar>(r,c)=bits.at<uchar>(r+1,c+1);
    //convert into numbers for the different rotations and check if the dictionary
    nrotations = 0;
    do
    {
        auto id= touulong(bit_inner);
        //find it
        for(size_t i=0;i<dict.size();i++)
            if( dict[i]==id)
                return i;//done!
        bit_inner = rotate(bit_inner);
        nrotations++;
    } while (nrotations < 4);
    return -1;

}
float MarkerDetector::getSubpixelValue(const cv::Mat &im_grey,const cv::Point2f &p){
     float x=int(p.x);
    float y=int(p.y);
    //cheat to avoid seg fault
    if (x<0 || x>=im_grey.cols-1 || y<0 || y>=im_grey.rows-1)   return 0;
    const uchar* ptr_y=im_grey.ptr<uchar>(y);
    const uchar* ptr_yp=im_grey.ptr<uchar>(y+1);
    float tl=float(ptr_y[int(x)]);
    float tr=float(ptr_y[int(x+1)]);
    float bl=float(ptr_yp[int(x)]);
    float br=float(ptr_yp[int(x+1)]);
    float a= float(x+1.f-p.x) * tl  + (p.x-x)*tr;
    float b= float(x+1.f-p.x) * bl  + (p.x-x)*br;
    return   (y+1-p.y)*a   + (p.y-y)*b;
 }
Marker  MarkerDetector::sort( const  Marker &marker){
    Marker res_marker=marker;
    /// sort the points in anti-clockwise order
    // trace a line between the first and second point.
    // if the thrid point is at the right side, then the points are anti-clockwise
    double dx1 = res_marker[1].x - res_marker[0].x;
    double dy1 = res_marker[1].y - res_marker[0].y;
    double dx2 = res_marker[2].x - res_marker[0].x;
    double dy2 = res_marker[2].y - res_marker[0].y;
    double o = (dx1 * dy2) - (dy1 * dx2);
    // if the third point is in the left side, then sort in anti-clockwise order
    if (o < 0.0)  std::swap(res_marker[1], res_marker[3]);
    return res_marker;
}
//obfuscate end
std::pair<cv::Mat,cv::Mat> Marker::estimatePose(cv::Mat cameraMatrix,cv::Mat distCoeffs,double markerSize) const{
    std::vector<cv::Point3d> markerCorners={ {-markerSize/2.f,markerSize/2.f,0.f},{markerSize/2.f,markerSize/2.f,0.f},{markerSize/2.f,-markerSize/2.f,0.f},{-markerSize/2.f,-markerSize/2.f,0.f}};
    cv::Mat Rvec,Tvec;
    cv::solvePnP(markerCorners,*this,cameraMatrix,distCoeffs,Rvec,Tvec,false,cv::SOLVEPNP_IPPE);
    return {Rvec,Tvec};
}
void Marker::draw(cv::Mat &in, const cv::Scalar color) const{
    auto _to_string=[](int i){ std::stringstream str;str<<i;return str.str();};
    float flineWidth=  std::max(1.f, std::min(5.f, float(in.cols) / 500.f));
    int lineWidth= round( flineWidth);
    for(int i=0;i<4;i++)
        cv::line(in, (*this)[i], (*this)[(i+1 )%4], color, lineWidth);
    auto p2 =  cv::Point2f(2.f * static_cast<float>(lineWidth), 2.f * static_cast<float>(lineWidth));
    cv::rectangle(in, (*this)[0] - p2, (*this)[0] + p2, cv::Scalar(0, 0, 255, 255), -1);
    cv::rectangle(in, (*this)[1] - p2, (*this)[1] + p2, cv::Scalar(0, 255, 0, 255), lineWidth);
    cv::rectangle(in, (*this)[2] - p2, (*this)[2] + p2, cv::Scalar(255, 0, 0, 255), lineWidth);
    // determine the centroid
    cv::Point2f cent(0, 0);
    for(auto &p:*this) cent+=p;
    cent/=4;
    float fsize=  std::min(3.0f, flineWidth * 0.75f);
    cv::putText(in,_to_string(id), cent-cv::Point2f(10*flineWidth,0),  cv::FONT_HERSHEY_SIMPLEX,fsize,cv::Scalar(255,255,255)-color, lineWidth,cv::LINE_AA);
}

 }
#endif
