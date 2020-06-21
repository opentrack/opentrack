Add a new Hamilton filter.

Hamilton Filter Key Features:
- Instead of square, round (spherical) floating dead zones and smoothing areas are applied. Due to this, the angular size of these zones does not change when the Pitch angle changes. Diagonally rotations is as easy as moving along the Yaw and Pitch axes.
- Rotations are not filtered by independent coordinates, but comprehensively, in 3D space. Rotations and movements are more natural. There are no view jumps at the borders of +/- 180 degrees.
- The possibility of increasing the smoothing of rotations when zooming (when the head is approaching the monitor, that is, when increasing the -Z coordinate) is introduced. This makes it possible to more accurately aim and monitor remote targets.

A full description of the Hamilton filter is available in Russian here:
https://sites.google.com/site/diyheadtracking/home/opentrack/opentrack-hamilton-filter

The Hamilton filter was tested by the Russian community, received positive reviews:
https://forum.il2sturmovik.ru/topic/5061-opentrack-актуальная-информация-по-проекту-решение-проблем-вопросы/page/24/
https://forums.eagle.ru/showthread.php?t=23280&page=249
