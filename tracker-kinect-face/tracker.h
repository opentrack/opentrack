


#include <cmath>

#include "api/plugin-api.hpp"
#include "compat/timer.hpp"
#include "compat/macros.hpp"

// Kinect Header files
#include <Kinect.h>
#include <Kinect.Face.h>

#pragma once

// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != nullptr)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = nullptr;
	}
}


class KinectFaceTracker : public ITracker
{
public:
	KinectFaceTracker();
	~KinectFaceTracker() override;
	module_status start_tracker(QFrame *) override;
	void data(double *data) override;

private:
	static const double incr[6];
	double last_x[6]{};
	Timer t;

	// Kinect stuff

	// define the face frame features required to be computed by this application
	static const DWORD c_FaceFrameFeatures =
		FaceFrameFeatures::FaceFrameFeatures_BoundingBoxInColorSpace
		| FaceFrameFeatures::FaceFrameFeatures_PointsInColorSpace
		| FaceFrameFeatures::FaceFrameFeatures_RotationOrientation
		| FaceFrameFeatures::FaceFrameFeatures_Happy
		| FaceFrameFeatures::FaceFrameFeatures_RightEyeClosed
		| FaceFrameFeatures::FaceFrameFeatures_LeftEyeClosed
		| FaceFrameFeatures::FaceFrameFeatures_MouthOpen
		| FaceFrameFeatures::FaceFrameFeatures_MouthMoved
		| FaceFrameFeatures::FaceFrameFeatures_LookingAway
		| FaceFrameFeatures::FaceFrameFeatures_Glasses
		| FaceFrameFeatures::FaceFrameFeatures_FaceEngagement;

	static const int       cColorWidth = 1920;
	static const int       cColorHeight = 1080;


	void Update();
	HRESULT InitializeDefaultSensor();
	void ProcessFaces();
	HRESULT UpdateBodyData(IBody** ppBodies);
	void ExtractFaceRotationInDegrees(const Vector4* pQuaternion, double* pPitch, double* pYaw, double* pRoll);

	// Current Kinect
	IKinectSensor*         m_pKinectSensor;

	// Coordinate mapper
	ICoordinateMapper*     m_pCoordinateMapper;

	// Color reader
	IColorFrameReader*     m_pColorFrameReader;

	// Body reader
	IBodyFrameReader*      m_pBodyFrameReader;

	// Face sources
	IFaceFrameSource*	   m_pFaceFrameSources[BODY_COUNT];

	// Face readers
	IFaceFrameReader*	   m_pFaceFrameReaders[BODY_COUNT];

	//
	RGBQUAD*               m_pColorRGBX;

	Vector4 faceRotation;

};
