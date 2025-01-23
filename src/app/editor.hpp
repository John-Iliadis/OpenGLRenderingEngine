//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_EDITOR_HPP
#define OPENGLRENDERINGENGINE_EDITOR_HPP

#include <glfw/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

class Renderer;
class ResourceManager;

class Editor
{
public:
    Editor(std::shared_ptr<Renderer> renderer, std::shared_ptr<ResourceManager> resourceManager);
    ~Editor();

    void update(float dt);
    void render();

private:
    void imguiInit();
    void imguiTerminate();
    void imguiBegin();
    void imguiEnd();

private:
    std::shared_ptr<Renderer> mRenderer;
    std::shared_ptr<ResourceManager> mResourceManager;
};

#endif //OPENGLRENDERINGENGINE_EDITOR_HPP
