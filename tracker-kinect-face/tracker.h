


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
	module_status start_tracker(QFrame* aFrame) override;
	void data(double *data) override;
	bool center() override;

private:
	Timer t;

	// Kinect stuff
	static const int       cColorWidth = 1920;
	static const int       cColorHeight = 1080;


	void Update();
	HRESULT InitializeDefaultSensor();
	void ProcessFaces();
	HRESULT UpdateBodyData(IBody** ppBodies);
	void ExtractFaceRotationInDegrees(const Vector4* pQuaternion, float* pPitch, float* pYaw, float* pRoll);

	// Current Kinect
	IKinectSensor*         m_pKinectSensor;

	// Coordinate mapper
	ICoordinateMapper*     m_pCoordinateMapper;

	// Color reader
	IColorFrameReader*     m_pColorFrameReader;

	// Body reader
	IBodyFrameReader*      m_pBodyFrameReader;

	// Face sources
	IHighDefinitionFaceFrameSource*	   m_pFaceFrameSource;

	// Face readers
	IHighDefinitionFaceFrameReader*	   m_pFaceFrameReader;

	//
	RGBQUAD*               m_pColorRGBX;

	CameraSpacePoint iLastFacePosition;
	CameraSpacePoint iFacePosition;
	CameraSpacePoint iFacePositionCenter;

	Vector4 iFaceRotationQuaternion;
	// As Yaw, Pitch, Roll
	CameraSpacePoint iLastFaceRotation;
	CameraSpacePoint iFaceRotation;
	CameraSpacePoint iFaceRotationCenter;
};
