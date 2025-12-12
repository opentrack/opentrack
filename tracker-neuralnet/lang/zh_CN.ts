<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>Form</name>
    <message>
        <source>Tracker settings</source>
        <translation>追踪器设置</translation>
    </message>
    <message>
        <source>Internal Noise Filter</source>
        <translation>内置噪声过滤器</translation>
    </message>
    <message>
        <source>Smoothing Alpha</source>
        <translation>平滑强度</translation>
    </message>
    <message>
        <source>Show</source>
        <translation>显示画面</translation>
    </message>
    <message>
        <source>Zoom</source>
        <translation>缩放比例</translation>
    </message>
    <message>
        <source>Diagonal FOV</source>
        <translation>对角线视场角</translation>
    </message>
    <message>
        <source>Camera name</source>
        <translation>摄像头名称</translation>
    </message>
    <message>
        <source>Frames per second</source>
        <translation>帧数/秒</translation>
    </message>
    <message>
        <source>Camera settings</source>
        <translation>摄像头设置</translation>
    </message>
    <message>
        <source>Camera Configuration</source>
        <translation>摄像头配置</translation>
    </message>
    <message>
        <source>Head Center Offset</source>
        <translation>头部中心偏移</translation>
    </message>
    <message>
        <source> mm</source>
        <translation> 毫米</translation>
    </message>
    <message>
        <source>Use only yaw and pitch while calibrating.
Don&apos;t roll or change position.</source>
        <translation>校准时只能转头 (使用偏航、俯仰、滚转) 。
身体不要移动位置 (禁用X轴、Y轴、Z轴) 。</translation>
    </message>
    <message>
        <source>Start calibration</source>
        <translation>开始校准</translation>
    </message>
    <message>
        <source>Right</source>
        <translation>向右</translation>
    </message>
    <message>
        <source>Forward</source>
        <translation>向前</translation>
    </message>
    <message>
        <source>Up</source>
        <translation>向上</translation>
    </message>
    <message>
        <source>MJPEG</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Tuning / Debug</source>
        <translation>调整/调试</translation>
    </message>
    <message>
        <source>Thread Count</source>
        <translation>线程数</translation>
    </message>
    <message>
        <source>Resolution</source>
        <translation>分辨率</translation>
    </message>
    <message>
        <source>Field of view. Needed to transform the pose to world coordinates.</source>
        <translation>视野范围，调整视场角确保能完整捕捉目标区域 (需要摄像头硬件支持) 。</translation>
    </message>
    <message>
        <source>Requested video frame rate. Actual setting may not be supported by the camera.</source>
        <translation>预设视频帧率 (实际效果取决于摄像头支持) 。</translation>
    </message>
    <message>
        <source>The requested resolution for cases where the camera delivers maximum frame rate only for a particular resolution. The image may still be downscaled to the internal resolution.</source>
        <translation>预设分辨率 (解决摄像头仅在特定分辨率下才能达到最高帧率的情况) ，如果设定的分辨率不支持，系统会自动适配摄像头支持的分辨率。</translation>
    </message>
    <message>
        <source>Number of threads. Can be used to balance the CPU load between the game and the tracker.</source>
        <translation>线程数，用于平衡游戏进程与跟踪器的CPU资源占用。</translation>
    </message>
    <message>
        <source>Face Crop Options:</source>
        <translation>人脸裁剪选项：</translation>
    </message>
    <message>
        <source>Show the image patch that the pose estimation model sees.</source>
        <translation>显示姿态网络模型识别到的原始图像。</translation>
    </message>
    <message>
        <source>Amount of smoothing of the face region coordinates. Can help stabilize the pose.</source>
        <translation>人脸区域坐标平滑度，数值越大姿态输出越稳定。</translation>
    </message>
    <message>
        <source>Zoom factor for the face region. Applied before the patch is fed into the pose estimation model. There is a sweet spot near 1.</source>
        <translation>人脸区域缩放比例 (预处理参数) ，数值接近1.0时效果最佳。</translation>
    </message>
    <message>
        <source>Select the pose network. Changes take affect on the next tracker start</source>
        <translation>选择姿态网络模型，更改将在下次启动跟踪器时生效</translation>
    </message>
    <message>
        <source>Select Pose Net ONNX</source>
        <translation>选择ONNX姿态网络模型</translation>
    </message>
    <message>
        <source>&lt;the pose net file&gt;</source>
        <translation>&lt;姿态网络模型文件&gt;</translation>
    </message>
</context>
<context>
    <name>neuralnet_tracker_ns::NeuralNetDialog</name>
    <message>
        <source>Default</source>
        <translation>默认</translation>
    </message>
    <message>
        <source>Tracker Offline</source>
        <translation>跟踪器离线</translation>
    </message>
    <message>
        <source>%1x%2 @ %3 FPS / Inference: %4 ms</source>
        <translation>分辨率 %1×%2｜帧率 %3 帧｜推理耗时 %4 毫秒</translation>
    </message>
    <message>
        <source>%1 yaw samples. Yaw more to %2 samples for stable calibration.</source>
        <translation>当前偏航样本：%1 个 (需达到 %2 个以完成稳定校准) 。</translation>
    </message>
    <message>
        <source>%1 pitch samples. Pitch more to %2 samples for stable calibration.</source>
        <translation>当前俯仰样本：%1 个 (需达到 %2 个以完成稳定校准) 。</translation>
    </message>
    <message>
        <source>%1 samples. Over %2, good!</source>
        <translation>已采集 %1 个样本（要求不少于 %2 个），状态良好！</translation>
    </message>
    <message>
        <source>Stop calibration</source>
        <translation>结束校准</translation>
    </message>
    <message>
        <source>Start calibration</source>
        <translation>开始校准</translation>
    </message>
    <message>
        <source>Select Pose Net ONNX</source>
        <translation>选择ONNX姿态网络模型</translation>
    </message>
    <message>
        <source>ONNX Files (*.onnx)</source>
        <translation>ONNX 文件 (*.onnx)</translation>
    </message>
</context>
</TS>
