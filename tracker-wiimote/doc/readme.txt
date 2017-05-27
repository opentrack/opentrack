The following setup is required:

Bluez 4.101 required, 5.0 recommended.
Use bluetoothctl's 'pair', 'trust' and 'connect' commands to ensure that your controller auto-connects 
if a button is pressed and disconnects if power button is pressed longer. 


To acces the device as non-root:

Create a group 'player' and add your normal user to that group

Create a file /etc/udev/rules.d/99-input.rules and insert a line:

ATTRS{name}=="Nintendo Wii Remote IR", SYMLINK+="input/wii_ir", GROUP="player"

This will create a symbolic link and allow read access for users in group 'player'.


For opentracks 'libevdev' protocol try this additional line in your .rules file:

KERNEL=="uinput", SYMLINK+="input/uinput", GROUP="player"

hid-wiimote and uinput have to be unloaded and reloaded (rmmod, modprobe) to apply the new rules.

Ensure that 'hid-wiimote' and 'uinput' are loaded automatically. 