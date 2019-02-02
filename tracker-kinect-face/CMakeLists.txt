# Kinect SDK is Windows only
if (WIN32)
	# Setup cache variable to Kinect SDK path
	if(DEFINED ENV{KINECTSDK20_DIR})
		set(KINECTSDK20_DIR $ENV{KINECTSDK20_DIR} CACHE PATH $ENV{KINECTSDK20_DIR})
	else()
		set(KINECTSDK20_DIR $ENV{KINECTSDK20_DIR} CACHE PATH "")
	endif()

	# If we have a valid SDK path, try build that tracker
	if(EXISTS ${KINECTSDK20_DIR})
		# Register our module
		otr_module(tracker-kinect-face)

		# Add include path to Kinect SDK 
		target_include_directories(opentrack-tracker-kinect-face SYSTEM PUBLIC ${KINECTSDK20_DIR}/inc)
	
		# Check processor architecture 
		if(CMAKE_SIZEOF_VOID_P EQUAL 4)
			# 32 bits        
			set (kinect-arch-dir "x86")
		elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
			# 64 bits
			set (kinect-arch-dir "x64")		
		endif()
	
		# Link against Kinect SDK libraries
		target_link_libraries(opentrack-tracker-kinect-face ${KINECTSDK20_DIR}/lib/${kinect-arch-dir}/Kinect20.lib ${KINECTSDK20_DIR}/lib/${kinect-arch-dir}/Kinect20.Face.lib)

		# Install Kinect Face DLL
		install(FILES ${KINECTSDK20_DIR}/Redist/Face/${kinect-arch-dir}/Kinect20.Face.dll DESTINATION ./modules/ PERMISSIONS ${opentrack-perms-exec})
		# Install Kinect Face Database
		install(DIRECTORY ${KINECTSDK20_DIR}/Redist/Face/${kinect-arch-dir}/NuiDatabase DESTINATION ./modules/)

	endif()

endif()