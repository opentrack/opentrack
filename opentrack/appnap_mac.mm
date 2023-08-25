#ifdef __APPLE__

#import <Foundation/Foundation.h>

/**
 * Used to prevent macOS from throttling the opentrack process.
 */

id token = nil;

void disable_appnap_start();
void disable_appnap_stop();

void disable_appnap_start() {

    if(token){
        NSLog(@"disable_appnap_start: already started");
        return;
    }


    NSLog(@"disable_appnap_start");
    token = [[NSProcessInfo processInfo]
      beginActivityWithOptions: NSActivityUserInitiatedAllowingIdleSystemSleep
      reason: @"Disable AppNap"];
    [token retain];
}

void disable_appnap_stop() {
    if(!token){
        NSLog(@"disable_appnap_start: not started");
        return;
    }

    NSLog(@"disable_appnap_stop");
    [[NSProcessInfo processInfo] endActivity:token];
    [token release];
    token = nil;
}



#endif




