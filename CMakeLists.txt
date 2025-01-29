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
        src/app/application.cpp
        src/app/application.hpp
        src/window/event.hpp
        src/window/input.cpp
        src/window/input.hpp
        src/window/window.cpp
        src/window/window.hpp
        src/utils.cpp
        src/utils.hpp
        src/opengl/utils.cpp
        src/opengl/utils.hpp
        src/renderer/camera.hpp
        src/renderer/camera.cpp
        src/opengl/shader.cpp
        src/opengl/shader.hpp
        src/opengl/texture.cpp
        src/opengl/texture.hpp
        src/opengl/framebuffer.cpp
        src/opengl/framebuffer.hpp
        src/opengl/buffer.cpp
        src/opengl/buffer.hpp
        src/app/editor.cpp
        src/app/editor.hpp
        src/renderer/renderer.cpp
        src/renderer/renderer.hpp
        src/resource/resource_manager.cpp
        src/resource/resource_manager.hpp
        src/resource/resource_importer.cpp
        src/resource/resource_importer.hpp
        src/resource/loaded_resource.hpp
        src/renderer/instanced_mesh.cpp
        src/renderer/instanced_mesh.hpp
        src/renderer/material.hpp
        src/renderer/model.hpp
        src/app/simple_notification_service.cpp
        src/app/simple_notification_service.hpp
        src/scene_graph/scene_node.cpp
        src/scene_graph/scene_node.hpp
        src/scene_graph/mesh_node.cpp
        src/scene_graph/mesh_node.hpp
        src/scene_graph/light_node.cpp
        src/scene_graph/light_node.hpp
        src/scene_graph/scene_graph.cpp
        src/scene_graph/scene_graph.hpp
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
        TINYGLTF_USE_CPP14
        TINYGLTF_NO_EXTERNAL_IMAGE
)

target_link_options(${PROJECT_NAME} PRIVATE -static)
