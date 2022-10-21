/*
 * Copyright (c) 2015-2016, Intel Corporation
 *   Author: Xavier Hallade <xavier.hallade@intel.com>
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "ftnoir_tracker_rs_impl.h"

#ifdef _WIN32
#   include <windows.h>
#endif
#include <cstdlib>

#include <pxcsensemanager.h>
#if 0
#include <RealSense/Face/FaceModule.h>
#include <RealSense/Face/FaceConfiguration.h>
using PXCFaceData = Intel::RealSense::Face::FaceData;
using PXCFaceConfiguration = Intel::RealSense::Face::FaceConfiguration;
#else
#include <pxcfacemodule.h>
#include <pxcfaceconfiguration.h>
#endif

constexpr size_t kPreviewStreamWidth = 640;
constexpr size_t kPreviewStreamHeight = 480;

PXCSenseManager* g_senseManager = NULL;
PXCFaceData* g_faceData = NULL;
void* g_previewImage = NULL;

CRITICAL_SECTION g_criticalSection;

pxcStatus set_face_module_configuration(PXCFaceModule *faceModule) {
        pxcStatus retStatus = PXC_STATUS_NO_ERROR;
        PXCFaceConfiguration *faceConfig = NULL;

        if (faceModule == NULL) {
                return PXC_STATUS_HANDLE_INVALID;
        }

        faceConfig = faceModule->CreateActiveConfiguration();
        if (faceConfig == NULL) {
                return PXC_STATUS_HANDLE_INVALID;
        }

        faceConfig->pose.isEnabled = true;
        faceConfig->pose.maxTrackedFaces = 1;
        faceConfig->pose.smoothingLevel = PXCFaceConfiguration::SmoothingLevelType::SMOOTHING_DISABLED;
        faceConfig->strategy = PXCFaceConfiguration::STRATEGY_CLOSEST_TO_FARTHEST;
        faceConfig->detection.isEnabled = false;
        faceConfig->landmarks.isEnabled = false;
        faceConfig->DisableAllAlerts();
        faceConfig->SetTrackingMode(PXCFaceConfiguration::FACE_MODE_COLOR_PLUS_DEPTH);

        retStatus = faceConfig->ApplyChanges();

        faceConfig->Release();
        return retStatus;
}

pxcStatus retrieve_preview_from_frame() {
        pxcStatus retStatus = PXC_STATUS_NO_ERROR;
        PXCCapture::Sample* sample = NULL;
        PXCImage::ImageInfo info;
        PXCImage::ImageData depthData;

        if (g_senseManager == NULL) {
                return PXC_STATUS_HANDLE_INVALID;
        }

        sample = g_senseManager->QuerySample();
        if (sample == NULL || sample->depth == NULL) {
                return PXC_STATUS_DATA_UNAVAILABLE;
        }

        info = sample->depth->QueryInfo();
        if (info.width > kPreviewStreamWidth || info.height > kPreviewStreamHeight) {
                return PXC_STATUS_PARAM_UNSUPPORTED;
        }

        retStatus = sample->depth->AcquireAccess(PXCImage::Access::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &depthData);
        if (retStatus == PXC_STATUS_NO_ERROR) {
                EnterCriticalSection(&g_criticalSection);

                for (int i = 0; i < info.height; ++i)
                        for (int j = 0; j < info.width; ++j)
                                ((unsigned char*)g_previewImage)[i*info.width + j] = ((unsigned char*)depthData.planes[0])[(i + 1) * 4 * info.width - 4 * (j + 1)]; //mirror and convert from RGB32 to Y8.

                LeaveCriticalSection(&g_criticalSection);

                sample->depth->ReleaseAccess(&depthData);
        }

        return retStatus;
}

int rs_tracker_impl_start() {
        pxcStatus retStatus = PXC_STATUS_NO_ERROR;
        PXCFaceModule *faceModule = NULL;

        InitializeCriticalSection(&g_criticalSection);
        g_previewImage = malloc(kPreviewStreamWidth*kPreviewStreamHeight);
        memset(g_previewImage, 0, kPreviewStreamWidth*kPreviewStreamHeight);

        g_senseManager = PXCSenseManager::CreateInstance();
        if (g_senseManager == NULL) {
                rs_tracker_impl_end();
                return PXC_STATUS_HANDLE_INVALID;
        }

        retStatus = g_senseManager->EnableFace();
        if (retStatus != PXC_STATUS_NO_ERROR) {
                rs_tracker_impl_end();
                return retStatus;
        }

        faceModule = g_senseManager->QueryFace();
        if (faceModule == NULL) {
                rs_tracker_impl_end();
                return PXC_STATUS_HANDLE_INVALID;
        }

        retStatus = set_face_module_configuration(faceModule);
        if (retStatus != PXC_STATUS_NO_ERROR) {
                rs_tracker_impl_end();
                return retStatus;
        }

        retStatus = g_senseManager->Init();
        if (retStatus != PXC_STATUS_NO_ERROR) {
                rs_tracker_impl_end();
                return retStatus;
        }

        g_faceData = faceModule->CreateOutput();
        if (g_faceData == NULL) {
                rs_tracker_impl_end();
                return PXC_STATUS_HANDLE_INVALID;
        }

        return retStatus;
}

int rs_tracker_impl_get_preview(void* previewImageOut, int width, int height) {
        if (width != kPreviewStreamWidth || height != kPreviewStreamHeight)
                return PXC_STATUS_PARAM_UNSUPPORTED;

        if (g_previewImage == NULL)
                return PXC_STATUS_DATA_UNAVAILABLE;

        EnterCriticalSection(&g_criticalSection);
        if (g_previewImage != NULL)
                memcpy(previewImageOut, g_previewImage, kPreviewStreamWidth*kPreviewStreamHeight);
        LeaveCriticalSection(&g_criticalSection);

        return PXC_STATUS_NO_ERROR;
}

int rs_tracker_impl_update_pose(double *data) {//TODO: add bool preview activated.
        pxcStatus retStatus = PXC_STATUS_NO_ERROR;
        PXCFaceData::PoseEulerAngles angles;
        PXCFaceData::HeadPosition headPosition;
        PXCFaceData::Face *face = NULL;
        PXCFaceData::PoseData *pose = NULL;
        bool poseAnglesAvailable = false;
        bool headPositionAvailable = false;

        if (g_senseManager != NULL && g_faceData != NULL && g_previewImage != NULL
                && (retStatus = g_senseManager->AcquireFrame(true, 30)) == PXC_STATUS_NO_ERROR) {

                retrieve_preview_from_frame();

                retStatus = g_faceData->Update();
                if (retStatus != PXC_STATUS_NO_ERROR) {
                        rs_tracker_impl_end();
                        return retStatus;
                }

                pxcI32 numberOfDetectedFaces = g_faceData->QueryNumberOfDetectedFaces();
                for (int i = 0; i < numberOfDetectedFaces; ++i) {
                        face = g_faceData->QueryFaceByIndex(i);
                        if (face == NULL) continue;

                        pose = face->QueryPose();
                        if (pose == NULL) continue;

                        poseAnglesAvailable = !!pose->QueryPoseAngles(&angles);
                        if (!poseAnglesAvailable) continue;

                        headPositionAvailable = !!pose->QueryHeadPosition(&headPosition);
                        if (!headPositionAvailable) continue;

                        //TODO: use pxcI32 pose->QueryConfidence() ?

                        //x, y, z: cm
                        data[0] = headPosition.headCenter.x / 10.;
                        data[1] = headPosition.headCenter.y / 10.;
                        data[2] = headPosition.headCenter.z / 10.;

                        //yaw, pitch, roll: degrees
                        data[3] = -angles.yaw;
                        data[4] = angles.pitch;
                        data[5] = angles.roll;

                        break;
                }

                g_senseManager->ReleaseFrame();
        }

        return retStatus;
}

int rs_tracker_impl_end() {
        if (g_faceData != NULL) {
                g_faceData->Release();
                g_faceData = NULL;
        }

        if (g_senseManager != NULL) {
                g_senseManager->Release();
                g_senseManager = NULL;
        }

        EnterCriticalSection(&g_criticalSection);
        free(g_previewImage);
        g_previewImage = NULL;
        DeleteCriticalSection(&g_criticalSection);

        return PXC_STATUS_NO_ERROR;
}
