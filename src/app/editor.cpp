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
            if (ImGui::MenuItem("Import Model"))
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

    static int selectedIndex = 0;
    static const std::array<const char*, 3> items {{
        "Models",
        "Materials",
        "Textures"
    }};

    {
        ImGui::BeginGroup();

        for (int i = 0; i < items.size(); ++i)
        {
            const ImGuiStyle& style = ImGui::GetStyle();
            ImVec2 selectableSize = ImGui::CalcTextSize(items.at(i)) + style.FramePadding * 2.f;

            ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[selectedIndex == i? ImGuiCol_ButtonHovered : ImGuiCol_Button]);

            if (ImGui::Button(items.at(i), selectableSize))
                selectedIndex = i;

            ImGui::PopStyleColor();

            if (i < items.size() - 1)
                ImGui::SameLine();
        }

        ImGui::EndGroup();
        ImGui::Separator();
    }

    if (selectedIndex == 0)
        displayModels();

    if (selectedIndex == 1)
        displayMaterials();

    if (selectedIndex == 2)
        displayTextures();

    ImGui::End();
}

void Editor::displayModels()
{
    for (auto& [modelID, modelMetadata] : mResourceManager->mModelMetaData)
    {
         if (ImGui::Button(modelMetadata.c_str()))
         {

         }
    }
}

void Editor::displayMaterials()
{
    for (auto& [materialID, materialMetadata] : mResourceManager->mMaterialMetaData)
    {
        ImGui::Button(materialMetadata.c_str());
    }
}

void Editor::displayTextures()
{
    for (auto& [textureID, textureMetadata] : mResourceManager->mTextureMetaData)
    {
        ImGui::Button(textureMetadata.c_str());
    }
}

void Editor::sceneGraph()
{
    ImGui::Begin("Scene Graph");

    for (auto child : mSceneGraph.mRoot.children())
        sceneNodeRecursive(child);

    ImGui::Dummy(ImGui::GetContentRegionAvail());
    sceneNodeDragDropTarget(&mSceneGraph.mRoot);

    ImGui::End();
}

void Editor::sceneNodeRecursive(SceneNode *node)
{
    static constexpr ImGuiTreeNodeFlags treeNodeFlags {
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth
    };

    if (ImGui::TreeNodeEx((void*)(intptr_t)node, treeNodeFlags, node->name().c_str()))
    {
        sceneNodeDragDropSource(node);
        sceneNodeDragDropTarget(node);

        for (auto child : node->children())
            sceneNodeRecursive(child);

        ImGui::TreePop();
    }
}

void Editor::sceneNodeDragDropSource(SceneNode *node)
{
    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("SceneNode", &node, sizeof(SceneNode*));
        ImGui::EndDragDropSource();
    }
}

void Editor::sceneNodeDragDropTarget(SceneNode *node)
{
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SceneNode"))
        {
            SceneNode* transferNode = *(SceneNode**)payload->Data;

            transferNode->orphan();
            node->addChild(transferNode);
        }

        ImGui::EndDragDropTarget();
    }
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
