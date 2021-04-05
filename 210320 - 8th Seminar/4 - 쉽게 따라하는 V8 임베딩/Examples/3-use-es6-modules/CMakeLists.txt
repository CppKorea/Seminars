# Copyright (c) 2021 Chanjung Kim. All rights reserved.
# Licensed under the MIT License.

get_filename_component(EXAMPLE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_executable(${EXAMPLE_NAME} Source/Main.cc)
target_link_libraries(${EXAMPLE_NAME} v8)