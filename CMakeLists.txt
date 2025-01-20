cmake_minimum_required(VERSION 3.14)
project(ImageProcProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ./gui_files)
#===========================================================

set(LIBRARY_DIR "D:/LIBRARY")
set(COMPILER "msvc2019_64")
set(TORCH_VERSION "libtorch113")
set(CV_VERSION "opencv410")

if(EXISTS "/home/broms/LIBRARY_LOC")
set(LIBRARY_DIR "/home/broms/LIBRARY_LOC")
set(COMPILER "gcc_64")
set(TORCH_VERSION "libtorch")
endif()

if(EXISTS "/home/broms/LIBRARY")
set(LIBRARY_DIR "/home/broms/LIBRARY")
set(COMPILER "gcc_64")
set(TORCH_VERSION "libtorch")
endif()


if(EXISTS "${LIBRARY_DIR}/IMAGE_PROCESSING/OPENCV/opencv410")
set(CV_VERSION "opencv410")
endif()

if(EXISTS "${LIBRARY_DIR}/IMAGE_PROCESSING/OPENCV/opencv451")
set(CV_VERSION "opencv451")
endif()

if(EXISTS "${LIBRARY_DIR}/IMAGE_PROCESSING/OPENCV/opencv470")
set(CV_VERSION "opencv470")
endif()

set(OPENCV_LIB_DIR "${LIBRARY_DIR}/IMAGE_PROCESSING/OPENCV/${CV_VERSION}/")
set(QT_LIB_DIR "${LIBRARY_DIR}/Qt653/6.5.3/${COMPILER}")
set(PYTORCH_DIR "${LIBRARY_DIR}/NN_LIBRARY/LIBTORCH/${TORCH_VERSION}/")
set(Torch_DIR ${PYTORCH_DIR})

set(Qt6_DIR "${QT_LIB_DIR}/lib/cmake/Qt6")
set(Qt6Widgets_DIR "${QT_LIB_DIR}/lib/cmake/Qt6Widgets")
set(Qt6WidgetsTools_DIR "${QT_LIB_DIR}/lib/cmake/Qt6WidgetsTools")
set(Qt6SerialPort_DIR "${QT_LIB_DIR}/lib/cmake/Qt6SerialPort")
set(Qt6PrintSupport_DIR "${QT_LIB_DIR}/lib/cmake/Qt6PrintSupport")
set(Qt6Network_DIR "${QT_LIB_DIR}/lib/cmake/Qt6Network")
set(Qt6Gui_DIR "${QT_LIB_DIR}/lib/cmake/Qt6Gui")
set(Qt6GuiTools_DIR "${QT_LIB_DIR}/lib/cmake/Qt6GuiTools")
set(Qt6EntryPointPrivate_DIR "${QT_LIB_DIR}/lib/cmake/Qt6EntryPointPrivate")
set(Qt6Core_DIR "${QT_LIB_DIR}/lib/cmake/Qt6Core")
set(Qt6CoreTools_DIR "${QT_LIB_DIR}/lib/cmake/Qt6CoreTools")

message("SET QT     LIBRARY - ${Qt6_DIR}")
message("SET OPENCV LIBRARY - ${LIBRARY_DIR}/IMAGE_PROCESSING/OPENCV/${CV_VERSION}")
message("SET TORCH  LIBRARY - ${PYTORCH_DIR}")
#====================SUBPROJECTS=============================
#add_subdirectory(PYTORCH_PROCESSING)

if( UNIX )
SET(TORCH_PROCESS_MODULE "${CMAKE_CURRENT_SOURCE_DIR}/build/PYTORCH_PROCESSING/libTorchProcessingModule.so") 
endif()

if( WIN32 )
SET(TORCH_PROCESS_MODULE "${CMAKE_CURRENT_SOURCE_DIR}/build/PYTORCH_PROCESSING/TorchProcessingModule.lib") 
endif()

#============================================================
#=========================FIND LIBRARIES=====================

if(NOT EXISTS ${QT_LIB_DIR})
message("QT LIBRARY NOT FOUND IN: ${LIBRARY_DIR}")
endif()

if(NOT EXISTS ${OPENCV_LIB_DIR})
message("OPENCV LIBRARY NOT FOUND IN: ${OPENCV_LIB_DIR}")
endif()

if(NOT EXISTS ${PYTORCH_DIR})
message("PYTORCH LIBRARY NOT FOUND IN: ${LIBRARY_DIR}/NN_LIBRARY")
endif()

list(APPEND CMAKE_PREFIX_PATH ${QT_LIB_DIR})
list(APPEND CMAKE_PREFIX_PATH ${OPENCV_LIB_DIR})
list(APPEND CMAKE_PREFIX_PATH ${PYTORCH_DIR})

find_package(Qt6 COMPONENTS Widgets Network Core SerialPort PrintSupport REQUIRED)
find_package(OpenCV REQUIRED )
find_package(Torch REQUIRED)

message("OPENCV LIBS "${OpenCV_LIBS})
#=============================================================

#=========================INCLUDES============================
include_directories("${PYTORCH_DIR}/include")
include_directories("${PYTORCH_DIR}/include/torch/csrc/api/include")
include_directories("./gui_sources")
include_directories("./gui_sources/GRAPHICS_WINDOW")
#include_directories("${PROJECT_SOURCE_DIR}/PYTORCH_PROCESSING") #SUBPROJECT PYTORCH_PROCESSING
include_directories("${PROJECT_SOURCE_DIR}/CV_IMAGE_PROCESSING") 
include_directories("${PROJECT_SOURCE_DIR}/CV_AIM_IMITATION")
include_directories("${PROJECT_SOURCE_DIR}/UTILITES") 
include_directories("${PROJECT_SOURCE_DIR}/COMMON") 
include_directories("${PROJECT_SOURCE_DIR}/AIMING_MODULES") 
include_directories("${PROJECT_SOURCE_DIR}/DEVICES") 
include_directories("${PROJECT_SOURCE_DIR}/DEVICES/LaserPointerControlLib") 
include_directories("${PROJECT_SOURCE_DIR}/DEVICES/MotorImitatorLib") 
include_directories("${PROJECT_SOURCE_DIR}/TRANSMISSION/DATA_STRUCTURES") 
include_directories("${PROJECT_SOURCE_DIR}/TRANSMISSION/EMBEDED_RING_BUFFER") 
include_directories("${PROJECT_SOURCE_DIR}/TRANSMISSION/GENERIC_MESSAGES") 
include_directories("${PROJECT_SOURCE_DIR}/TRANSMISSION/TRANSMISSION_MODULES") 


include_directories("${LIBRARY_DIR}/IMAGE_PROCESSING/${CV_VERSION}/build/include")
include_directories("${LIBRARY_DIR}/MATH_LIBRARY/")
include_directories("C:/HEADERS_STD")
message("MATH LIBRARY: ${LIBRARY_DIR}/MATH_LIBRARY/")

#include_directories("${LIBRARY_DIR}/MATH_LIBRARY")
#include_directories("${LIBRARY_DIR}/COMMON_LIBRARRY/boost_1_68_0/boost_1_68_0")

#include_directories("${LIBRARY_DIR}/NN_LIBRARY/LIB_TORCH/onnxruntime/include/")
#link_directories("${LIBRARY_DIR}/NN_LIBRARY/ONNX/onnxruntime/lib/")

#include_directories("../${PROJECT_SOURCE_DIR}")

#=========================ARAVIS CAMRA LIBRARY ============================

if (UNIX)
include_directories("/usr/local/include/aravis-0.8")
include_directories("/usr/include/glib-2.0")
include_directories("/usr/lib/x86_64-linux-gnu/glib-2.0/include")
endif (UNIX)
#=========================SUBPROJECTS============================

add_subdirectory("ROTATE_OPTIMIZATION/RotationContainerLib")
include_directories("${PROJECT_SOURCE_DIR}/ROTATE_OPTIMIZATION/RotationContainerLib")
include_directories("${PROJECT_SOURCE_DIR}/ROTATE_OPTIMIZATION")
#==============================================================

#======================LINK PROJECT===========================
add_executable (ImageProcProject
        "main.cpp"
        "controller_process_class.cpp"
        "./AIM_IMAGE_IMITATION/imitator_image_aim.cpp"
        "./AIM_IMAGE_IMITATION/imitator_image_aim.h"

        "./AIM_IMAGE_IMITATION/sinus_generator_class.cpp"
        "./AIM_IMAGE_IMITATION/widget_sinus_source.cpp"
        "./AIM_IMAGE_IMITATION/widget_sinus_source.ui"

        "./CV_IMAGE_PROCESSING/cv_image_processing.cpp"
        "./CV_IMAGE_PROCESSING/cv_image_processing.h"
        "./CAMERA_INTERFACE/camera_interface_class.cpp"
        "./CAMERA_INTERFACE/camera_interface_class.h"

        "./UTILITES/debug_output_filter.cpp"
        "./UTILITES/register_settings.cpp"
        "./UTILITES/engine_statistics.cpp"
        "./UTILITES/transform_coord_class.cpp"
        "./ROTATE_OPTIMIZATION/rotation_find_executor.cpp"

        "./DEVICES/scanator_control_class.cpp"

        "./AIMING_MODULES/aiming_class.cpp"
        "./AIMING_MODULES/pid_class.cpp"
        "./AIMING_MODULES/rotate_vector_class.cpp"

        "./COMMON/interface_pass_coord.cpp"
        "./COMMON/interface_image_source.cpp"

        "./TRANSMISSION/DATA_STRUCTURES/message_command_structures.cpp"

        "./TRANSMISSION/DATA_STRUCTURES/engine_type_register.cpp"
        "./TRANSMISSION/TRANSMISSION_MODULES/engine_udp_interface.cpp"

        "./gui_sources/widget_process_controller.cpp"
        "./gui_sources/widget_processing_image_generic.cpp"
        "./gui_sources/widget_imitator_control.cpp"
        "./gui_sources/widget_processing_image_control.cpp"
        "./gui_sources/widget_camera_control.cpp"
        "./gui_sources/widget_adjustable.cpp"
        "./gui_sources/widget_scene_node_interface.cpp"
        "./gui_sources/labelimage.cpp"
        "./gui_sources/link_line.cpp"
        "./gui_sources/node.cpp"
        "./gui_sources/widget_main_window.cpp"
        "./gui_sources/widget_output_image_display.cpp"
        "./gui_sources/widget_output_text_display.cpp"
        "./gui_sources/qcustomplot.cpp"
        "./gui_sources/widget_scanator_control.cpp"
        "./gui_sources/widget_aiming_control.cpp"

        "./gui_sources/GRAPHICS_WINDOW/widget_graphics_plot.cpp"
        "./gui_sources/GRAPHICS_WINDOW/widget_graphics_plot.ui"

        "./gui_files/widget_processing_image_control.ui"
        "./gui_files/widget_processing_image_generic.ui"
        "./gui_files/widget_processing_image_control.ui"
        "./gui_files/widget_camera_control.ui"
        "./gui_files/widget_imitator_control.ui"
        "./gui_files/widget_main_window.ui"
        "./gui_files/widget_output_image_display.ui"
        "./gui_files/widget_output_text_display.ui"
        "./gui_files/widget_scanator_control.ui"
        "./gui_files/widget_aiming_control.ui"
        "./gui_files/pictures.qrc"
        )
target_link_libraries(ImageProcProject ${OpenCV_LIBS})
target_link_libraries(ImageProcProject Qt6::Widgets)
target_link_libraries(ImageProcProject Qt6::Network)
target_link_libraries(ImageProcProject Qt6::Core)
target_link_libraries(ImageProcProject Qt6::PrintSupport)

if (UNIX)
target_link_libraries(ImageProcProject /lib/x86_64-linux-gnu/libglib-2.0.so)
target_link_libraries(ImageProcProject /lib/x86_64-linux-gnu/libgobject-2.0.so)
target_link_libraries(ImageProcProject /usr/local/lib/x86_64-linux-gnu/libaravis-0.8.so)
target_link_libraries(ImageProcProject "${CMAKE_CURRENT_SOURCE_DIR}/DEVICES/LaserPointerControlLib/libLaserControlLib.so")
#target_link_libraries(ImageProcProject "${CMAKE_CURRENT_SOURCE_DIR}/DEVICES/MotorImitatorLib/libMotorControlLib.so")

target_link_libraries(ImageProcProject "${PYTORCH_DIR}/lib/libtorch_cpu.so")
target_link_libraries(ImageProcProject "${PYTORCH_DIR}/lib/libtorch.so")
target_link_libraries(ImageProcProject "${PYTORCH_DIR}/lib/libc10.so")
target_link_libraries(ImageProcProject "${CMAKE_CURRENT_SOURCE_DIR}/build/ROTATE_OPTIMIZATION/RotationContainerLib/libRotateOptimization.so")
endif (UNIX)

if (WIN32)
target_link_libraries(ImageProcProject "${PYTORCH_DIR}/lib/torch_cpu.lib")
target_link_libraries(ImageProcProject "${PYTORCH_DIR}/lib/torch.lib")
target_link_libraries(ImageProcProject "${PYTORCH_DIR}/lib/c10.lib")
target_link_libraries(ImageProcProject "${CMAKE_CURRENT_SOURCE_DIR}/build/ROTATE_OPTIMIZATION/RotationContainerLib/RotateOptimization.lib")
endif (WIN32)



#target_link_libraries(ImageProcProject onnxruntime.so)

#link_directories("${LIBRARY_DIR}/NN_LIBRARY/libtorch/lib")
#target_link_libraries(ImageProcProject "/home/broms/PROGRAM_LIBRARY/NN_LIBRARY/libtorch/lib/libtorch_cpu.so")
#target_link_libraries(ImageProcProject "/home/broms/PROGRAM_LIBRARY/NN_LIBRARY/libtorch/lib/libtorch.so")
#target_link_libraries(ImageProcProject "/home/broms/PROGRAM_LIBRARY/NN_LIBRARY/libtorch/lib/libc10.so")

#message("ROTATE LIB: ${CMAKE_CURRENT_SOURCE_DIR}/build/libRotateOptimization.a")
#==========================================================
