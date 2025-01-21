//
// Created by Gianni on 21/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_OPENGL_IMGUI_HPP
#define OPENGLRENDERINGENGINE_OPENGL_IMGUI_HPP

#include <glfw/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

void imguiInit();
void imguiTerminate();

void imguiBegin();
void imguiEnd();

#endif //OPENGLRENDERINGENGINE_OPENGL_IMGUI_HPP
