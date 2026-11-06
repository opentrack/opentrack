#ifdef __APPLE__

#import <AppKit/AppKit.h>

#include "background-app.hpp"

void set_background_app()
{
    if ([NSApp isActive] || NSApp == nil)
    {
        // NSApp may be nil before QApplication is constructed; try again later
        NSLog(@"set_background_app: NSApp not ready");
        return;
    }
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
}

#endif
