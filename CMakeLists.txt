cmake_minimum_required(VERSION 3.10)

project(CoreAudioExtAudioFileExample)
set(CMAKE_CXX_STANDARD 14)

add_executable(decode decode.cpp)
target_link_libraries(decode PRIVATE "-framework CoreFoundation -framework AudioToolbox")

add_executable(encode encode.cpp)
target_link_libraries(encode PRIVATE "-framework CoreFoundation -framework AudioToolbox")

add_executable(decode_with_callbacks decode_with_callbacks.cpp)
target_link_libraries(decode_with_callbacks PRIVATE "-framework CoreFoundation -framework AudioToolbox")

add_executable(detect_audio_format detect_audio_format.cpp)
target_link_libraries(detect_audio_format PRIVATE "-framework CoreFoundation -framework AudioToolbox")

add_executable(encode_planar_data encode_planar_data.cpp)
target_link_libraries(encode_planar_data PRIVATE "-framework CoreFoundation -framework AudioToolbox")

add_executable(encode_with_callbacks encode_with_callbacks.cpp file_output_stream.cpp)
target_link_libraries(encode_with_callbacks PRIVATE "-framework CoreFoundation -framework AudioToolbox")