cmake_minimum_required(VERSION 3.14)

project(RotateOptimization LANGUAGES CXX)

message("ROTATE OPTIMIZATION LIBRARY ADD")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


cmake_path(GET PROJECT_SOURCE_DIR PARENT_PATH SOURCE_DIR)
message("MAIN PROJECT SOURCE: "${SOURCE_DIR})
set(USER "broms")

if(NOT EXISTS "/home/${USER}/LIBRARY")
message("LIBRARY_DIR: [ /home/${USER}/LIBRARY ] NOT FOUND")
endif()
set(LIBRARY_DIR "/home/${USER}/LIBRARY")

set(CMAKE_PREFIX_PATH "${LIBRARY_DIR}/Qt653/6.5.3/gcc_64/")
find_package(Qt6 COMPONENTS Core REQUIRED)

set(CMAKE_PREFIX_PATH "${LIBRARY_DIR}/NN_LIBRARY/LIBTORCH/libtorch/")
find_package(Torch REQUIRED)

add_library(RotateOptimization SHARED
  RotateOptimization_global.h
  RotateOperationContainer.cpp
  RotateOperationContainer.h
)

include_directories("${LIBRARY_DIR}/NN_LIBRARY/LIBTORCH/libtorch/include")
include_directories("${LIBRARY_DIR}/NN_LIBRARY/LIBTORCH/libtorch/include/torch/csrc/api/include")
include_directories("${SOURCE_DIR}")
#=====================================================================
link_directories("${LIBRARY_DIR}/NN_LIBRARY/LIBTORCH/libtorch/lib")
target_link_libraries(RotateOptimization Qt6::Core)
#=====================================================================

if (WIN32)
target_link_libraries(RotateOptimization "${PYTORCH_DIR}/lib/torch_cpu.lib")
target_link_libraries(RotateOptimization "${PYTORCH_DIR}/lib/torch.lib")
target_link_libraries(RotateOptimization "${PYTORCH_DIR}/lib/c10.lib")
endif (WIN32)

if (UNIX)
target_link_libraries(RotateOptimization "${PYTORCH_DIR}/lib/libtorch_cpu.so")
target_link_libraries(RotateOptimization "${PYTORCH_DIR}/lib/libtorch.so")
target_link_libraries(RotateOptimization "${PYTORCH_DIR}/lib/libc10.so")
endif (UNIX)

target_compile_definitions(RotateOptimization PRIVATE ROTATEOPTIMIZATION_LIBRARY)

