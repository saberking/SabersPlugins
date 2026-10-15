# DISTRHO Plugin Framework (DPF)
# Copyright (C) 2021 Jean Pierre Cimalando <jp-dev@inbox.ru>
# Copyright (C) 2021-2025 Filipe Coelho <falktx@falktx.com>
# SPDX-License-Identifier: ISC

cmake_minimum_required(VERSION 3.7...3.31)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(NAME SabersPlugins)

if(NOT CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{CMAKE_TOOLCHAIN_FILE}" CACHE FILEPATH "Toolchain file")
endif()
project(${NAME})


execute_process(
    COMMAND git apply "${CMAKE_CURRENT_SOURCE_DIR}/dpf_changes.patch"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dpf"
    ERROR_QUIET
)
execute_process(
    COMMAND git apply "${CMAKE_CURRENT_SOURCE_DIR}/focus.patch"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dpf"
    ERROR_QUIET
)
# execute_process(
#     COMMAND git apply "${CMAKE_CURRENT_SOURCE_DIR}/dpf_widgets_changes.patch"
#     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dpf-widgets"
#     ERROR_QUIET
# ) this was for opengl screenbuffer but not necessary

add_subdirectory(dpf)
add_compile_definitions(ImDrawIdx=unsigned\ int)


list(APPEND TARGETS_LIST "clap")
list(APPEND TARGETS_LIST "vst3")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND TARGETS_LIST "jack")
endif()

dpf_add_plugin(BAKED
  TARGETS ${TARGETS_LIST}
  FILES_DSP
      BAKED/PluginDSP.cpp
      common/external/base64.cpp
  FILES_UI
      BAKED/PluginUI.cpp
      common/external/implot.cpp
      common/external/implot_items.cpp
      dpf-widgets/opengl/DearImGui.cpp)

target_sources(BAKED
  PRIVATE
    BAKED/Parameters.hpp
    common/DragAndDrop.hpp
    common/WinConsoleOutput.hpp
    BAKED/Editor.hpp
    BAKED/AudioData.hpp

    BAKED/PluginDSP.hpp
    BAKED/Defines.hpp
    BAKED/features.txt
    BAKED/Undo.hpp
)

target_include_directories(BAKED PUBLIC common)
target_include_directories(BAKED PUBLIC BAKED)

target_include_directories(BAKED PUBLIC dpf-widgets/generic)
target_include_directories(BAKED PUBLIC dpf-widgets/opengl)
target_include_directories(BAKED PUBLIC dpf-widgets/opengl/DearImGui)

target_link_libraries(BAKED-ui PRIVATE comctl32)

dpf_add_plugin(neural
  TARGETS ${TARGETS_LIST}
  FILES_DSP
      neural/PluginDSP.cpp
      common/external/base64.cpp
  FILES_UI
      neural/PluginUI.cpp
      # src/external/implot.cpp
      # src/external/implot_items.cpp
      dpf-widgets/opengl/DearImGui.cpp)

target_sources(neural
  PRIVATE
    neural/Parameters.hpp
    # src/DragAndDrop.hpp
    common/WinConsoleOutput.hpp
    # src/Editor.hpp
    # src/AudioData.hpp

    neural/PluginDSP.hpp
    neural/Defines.hpp
    # src/features.txt
)

target_include_directories(neural PUBLIC common)

target_include_directories(neural PUBLIC neural)
target_include_directories(neural PUBLIC dpf-widgets/generic)
target_include_directories(neural PUBLIC dpf-widgets/opengl)
target_include_directories(neural PUBLIC dpf-widgets/opengl/DearImGui)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(neural-ui PRIVATE comctl32)

endif()
# 1. Enable full optimizations, fast-math, and native SIMD instructions (AVX/AVX2/NEON)
target_compile_options(neural PRIVATE
    -O3
    -Ofast
    -march=native
)

# 2. Crucial for Eigen: Disable safety/assert checks so it can compile down to pure SIMD registers
target_compile_definitions(neural PRIVATE
    NDEBUG
        EIGEN_DONT_PARALLELIZE  # 🚀 FIXES MULTI-INSTANCE AUDIO CRASHES PERMANENTLY
)

dpf_add_plugin(GRAM
  TARGETS ${TARGETS_LIST}
  FILES_DSP
      GRAM/PluginDSP.cpp
      common/external/base64.cpp
  FILES_UI
      GRAM/PluginUI.cpp
      # GRAM/external/implot.cpp
      # GRAM/external/implot_items.cpp
      dpf-widgets/opengl/DearImGui.cpp)

target_sources(GRAM
  PRIVATE
    GRAM/Parameters.hpp
    # src/DragAndDrop.hpp
    common/WinConsoleOutput.hpp
    # src/Editor.hpp
    # src/AudioData.hpp

    GRAM/PluginDSP.hpp
    GRAM/Defines.hpp
    # src/features.txt
)

target_include_directories(GRAM PUBLIC common)
target_include_directories(GRAM PUBLIC GRAM)
target_include_directories(GRAM PUBLIC dpf-widgets/generic)
target_include_directories(GRAM PUBLIC dpf-widgets/opengl)
target_include_directories(GRAM PUBLIC dpf-widgets/opengl/DearImGui)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(GRAM-ui PRIVATE comctl32)

endif()
# 1. Enable full optimizations, fast-math, and native SIMD instructions (AVX/AVX2/NEON)
target_compile_options(GRAM PRIVATE
    -O3
    -Ofast
    -march=native
)

# 2. Crucial for Eigen: Disable safety/assert checks so it can compile down to pure SIMD registers
target_compile_definitions(GRAM PRIVATE
    NDEBUG
        EIGEN_DONT_PARALLELIZE  # 🚀 FIXES MULTI-INSTANCE AUDIO CRASHES PERMANENTLY
)