/* Copyright (c) 2019, Stephane Lenclud <github@lenclud.com>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <cmath>

#include "api/plugin-api.hpp"
#include "compat/timer.hpp"
#include "video/video-widget.hpp"

// Kinect Header files
#include <Kinect.h>
#include <Kinect.Face.h>

// @deprecated Use UniqueInterface instead. Remove it at some point.
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != nullptr)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = nullptr;
	}
}

template<class Interface>
inline void ReleaseInterface(Interface* pInterfaceToRelease)
{
	if (pInterfaceToRelease != nullptr)
	{
		pInterfaceToRelease->Release();
	}
}

// Safely use Microsoft interfaces.
template<typename T>
class UniqueInterface : public std::unique_ptr<T, decltype(&ReleaseInterface<T>)> ///**/
{
public:
	UniqueInterface() : std::unique_ptr<T, decltype(&ReleaseInterface<T>)>(nullptr, ReleaseInterface<T>){}
	// Access pointer, typically for creation
	T** PtrPtr() { return &iPtr; };
	// Called this once the pointer was created
	void Reset() { std::unique_ptr<T, decltype(&ReleaseInterface<T>)>::reset(iPtr); }
	// If ever you want to release that interface before the object is deleted
	void Free() { iPtr = nullptr; Reset(); }	
private:
	T* iPtr = nullptr;
};


//
//
//
class KinectFaceTracker : public ITracker
{
public:
	KinectFaceTracker();
	~KinectFaceTracker() override;
	module_status start_tracker(QFrame* aFrame) override;
	void data(double *data) override;
	bool center() override;

private:
	

	// Kinect stuff
	void Update();
	HRESULT InitializeDefaultSensor();
	void ProcessFaces();
	HRESULT UpdateBodyData(IBody** ppBodies);
	void ExtractFaceRotationInDegrees(const Vector4* pQuaternion, float* pPitch, float* pYaw, float* pRoll);
	static IBody* FindClosestBody(IBody** aBodies);
	static IBody* FindTrackedBodyById(IBody** aBodies,UINT64 aTrackingId);
	
	//
	Timer iTimer;

	// Current Kinect
	IKinectSensor* iKinectSensor = nullptr;

	// Color reader
	IColorFrameReader* iColorFrameReader = nullptr;

	// Body reader
	IBodyFrameReader* iBodyFrameReader = nullptr;

	// Face sources
	IHighDefinitionFaceFrameSource*	iFaceFrameSource = nullptr;

	// Face readers
	IHighDefinitionFaceFrameReader*	iFaceFrameReader = nullptr;

	//
	RGBQUAD* iColorRGBX = nullptr;

	RectI iFaceBox = { 0 };

	// Face position
	CameraSpacePoint iLastFacePosition = { 0 };
	CameraSpacePoint iFacePosition = { 0 };
	CameraSpacePoint iFacePositionCenter = { 0 };

	Vector4 iFaceRotationQuaternion = { 0 };
	// As Yaw, Pitch, Roll
	CameraSpacePoint iLastFaceRotation = { 0 };
	CameraSpacePoint iFaceRotation = { 0 };
	CameraSpacePoint iFaceRotationCenter = { 0 };
	//
	std::unique_ptr<video_widget> iVideoWidget;
	std::unique_ptr<QLayout> iLayout;

	// Id of the body currently being tracked
	UINT64 iTrackingId = 0;
};
