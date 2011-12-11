cmake_minimum_required(VERSION 2.8)

ADD_EXECUTABLE(main main.cpp capture.cpp cluster.cpp skin-detector.cpp tracker.cpp)

FIND_PACKAGE(OpenCV REQUIRED)
TARGET_LINK_LIBRARIES(main ${OpenCV_LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

