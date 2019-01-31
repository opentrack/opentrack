
if(DEFINED ENV{KINECTSDK20_DIR})
    otr_module(tracker-kinect-face)
	target_include_directories(opentrack-tracker-kinect-face SYSTEM PUBLIC $ENV{KINECTSDK20_DIR}inc)
	target_link_libraries(opentrack-tracker-kinect-face $ENV{KINECTSDK20_DIR}lib/x86/kinect20.lib $ENV{KINECTSDK20_DIR}lib/x86/kinect20.face.lib)
endif()
