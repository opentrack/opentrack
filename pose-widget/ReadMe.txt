Hi everyone!

In the extreme version of opentrack-2.3.12, the Octopus pose is still displayed incorrectly.
The pose-widget is responsible for displaying the Octopus pose in opentrack. I have fixed this widget.

Fixed bugs:
- The turns and movements of the Octopussy are now performed truly independently of each other, as it should be in 6DOF.
- When cornering, there is no "gimbal lock" at Pitch = +/- 90 degrees.
- Fixed directions of axes of rotations and positions. Now the Octopus pose displays the actual direction of view on the plane.
- Fixed display of the back (green) surface of the Octopus. Previously, it was displayed mirrored.

Additional features:
- Applied "perspective projection", the picture becomes more voluminous.
- Added lighting effect from above, with the same purpose.
- Added background fill for the widget. This makes it possible to see the borders of the widget.
- Added X and Y axes. This helps to estimate how far the Octopus is deviated from the center.
- Added [Mirror] checkbox, mirroring positions and rotations. It is often more convenient to observe the Octopussy's mirror pose.
- If before compilation in the file "pose-widget.hpp" include line 19: "#define TEST", then a rectangular frame will be drawn around the Octopus. This is useful when testing a pose-widget to assess distortion and size.

The corrected pose-widget now displays the Octopus pose correctly and can be used to check opentrack settings, sometimes even without launching the flight simulator.

A video of the corrected pose-widget is available here:

https://youtu.be/my4_VOwGmq4

fixed by GO63-samara <go1@list.ru> <github.com/GO63-samara> 2020
