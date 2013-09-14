#ifndef SM_API_TESTAPPCONSOLE_UTILS_H
#define SM_API_TESTAPPCONSOLE_UTILS_H

#include "lock.h"
#include <exception>
#include <iostream>

#define THROW_ON_ERROR(x) \
{ \
    smReturnCode result = (x); \
    if (result < 0) \
    { \
        std::stringstream s; \
        s << "API error code: " << result; \
        std::cerr << s; \
        throw std::exception(); \
    } \
}

namespace sm
{
    namespace faceapi
    {
        namespace samplecode
        {
            // Global variables
            Mutex g_mutex;
            bool g_ctrl_c_detected(false);
            bool g_do_head_pose_printing(false);
            bool g_do_face_data_printing(false);
            unsigned short g_overlay_flags(SM_API_VIDEO_DISPLAY_HEAD_MESH | SM_API_VIDEO_DISPLAY_PERFORMANCE);

            // CTRL-C handler function
            void __cdecl CtrlCHandler(int)
            {
                Lock lock(g_mutex);
                std::cout << "Ctrl-C detected, stopping..." << std::endl;
                g_ctrl_c_detected = true;
            }

            // Radians to degrees conversion
            float rad2deg(float rad)
            {
                return rad*57.2957795f;
            }

            void toggleFlag(unsigned short &val, unsigned short flag)
            {
                if (val & flag)
                {
                    val = val & ~flag;
                }
                else
                {
                    val = val | flag;
                }
            }

            // Save an image to PNG file
            smReturnCode saveToPNGFile(const std::string& filepath, smImageInfo image_info)
            {
                smBool ok;
                smReturnCode error;
                
                // Create an API string
                smStringHandle filepath_handle = 0;
                ok = (error = smStringCreate(&filepath_handle)) == SM_API_OK;
                ok = ok && (error = smStringReadBuffer(filepath_handle,filepath.c_str(),filepath.size())) == SM_API_OK;

                // Create an API image
                smImageHandle image_handle = 0;       
                smImageMemoryCopyMode copy_mode = SM_API_IMAGE_MEMORYCOPYMODE_AUTO;        
                ok = ok && (error = smImageCreateFromInfo(&image_info,&copy_mode,&image_handle)) == SM_API_OK;

                // Save the image as PNG
                ok = ok && (error = smImageSaveToPNG(image_handle,filepath_handle)) == SM_API_OK;

                // Destroy the image and string
                smStringDestroy(&filepath_handle);
                smImageDestroy(&image_handle);
                return error;
            }

            // Stream operators for printing

            std::ostream &operator<<(std::ostream & os, const smSize2i &s)
            {
                return os << "[" << s.h << "," << s.h << "]";
            }

            std::ostream &operator<<(std::ostream & os, const smCoord3f &pos)
            {
                return os << "(" << pos.x << "," << pos.y << "," << pos.z << ")";
            }

            std::ostream &operator<<(std::ostream & os, const smRotEuler &rot)
            {
                return os << "(" << rad2deg(rot.x_rads) << "," << rad2deg(rot.y_rads) << "," << rad2deg(rot.z_rads) << ")";
            }

            std::ostream &operator<<(std::ostream & os, const smPixel &p)
            {
                return os << "[" << static_cast<int>(p.x) << "," << static_cast<int>(p.y) << "]";
            }

            std::ostream &operator<<(std::ostream & os, const smFaceTexCoord &ftc)
            {
                return os << "{" << ftc.u << "," << ftc.v << "}";
            }

            std::ostream &operator<<(std::ostream & os, const smFaceLandmark &lm)
            {
                return os << "id "<< lm.id << " fc" << lm.fc << " ftc" << lm.ftc << " pc" << lm.pc << " wc" << lm.wc;
            }

            std::ostream &operator<<(std::ostream & os, const smImageInfo &im)
            {
                os << "format ";
                switch (im.format)
                {
                case SM_API_IMAGECODE_GRAY_8U:
                    os << "GRAY_8U";
                    break;
                case SM_API_IMAGECODE_GRAY_16U:
                    os << "GRAY_16U";
                    break;
                case SM_API_IMAGECODE_YUY2:
                    os << "YUY2";
                    break;
                case SM_API_IMAGECODE_I420:
                    os << "I420";
                    break;
                case SM_API_IMAGECODE_BGRA_32U:
                    os << "BGRA_32U";
                    break;
                case SM_API_IMAGECODE_ARGB_32U:
                    os << "ARGB_32U";
                    break;
                case SM_API_IMAGECODE_BGR_24U:
                    os << "BGR_24U";
                    break;
                case SM_API_IMAGECODE_RGB_24U:
                    os << "RGB_24U";
                    break;
                default:
                    os << "unknown";
                    break;
                }
                os << " res" << im.res;
                os << " plane_addr(" << static_cast<void *>(im.plane_addr[0]) << "," 
                                     << static_cast<void *>(im.plane_addr[1]) << "," 
                                     << static_cast<void *>(im.plane_addr[2]) << "," 
                                     << static_cast<void *>(im.plane_addr[3]) << ")";
                os << " step_bytes(" << im.step_bytes[0] << "," << im.step_bytes[1] << "," << im.step_bytes[2] << "," << im.step_bytes[3] << ")";
                os << " user_data " << im.user_data;
                return os;
            }

            std::ostream &operator<<(std::ostream & os, const smFaceTexture &t)
            {
                os << "type ";
                switch (t.type)
                {
                    case SM_ORTHOGRAPHIC_PROJECTION:
                        os << "orthographic";
                        break;
                    default:
                        os << "unknown"; 
                        break;
                }
                os << " origin" << t.origin << " scale" << t.scale << std::endl;
                os << " image_info " << t.image_info << std::endl;
                os << " num_mask_landmarks " << t.num_mask_landmarks << std::endl;
                for (int i=0; i<t.num_mask_landmarks; i++)
                {
                    os << "  " << t.mask_landmarks[i] << std::endl;
                }
                return os;
            }

            // Stream operator for printing face landmarks
            std::ostream &operator<<(std::ostream &os, const smEngineFaceData &face_data)
            {
                fixed(os);
                showpos(os);
                os.precision(2);
                os << "Face Data: " 
                   << "origin_wc" << face_data.origin_wc << " "
                   << "num_landmarks " << face_data.num_landmarks 
                   << std::endl;
                for (int i=0; i<face_data.num_landmarks; i++)
                {
                    os << "  " << face_data.landmarks[i] << std::endl;
                }
                // Print any face texture info
                if (face_data.texture)
                {
                    os << "Face Texture: " << *face_data.texture;
                }
                return os;
            }

            // Stream operator for printing head-pose data
            std::ostream &operator<<(std::ostream & os, const smEngineHeadPoseData &head_pose)
            {
                fixed(os);
                showpos(os);
                os.precision(2);
                return os << "Head Pose: " 
                          << "head_pos" << head_pose.head_pos << " " 
                          << "head_rot" << head_pose.head_rot << " "
                          << "left_eye_pos" << head_pose.left_eye_pos << " "
                          << "right_eye_pos" << head_pose.right_eye_pos << " " 
                          << "confidence " << head_pose.confidence;
            }

            std::ostream &operator<<(std::ostream & os, const smCameraVideoFrame &vf)
            {
                fixed(os);
                showpos(os);
                os.precision(2);
                return os << "Framenum: " << vf.frame_num;
            }

            // Handles keyboard events: return false if quit.
            bool processKeyPress(smEngineHandle engine_handle, smVideoDisplayHandle video_display_handle)
            {
                Lock lock(g_mutex);
                if (g_ctrl_c_detected)
                {
                    return false;
                }
                if (!_kbhit())
                {
                    return true;
                }
                int key = _getch();
                switch (key)
                {
                case 'q':
                    return false;
                case 'r':
                    {
                        // Manually restart the tracking
                        THROW_ON_ERROR(smEngineStart(engine_handle));
                        std::cout << "Restarting tracking" << std::endl; 
                    }
                    return true;
                case 'a':
                    {
                        // Toggle auto-restart mode
                        int on;
                        THROW_ON_ERROR(smHTGetAutoRestartMode(engine_handle,&on));
                        THROW_ON_ERROR(smHTSetAutoRestartMode(engine_handle,!on));
                        std::cout << "Autorestart-mode is " << (on?"on":"off") << std::endl; 
                    }
                    return true;
                case '1':
                    toggleFlag(g_overlay_flags,SM_API_VIDEO_DISPLAY_REFERENCE_FRAME);
                    THROW_ON_ERROR(smVideoDisplaySetFlags(video_display_handle,g_overlay_flags));
                    return true;
                case '2':
                    toggleFlag(g_overlay_flags,SM_API_VIDEO_DISPLAY_PERFORMANCE);
                    THROW_ON_ERROR(smVideoDisplaySetFlags(video_display_handle,g_overlay_flags));
                    return true;
                case '3':
                    toggleFlag(g_overlay_flags,SM_API_VIDEO_DISPLAY_HEAD_MESH);
                    THROW_ON_ERROR(smVideoDisplaySetFlags(video_display_handle,g_overlay_flags));
                    return true;
                case '4':
                    toggleFlag(g_overlay_flags,SM_API_VIDEO_DISPLAY_LANDMARKS);
                    THROW_ON_ERROR(smVideoDisplaySetFlags(video_display_handle,g_overlay_flags));
                    return true;
                case 'l':
                    if (smAPINonCommercialLicense() == SM_API_TRUE)
                    {
                        return false;
                    }
                    else
                    {
                        int on;
                        THROW_ON_ERROR(smHTGetLipTrackingEnabled(engine_handle,&on));
                        THROW_ON_ERROR(smHTSetLipTrackingEnabled(engine_handle,!on));
                    }
                    return true;
                case 'e':
                    if (smAPINonCommercialLicense() == SM_API_TRUE)
                    {
                        return false;
                    }
                    else
                    {
                        int on;
                        THROW_ON_ERROR(smHTGetEyebrowTrackingEnabled(engine_handle,&on));
                        THROW_ON_ERROR(smHTSetEyebrowTrackingEnabled(engine_handle,!on));
                    }
                    return true;
                case 'h':
                    if (smEngineIsLicensed(engine_handle) != SM_API_OK)
                    {
                        return false;
                    }
                    else
                    {
                        g_do_head_pose_printing = !g_do_head_pose_printing;
                        std::cout << "HeadPose printing is " << (g_do_head_pose_printing?"on":"off") << std::endl; 
                    }
                    return true;
                case 'f':
                    if (smEngineIsLicensed(engine_handle) != SM_API_OK)
                    {
                        return false;
                    }
                    else
                    {
                        g_do_face_data_printing = !g_do_face_data_printing;
                        std::cout << "FaceData printing is " << (g_do_face_data_printing?"on":"off") << std::endl; 
                    }
                    return true;
                default:
                    return true;
                }
            }

            // Setup console window geometry / font etc
            void initConsole()
            {
                HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
                // Buffer of 255 x 1024
                COORD buffer_size;
                buffer_size.X = 255;
                buffer_size.Y = 1024;
                SetConsoleScreenBufferSize(console_handle, buffer_size);
                // Window size of 120 x 50
                SMALL_RECT window_size;
                window_size.Left = 0;
                window_size.Right = 120;
                window_size.Top = 0;
                window_size.Bottom = 50;
                SetConsoleWindowInfo(console_handle,TRUE,&window_size);
                // Green text
                SetConsoleTextAttribute(console_handle, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
//				ShowWindow(GetConsoleWindow(), SW_HIDE);
            }
        }
    }
}

#endif
