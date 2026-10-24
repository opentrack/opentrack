<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<!-- Copyright (c) 2026 Jonatan Nyberg
     SPDX-License-Identifier: ISC -->
<TS version="2.1" language="sv">
    <context>
        <name>Form</name>
        <message>
            <source>Tracker settings</source>
            <translation>Spårarinställningar</translation>
        </message>
        <message>
            <source>Diagonal FOV</source>
            <translation>Diagonalt synfält</translation>
        </message>
        <message>
            <source>Camera settings</source>
            <translation>Kamerainställningar</translation>
        </message>
        <message>
            <source>Frames per second</source>
            <translation>Bildrutor per sekund</translation>
        </message>
        <message>
            <source>Camera name</source>
            <translation>Kameranamn</translation>
        </message>
        <message>
            <source>Camera Configuration</source>
            <translation>Kamerakonfiguration</translation>
        </message>
        <message>
            <source>Head Center Offset</source>
            <translation>Förskjutning för huvudets centrum</translation>
        </message>
        <message>
            <source> mm</source>
            <translation> mm</translation>
        </message>
        <message>
            <source>Use only yaw and pitch while calibrating.
Don't roll or change position.</source>
            <translation>Använd endast girning och tippning vid kalibrering.
Undvik rollning och ändra inte position.</translation>
        </message>
        <message>
            <source>Start calibration</source>
            <translation>Starta kalibrering</translation>
        </message>
        <message>
            <source>Right</source>
            <translation>Höger</translation>
        </message>
        <message>
            <source>Forward</source>
            <translation>Framåt</translation>
        </message>
        <message>
            <source>Up</source>
            <translation>Uppåt</translation>
        </message>
        <message>
            <source>Face Crop Options:</source>
            <translation>Alternativ för ansiktsutsnitt:</translation>
        </message>
        <message>
            <source>MJPEG</source>
            <translation>MJPEG</translation>
        </message>
        <message>
            <source>Tuning / Debug</source>
            <translation>Finjustering / felsökning</translation>
        </message>
        <message>
            <source>Thread Count</source>
            <translation>Antal trådar</translation>
        </message>
        <message>
            <source>Resolution</source>
            <translation>Upplösning</translation>
        </message>
        <message>
            <source>Field of view. Needed to transform the pose to world coordinates.</source>
            <translation>Synfält. Behövs för att omvandla position och orientering till världskoordinater.</translation>
        </message>
        <message>
            <source>Requested video frame rate. Actual setting may not be supported by the camera.</source>
            <translation>Önskad bildfrekvens för video. Inställningen kanske inte stöds av kameran.</translation>
        </message>
        <message>
            <source>The requested resolution for cases where the camera delivers maximum frame rate only for a particular resolution. The image may still be downscaled to the internal resolution.</source>
            <translation>Önskad upplösning för kameror som bara ger maximal bildfrekvens vid en viss upplösning. Bilden kan ändå skalas ned till den interna upplösningen.</translation>
        </message>
        <message>
            <source>Number of threads. Can be used to balance the CPU load between the game and the tracker.</source>
            <translation>Antal trådar. Kan användas för att balansera processorbelastningen mellan spelet och spåraren.</translation>
        </message>
        <message>
            <source>Show the image patch that the pose estimation model sees.</source>
            <translation>Visa det bildutsnitt som modellen för lägesberäkning använder.</translation>
        </message>
        <message>
            <source>Amount of smoothing of the face region coordinates. Can help stabilize the pose.</source>
            <translation>Hur mycket ansiktsområdets koordinater jämnas ut. Kan göra det beräknade läget stabilare.</translation>
        </message>
        <message>
            <source>Internal Noise Filter</source>
            <translation>Internt brusfilter</translation>
        </message>
        <message>
            <source>Zoom factor for the face region. Applied before the patch is fed into the pose estimation model. There is a sweet spot near 1.</source>
            <translation>Zoomfaktor för ansiktsområdet. Tillämpas innan utsnittet matas in i modellen för lägesberäkning. Ett värde nära 1 brukar fungera bäst.</translation>
        </message>
        <message>
            <source>Smoothing Alpha</source>
            <translation>Alfavärde för utjämning</translation>
        </message>
        <message>
            <source>Show</source>
            <translation>Visa</translation>
        </message>
        <message>
            <source>Zoom</source>
            <translation>Zoom</translation>
        </message>
        <message>
            <source>Select Pose Net ONNX</source>
            <translation>Välj ONNX-nätverk för lägesberäkning</translation>
        </message>
        <message>
            <source>&lt;the pose net file&gt;</source>
            <translation>&lt;filen för lägesnätverket&gt;</translation>
        </message>
        <message>
            <source>Select the pose network. Changes take affect on the next tracker start</source>
            <translation>Välj nätverket för lägesberäkning. Ändringarna gäller nästa gång spåraren startas</translation>
        </message>
    </context>
    <context>
        <name>neuralnet_tracker_ns::NeuralNetDialog</name>
        <message>
            <source>Default</source>
            <translation>Standard</translation>
        </message>
        <message>
            <source>Tracker Offline</source>
            <translation>Spåraren är frånkopplad</translation>
        </message>
        <message>
            <source>%1x%2 @ %3 FPS / Inference: %4 ms</source>
            <translation>%1x%2 @ %3 FPS / Inferens: %4 ms</translation>
        </message>
        <message>
            <source>%1 yaw samples. Yaw more to %2 samples for stable calibration.</source>
            <translation>%1 girningsmätningar. Vrid huvudet mer i sidled tills du når %2 mätningar för stabil kalibrering.</translation>
        </message>
        <message>
            <source>%1 pitch samples. Pitch more to %2 samples for stable calibration.</source>
            <translation>%1 tippningsmätningar. Tippa huvudet mer uppåt och nedåt tills du når %2 mätningar för stabil kalibrering.</translation>
        </message>
        <message>
            <source>%1 samples. Over %2, good!</source>
            <translation>%1 mätningar. Fler än %2, bra!</translation>
        </message>
        <message>
            <source>Stop calibration</source>
            <translation>Stoppa kalibrering</translation>
        </message>
        <message>
            <source>Start calibration</source>
            <translation>Starta kalibrering</translation>
        </message>
        <message>
            <source>Select Pose Net ONNX</source>
            <translation>Välj ONNX-nätverk för lägesberäkning</translation>
        </message>
        <message>
            <source>ONNX Files (*.onnx)</source>
            <translation>ONNX-filer (*.onnx)</translation>
        </message>
    </context>
</TS>
