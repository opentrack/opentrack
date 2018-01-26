__________________________________________________________________

     - WiiYourself! - native C++ Wiimote library  v1.15
       (c) gl.tter 2007-10 - http://gl.tter.org
__________________________________________________________________

This marks the likely-final release of my free & fully-featured
 Wiimote native C++ library for Windows.

Originally based on Brian Peek's 'Managed Wiimote Library'
 (http://blogs.msdn.com/coding4fun/archive/2007/03/14/1879033.aspx)
 I then rewrote and extended it considerably.

There's no documentation - check Brian's article for a good
overview and general 'Wiimote with Windows' info - but the
source code has extensive comments, and the demo app should help
you make sense of it all.  Any questions, join my mailing list
(below).

Check License.txt for the (few) conditions of use, and
 History.txt for important changes from previous versions.
_____

Notes:

  - the library consists only of wiimote.cpp & wiimote.h.  Simply add
     them to your project and include the header.

  - VC 2005 & 2008 projects, and a MSYS makefile for MinGW for the
     demo program are included.
  
  -	for MSYS:
        at the MSYS prompt type:  make -f Makefile.MSYS
         (it will create a folder named MinGW whith the binaries
          and proper folder structure)
        
  - The Windows Driver Development Kit (WDK or DDK) is required to
     build (for the HID API).  It's a free download from MS page
     (no need to register).  Currently it's found here (or search
     for it on microsoft.com):
  
     'Windows Driver Kit (WDK) 7.1.0'
     http://www.microsoft.com/whdc/DevTools/WDK/WDKpkg.mspx

    Unfortunately it changes frequently so the instructions below
     may be wrong or incomplete.  Google or ask on my mailing
     list for help if needed:
     
        - add its 'inc' and 'inc/api' dirs to your include paths
         (make sure they are *above* other paths, ie. searched first)
        
        - add its 'lib/win7/i386' dir to your library paths.
     
    You can also use earlier versions known as the 'DDK'.  The parts
     of the HID API I use haven't changed in a long time.  These paths
     are usually used:
      
        - add 'inc/wxp' dir to your include paths (*above* others)
        - add 'lib/wxp/i386' to your library paths.
      
    Notes:
      - do _not_ add any 'STL' paths from the WDK/DDK as they can cause
         build problems.
      - the order of your include paths can be important.  If you placed
         them above the paths and are still getting compilation errors,
         try moving them around.
  
  - The library is Unicode-ready via <tchar.h> (see demo project).
  
  - if you're not using VC you need to link with these libraries:
        setupapi.lib
        winmm.lib
        hid.lib     (from the DDK)
__________________________  

Wiimote installation notes:
  
  The Wiimote needs to be 'paired' (Bluetooth connected) with the
   PC before you can install/use it.  Pressing 1 & 2 simultaneously
   puts it into 'discoverable' mode for a few seconds (LEDs will
   flash - the number of LEDs reflects the battery level).
   
  It will be detected as 'Nintendo RVL-CNT-01'.
     
  Stack-specific instructions:

  - Windows' built-in Bluetooth stack:

    1) open up the Bluetooth control panel.
    2) press _and hold_ 1 & 2 on the wiimote (LEDs flash) until the
       installation is complete (otherwise the wiimote usually times
       out half-way through the procedure, and although it may seem
       to have installed it's never 'connected' and doesn't work).
    3) add a new device - it should find it. don't use a password.
    4) when the installation is fully complete, let go of 1&2. The
       Bluetooth panel should now show it 'connected'.
    
    if something goes wrong you need to uninstall it and try again.
    
    if you un-pair the wiimote later (see below), it seems you need
     to remove and install it all over again to get it to work (if you
     know a workaround, let me know).
       
  - Toshiba stack:

    straight forward, press 1 & 2 on the Wiimote (you don't need to
    hold them if you're quick) and click 'New Connection'.
    
    once found, you can pair it anytime again by right-clicking its
     device icon (and pressing 1 & 2 as before) - you can also set up
     a desktop shortcut that enters discovery mode immediately.

  - Widcomm stack:

    1) Open 'My Bluetooth Places'.
    2) Press and hold 1 & 2 (until the process is complete).
    3) Click 'View Devices in Range'.
    4) Wiimote is detected as Nintendo RVL-CNT-01.
    5) Select it, then click 'Bluetooth Setup Wizard'.
    5) Click 'Skip' (no password).
    6) Now it should be connected (you can let go of 1 & 2).

    Troubleshooting:
        - the device seems to be connected but the Demo can't find it
          (CreateFile() fails with error 5 'Access Denied'),
     or - it disconnects almost immediately after connection
     or - asks for a password a few seconds after connection

     Try uninstalling all HID devices from Device Manager, and then
      redetecting them with 'Scan for hardware changes' (I had all
      these problems and that fixed it for me).

  - Other stacks
  
    similar to the above (contribute instructions?)
    
  
  - Disconnect/un-pair to save power (any stack):

    hold the Wiimote Power button for a few seconds - it automatically
    unpairs itself, re-enters pairing mode for a few seconds
    (flashing LEDs), then times out and (effecively) switches off.

__________________________  

Balance Boards notes:

  Balance Boards are installed using the same procedure as wiimotes, by
   holding the 'Sync' button in the battery compartment.  The Bluetooth
   stack detectes them as 'Nintendo RVL-WBC-01'.

  They report to the library as wiimotes, with a permanent BALANCE_BOARD
   extension.  They only have one button (A), and no IR/Acceleration/Rumble
   or Speaker support.  There is only one supported 'report type' so the
   library sets this automatically (see the demo for details).
   
   You can detect them with the wiimote::IsBalanceBoard() call.
   
   The boards tested so far all report up to ~ +-2.5KB weight offsets even
    where there is no weight placed on them (ie. 'at rest').  It is unknown
    if this is normal sensor inaccuracy/drift, or if the calibration values
    read from the board are interpreted incorrectly.  For now the library
    automatically subtracts the first incoming sensor values after a Connect()
    call from all future non-raw values (the raw values are never modified).
    
    The offsets used are exposed in wiimote_state::balance_board::AtRestKG.
   
    - if the board wasn't at rest during the Connect() call these offsets
       will be wrong - the app can measure them manually (when the board
       is at rest) by calling  wiimote::CalibrateAtRest().

__________________________  

MotionPlus notes:

  Once activated it's reported like any other extension, but needs to be
   manually enabled (see demo for an example):
      - Test first if it's connected via MotionPlusConnected()
      - Then call EnableMotionPlus()
      - It will then replace any other extension connected to it, unil you
         disable the Motion+ again with DisableMotionPlus().
   
  The speed values are believed to be correct.  The calibration values are not
   yet understood, so the data is currently uncalibrated.  
   
  Some technical details are mentioned in this interview with the MotionPlus
   hardware/software engineers:
   http://uk.wii.com/wii/en_GB/software/iwata_asks_motionplus_volume_1_2162.html#top
   

  * Special thanks to the guys at WiiBrew.org, and all contributing hackers *
    for figuring out & documenting the wiimote protocols:
    http://wiibrew.org/wiki/Wiimote/Extension_Controllers#Wii_Motion_Plus
   

Sign up to the mailing list to stay in the loop, exchange ideas or help each
 other out: http://gl.tter.org/mailman/listinfo/wiiyourself_gl.tter.org
__

gl.tter  (glATr-i-lDOTnet)


