//
// Created by Gianni on 23/01/2025.
//

#include "resource_manager.hpp"

static gpu_tex_handle64_t makeBindless(uint32_t textureID)
{
    gpu_tex_handle64_t gpuTextureHandle = glGetTextureHandleARB(textureID);
    glMakeTextureHandleResidentARB(gpuTextureHandle);
    return gpuTextureHandle;
}

template <typename T>
static bool resourceLoaded(const std::unordered_map<T, std::filesystem::path>& resourceMap, const std::filesystem::path& path)
{
    return std::any_of(resourceMap.begin(), resourceMap.end(), [&path] (const auto& pair) {
        return pair.second == path;
    });
}

template <typename T>
static std::optional<uuid64_t> getTID(const std::unordered_map<uuid64_t, T>& resourceMap, const T& t)
{
    for (const auto& [id, item] : resourceMap)
        if (item == t)
            return id;
    return {};
}

ResourceManager::ResourceManager()
    : SubscriberSNS({Topic::Type::Resources, Topic::Type::SceneGraph})
    , mBindlessTextureSSBO(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, 1, 1024 * sizeof(gpu_tex_handle64_t), nullptr)
    , mMaterialsSSBO(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, 2, 256 * sizeof(Material), nullptr)
{
    loadDefaultTextures();
    loadDefaultMaterial();
}

ResourceManager::~ResourceManager()
{
    for (gpu_tex_handle64_t gpuTextureHandle : mBindlessTextureArray)
        glMakeTextureHandleNonResidentARB(gpuTextureHandle);
}

bool ResourceManager::importModel(const std::filesystem::path &path)
{
    if (resourceLoaded(mModelPaths, path))
    {
        debugLog(std::format("Model \"{}\" is already loaded.", path.string()));
        return false;
    }

    EnqueueCallback callback = [this] (Task&& t) {mTaskQueue.push(std::move(t));};
    mLoadedModelFutures.push_back(ResourceImporter::loadModel(path, callback));

    return true;
}

void ResourceManager::notify(const Message &message)
{
    if (const auto m = message.getIf<Message::MeshInstanceUpdate>())
    {
        mMeshes.at(m->meshID)->updateInstance(m->instanceID, m->transformation, m->objectID, m->matIndex);
    }

    if (const auto m = message.getIf<Message::RemoveMeshInstance>())
    {
        mMeshes.at(m->meshID)->removeInstance(m->instanceID);
    }

    if (const auto m = message.getIf<Message::MaterialDeleted>())
    {
        for (auto& [materialID, materialIndex] : mMaterials)
        {
            if (materialIndex == m->removeIndex)
                materialIndex = 0;

            if (m->transferIndex.has_value() && m->transferIndex == materialIndex)
                materialIndex = m->removeIndex;
        }
    }

    if (const auto m = message.getIf<Message::TextureDeleted>())
    {
        for (auto& material : mMaterialArray)
        {
            if (m->removedIndex == material.baseColorTexIndex)
                material.baseColorTexIndex = DefaultBaseColorTexIndex;

            if (m->removedIndex == material.metallicRoughnessTexIndex)
                material.metallicRoughnessTexIndex = DefaultMetallicRoughnessTexIndex;

            if (m->removedIndex == material.normalTexIndex)
                material.normalTexIndex = DefaultNormalTexIndex;

            if (m->removedIndex == material.aoTexIndex)
                material.aoTexIndex = DefaultAoTexIndex;

            if (m->removedIndex == material.emissionTexIndex)
                material.emissionTexIndex = DefaultEmissionTexIndex;

            if (m->transferIndex.has_value() && m->transferIndex == material.baseColorTexIndex)
                material.baseColorTexIndex = m->removedIndex;

            if (m->transferIndex.has_value() && m->transferIndex == material.metallicRoughnessTexIndex)
                material.metallicRoughnessTexIndex = m->removedIndex;

            if (m->transferIndex.has_value() && m->transferIndex == material.normalTexIndex)
                material.normalTexIndex = m->removedIndex;

            if (m->transferIndex.has_value() && m->transferIndex == material.aoTexIndex)
                material.aoTexIndex = m->removedIndex;

            if (m->transferIndex.has_value() && m->transferIndex == material.emissionTexIndex)
                material.emissionTexIndex = m->removedIndex;
        }

        mMaterialsSSBO.update(0, mMaterialArray.size() * sizeof(Material), mMaterialArray.data());
    }
}

void ResourceManager::processMainThreadTasks()
{
    while (auto task = mTaskQueue.pop())
        (*task)();

    for (auto itr = mLoadedModelFutures.begin(); itr != mLoadedModelFutures.end();)
    {
        if (itr->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            auto loadedModel = itr->get();
            addModel(loadedModel);
            itr = mLoadedModelFutures.erase(itr);
        }
        else
            ++itr;
    }
}

std::shared_ptr<Model> ResourceManager::getModel(uuid64_t id)
{
    return mModels.at(id);
}

std::shared_ptr<InstancedMesh> ResourceManager::getMesh(uuid64_t id)
{
    return mMeshes.at(id);
}

std::shared_ptr<Texture> ResourceManager::getTexture(uuid64_t id)
{
    return mTextures.at(id);
}

std::shared_ptr<Texture2D> ResourceManager::getTextureFromIndex(index_t texIndex)
{
    gpu_tex_handle64_t gpuTexHandle = mBindlessTextureArray.at(texIndex);

    for (const auto& [texID, gpuTexHandle_] : mBindlessTextureMap)
        if (gpuTexHandle_ == gpuTexHandle)
            return mTextures.at(texID);

    return nullptr;
}

index_t ResourceManager::getMatIndex(uuid64_t id)
{
    return mMaterials.at(id);
}

void ResourceManager::updateMaterial(index_t materialIndex)
{
    mMaterialsSSBO.update(materialIndex * sizeof(Material), sizeof(Material), &mMaterialArray.at(materialIndex));
}

void ResourceManager::addModel(std::shared_ptr<LoadedModelData> modelData)
{
    // add meshes, textures, and materials to resources
    std::unordered_map<index_t, uuid64_t> loadedMeshIndexToMeshUUID = addMeshes(modelData);
    std::unordered_map<index_t, uint32_t> loadedTextureIndexToResourceIndex = addTextures(modelData);
    std::unordered_map<std::string, uuid64_t> loadedMatNameToMatID = addMaterials(modelData, loadedTextureIndexToResourceIndex);

    // add model
    uuid64_t modelID = UUIDRegistry::generateModelID();
    std::shared_ptr<Model> model = std::make_shared<Model>();

    model->root = createModelNodeHierarchy(modelData, modelData->root, loadedMeshIndexToMeshUUID, loadedMatNameToMatID);
    model->bb = modelData->bb;
    model->mappedMaterials = loadedMatNameToMatID;

    mModels.emplace(modelID, model);
    mModelNames.emplace(modelID, modelData->name);
    mModelPaths.emplace(modelID, modelData->path);
}

std::unordered_map<index_t, uuid64_t> ResourceManager::addMeshes(std::shared_ptr<LoadedModelData> modelData)
{
    std::unordered_map<index_t, uuid64_t> loadedMeshIndexToMeshUUID;

    for (size_t i = 0; i < modelData->meshes.size(); ++i)
    {
        const auto& loadedMesh = modelData->meshes.at(i);

        uuid64_t meshID = UUIDRegistry::generateMeshID();
        mMeshes.emplace(meshID, loadedMesh.mesh);
        mMeshNames.emplace(meshID, loadedMesh.name);

        loadedMeshIndexToMeshUUID.emplace(i, meshID);
    }

    return loadedMeshIndexToMeshUUID;
}

std::unordered_map<index_t, uint32_t> ResourceManager::addTextures(std::shared_ptr<LoadedModelData> modelData)
{
    std::unordered_map<index_t, uint32_t> loadedTextureIndexToResourceIndex;

    for (size_t i = 0; i < modelData->textures.size(); ++i)
    {
        const auto& [texture, texturePath] = modelData->textures.at(i);

        uuid64_t textureID = UUIDRegistry::generateTextureID();
        mTextures.emplace(textureID, texture);
        mTextureNames.emplace(textureID, texturePath.filename().string());
        mTexturePaths.emplace(textureID, texturePath);

        gpu_tex_handle64_t gpuTexHandle = makeBindless(texture->id());
        mBindlessTextureMap.emplace(textureID, gpuTexHandle);
        mBindlessTextureArray.push_back(gpuTexHandle);

        loadedTextureIndexToResourceIndex.emplace(i, mBindlessTextureArray.size() - 1);
    }

    mBindlessTextureSSBO.update(0, mBindlessTextureArray.size() * sizeof(gpu_tex_handle64_t), mBindlessTextureArray.data());

    return loadedTextureIndexToResourceIndex;
}

std::unordered_map<std::string, uuid64_t> ResourceManager::addMaterials(std::shared_ptr<LoadedModelData> modelData,
                                                                        const std::unordered_map<index_t, uint32_t> &loadedTextureIndexToResourceIndex)
{
    std::unordered_map<std::string, uuid64_t> loadedMatNameToMatID;

    for (size_t i = 0; i < modelData->materials.size(); ++i)
    {
        const auto& loadedMaterial = modelData->materials.at(i);

        Material material {
            .baseColorTexIndex = DefaultBaseColorTexIndex,
            .metallicRoughnessTexIndex = DefaultMetallicRoughnessTexIndex,
            .normalTexIndex = DefaultNormalTexIndex,
            .aoTexIndex = DefaultNormalTexIndex,
            .emissionTexIndex = DefaultEmissionTexIndex,
            .baseColorFactor = loadedMaterial.baseColorFactor,
            .emissionFactor = loadedMaterial.emissionColorFactor,
            .metallicFactor = loadedMaterial.metallicFactor,
            .roughnessFactor = loadedMaterial.roughnessFactor,
            .occlusionFactor = loadedMaterial.occlusionFactor,
            .tiling = glm::vec2(1.f),
            .offset = glm::vec2(0.f)
        };

        if (loadedMaterial.baseColorTexIndex != -1)
        {
            material.baseColorTexIndex = loadedTextureIndexToResourceIndex.at(modelData->getTextureIndex(loadedMaterial.baseColorTexIndex));
        }

        if (loadedMaterial.metallicRoughnessTexIndex != -1)
        {
            material.metallicRoughnessTexIndex = loadedTextureIndexToResourceIndex.at(modelData->getTextureIndex(loadedMaterial.metallicRoughnessTexIndex));
        }

        if (loadedMaterial.normalTexIndex != -1)
        {
            material.normalTexIndex = loadedTextureIndexToResourceIndex.at(modelData->getTextureIndex(loadedMaterial.normalTexIndex));
        }

        if (loadedMaterial.aoTexIndex != -1)
        {
            material.aoTexIndex = loadedTextureIndexToResourceIndex.at(modelData->getTextureIndex(loadedMaterial.aoTexIndex));
        }

        if (loadedMaterial.emissionTexIndex != -1)
        {
            material.emissionTexIndex = loadedTextureIndexToResourceIndex.at(modelData->getTextureIndex(loadedMaterial.emissionTexIndex));
        }

        uuid64_t materialID = UUIDRegistry::generateMaterialID();
        mMaterials.emplace(materialID, mMaterialArray.size());
        mMaterialNames.emplace(materialID, loadedMaterial.name);
        mMaterialArray.push_back(material);

        loadedMatNameToMatID.emplace(loadedMaterial.name, materialID);
    }

    mMaterialsSSBO.update(0, mMaterialArray.size() * sizeof(Material), mMaterialArray.data());

    return loadedMatNameToMatID;
}

Model::Node ResourceManager::createModelNodeHierarchy(std::shared_ptr<LoadedModelData> modelData,
                                                      const LoadedModelData::Node& loadedNode,
                                                      const std::unordered_map<index_t, uuid64_t>& loadedMeshIndexToMeshUUID,
                                                      const std::unordered_map<std::string, uuid64_t>& loadedMatNameToMatID)
{
    Model::Node node {
        .name = loadedNode.name,
        .transformation = loadedNode.transformation
    };

    if (auto meshIndex = loadedNode.meshIndex)
    {
        node.meshID = loadedMeshIndexToMeshUUID.at(*meshIndex);

        if (auto matIndex = modelData->meshes.at(*meshIndex).materialIndex)
        {
            node.materialName = modelData->materials.at(*matIndex).name;
        }
    }

    for (const auto& child : loadedNode.children)
        node.children.push_back(
            createModelNodeHierarchy(modelData, child, loadedMeshIndexToMeshUUID, loadedMatNameToMatID));

    return node;
}

void ResourceManager::deleteModel(uuid64_t id)
{
    std::shared_ptr<Model> model = mModels.at(id);

    // delete model
    mModels.erase(id);
    mModelNames.erase(id);
    mModelPaths.erase(id);

    // delete model meshes
    std::unordered_set<uuid64_t> meshIDs = getModelMeshIDs(*model);

    for (uuid64_t meshID : meshIDs)
    {
        mMeshes.erase(meshID);
        mMeshNames.erase(meshID);
    }

    // send message
    SNS::publishMessage(Topic::Type::Resources, Message::create<Message::ModelDeleted>(id, meshIDs));
}

void ResourceManager::deleteTexture(uuid64_t id)
{
    std::shared_ptr<Texture2D> texture = mTextures.at(id);
    index_t removeIndex = mBindlessTextureMap.at(id);

    // delete texture
    mTextures.erase(id);
    mTextureNames.erase(id);
    mTexturePaths.erase(id);
    mBindlessTextureMap.erase(id);

    // make bindless texture non resident
    glMakeTextureHandleNonResidentARB(mBindlessTextureArray.at(removeIndex));

    // remove bindless texture handle
    std::optional<index_t> transferIndex;
    if (removeIndex != mBindlessTextureArray.size() - 1)
    {
        index_t lastIndex = mBindlessTextureArray.size() - 1;
        std::swap(mBindlessTextureArray.at(lastIndex), mBindlessTextureArray.at(removeIndex));
        transferIndex = lastIndex;
    }

    mBindlessTextureArray.pop_back();

    // update bindless texture ssbo
    mBindlessTextureSSBO.update(0, mBindlessTextureArray.size() * sizeof(gpu_tex_handle64_t), mBindlessTextureArray.data());

    // send message
    SNS::publishMessage(Topic::Type::SceneGraph, Message::create<Message::TextureDeleted>(id, removeIndex, transferIndex));
}

void ResourceManager::deleteMaterial(uuid64_t id)
{
    index_t removeIndex = mMaterials.at(id);

    // delete material
    mMaterials.erase(id);
    mMaterialNames.erase(id);

    // reformat material array
    std::optional<index_t> transferIndex;
    if (removeIndex != mMaterialArray.size() - 1)
    {
        index_t lastIndex = mMaterialArray.size() - 1;
        std::swap(mMaterialArray.at(lastIndex), mMaterialArray.at(removeIndex));
        transferIndex = lastIndex;
    }

    mMaterialArray.pop_back();

    // update SSBO
    mMaterialsSSBO.update(0, mMaterialArray.size() * sizeof(Material), mMaterialArray.data());

    // send message
    SNS::publishMessage(Topic::Type::Resources, Message::create<Message::MaterialDeleted>(id, removeIndex, transferIndex));
}

void ResourceManager::loadDefaultTextures()
{
    TextureSpecification textureSpecification {
        .width = 1,
        .height = 1,
        .format = TextureFormat::RGBA8,
        .dataType = TextureDataType::UINT8,
        .wrapMode = TextureWrap::Repeat,
        .filterMode = TextureFilter::Nearest,
        .generateMipMaps = false
    };

    float baseColorTexData[4] {1.f, 1.f, 1.f, 1.f};
    float metallicRoughnessTexData[4] {1.f, 1.f, 1.f, 1.f};
    float normalTexData[4] {0.5f, 0.5f, 1.f, 0.f};
    float aoTexData[4] {1.f, 1.f, 1.f, 1.f};
    float emissionTexData[4] {0.f, 0.f, 0.f, 0.f};

    auto baseColorTex = std::make_shared<Texture2D>(textureSpecification, baseColorTexData);
    auto metallicRoughnessTex = std::make_shared<Texture2D>(textureSpecification, metallicRoughnessTexData);
    auto normalTex = std::make_shared<Texture2D>(textureSpecification, normalTexData);
    auto aoTex = std::make_shared<Texture2D>(textureSpecification, aoTexData);
    auto emissionTex = std::make_shared<Texture2D>(textureSpecification, emissionTexData);

    uuid64_t baseColorID = UUIDRegistry::getDefTexID(MatTexType::BaseColor);
    uuid64_t metallicRoughnessID = UUIDRegistry::getDefTexID(MatTexType::MetallicRoughness);
    uuid64_t normalID = UUIDRegistry::getDefTexID(MatTexType::Normal);
    uuid64_t aoID = UUIDRegistry::getDefTexID(MatTexType::Ao);
    uuid64_t emissionID = UUIDRegistry::getDefTexID(MatTexType::Emission);

    mTextures.emplace(baseColorID, baseColorTex);
    mTextures.emplace(metallicRoughnessID, metallicRoughnessTex);
    mTextures.emplace(normalID, normalTex);
    mTextures.emplace(aoID, aoTex);
    mTextures.emplace(emissionID, emissionTex);

    mTextureNames.emplace(baseColorID, "Default Base Color");
    mTextureNames.emplace(metallicRoughnessID, "Default Metallic Roughness");
    mTextureNames.emplace(normalID, "Default Normal");
    mTextureNames.emplace(aoID, "Default Ambient Occlusion");
    mTextureNames.emplace(emissionID, "Default Emission");

    gpu_tex_handle64_t baseColorTexHandle = makeBindless(baseColorTex->id());
    gpu_tex_handle64_t metallicRoughnessTexHandle = makeBindless(metallicRoughnessTex->id());
    gpu_tex_handle64_t normalTexHandle = makeBindless(normalTex->id());
    gpu_tex_handle64_t aoTexHandle = makeBindless(aoTex->id());
    gpu_tex_handle64_t emissionTexHandle = makeBindless(emissionTex->id());

    mBindlessTextureMap.emplace(baseColorID, baseColorTexHandle);
    mBindlessTextureMap.emplace(metallicRoughnessID, metallicRoughnessTexHandle);
    mBindlessTextureMap.emplace(normalID, normalTexHandle);
    mBindlessTextureMap.emplace(aoID, aoTexHandle);
    mBindlessTextureMap.emplace(emissionID, emissionTexHandle);

    mBindlessTextureArray.push_back(baseColorTexHandle);
    mBindlessTextureArray.push_back(metallicRoughnessTexHandle);
    mBindlessTextureArray.push_back(normalTexHandle);
    mBindlessTextureArray.push_back(aoTexHandle);
    mBindlessTextureArray.push_back(emissionTexHandle);

    mBindlessTextureSSBO.update(0, mBindlessTextureArray.size() * sizeof(gpu_tex_handle64_t), mBindlessTextureArray.data());
}

void ResourceManager::loadDefaultMaterial()
{
    Material material {
        .baseColorTexIndex = DefaultBaseColorTexIndex,
        .metallicRoughnessTexIndex = DefaultMetallicRoughnessTexIndex,
        .normalTexIndex = DefaultNormalTexIndex,
        .aoTexIndex = DefaultAoTexIndex,
        .emissionTexIndex = DefaultEmissionTexIndex,
        .baseColorFactor = glm::vec4(0.5f, 0.5f, 0.5f, 1.f),
        .emissionFactor = glm::vec4(0.f),
        .metallicFactor = 1.f,
        .roughnessFactor = 1.f,
        .occlusionFactor = 1.f,
        .tiling = glm::vec2(1.f),
        .offset = glm::vec2(0.f)
    };

    uuid64_t materialID = UUIDRegistry::getDefMatID();
    mMaterials.emplace(materialID, 0);
    mMaterialNames.emplace(materialID, "Default Material");
    mMaterialArray.push_back(material);

    mMaterialsSSBO.update(0, sizeof(Material), mMaterialArray.data());
}

std::optional<uuid64_t> ResourceManager::getModelID(const std::shared_ptr<Model>& model)
{
    return getTID(mModels, model);
}

std::optional<uuid64_t> ResourceManager::getMeshID(const std::shared_ptr<InstancedMesh>& mesh)
{
    return getTID(mMeshes, mesh);
}

std::optional<uuid64_t> ResourceManager::getTextureID(const std::shared_ptr<Texture2D>& texture)
{
    return getTID(mTextures, texture);
}

index_t ResourceManager::getTextureIndex(uuid64_t id)
{
    gpu_tex_handle64_t gpuTexHandle = mBindlessTextureMap.at(id);

    for (index_t i = 0; i < mBindlessTextureArray.size(); ++i)
        if (gpuTexHandle == mBindlessTextureArray.at(i))
            return i;
    return -1;
}

uuid64_t ResourceManager::getTexIDFromIndex(index_t texIndex)
{
    gpu_tex_handle64_t gpuTexHandle = mBindlessTextureArray.at(texIndex);

    for (const auto& [textureID, gpuTexHandle_] : mBindlessTextureMap)
        if (gpuTexHandle == gpuTexHandle_)
            return textureID;

    return -1;
}
