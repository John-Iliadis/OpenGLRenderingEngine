//
// Created by Gianni on 23/01/2025.
//

#include "editor.hpp"
#include "../resource/resource_manager.hpp"

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
    ImGui::ShowDemoWindow();
    mainMenuBar();
    assetPanel();
    sceneGraph();
    viewport();
}

void Editor::render()
{
    imguiEnd();
}

void Editor::mainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("ImportModel"))
            {
                std::filesystem::path path = fileDialog();

                if (!path.empty())
                    mResourceManager->importModel(path);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings"))
        {
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Editor::assetPanel()
{
    ImGui::Begin("Assets");
    ImGui::End();
}

void Editor::sceneGraph()
{
    ImGui::Begin("Scene Graph");
    ImGui::End();
}

void Editor::viewport()
{
    ImGui::Begin("Viewport");
    ImGui::End();
}

void Editor::imguiInit()
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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
