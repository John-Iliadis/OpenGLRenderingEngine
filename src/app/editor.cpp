//
// Created by Gianni on 23/01/2025.
//

#include "editor.hpp"

Editor::Editor(std::shared_ptr<Renderer> renderer, std::shared_ptr<ResourceManager> resourceManager)
    : mRenderer(renderer)
    , mResourceManager(resourceManager)
{
    imguiInit();
}

Editor::~Editor()
{
    imguiTerminate();
}

void Editor::update(float dt)
{
    imguiBegin();
}

void Editor::render()
{
    imguiEnd();
}

void Editor::imguiInit()
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
    ImGui_ImplOpenGL3_Init("#version 460 core");
}

void Editor::imguiTerminate()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Editor::imguiBegin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();
}

void Editor::imguiEnd()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
