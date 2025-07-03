# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(native_opencv_library VERSION 0.0.1 LANGUAGES C CXX)

# opencv
set(OpenCV_STATIC ON)
set(OpenCV_DIR $ENV{OPENCV_ANDROID}/sdk/native/jni)
find_package (OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

# compile native_opencv.cpp to a shared object library called libnative_opencv.so
add_library(native_opencv SHARED
  "native_opencv.cpp"
)

set_target_properties(native_opencv PROPERTIES
  PUBLIC_HEADER native_opencv.h
  OUTPUT_NAME "native_opencv"
)

target_compile_definitions(native_opencv PUBLIC DART_SHARED_LIB)

find_library(log-lib log)

target_link_libraries( # Specifies the target library.
    native_opencv
    ${OpenCV_LIBS}
    ${log-lib}
)
