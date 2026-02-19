/* Copyright (c) 2019, Stéphane Lenclud <github@lenclud.com>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "kinect_face_tracker.h"

#include <QLayout>
#include <QPainter>

#include "compat/check-visible.hpp"

static const int KColorWidth = 1920;
static const int KColorHeight = 1080;

///
bool IsValidRect(const RectI& aRect)
{
	if (aRect.Bottom != 0)
	{
		return true;
	}

	if (aRect.Left != 0)
	{
		return true;
	}

	if (aRect.Right != 0)
	{
		return true;
	}

	if (aRect.Top != 0)
	{
		return true;
	}

	return false;
}

///
bool IsNullVetor(const Vector4& aVector)
{
	if (aVector.w != 0)
	{
		return false;
	}

	if (aVector.x != 0)
	{
		return false;
	}

	if (aVector.y != 0)
	{
		return false;
	}

	if (aVector.z != 0)
	{
		return false;
	}

	return true;
}

///
bool IsNullPoint(const CameraSpacePoint& aPoint)
{
	if (aPoint.X != 0)
	{
		return false;
	}

	if (aPoint.Y != 0)
	{
		return false;
	}

	if (aPoint.Z != 0)
	{
		return false;
	}

	return true;
}


KinectFaceTracker::KinectFaceTracker()
{
	// create heap storage for color pixel data in RGBX format
	iColorRGBX = new RGBQUAD[KColorWidth * KColorHeight];
}

KinectFaceTracker::~KinectFaceTracker()
{
	if (iColorRGBX)
	{
		delete[] iColorRGBX;
		iColorRGBX = nullptr;
	}

	// clean up Direct2D
	//SafeRelease(m_pD2DFactory);

	// done with face sources and readers
	SafeRelease(iFaceFrameSource);
	SafeRelease(iFaceFrameReader);

	// done with body frame reader
	SafeRelease(iBodyFrameReader);

	// done with color frame reader
	SafeRelease(iColorFrameReader);

	// close the Kinect Sensor
	if (iKinectSensor)
	{
		iKinectSensor->Close();
	}

	SafeRelease(iKinectSensor);
}

module_status KinectFaceTracker::start_tracker(QFrame* aFrame)
{
	iTimer.start();

	if (SUCCEEDED(InitializeDefaultSensor()))
	{
		// Setup our video preview widget
		iVideoWidget = std::make_unique<video_widget>(aFrame);
		iLayout = std::make_unique<QHBoxLayout>(aFrame);
		iLayout->setContentsMargins(0, 0, 0, 0);
		iLayout->addWidget(&*iVideoWidget);
		aFrame->setLayout(&*iLayout);
		//video_widget->resize(video_frame->width(), video_frame->height());
		aFrame->show();

		return status_ok();
	}

	return error("Kinect init failed!");
}


bool KinectFaceTracker::center()
{
	// Mark our center
	iFacePositionCenter = iFacePosition;
	iFaceRotationCenter = iFaceRotation;
	return true;
}

//
//
//
void KinectFaceTracker::data(double *data)
{
	const double dt = iTimer.elapsed_seconds();

	const double KMinDelayInSeconds = 1.0 / 30.0; // Pointless running faster than Kinect hardware itself
	if (dt > KMinDelayInSeconds)
	{
		iTimer.start(); // Reset our timer
		//OutputDebugStringA("Updating frame!\n");
		Update();
		ExtractFaceRotationInDegrees(&iFaceRotationQuaternion, &iFaceRotation.X, &iFaceRotation.Y, &iFaceRotation.Z);
		//Check if data is valid
		if (IsValidRect(iFaceBox))
		{
			// We have valid tracking retain position and rotation
			iLastFacePosition = iFacePosition;
			iLastFaceRotation = iFaceRotation;
		}
		else
		{
			//TODO: after like 5s without tracking reset position to zero
			//TODO: Instead of hardcoding that delay add it to our settings
		}
	}
	else
	{
		//OutputDebugStringA("Skipping frame!\n");
	}

	// Feed our framework our last valid position and rotation
	data[0] = (iLastFacePosition.X - iFacePositionCenter.X) * 100; // Convert to centimer to be in a range that suites opentrack.
	data[1] = (iLastFacePosition.Y - iFacePositionCenter.Y) * 100;
	data[2] = (iLastFacePosition.Z - iFacePositionCenter.Z) * 100;

	// Yaw, Picth, Roll
	data[3] = -(iLastFaceRotation.X - iFaceRotationCenter.X); // Invert to be compatible with ED out-of-the-box
	data[4] = (iLastFaceRotation.Y - iFaceRotationCenter.Y);
	data[5] = (iLastFaceRotation.Z - iFaceRotationCenter.Z);
}


/// <summary>
/// Converts rotation quaternion to Euler angles
/// And then maps them to a specified range of values to control the refresh rate
/// </summary>
/// <param name="pQuaternion">face rotation quaternion</param>
/// <param name="pPitch">rotation about the X-axis</param>
/// <param name="pYaw">rotation about the Y-axis</param>
/// <param name="pRoll">rotation about the Z-axis</param>
void KinectFaceTracker::ExtractFaceRotationInDegrees(const Vector4* pQuaternion, float* pYaw, float* pPitch, float* pRoll)
{
	double x = pQuaternion->x;
	double y = pQuaternion->y;
	double z = pQuaternion->z;
	double w = pQuaternion->w;

	// convert face rotation quaternion to Euler angles in degrees
	double dPitch, dYaw, dRoll;
	dPitch = atan2(2 * (y * z + w * x), w * w - x * x - y * y + z * z) / M_PI * 180.0;
	dYaw = asin(2 * (w * y - x * z)) / M_PI * 180.0;
	dRoll = atan2(2 * (x * y + w * z), w * w + x * x - y * y - z * z) / M_PI * 180.0;

	// clamp rotation values in degrees to a specified range of values to control the refresh rate
	/*
	double increment = c_FaceRotationIncrementInDegrees;
	*pPitch = static_cast<int>(floor((dPitch + increment/2.0 * (dPitch > 0 ? 1.0 : -1.0)) / increment) * increment);
	*pYaw = static_cast<int>(floor((dYaw + increment/2.0 * (dYaw > 0 ? 1.0 : -1.0)) / increment) * increment);
	*pRoll = static_cast<int>(floor((dRoll + increment/2.0 * (dRoll > 0 ? 1.0 : -1.0)) / increment) * increment);
	*/

	*pPitch = dPitch;
	*pYaw = dYaw;
	*pRoll = dRoll;
}



/// <summary>
/// Initializes the default Kinect sensor
/// </summary>
/// <returns>S_OK on success else the failure code</returns>
HRESULT KinectFaceTracker::InitializeDefaultSensor()
{
	HRESULT hr;

	// Get and open Kinect sensor
	hr = GetDefaultKinectSensor(&iKinectSensor);
	if (SUCCEEDED(hr))
	{
		hr = iKinectSensor->Open();
	}

	// Create color frame reader
	if (SUCCEEDED(hr))
	{
		UniqueInterface<IColorFrameSource> colorFrameSource;
		hr = iKinectSensor->get_ColorFrameSource(colorFrameSource.PtrPtr());
		colorFrameSource.Reset();

		if (SUCCEEDED(hr))
		{
			hr = colorFrameSource->OpenReader(&iColorFrameReader);
		}
	}

	// Create body frame reader
	if (SUCCEEDED(hr))
	{
		UniqueInterface<IBodyFrameSource> bodyFrameSource;
		hr = iKinectSensor->get_BodyFrameSource(bodyFrameSource.PtrPtr());
		bodyFrameSource.Reset();

		if (SUCCEEDED(hr))
		{
			hr = bodyFrameSource->OpenReader(&iBodyFrameReader);
		}
	}

	// Create HD face frame source
	if (SUCCEEDED(hr))
	{
		// create the face frame source by specifying the required face frame features
		hr = CreateHighDefinitionFaceFrameSource(iKinectSensor, &iFaceFrameSource);
	}

	// Create HD face frame reader
	if (SUCCEEDED(hr))
	{
		// open the corresponding reader
		hr = iFaceFrameSource->OpenReader(&iFaceFrameReader);
	}

	return hr;
}



/// <summary>
/// Main processing function
/// </summary>
void KinectFaceTracker::Update()
{
	if (!iColorFrameReader || !iBodyFrameReader)
	{
		return;
	}

	IColorFrame* pColorFrame = nullptr;
	HRESULT hr = iColorFrameReader->AcquireLatestFrame(&pColorFrame);

	if (SUCCEEDED(hr))
	{
		INT64 nTime = 0;
		IFrameDescription* pFrameDescription = nullptr;
		int nWidth = 0;
		int nHeight = 0;
		ColorImageFormat imageFormat = ColorImageFormat_None;
		UINT nBufferSize = 0;
		RGBQUAD *pBuffer = nullptr;

		hr = pColorFrame->get_RelativeTime(&nTime);

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_FrameDescription(&pFrameDescription);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Width(&nWidth);
		}

		if (SUCCEEDED(hr))
		{
			hr = pFrameDescription->get_Height(&nHeight);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
		}

		if (SUCCEEDED(hr))
		{
			//DrawStreams(nTime, pBuffer, nWidth, nHeight);
			ProcessFaces();
		}

		if (check_is_visible())
		{
			//OutputDebugStringA("Widget visible!\n");
			// If our widget is visible we feed it our frame
			if (SUCCEEDED(hr))
			{
				// Fetch color buffer
				if (imageFormat == ColorImageFormat_Rgba)
				{
					hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
				}
				else if (iColorRGBX)
				{
					pBuffer = iColorRGBX;
					nBufferSize = KColorWidth * KColorHeight * sizeof(RGBQUAD);
					hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Rgba);
				}
				else
				{
					hr = E_FAIL;
				}

			}

			if (SUCCEEDED(hr))
			{
				// Setup our image
				QImage image((const unsigned char*)pBuffer, KColorWidth, KColorHeight, sizeof(RGBQUAD)*KColorWidth, QImage::Format_RGBA8888);
				if (IsValidRect(iFaceBox))
				{
					// Draw our face bounding box
					QPainter painter(&image);
					painter.setBrush(Qt::NoBrush);
					painter.setPen(QPen(Qt::red, 8));
					painter.drawRect(iFaceBox.Left, iFaceBox.Top, iFaceBox.Right - iFaceBox.Left, iFaceBox.Bottom - iFaceBox.Top);
					bool bEnd = painter.end();
					(void)bEnd;
				}

				// Update our video preview
				iVideoWidget->update_image(image);
			}

		}


		SafeRelease(pFrameDescription);
	}

	SafeRelease(pColorFrame);
}


/// <summary>
/// Updates body data
/// </summary>
/// <param name="ppBodies">pointer to the body data storage</param>
/// <returns>indicates success or failure</returns>
HRESULT KinectFaceTracker::UpdateBodyData(IBody** ppBodies)
{
	HRESULT hr = E_FAIL;

	if (iBodyFrameReader != nullptr)
	{
		IBodyFrame* pBodyFrame = nullptr;
		hr = iBodyFrameReader->AcquireLatestFrame(&pBodyFrame);
		if (SUCCEEDED(hr))
		{
			hr = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, ppBodies);
		}
		SafeRelease(pBodyFrame);
	}

	return hr;
}


float VectorLengthSquared(CameraSpacePoint point)
{
	float lenghtSquared = pow(point.X, 2) + pow(point.Y, 2) + pow(point.Z, 2);

	//result = Math.Sqrt(result);
	return lenghtSquared;
}

//
// Finds the closest body from the sensor if any
//
IBody* KinectFaceTracker::FindClosestBody(IBody** aBodies)
{
	IBody* result = nullptr;
	float closestBodyDistance = std::numeric_limits<float>::max();

	for(int i=0;i<BODY_COUNT;i++)
	{
		BOOLEAN tracked;
		aBodies[i]->get_IsTracked(&tracked);

		if (tracked)
		{
			Joint joints[JointType_Count];
			HRESULT hr = aBodies[i]->GetJoints(JointType_Count,joints);
			if (FAILED(hr))
			{
				continue;
			}

			auto currentLocation = joints[JointType_SpineBase].Position;
			auto currentDistance = VectorLengthSquared(currentLocation);

			if (result == nullptr || currentDistance < closestBodyDistance)
			{
				result = aBodies[i];
				closestBodyDistance = currentDistance;
			}
		}
	}

	return result;
}

//
// Search our list of body for the one matching our id
//
IBody* KinectFaceTracker::FindTrackedBodyById(IBody** aBodies, UINT64 aTrackingId)
{
	float closestBodyDistance = std::numeric_limits<float>::max();
	(void)closestBodyDistance;

	for (int i = 0; i < BODY_COUNT; i++)
	{
		BOOLEAN tracked;
		HRESULT hr = aBodies[i]->get_IsTracked(&tracked);

		if (tracked)
		{
			if (SUCCEEDED(hr) && tracked)
			{
				UINT64 trackingId = 0;
				hr = aBodies[i]->get_TrackingId(&trackingId);

				if (SUCCEEDED(hr) && aTrackingId == trackingId)
				{
					return aBodies[i];
				}
			}
		}
	}

	return nullptr;
}


/// <summary>
/// Processes new face frames
/// </summary>
void KinectFaceTracker::ProcessFaces()
{
	HRESULT hr=0;
	IBody* bodies[BODY_COUNT] = { 0 }; // Each bodies will need to be released
	bool bHaveBodyData = SUCCEEDED(UpdateBodyData(bodies));
	if (!bHaveBodyData)
	{
		return;
	}

	// Try keep tracking the same body
	IBody* body = FindTrackedBodyById(bodies, iTrackingId);
	if (body == nullptr)
	{
		// The body we were tracking is gone, try tracking the closest body if any
		body = FindClosestBody(bodies);
		if (body != nullptr)
		{
			// Update our face source with our new body id
			hr = body->get_TrackingId(&iTrackingId);
			if (SUCCEEDED(hr))
			{
				// Tell our face source to use the given body id
				hr = iFaceFrameSource->put_TrackingId(iTrackingId);
				//OutputDebugStringA("Tracking new body!\n");
			}
		}
	}

	// retrieve the latest face frame from this reader
	IHighDefinitionFaceFrame* pFaceFrame = nullptr;
	if (SUCCEEDED(hr))
	{
		hr = iFaceFrameReader->AcquireLatestFrame(&pFaceFrame);
	}

	BOOLEAN bFaceTracked = false;
	if (SUCCEEDED(hr) && nullptr != pFaceFrame)
	{
		// check if a valid face is tracked in this face frame
		hr = pFaceFrame->get_IsTrackingIdValid(&bFaceTracked);
	}

	if (SUCCEEDED(hr))
	{
		if (bFaceTracked)
		{
			//OutputDebugStringA("Tracking face!\n");

			//IFaceFrameResult* pFaceFrameResult = nullptr;
			IFaceAlignment* pFaceAlignment = nullptr;
			CreateFaceAlignment(&pFaceAlignment); // TODO: check return?
			//D2D1_POINT_2F faceTextLayout;

			//hr = pFaceFrame->get_FaceFrameResult(&pFaceFrameResult);

			hr = pFaceFrame->GetAndRefreshFaceAlignmentResult(pFaceAlignment);

			// need to verify if pFaceFrameResult contains data before trying to access it
			if (SUCCEEDED(hr) && pFaceAlignment != nullptr)
			{
				hr = pFaceAlignment->get_FaceBoundingBox(&iFaceBox);
				//pFaceFrameResult->get_FaceBoundingBoxInColorSpace();

				if (SUCCEEDED(hr))
				{
					//hr = pFaceFrameResult->GetFacePointsInColorSpace(FacePointType::FacePointType_Count, facePoints);
					hr = pFaceAlignment->get_HeadPivotPoint(&iFacePosition);
				}

				if (SUCCEEDED(hr))
				{
					//hr = pFaceFrameResult->get_FaceRotationQuaternion(&faceRotation);
					hr = pFaceAlignment->get_FaceOrientation(&iFaceRotationQuaternion);
				}

				if (SUCCEEDED(hr))
				{
					//hr = pFaceFrameResult->GetFaceProperties(FaceProperty::FaceProperty_Count, faceProperties);
				}

				if (SUCCEEDED(hr))
				{
					//hr = GetFaceTextPositionInColorSpace(ppBodies[0], &faceTextLayout);
				}

				if (SUCCEEDED(hr))
				{
					// draw face frame results
					//m_pDrawDataStreams->DrawFaceFrameResults(0, &faceBox, facePoints, &faceRotation, faceProperties, &faceTextLayout);
				}
			}

			SafeRelease(pFaceAlignment);
		}

		SafeRelease(pFaceFrame);
	}

	if (bHaveBodyData)
	{
		for (int i = 0; i < _countof(bodies); ++i)
		{
			SafeRelease(bodies[i]);
		}
	}
}
