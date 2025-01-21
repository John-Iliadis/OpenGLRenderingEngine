cmake_minimum_required(VERSION 3.28)
project(OpenGLRenderingEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)
set(DEPENDENCY_DIR ${CMAKE_SOURCE_DIR}/dependencies)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(FATAL_ERROR "MinGW compiler required")
endif()

file(GLOB INCLUDE_DIRS ${DEPENDENCY_DIR}/*/include)
file(GLOB LIBS ${DEPENDENCY_DIR}/*/lib/*.a)
file(GLOB IMGUI_SRC ${DEPENDENCY_DIR}/imgui/include/imgui/*.cpp)

add_executable(${PROJECT_NAME}
        ${IMGUI_SRC}
        src/main.cpp
        src/pch.cpp
        src/pch.hpp
        src/application.cpp
        src/application.hpp
        src/window/event.hpp
        src/window/input.cpp
        src/window/input.hpp
        src/window/window.cpp
        src/window/window.hpp
        src/utils.cpp
        src/utils.hpp
        src/opengl/opengl_utils.cpp
        src/opengl/opengl_utils.hpp
        src/renderer/camera.hpp
        src/renderer/camera.cpp
)

add_subdirectory(${DEPENDENCY_DIR}/glad/)

target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.hpp)

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX -d)

find_package(OpenGL REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE
        OpenGL::GL
        glad
        ${LIBS}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:DEBUG_MODE>
        GLFW_INCLUDE_NONE
        GLFW_EXPOSE_NATIVE_WIN32
        STB_IMAGE_IMPLEMENTATION
        GLM_FORCE_RADIANS
        IMGUI_DEFINE_MATH_OPERATORS
)

target_link_options(${PROJECT_NAME} PRIVATE -static)
