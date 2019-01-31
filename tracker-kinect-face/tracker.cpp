

#include "tracker.h"


const double KinectFaceTracker::incr[6] =
{
	50, 40, 80,
	70, 5, 3
};

KinectFaceTracker::KinectFaceTracker():
	m_pKinectSensor(nullptr),
	m_pCoordinateMapper(nullptr),
	m_pColorFrameReader(nullptr),
	m_pColorRGBX(nullptr),
	m_pBodyFrameReader(nullptr)

{
	for (int i = 0; i < BODY_COUNT; i++)
	{
		m_pFaceFrameSources[i] = nullptr;
		m_pFaceFrameReaders[i] = nullptr;
	}

	// create heap storage for color pixel data in RGBX format
	m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];
}

KinectFaceTracker::~KinectFaceTracker()
{
	if (m_pColorRGBX)
	{
		delete[] m_pColorRGBX;
		m_pColorRGBX = nullptr;
	}

	// clean up Direct2D
	//SafeRelease(m_pD2DFactory);

	// done with face sources and readers
	for (int i = 0; i < BODY_COUNT; i++)
	{
		SafeRelease(m_pFaceFrameSources[i]);
		SafeRelease(m_pFaceFrameReaders[i]);
	}

	// done with body frame reader
	SafeRelease(m_pBodyFrameReader);

	// done with color frame reader
	SafeRelease(m_pColorFrameReader);

	// done with coordinate mapper
	SafeRelease(m_pCoordinateMapper);

	// close the Kinect Sensor
	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}

	SafeRelease(m_pKinectSensor);
}

module_status KinectFaceTracker::start_tracker(QFrame*)
{
	t.start();

	if (SUCCEEDED(InitializeDefaultSensor()))
	{
		return status_ok();
	}

	return error("Kinect init failed!");
}

#ifdef EMIT_NAN
#   include <cstdlib>
#endif

void KinectFaceTracker::data(double *data)
{
	const double dt = t.elapsed_seconds();
	t.start();

#ifdef EMIT_NAN
	if ((rand() % 4) == 0)
	{
		for (int i = 0; i < 6; i++)
			data[i] = 0. / 0.;
	}
	else
#endif
		for (int i = 0; i < 6; i++)
		{
			double x = last_x[i] + incr[i] * dt;
			if (x > 180)
				x = -360 + x;
			else if (x < -180)
				x = 360 + x;
			x = copysign(fmod(fabs(x), 360), x);
			last_x[i] = x;

			if (i >= 3)
			{
				data[i] = x;
			}
			else
			{
				data[i] = x * 100 / 180.;
			}
		}


	Update();
	//TODO: check if data is valid
	data[0] = 0;
	data[1] = 0;
	data[2] = 0;
	ExtractFaceRotationInDegrees(&faceRotation,&data[3], &data[4], &data[5]);

}


/// <summary>
/// Converts rotation quaternion to Euler angles 
/// And then maps them to a specified range of values to control the refresh rate
/// </summary>
/// <param name="pQuaternion">face rotation quaternion</param>
/// <param name="pPitch">rotation about the X-axis</param>
/// <param name="pYaw">rotation about the Y-axis</param>
/// <param name="pRoll">rotation about the Z-axis</param>
void KinectFaceTracker::ExtractFaceRotationInDegrees(const Vector4* pQuaternion, double* pYaw, double* pPitch, double* pRoll)
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

	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr))
	{
		return hr;
	}

	if (m_pKinectSensor)
	{
		// Initialize Kinect and get color, body and face readers
		IColorFrameSource* pColorFrameSource = nullptr;
		IBodyFrameSource* pBodyFrameSource = nullptr;

		hr = m_pKinectSensor->Open();

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		}

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
		}

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
		}

		if (SUCCEEDED(hr))
		{
			// create a face frame source + reader to track each body in the fov
			for (int i = 0; i < BODY_COUNT; i++)
			{
				if (SUCCEEDED(hr))
				{
					// create the face frame source by specifying the required face frame features
					hr = CreateFaceFrameSource(m_pKinectSensor, 0, c_FaceFrameFeatures, &m_pFaceFrameSources[i]);
				}
				if (SUCCEEDED(hr))
				{
					// open the corresponding reader
					hr = m_pFaceFrameSources[i]->OpenReader(&m_pFaceFrameReaders[i]);
				}
			}
		}

		SafeRelease(pColorFrameSource);
		SafeRelease(pBodyFrameSource);
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		//SetStatusMessage(L"No ready Kinect found!", 10000, true);
		return E_FAIL;
	}

	return hr;
}

/// <summary>
/// Main processing function
/// </summary>
void KinectFaceTracker::Update()
{
	if (!m_pColorFrameReader || !m_pBodyFrameReader)
	{
		return;
	}

	IColorFrame* pColorFrame = nullptr;
	HRESULT hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);

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
			if (imageFormat == ColorImageFormat_Bgra)
			{
				hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
			}
			else if (m_pColorRGBX)
			{
				pBuffer = m_pColorRGBX;
				nBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
			}
			else
			{
				hr = E_FAIL;
			}
		}

		if (SUCCEEDED(hr))
		{
			//DrawStreams(nTime, pBuffer, nWidth, nHeight);
			ProcessFaces();
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

	if (m_pBodyFrameReader != nullptr)
	{
		IBodyFrame* pBodyFrame = nullptr;
		hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);
		if (SUCCEEDED(hr))
		{
			hr = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, ppBodies);
		}
		SafeRelease(pBodyFrame);
	}

	return hr;
}



/// <summary>
/// Processes new face frames
/// </summary>
void KinectFaceTracker::ProcessFaces()
{
	HRESULT hr;
	IBody* ppBodies[BODY_COUNT] = { 0 };
	bool bHaveBodyData = SUCCEEDED(UpdateBodyData(ppBodies));

	// iterate through each face reader
	for (int iFace = 0; iFace < BODY_COUNT; ++iFace)
	{
		// retrieve the latest face frame from this reader
		IFaceFrame* pFaceFrame = nullptr;
		hr = m_pFaceFrameReaders[iFace]->AcquireLatestFrame(&pFaceFrame);

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
				IFaceFrameResult* pFaceFrameResult = nullptr;
				RectI faceBox = { 0 };
				PointF facePoints[FacePointType::FacePointType_Count];				
				DetectionResult faceProperties[FaceProperty::FaceProperty_Count];
				//D2D1_POINT_2F faceTextLayout;

				hr = pFaceFrame->get_FaceFrameResult(&pFaceFrameResult);

				// need to verify if pFaceFrameResult contains data before trying to access it
				if (SUCCEEDED(hr) && pFaceFrameResult != nullptr)
				{
					hr = pFaceFrameResult->get_FaceBoundingBoxInColorSpace(&faceBox);

					if (SUCCEEDED(hr))
					{
						hr = pFaceFrameResult->GetFacePointsInColorSpace(FacePointType::FacePointType_Count, facePoints);
					}

					if (SUCCEEDED(hr))
					{
						hr = pFaceFrameResult->get_FaceRotationQuaternion(&faceRotation);
					}

					if (SUCCEEDED(hr))
					{
						hr = pFaceFrameResult->GetFaceProperties(FaceProperty::FaceProperty_Count, faceProperties);
					}

					if (SUCCEEDED(hr))
					{
						//hr = GetFaceTextPositionInColorSpace(ppBodies[iFace], &faceTextLayout);
					}

					if (SUCCEEDED(hr))
					{
						// draw face frame results
						//m_pDrawDataStreams->DrawFaceFrameResults(iFace, &faceBox, facePoints, &faceRotation, faceProperties, &faceTextLayout);
					}
				}

				SafeRelease(pFaceFrameResult);
			}
			else
			{
				// face tracking is not valid - attempt to fix the issue
				// a valid body is required to perform this step
				if (bHaveBodyData)
				{
					// check if the corresponding body is tracked 
					// if this is true then update the face frame source to track this body
					IBody* pBody = ppBodies[iFace];
					if (pBody != nullptr)
					{
						BOOLEAN bTracked = false;
						hr = pBody->get_IsTracked(&bTracked);

						UINT64 bodyTId;
						if (SUCCEEDED(hr) && bTracked)
						{
							// get the tracking ID of this body
							hr = pBody->get_TrackingId(&bodyTId);
							if (SUCCEEDED(hr))
							{
								// update the face frame source with the tracking ID
								m_pFaceFrameSources[iFace]->put_TrackingId(bodyTId);
							}
						}
					}
				}
			}
		}

		SafeRelease(pFaceFrame);
	}

	if (bHaveBodyData)
	{
		for (int i = 0; i < _countof(ppBodies); ++i)
		{
			SafeRelease(ppBodies[i]);
		}
	}
}
