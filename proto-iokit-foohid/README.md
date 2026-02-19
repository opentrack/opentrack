# Virtual Joystick Support for macOS

Adds support for a virtual joystick output, based on the foohid driver for virtual HID devices.

## Usage

Install the latest [foohid HID driver](https://github.com/unbit/foohid/releases/).

Select "Virtual Joystick" as the output in opentrack.

The X, Y, Z, Yaw, Pitch, and Roll tracking axes are mapped to the joystick's X, Y, Z, RX, RY,
and RZ axes respectively.

# Building

The implementation only uses standard libraries that are pre-installed on macOS. There are no
special requirements for building.
