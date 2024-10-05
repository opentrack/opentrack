#ifdef __APPLE__

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <vector>
#import <QString.h>

/*
* Unfortunately opencv currently does not offer to enumerate the cameras (https://github.com/opencv/opencv/issues/4269).
+ Opentrack previously used QCameraInfo::availableCameras() to do that.
* I don't know how that's implemented, but it conflicts with opencv's implementation on macOS.
* If the order differ then the index numbers do not match and the behaviour is totally weird in case you have more than one camera.
* That's why I now use the same code-snipped from opencv to enumerate the cameras. It's not ideal but at least a viable and working solution.
*/

std::vector<QString> apple_get_camera_names(){
    std::vector<QString> ret;
    // see opencv/modules/videoio/src/cap_avfoundation_mac.mm (branch 4.x):
    //----------------------------------------------------
    NSAutoreleasePool *localpool = [[NSAutoreleasePool alloc] init];
    
    NSArray<AVCaptureDevice*> *devices = [[AVCaptureDevice devicesWithMediaType: AVMediaTypeVideo]
        arrayByAddingObjectsFromArray:[AVCaptureDevice devicesWithMediaType:AVMediaTypeMuxed]];
        
    devices = [devices
        sortedArrayUsingComparator:^NSComparisonResult(AVCaptureDevice *d1,
                                                       AVCaptureDevice *d2){
            return [d1.uniqueID compare:d2.uniqueID];
        }
    ];
    //----------------------------------------------------
    for (AVCaptureDevice* device in devices) {
        NSString* name = [device localizedName];
        ret.push_back(QString([name cStringUsingEncoding: NSUTF8StringEncoding]));
    }
    
    [localpool drain];
    return ret;
}
#endif




