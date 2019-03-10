# Kinect SDK is Windows only
# If we have a valid SDK path, try build that tracker
if(SDK_KINECT20)
	# Register our module
	otr_module(tracker-kinect-face)
	# Setup Kinect for this module
	otr_kinect_setup()
	# Link against video utilities, needed for video preview
	target_link_libraries(${self} opentrack-video)

endif()
