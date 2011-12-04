cmake_minimum_required(VERSION 2.8)

ADD_EXECUTABLE(main main.cpp capture.cpp cluster.cpp skin-detector.cpp tracker.cpp)

#INCLUDE_DIRECTORIES(../include/)
#INCLUDE_DIRECTORIES(../include/opencv/)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# Mac OS X specific code
	TARGET_LINK_LIBRARIES(
		main
		${PROJECT_SOURCE_DIR}/lib/libopencv_core.2.3.1.dylib
		${PROJECT_SOURCE_DIR}/lib/libopencv_highgui.2.3.1.dylib
		${PROJECT_SOURCE_DIR}/lib/libopencv_imgproc.2.3.1.dylib
		${PROJECT_SOURCE_DIR}/lib/libopencv_features2d.2.3.1.dylib
	)
	SET(OperatingSystem "Mac OS X")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	# Linux specific code
	TARGET_LINK_LIBRARIES(
		main
		${PROJECT_SOURCE_DIR}/lib/libopencv_core.so.2.3.1
		${PROJECT_SOURCE_DIR}/lib/libopencv_highgui.so.2.3.1
		${PROJECT_SOURCE_DIR}/lib/libopencv_imgproc.so.2.3.1
		${PROJECT_SOURCE_DIR}/lib/libopencv_features2d.so.2.3.1
	)
	SET(OperatingSystem "Linux")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

MESSAGE(STATUS "The executable file will be in" ${PROJECT_BINARY_DIR}/bin)

