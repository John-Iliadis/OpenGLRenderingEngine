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

        textureDragDropSource(textureID);
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
    ImGui::Begin("Inspector", &mShowInspectorPanel, ImGuiWindowFlags_NoScrollbar);

    if (auto objectType = UUIDRegistry::getObjectType(mSelectedObjectID))
        if (*objectType == ObjectType::Model)
            modelInspector(mSelectedObjectID);

    if (auto objectType = UUIDRegistry::getObjectType(mSelectedObjectID))
        if (*objectType == ObjectType::Material)
            materialInspector(mSelectedObjectID);

    if (auto objectType = UUIDRegistry::getObjectType(mSelectedObjectID))
        if (*objectType == ObjectType::Texture)
            textureInspector(mSelectedObjectID);

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

    ImGui::Text("Asset Type: Model");
    ImGui::Text("Name: %s", mResourceManager->mModelNames.at(modelID).c_str());
    ImGui::Separator();

    // todo: put this into a function
    if (ImGui::CollapsingHeader("Mapped Materials", ImGuiTreeNodeFlags_DefaultOpen))
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

    ImGui::Text("Todo: Add reset button");
}

void Editor::materialInspector(uuid64_t materialID)
{
    static constexpr ImGuiColorEditFlags sColorEditFlags {
        ImGuiColorEditFlags_DisplayRGB |
        ImGuiColorEditFlags_AlphaBar
    };

    index_t matIndex = mResourceManager->getMatIndex(materialID);
    Material& material = mResourceManager->mMaterialArray.at(matIndex);

    ImGui::Text("Asset Type: Material");
    ImGui::Text("Name: %s", mResourceManager->mMaterialNames.at(materialID).c_str());
    ImGui::Separator();

    bool matNeedsUpdate = false;

    if (ImGui::CollapsingHeader("Material Textures", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (materialTextureInspector(material.baseColorTexIndex, "Base Color"))
            matNeedsUpdate = true;
        if (materialTextureInspector(material.metallicRoughnessTexIndex, "Metallic Roughness"))
            matNeedsUpdate = true;
        if (materialTextureInspector(material.normalTexIndex, "Normal"))
            matNeedsUpdate = true;
        if (materialTextureInspector(material.aoTexIndex, "Ambient Occlusion"))
            matNeedsUpdate = true;
        if (materialTextureInspector(material.emissionTexIndex, "Emission"))
            matNeedsUpdate = true;
    }

    if (ImGui::CollapsingHeader("Factors", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::ColorEdit4("Base Color Factor", &material.baseColorFactor[0], sColorEditFlags))
            matNeedsUpdate = true;

        if (ImGui::ColorEdit3("Emission Factor", &material.emissionFactor[0], sColorEditFlags))
            matNeedsUpdate = true;

        if (ImGui::SliderFloat("Metallic Factor", &material.metallicFactor, 0.f, 1.f))
            matNeedsUpdate = true;

        if (ImGui::SliderFloat("Roughness Factor", &material.roughnessFactor, 0.f, 1.f))
            matNeedsUpdate = true;

        if (ImGui::SliderFloat("Occlusion Factor", &material.occlusionFactor, 0.f, 1.f))
            matNeedsUpdate = true;
    }

    if (ImGui::CollapsingHeader("Texture Coordinates", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::DragFloat2("Tiling", &material.tiling[0], 0.01f))
            matNeedsUpdate = true;

        if (ImGui::DragFloat2("Offset", &material.offset[0], 0.01f))
            matNeedsUpdate = true;
    }

    if (matNeedsUpdate)
        mResourceManager->updateMaterial(matIndex);
}

bool Editor::materialTextureInspector(index_t &textureIndex, std::string label)
{
    static constexpr ImVec2 sImageSize = ImVec2(20.f, 20.f);
    static constexpr ImVec2 sTooltipImageSize = ImVec2(250.f, 250.f);
    static constexpr ImVec2 sImageButtonFramePadding = ImVec2(2.f, 2.f);
    static const ImVec2 sTextSize = ImGui::CalcTextSize("H");

    std::shared_ptr<Texture2D> texture = mResourceManager->getTextureFromIndex(textureIndex);

    bool matNeedsUpdate = false;

    uuid64_t textureID = mResourceManager->getTexIDFromIndex(textureIndex);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, sImageButtonFramePadding);
    if (ImGui::ImageButton((ImTextureID)(intptr_t)texture->id(), sImageSize))
        ImGui::OpenPopup(label.c_str());
    ImGui::PopStyleVar();

    if (ImGui::BeginPopup(label.c_str()))
    {
        ImGui::SeparatorText("Select Texture:");

        if (auto selectedTexID = textureCombo(textureID))
        {
            textureIndex = mResourceManager->getTextureIndex(*selectedTexID);
            matNeedsUpdate = true;
        }

        ImGui::EndPopup();
    }

    if (textureDragDropTarget(textureIndex))
        matNeedsUpdate = true;

    if (ImGui::BeginItemTooltip())
    {
        ImGui::Image((ImTextureID)(intptr_t)texture->id(), sTooltipImageSize);
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sImageSize.y / 2.f - sTextSize.y / 2.f + 1.f);
    ImGui::Text("%s | (%s)", label.c_str(), mResourceManager->mTextureNames.at(textureID).c_str());

    return matNeedsUpdate;
}

void Editor::modelDragDropSource(uuid64_t modelID)
{
    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("Model", &modelID, sizeof(uuid64_t));
        ImGui::Text("%s (Model)", mResourceManager->mModelNames.at(modelID).c_str());
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

void Editor::textureDragDropSource(uuid64_t textureID)
{
    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("Texture", &textureID, sizeof(uuid64_t));
        ImGui::Text("%s (Texture)", mResourceManager->mTextureNames.at(textureID).c_str());
        ImGui::EndDragDropSource();
    }
}

bool Editor::textureDragDropTarget(index_t& textureIndex)
{
    if (ImGui::BeginDragDropTarget())
    {
        checkPayloadType("Texture");

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Texture"))
        {
            uuid64_t textureID = *(uuid64_t*)payload->Data;
            textureIndex = mResourceManager->getTextureIndex(textureID);
            return true;
        }

        ImGui::EndDragDropTarget();
    }

    return false;
}

std::optional<uuid64_t> Editor::textureCombo(uuid64_t selectedTextureID)
{
    for (const auto& [textureID, textureName] : mResourceManager->mTextureNames)
    {
        bool selected = selectedTextureID == textureID;

        if (ImGui::Selectable(textureName.c_str(), selected))
            return textureID;
    }

    return std::nullopt;
}

void Editor::textureInspector(uuid64_t textureID)
{
    auto texture = mResourceManager->getTexture(textureID);
    const ImTextureID imTextureId = (ImTextureID)(intptr_t)texture->id();
    const ImVec2 textureSize(texture->width(), texture->height());

    ImGui::Text("Asset Type: Texture");
    ImGui::Text("Name: %s", mResourceManager->mTextureNames.at(textureID).c_str());

    ImGui::SeparatorText("Texture Info");

    ImGui::Text("Format: %s", toStr(texture->format()));
    ImGui::Text("Data Type: %s", toStr(texture->dataType()));
    ImGui::Text("Wrap Mode: %s", toStr(texture->wrapMode()));
    ImGui::Text("Filter Mode: %s", toStr(texture->filterMode()));
    ImGui::Text("Size: %dx%d", (int)textureSize.x, (int)textureSize.y);

    ImGui::SeparatorText("Preview");

    float windowWidth = ImGui::GetContentRegionAvail().x;
    float newHeight = (windowWidth / textureSize.x) * textureSize.y;
    float xPadding = (ImGui::GetContentRegionAvail().x - windowWidth) * 0.5f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xPadding);
    ImGui::Image(imTextureId, ImVec2(windowWidth, newHeight));
}
