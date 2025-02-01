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
    : SubscriberSNS({Topic::Type::ResourceManager, Topic::Type::SceneGraph})
    , mBindlessTextureSSBO(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, 1, 1024 * sizeof(gpu_tex_handle64_t), nullptr)
    , mMaterialsSSBO(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, 2, 256 * sizeof(Material), nullptr)
{
    loadDefaultTextures();
    loadDefaultMaterial();
}

ResourceManager::~ResourceManager()
{
    for (gpu_tex_handle64_t gpuTextureHandle : mBindlessTextures)
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
        mMeshes.at(m->meshID)->updateInstance(m->instanceID, m->transformation, m->objectID, m->materialIndex);
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
            for (uint32_t i = 0; i < MaterialTextureCount; ++i)
            {
                if (material.textures[i] == m->removedIndex)
                    material.textures[i] = gDefaultTextureMap.at(i);

                if (m->transferIndex.has_value() && material.textures[i] == m->transferIndex)
                    material.textures[i] = m->removedIndex;
            }
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

// todo: make this easier to work with
void ResourceManager::addModel(std::shared_ptr<LoadedModelData> modelData)
{
    // create model
    std::shared_ptr<Model> model = std::make_shared<Model>();

    model->root = std::move(modelData->root);
    model->bb = modelData->bb;

    uuid64_t modelID = UUIDRegistry::generateModelID();

    mModels.emplace(modelID, model);
    mModelNames.emplace(modelID, modelData->name);
    mModelPaths.emplace(modelID, modelData->path);

    // load textures
    std::unordered_map<index_t, index_t> insertedTextureIndexMap; // loaded texture index -> resource index
    for (size_t i = 0; i < modelData->textures.size(); ++i)
    {
        const auto& [texture, texturePath] = modelData->textures.at(i);

        uuid64_t textureID = UUIDRegistry::generateTextureID();
        mTextures.emplace(textureID, texture);
        mTextureNames.emplace(textureID, texturePath.filename().string());
        mTexturePaths.emplace(textureID, texturePath);

        gpu_tex_handle64_t gpuTextureHandle = makeBindless(texture->id());
        mBindlessTextureMap.emplace(textureID, gpuTextureHandle);
        mBindlessTextures.push_back(gpuTextureHandle);

        insertedTextureIndexMap.emplace(i, mBindlessTextures.size() - 1);
    }

    // load materials
    std::unordered_map<index_t, index_t> insertedMaterialIndexMap; // loaded material index -> resource index
    for (size_t i = 0; i < modelData->materials.size(); ++i)
    {
        const LoadedModelData::Material& loadedMaterial = modelData->materials.at(i);

        Material material;
        material.workflow = MetallicWorkflow;
        material.baseColorFactor = loadedMaterial.baseColorFactor;
        material.emissionFactor = loadedMaterial.emissionFactor;
        material.specularGlossinessFactor = loadedMaterial.specularGlossinessFactor;
        material.metallicFactor = loadedMaterial.metallicFactor;
        material.roughnessFactor = loadedMaterial.roughnessFactor;
        material.occlusionFactor = loadedMaterial.occlusionFactor;
        material.specularFactor = loadedMaterial.specularFactor;

        for (int32_t ii = 0; ii < MaterialTextureCount; ++ii)
        {
            int32_t textureIndex = loadedMaterial.textures[ii];

            if (textureIndex != -1)
            {
                index_t indirectIndex = modelData->indirectTextureMap.at(textureIndex);
                material.textures[ii] = insertedTextureIndexMap.at(indirectIndex);
            }
        }

        uuid64_t materialID = UUIDRegistry::generateMaterialID();
        mMaterials.emplace(materialID, mMaterials.size() - 1);
        mMaterialNames.emplace(materialID, loadedMaterial.name);
        mMaterialArray.push_back(material);

        model->indirectMaterialMap.emplace(i, loadedMaterial.name);
        model->mappedMaterials.emplace(loadedMaterial.name, mMaterials.size() - 1);

        insertedMaterialIndexMap.emplace(i, mMaterials.size() - 1);
    }

    // load meshes
    for (size_t i = 0; i < modelData->meshes.size(); ++i)
    {
        const LoadedModelData::Mesh& loadedMesh = modelData->meshes.at(i);

        uuid64_t meshID = UUIDRegistry::generateMeshID();
        mMeshes.emplace(meshID, loadedMesh.mesh);
        mMeshNames.emplace(meshID, loadedMesh.name);

        model->meshes.emplace_back(i, loadedMesh.materialIndex);
        model->indirectMeshMap.emplace(i, meshID);
    }

    // update SSBOs
    mBindlessTextureSSBO.update(0, mBindlessTextures.size() * sizeof(gpu_tex_handle64_t), mBindlessTextures.data());
    mMaterialsSSBO.update(0, mMaterialArray.size() * sizeof(Material), mMaterialArray.data());
}

void ResourceManager::deleteModel(uuid64_t id)
{
    std::shared_ptr<Model> model = mModels.at(id);

    // delete model
    mModels.erase(id);
    mModelNames.erase(id);
    mModelPaths.erase(id);

    // delete model meshes
    std::unordered_set<uuid64_t> meshIDs;
    for (const auto& mesh : model->meshes)
        meshIDs.insert(model->getMeshID(mesh));

    for (uuid64_t meshID : meshIDs)
    {
        mMeshes.erase(meshID);
        mMeshNames.erase(meshID);
    }

    // send message
    SNS::publishMessage(Topic::Type::ResourceManager, Message::create<Message::ModelDeleted>(id, meshIDs));
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
    glMakeTextureHandleNonResidentARB(mBindlessTextures.at(removeIndex));

    // remove bindless texture handle
    std::optional<index_t> transferIndex;
    if (removeIndex != mBindlessTextures.size() - 1)
    {
        index_t lastIndex = mBindlessTextures.size() - 1;
        std::swap(mBindlessTextures.at(lastIndex), mBindlessTextures.at(removeIndex));
        transferIndex = lastIndex;
    }

    mBindlessTextures.pop_back();

    // update bindless texture ssbo
    mBindlessTextureSSBO.update(0, mBindlessTextures.size() * sizeof(gpu_tex_handle64_t), mBindlessTextures.data());

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
    SNS::publishMessage(Topic::Type::ResourceManager, Message::create<Message::MaterialDeleted>(id, removeIndex, transferIndex));
}

void ResourceManager::loadDefaultTextures()
{
    float textureData[DefaultTextureCount][4] {
        {1.f, 1.f, 1.f, 1.f}, // BASE COLOR WHITE
        {0.5f, 0.5f, 0.5f, 1.f}, // BASE COLOR GREY
        {1.f, 1.f, 1.f, 1.f}, // METALLIC ROUGHNESS
        {0.5f, 0.5f, 1.f, 0.f}, // NORMAL
        {1.f, 1.f, 1.f, 1.f}, // AO
        {0.f, 0.f, 0.f, 0.f}, // EMISSION
        {1.f, 1.f, 1.f, 1.f}, // SPECULAR
        {0.f, 0.f, 0.f, 0.f} // DISPLACEMENT
    };

    const char* textureNames[] {
        "Default Base Color (White)",
        "Default Base Color (Grey)",
        "Default Metallic Roughness",
        "Default Normal",
        "Default Ambient Occlusion",
        "Default Emission",
        "Default Specular",
        "Default Displacement"
    };

    TextureSpecification textureSpecification {
        .width = 1,
        .height = 1,
        .format = TextureFormat::RGBA8,
        .dataType = TextureDataType::UINT8,
        .wrapMode = TextureWrap::Repeat,
        .filterMode = TextureFilter::Nearest,
        .generateMipMaps = false
    };

    for (uint32_t i = 0; i < DefaultTextureCount; ++i)
    {
        auto texture = std::make_shared<Texture2D>(textureSpecification, textureData[i]);
        uuid64_t textureId = UUIDRegistry::generateTextureID();
        mTextures.emplace(textureId, texture);
        mTextureNames.emplace(textureId, textureNames[i]);

        gpu_tex_handle64_t gpuTextureHandle = makeBindless(texture->id());
        mBindlessTextureMap.emplace(textureId, gpuTextureHandle);
        mBindlessTextures.push_back(gpuTextureHandle);
    }

    mBindlessTextureSSBO.update(0, mBindlessTextures.size() * sizeof(gpu_tex_handle64_t), mBindlessTextures.data());
}

void ResourceManager::loadDefaultMaterial()
{
    Material material;
    material.textures[BaseColor] = DefaultBaseColorGrey;

    uuid64_t materialID = UUIDRegistry::generateMaterialID();
    mMaterials.emplace(materialID, mMaterials.size());
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
