//
// Created by Gianni on 23/01/2025.
//

#include "editor.hpp"
#include "../resource/resource_manager.hpp"

Editor::Editor(std::shared_ptr<Renderer> renderer, std::shared_ptr<ResourceManager> resourceManager)
    : mRenderer(renderer)
    , mResourceManager(resourceManager)
    , mCamera({}, 30.f, 1920.f, 1080.f)
    , mSelectedObjectID()
    , mShowViewport(true)
    , mShowAssetPanel(true)
    , mShowSceneGraph(true)
    , mShowCameraPanel(false)
    , mShowInspectorPanel(true)
    , mShowRendererPanel(false)
    , mShowConsole(true)
    , mShowDebugPanel(true)
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

    if (mShowAssetPanel)
        assetPanel();

    if (mShowSceneGraph)
        sceneGraphPanel();

    if (mShowCameraPanel)
        cameraPanel();

    if (mShowInspectorPanel)
        inspectorPanel();

    if (mShowRendererPanel)
        rendererPanel();

    if (mShowDebugPanel)
        debugPanel();

    if (mShowConsole)
        console();

    if (mShowViewport) // update last
        viewportPreRender();

    mCamera.update(dt); // updates inside the viewport panel
}

void Editor::render()
{
    // renderer->renderScene

    if (mShowViewport)
        viewportPostRender();

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
            ImGui::MenuItem("Viewport", nullptr, &mShowViewport);
            ImGui::MenuItem("Asset Panel", nullptr, &mShowAssetPanel);
            ImGui::MenuItem("Scene Graph", nullptr, &mShowSceneGraph);
            ImGui::MenuItem("Camera Panel", nullptr, &mShowCameraPanel);
            ImGui::MenuItem("Inspector", nullptr, &mShowInspectorPanel);
            ImGui::MenuItem("Renderer", nullptr, &mShowRendererPanel);
            ImGui::MenuItem("Console", nullptr, &mShowConsole);
            ImGui::MenuItem("Debug", nullptr, &mShowDebugPanel);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Editor::assetPanel()
{
    ImGui::Begin("Assets", &mShowAssetPanel);

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
    for (const auto& [modelID, modelName] : mResourceManager->mModelNames)
    {
        if (ImGui::Selectable(modelName.c_str(), mSelectedObjectID == modelID))
        {
            mSelectedObjectID = modelID;
        }

        modelDragDropSource(modelID);
    }
}

void Editor::displayMaterials()
{
    for (const auto& [materialID, materialName] : mResourceManager->mMaterialNames)
    {
        if (ImGui::Selectable(materialName.c_str(), mSelectedObjectID == materialID))
        {
            mSelectedObjectID = materialID;
        }
    }
}

void Editor::displayTextures()
{
    for (const auto& [textureID, textureName] : mResourceManager->mTextureNames)
    {
        if (ImGui::Selectable(textureName.c_str(), mSelectedObjectID == textureID))
        {
            mSelectedObjectID = textureID;
        }
    }
}

void Editor::sceneGraphPanel()
{
    ImGui::Begin("Scene Graph", &mShowSceneGraph);

    for (auto child : mSceneGraph.mRoot.children())
        sceneNodeRecursive(child);

    // todo: improve drag drop area
    ImGui::Dummy(ImGui::GetContentRegionAvail());
    sceneNodeDragDropTarget(&mSceneGraph.mRoot);

    ImGui::End();

    mSceneGraph.updateTransforms();
}

void Editor::sceneNodeRecursive(SceneNode *node)
{
    ImGuiTreeNodeFlags treeNodeFlags {
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanFullWidth |
        ImGuiTreeNodeFlags_FramePadding
    };

    if (mSelectedObjectID == node->id())
        treeNodeFlags |= ImGuiTreeNodeFlags_Selected;

    if (node->children().empty())
        treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;

    bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)node, treeNodeFlags, node->name().c_str());

    if (ImGui::IsItemClicked())
        mSelectedObjectID = node->id();

    sceneNodeDragDropSource(node);
    sceneNodeDragDropTarget(node);

    if (nodeOpen)
    {
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
        ImGui::Text(std::format("{} (Scene Node)", node->name()).c_str());
        ImGui::EndDragDropSource();
    }
}

void Editor::sceneNodeDragDropTarget(SceneNode *node)
{
    if (ImGui::BeginDragDropTarget())
    {
        checkPayloadType("SceneNode");

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SceneNode"))
        {
            SceneNode* transferNode = *(SceneNode**)payload->Data;

            transferNode->orphan();
            transferNode->markDirty();

            node->addChild(transferNode);
        }

        ImGui::EndDragDropTarget();
    }
}

void Editor::viewportPreRender()
{
    static constexpr ImGuiWindowFlags windowFlags {
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoTitleBar
    };

    ImGui::Begin("Viewport", &mShowViewport, windowFlags);

    ImGui::Dummy(ImGui::GetContentRegionAvail());
    modelDragDropTarget();
}

// todo: come up with better approach
void Editor::viewportPostRender()
{
    ImGui::End();
}

void Editor::cameraPanel()
{
    ImGui::Begin("Camera", &mShowCameraPanel);

    ImGui::SliderFloat("Field of View", mCamera.fov(), 1.f, 177.f, "%0.f");
    ImGui::DragFloat("Near Plane", mCamera.nearPlane(), 0.1f, 0.1f, *mCamera.farPlane(), "%.1f");
    ImGui::DragFloat("Far Plane", mCamera.farPlane(), 0.1f, *mCamera.nearPlane(), FLT_MAX, "%.1f");
    ImGui::DragFloat("Fly Speed", mCamera.flySpeed(), 1.f, 0.f, FLT_MAX, "%.0f");
    ImGui::DragFloat("Pan Speed", mCamera.panSpeed(), 1.f, 0.f, FLT_MAX, "%.0f");
    ImGui::DragFloat("Z Scroll Offset", mCamera.zScrollOffset(), 1.f, 0.f, FLT_MAX, "%.0f");
    ImGui::DragFloat("Rotate Sensitivity", mCamera.rotateSensitivity(), 1.f, 0.f, FLT_MAX, "%.0f");
    ImGui::SeparatorText("Camera Mode");

    static int selectedIndex = 0;
    static const std::array<const char*, 3> items {{
        "First Person",
        "View",
        "Edit"
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
    }

    if (selectedIndex == 0)
        mCamera.setState(Camera::FIRST_PERSON);
    if (selectedIndex == 1)
        mCamera.setState(Camera::VIEW_MODE);
    if (selectedIndex == 2)
        mCamera.setState(Camera::EDITOR_MODE);

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

void Editor::modelDragDropSource(uuid64_t modelID)
{
    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("Model", &modelID, sizeof(uuid64_t));
        ImGui::Text(std::format("{} (Model)", mResourceManager->mModelNames.at(modelID)).c_str());
        ImGui::EndDragDropSource();
    }
}

void Editor::modelDragDropTarget()
{
    if (ImGui::BeginDragDropTarget())
    {
        checkPayloadType("Model");

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Model"))
        {
            uuid64_t modelID = *(uuid64_t*)payload->Data;
            std::shared_ptr<Model> model = mResourceManager->getModel(modelID);
            mSceneGraph.mRoot.addChild(createModelGraph(model, model->root, &mSceneGraph.mRoot));
        }

        ImGui::EndDragDropTarget();
    }
}

// todo: fix mesh instance::addInstance
// todo: what if node doesn't have a material
SceneNode *Editor::createModelGraph(std::shared_ptr<Model> model, const Model::Node &modelNode, SceneNode* parent)
{
    SceneNode* sceneNode;

    if (auto meshID = modelNode.meshID)
    {
        uint32_t instanceID = mResourceManager->mMeshes.at(*meshID)->addInstance({}, {}, {});
        index_t materialIndex = 0;
        std::string matName = "";

        if (auto materialID = model->getMaterialID(modelNode))
            materialIndex = mResourceManager->getMatIndex(*materialID);

        if (auto& matNameOpt = modelNode.materialName)
            matName = *matNameOpt;

        sceneNode = new MeshNode(NodeType::Mesh,
                                 modelNode.name,
                                 modelNode.transformation,
                                 parent,
                                 *meshID,
                                 instanceID,
                                 materialIndex,
                                 matName);
    }
    else
    {
        sceneNode = new SceneNode(NodeType::Empty, modelNode.name, modelNode.transformation, parent);
    }

    for (const auto& child : modelNode.children)
        sceneNode->addChild(createModelGraph(model, child, sceneNode));

    return sceneNode;
}

void Editor::checkPayloadType(const char *type)
{
    const ImGuiPayload* payload = ImGui::GetDragDropPayload();

    if (payload && strcmp(payload->DataType, type) != 0)
        ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
}

void Editor::inspectorPanel()
{
    ImGui::Begin("Inspector", &mShowInspectorPanel);

    if (auto objectType = UUIDRegistry::getObjectType(mSelectedObjectID))
        if (*objectType == ObjectType::Model)
            modelInspector(mSelectedObjectID);

    ImGui::End();
}

void Editor::rendererPanel()
{
    ImGui::Begin("Renderer", &mShowRendererPanel);
    ImGui::End();
}

void Editor::console()
{
    ImGui::Begin("Console", &mShowConsole);
    ImGui::End();
}

void Editor::debugPanel()
{
    ImGui::Begin("Debug", &mShowDebugPanel);
    ImGui::End();
}

void Editor::modelInspector(uuid64_t modelID)
{
    auto model = mResourceManager->getModel(modelID);

    ImGui::Text("Asset type: Model");
    ImGui::Text("Name: %s", mResourceManager->mModelNames.at(modelID).c_str());
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Mapped Materials"))
    {
        for (const auto& [mappedMaterialName, mappedMaterialID] : model->mappedMaterials)
        {
            const char* selectedMat = mResourceManager->mMaterialNames.at(mappedMaterialID).c_str();

            if (ImGui::BeginCombo(mappedMaterialName.c_str(), selectedMat))
            {
                for (const auto& [materialID, materialName] : mResourceManager->mMaterialNames)
                {
                    bool selected = mappedMaterialID == materialID;
                    if (ImGui::Selectable(materialName.c_str(), selected))
                    {
                        index_t materialIndex = mResourceManager->getMatIndex(materialID);
                        model->remapMaterial(mappedMaterialName, materialID, materialIndex);
                    }
                }

                ImGui::EndCombo();
            }
        }
    }
}
