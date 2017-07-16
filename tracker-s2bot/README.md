# S2Bot plugin for opentrack
This is a tracker providing 3D head tracking using the S2Bot helper application. 

S2Bot is originally a plugin that connect embedded devices with the [Scratch programming environment](https://scratch.mit.edu/). This plugin emulates the Scratch interface to get access to the sensor data from the various boards. It was developed and tested using the [micro:bit](https://www.microbit.co.uk) board, but should work with any of the S2Bot supported sensor boards that have accelerometers and/or magnetometers. S2Bot is available for Windows, Mac and Linux.

More information on S2Bot can be found [here](http://www.picaxe.com/Teaching/Other-Software/Scratch-Helper-Apps/)

# Installation and usage

1. Download & install S2Bot
2. Connect BL112 USB dongle
3. Connect micro:bit to computer via USB
4. Start S2Bot
5. Flash micro:bit with s2bot hex firmware from the S2Bot app
6. Select micro:bit from device selection dropdown
7. Press scan and select device (the device connection LED should go green after a few seconds, and accelerometer data should appear in the S2Bot status window)
8. Start opentrack
9. Select S2Bot plugin as input
10. Press start to start tracking (the Scratch connection LED will go green as S2Bot treats opentract as a Scratch environment)

# ISC License
Copyright (c) 2017, Attila Csipa

  Author: Attila Csipa <git@csipa.net>
  
Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

# Attribution

S2Bot and PICAXE® products are developed and distributed by Revolution Education Ltd 
BBC micro:bit is created by the British Broadcasting Corporation or BBC partners.
Scratch is created by the MIT Media Lab
