#pragma once

#include "options/options.hpp"
#include <opencv2/calib3d.hpp>

#include <QString>


namespace EasyTracker {

    using namespace options;

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wweak-vtables"
#endif

    struct Settings final : options::opts
    {
        using slider_value = options::slider_value;

        value<QString> camera_name{ b, "camera-name", "" };
        value<int> cam_res_x{ b, "camera-res-width", 640 },
            cam_res_y{ b, "camera-res-height", 480 },
            cam_fps{ b, "camera-fps", 30 };
        value<int> iMinBlobSize{ b, "iMinBlobSize", 4 }, iMaxBlobSize{ b, "iMaxBlobSize", 15 };
        value<int> DeadzoneRectHalfEdgeSize { b, "deadzone-rect-half-edge-size", 1 };

        // Type of custom model
        value<bool> iCustomModelThree{ b, "iCustomModelThree", true };
        value<bool> iCustomModelFour{ b, "iCustomModelFour", false };
        value<bool> iCustomModelFive{ b, "iCustomModelFive", false };
        value<bool> iClipModelThree{ b, "iClipModelThree", false };

        // Custom model vertices
        value<int> iVertexTopX{ b, "iVertexTopX", 0 }, iVertexTopY{ b, "iVertexTopY", 0 }, iVertexTopZ{ b, "iVertexTopZ", 0 };
        value<int> iVertexRightX{ b, "iVertexRightX", 0 }, iVertexRightY{ b, "iVertexRightY", 0 }, iVertexRightZ{ b, "iVertexRightZ", 0 };
        value<int> iVertexLeftX{ b, "iVertexLeftX", 0 }, iVertexLeftY{ b, "iVertexLeftY", 0 }, iVertexLeftZ{ b, "iVertexLeftZ", 0 };
        value<int> iVertexCenterX{ b, "iVertexCenterX", 0 }, iVertexCenterY{ b, "iVertexCenterY", 0 }, iVertexCenterZ{ b, "iVertexCenterZ", 0 };
        value<int> iVertexTopRightX{ b, "iVertexTopRightX", 0 }, iVertexTopRightY{ b, "iVertexTopRightY", 0 }, iVertexTopRightZ{ b, "iVertexTopRightZ", 0 };
        value<int> iVertexTopLeftX{ b, "iVertexTopLeftX", 0 }, iVertexTopLeftY{ b, "iVertexTopLeftY", 0 }, iVertexTopLeftZ{ b, "iVertexTopLeftZ", 0 };
        // Clip model vertices
        value<int> iVertexClipTopX{ b, "iVertexClipTopX", 0 }, iVertexClipTopY{ b, "iVertexClipTopY", 0 }, iVertexClipTopZ{ b, "iVertexClipTopZ", 0 };
        value<int> iVertexClipMiddleX{ b, "iVertexClipMiddleX", 0 }, iVertexClipMiddleY{ b, "iVertexClipMiddleY", 0 }, iVertexClipMiddleZ{ b, "iVertexClipMiddleZ", 0 };
        value<int> iVertexClipBottomX{ b, "iVertexClipBottomX", 0 }, iVertexClipBottomY{ b, "iVertexClipBottomY", 0 }, iVertexClipBottomZ{ b, "iVertexClipBottomZ", 0 };


        value<int> fov{ b, "camera-fov", 56 };

        value<bool> debug{ b, "debug", false };
        value<bool> iAutoCenter{ b, "iAutoCenter", true };
        value<int> iAutoCenterTimeout{ b, "iAutoCenterTimeout", 1000 };


        value<int> PnpSolver{ b, "pnp-solver", cv::SOLVEPNP_P3P };


        explicit Settings(const QString& name) : opts(name) {}
    };

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

} //
