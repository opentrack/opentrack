<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru_RU">
<context>
    <name>Form</name>
    <message>
        <source>Tracker settings</source>
        <translation>Настройки трекера</translation>
    </message>
    <message>
        <source>Diagonal FOV</source>
        <translation>Угол обзора</translation>
    </message>
    <message>
        <source>Camera settings</source>
        <translation>Настройки камеры</translation>
    </message>
    <message>
        <source>Frames per second</source>
        <translation>Кадры в секунду</translation>
    </message>
    <message>
        <source>Camera name</source>
        <translation>Камера</translation>
    </message>
    <message>
        <source>Camera Configuration</source>
        <translation>Конфигурация камеры</translation>
    </message>
    <message>
        <source>Head Center Offset</source>
        <translation>Смещение центра головы</translation>
    </message>
    <message>
        <source> mm</source>
        <translation> мм</translation>
    </message>
    <message>
        <source>Use only yaw and pitch while calibrating.
Don&apos;t roll or change position.</source>
        <translation>Поворачивайте голову влево-вправо и наклоняйте вверх-вниз.
Не наклоняйте набок и не смещайте голову в сторону.</translation>
    </message>
    <message>
        <source>Start calibration</source>
        <translation>Начать калибровку</translation>
    </message>
    <message>
        <source>Right</source>
        <translation>Вправо</translation>
    </message>
    <message>
        <source>Forward</source>
        <translation>Вперед</translation>
    </message>
    <message>
        <source>Up</source>
        <translation>Вверх</translation>
    </message>
    <message>
        <source>Show Network Input</source>
        <translation>Показать входные данные</translation>
    </message>
    <message>
        <source>MJPEG</source>
        <translation>Использовать MJPEG</translation>
    </message>
    <message>
        <source>Tuning / Debug</source>
        <translation>Тонкая настройка</translation>
    </message>
    <message>
        <source>ROI Smoothing Alpha</source>
        <translation>Сглаживание ROI</translation>
    </message>
    <message>
        <source>ROI Zoom</source>
        <translation>Масштабирование ROI</translation>
    </message>
    <message>
        <source>Thread Count</source>
        <translation>Количество потоков</translation>
    </message>
    <message>
        <source>Resolution</source>
        <translation>Разрешение</translation>
    </message>
    <message>
        <source>Field of view. Needed to transform the pose to world coordinates.</source>
        <translation>Угол обзора камеры. Требуется для преобразования положения головы в глобальные координаты</translation>
    </message>
    <message>
        <source>Requested video frame rate. Actual setting may not be supported by the camera.</source>
        <translation>Частота кадров. Реальные значения могут не поддерживаться камерой.</translation>
    </message>
    <message>
        <source>The requested resolution for cases where the camera delivers maximum frame rate only for a particular resolution. The image may still be downscaled to the internal resolution.</source>
        <translation>Разрешение камеры, для тех случаев, когда быстродействие камеры максимально в определенном разрешении. Может быть масштабировано до внутреннего разрешения.</translation>
    </message>
    <message>
        <source>Number of threads. Can be used to balance the CPU load between the game and the tracker.</source>
        <translation>Количество потоков. Используется для балансировки нагрузки на процессор между игрой и трекером.</translation>
    </message>
    <message>
        <source>Show the image patch that the pose estimation model sees.</source>
        <translation>Показать изображение, используемое моделью определения позиции</translation>
    </message>
    <message>
        <source>Amount of smoothing of the face region coordinates. Can help stabilize the pose.</source>
        <translation>Сглаживание координат области лица. Может помочь стабилизировать позицию.</translation>
    </message>
    <message>
        <source>Zoom factor for the face region. Applied before the patch is fed into the pose estimation model. There is a sweet spot near 1.</source>
        <translation>Фактор масштабирования области лица. Применяется перед передачей кадра в модель определения позиции. Наилучшие результаты близки к 1</translation>
    </message>
    <message>
        <source>Select Pose Net ONNX</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;the pose net file&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select the pose network. Changes take affect on the next tracker start</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Device</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>neuralnet_tracker_ns::NeuralNetDialog</name>
    <message>
        <source>Default</source>
        <translation>По умолчанию</translation>
    </message>
    <message>
        <source>Tracker Offline</source>
        <translation>Трекер выключен</translation>
    </message>
    <message>
        <source>%1x%2 @ %3 FPS / Inference: %4 ms</source>
        <translation>%1x%2 @ %3 FPS; Время оценки: %4 мс</translation>
    </message>
    <message>
        <source>%1 yaw samples. Yaw more to %2 samples for stable calibration.</source>
        <translation>Сэмплов поворота: %1.
Поворачивайте голову в стороны до %2 сэмплов для стабильной калибрации.</translation>
    </message>
    <message>
        <source>%1 pitch samples. Pitch more to %2 samples for stable calibration.</source>
        <translation>Сэмплов наклона: %1.
Наклоняйте голову вниз/вверх до %2 сэмплов для стабильной калибрации.</translation>
    </message>
    <message>
        <source>%1 samples. Over %2, good!</source>
        <translation>%1 сэмплов. Более %2, достаточно.</translation>
    </message>
    <message>
        <source>Stop calibration</source>
        <translation>Остановить калибровку</translation>
    </message>
    <message>
        <source>Start calibration</source>
        <translation>Начать калибровку</translation>
    </message>
    <message>
        <source>Select Pose Net ONNX</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>ONNX Files (*.onnx)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
