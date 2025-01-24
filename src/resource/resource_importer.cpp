//
// Created by Gianni on 23/01/2025.
//

#include "resource_importer.hpp"

int stbi_is_hdr(const char*);

static constexpr uint32_t sImportFlags
{
    aiProcess_CalcTangentSpace |
    aiProcess_JoinIdenticalVertices |
    aiProcess_Triangulate |
    aiProcess_RemoveComponent |
    aiProcess_GenNormals |
    aiProcess_ImproveCacheLocality |
    aiProcess_RemoveRedundantMaterials |
    aiProcess_SortByPType |
    aiProcess_GenUVCoords |
    aiProcess_OptimizeMeshes |
    aiProcess_OptimizeGraph
};

static constexpr int sRemoveComponents
{
    aiComponent_BONEWEIGHTS |
    aiComponent_ANIMATIONS |
    aiComponent_LIGHTS |
    aiComponent_CAMERAS |
    aiComponent_COLORS
};

static constexpr int sRemovePrimitives
{
    aiPrimitiveType_POINT |
    aiPrimitiveType_LINE
};

// todo: exception handling
namespace ResourceImporter
{
    std::future<std::shared_ptr<LoadedModel>> loadModel(const std::filesystem::path &path, EnqueueCallback callback)
    {
        return std::async(std::launch::async, [path, callback] () -> std::shared_ptr<LoadedModel> {
            std::shared_ptr<aiScene> assimpScene = loadAssimpScene(path);

            std::shared_ptr<LoadedModel> model = std::make_shared<LoadedModel>();
            model->modelName = path.stem().string();
            model->root = createModelGraph(*assimpScene->mRootNode);

            // load mesh data
            std::vector<std::future<MeshData>> meshDataFutures;
            for (uint32_t i = 0; i < assimpScene->mNumMeshes; ++i)
            {
                const aiMesh& assimpMesh = *assimpScene->mMeshes[i];
                meshDataFutures.push_back(createMeshData(assimpMesh));
            }

            // upload mesh data to opengl
            for (auto& meshDataFuture : meshDataFutures)
            {
                callback([model, meshData = meshDataFuture.get()] () {
                    model->meshes.push_back(createMesh(meshData));
                });
            }

            // load materials
            std::unordered_set<std::filesystem::path> uniqueTexturePaths;
            for (uint32_t i = 0; i < assimpScene->mNumMaterials; ++i)
            {
                const aiMaterial& assimpMaterial = *assimpScene->mMaterials[i];
                LoadedModel::Material material = createMaterial(assimpMaterial, path.parent_path());

                uniqueTexturePaths.insert(material.albedoTexturePath);
                uniqueTexturePaths.insert(material.specularTexturePath);
                uniqueTexturePaths.insert(material.roughnessTexturePath);
                uniqueTexturePaths.insert(material.metallicTexturePath);
                uniqueTexturePaths.insert(material.normalTexturePath);
                uniqueTexturePaths.insert(material.displacementTexturePath);
                uniqueTexturePaths.insert(material.aoTexturePath);
                uniqueTexturePaths.insert(material.emissionTexturePath);

                model->materials.push_back(material);
            }

            uniqueTexturePaths.erase("");

            // load texture data
            std::vector<std::future<std::shared_ptr<ImageLoader>>> loadedImageFutures;
            for (const auto& texturePath : uniqueTexturePaths)
                loadedImageFutures.push_back(createTextureData(texturePath));

            // upload texture data to opengl
            for (auto& loadedImageFuture : loadedImageFutures)
            {
                callback([model, loadedImage = loadedImageFuture.get()] () {
                    if (loadedImage->success())
                    {
                        model->textures.emplace(createTexturePair(*loadedImage));
                    }
                });
            }

            return model;
        });
    }

    LoadedModel::Mesh createMesh(const MeshData &meshData)
    {
        return {
            .name = meshData.name,
            .mesh = std::make_shared<InstancedMesh>(meshData.vertices, meshData.indices),
            .materialIndex = meshData.materialIndex
        };
    }

    std::pair<std::filesystem::path, Texture2D> createTexturePair(const ImageLoader &loadedImage)
    {
        assert(loadedImage.success());

        TextureSpecification textureSpecification{
            .width = loadedImage.width(),
            .height = loadedImage.height(),
            .format = loadedImage.format(),
            .dataType = loadedImage.dataType(),
            .wrapMode = TextureWrap::Repeat,
            .filterMode = TextureFilter::Anisotropic,
            .generateMipMaps = true
        };

        return std::make_pair(loadedImage.path(), Texture2D(textureSpecification, loadedImage.data()));
    }

    std::shared_ptr<aiScene> loadAssimpScene(const std::filesystem::path &path)
    {
        Assimp::Importer importer;
        importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, sRemoveComponents);
        importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, sRemovePrimitives);
        importer.SetPropertyBool(AI_CONFIG_PP_PTV_NORMALIZE, true);

        importer.ReadFile(path.string(), sImportFlags);

        std::shared_ptr<aiScene> assimpScene(importer.GetOrphanedScene());

        if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
        {
            debugLog(std::format("Failed to load model {}.", path.string()));
            return nullptr;
        }

        return assimpScene;
    }

    Model::Node createModelGraph(const aiNode &assimpNode)
    {
        Model::Node node{
            .name = assimpNode.mName.C_Str(),
            .model = assimpToGlmMat4(assimpNode.mTransformation)
        };

        for (uint32_t i = 0; i < assimpNode.mNumMeshes; ++i)
            node.meshes.push_back(i);

        for (uint32_t i = 0; i < assimpNode.mNumChildren; ++i)
            node.children.push_back(createModelGraph(*assimpNode.mChildren[i]));

        return node;
    }

    std::future<MeshData> createMeshData(const aiMesh &assimpMesh)
    {
        debugLog(std::format("Loading mesh {}", assimpMesh.mName.C_Str()));
        return std::async(std::launch::async, [&assimpMesh]() -> MeshData
        {
            return {
                .name = assimpMesh.mName.C_Str(),
                .vertices = ResourceImporter::loadMeshVertices(assimpMesh),
                .indices = ResourceImporter::loadMeshIndices(assimpMesh),
                .materialIndex = assimpMesh.mMaterialIndex
            };
        });
    }

    std::future<std::shared_ptr<ImageLoader>> createTextureData(const std::filesystem::path &path)
    {
        debugLog(std::format("ResourceImporter: Loading {}", path.string()));
        return std::async(std::launch::async, [path]() -> std::shared_ptr<ImageLoader>
        {
            std::shared_ptr<ImageLoader> loadedImage = std::make_shared<ImageLoader>(path.string());

            if (!loadedImage->success())
            {
                debugLog(std::format("ResourceImporter: Failed to load {}", path.string()));
            }

            return loadedImage;
        });
    }

    LoadedModel::Material createMaterial(const aiMaterial &assimpMaterial, const std::filesystem::path &directory)
    {
        auto texturePath = [&assimpMaterial, &directory](aiTextureType textureType) -> std::filesystem::path
        {
            std::string textureFilename = getTextureName(assimpMaterial, textureType);

            if (textureFilename.empty())
                return {};

            return directory / textureFilename;
        };

        LoadedModel::Material material{
            .name = assimpMaterial.GetName().C_Str(),
            .albedoTexturePath = texturePath(aiTextureType_DIFFUSE),
            .specularTexturePath = texturePath(aiTextureType_SPECULAR),
            .roughnessTexturePath = texturePath(aiTextureType_DIFFUSE_ROUGHNESS),
            .metallicTexturePath = texturePath(aiTextureType_METALNESS),
            .normalTexturePath = texturePath(aiTextureType_HEIGHT),
            .displacementTexturePath = texturePath(aiTextureType_DISPLACEMENT),
            .aoTexturePath = texturePath(aiTextureType_AMBIENT_OCCLUSION),
            .emissionTexturePath = texturePath(aiTextureType_EMISSIVE)
        };

        aiColor4D albedoColor;
        aiColor4D emissionColor;

        if (assimpMaterial.Get(AI_MATKEY_COLOR_DIFFUSE, albedoColor) == AI_SUCCESS)
            material.albedoColor = *reinterpret_cast<glm::vec4 *>(&albedoColor);
        if (assimpMaterial.Get(AI_MATKEY_COLOR_EMISSIVE, emissionColor) == AI_SUCCESS)
            material.emissionColor = *reinterpret_cast<glm::vec4 *>(&emissionColor);

        return material;
    }

    std::vector<InstancedMesh::Vertex> loadMeshVertices(const aiMesh &mesh)
    {
        std::vector<InstancedMesh::Vertex> vertices(mesh.mNumVertices);

        for (size_t i = 0; i < vertices.size(); ++i)
        {
            vertices.at(i).position = *reinterpret_cast<glm::vec3 *>(&mesh.mVertices[i]);
            vertices.at(i).normal = *reinterpret_cast<glm::vec3 *>(&mesh.mNormals[i]);

            if (mesh.HasTangentsAndBitangents())
            {
                vertices.at(i).tangent = *reinterpret_cast<glm::vec3 *>(&mesh.mTangents[i]);
                vertices.at(i).bitangent = *reinterpret_cast<glm::vec3 *>(&mesh.mBitangents[i]);
            }

            if (mesh.HasTextureCoords(0))
            {
                vertices.at(i).texCoords = *reinterpret_cast<glm::vec2 *>(&mesh.mTextureCoords[0][i]);
            }
        }

        return vertices;
    }

    std::vector<uint32_t> loadMeshIndices(const aiMesh &mesh)
    {
        std::vector<uint32_t> indices(mesh.mNumFaces * 3);

        for (uint32_t i = 0; i < mesh.mNumFaces; ++i)
        {
            const aiFace &face = mesh.mFaces[i];

            indices.at(i) = face.mIndices[0];
            indices.at(i + 1) = face.mIndices[1];
            indices.at(i + 2) = face.mIndices[2];
        }

        return indices;
    }

    glm::mat4 assimpToGlmMat4(const aiMatrix4x4 &matrix)
    {
        return {
            matrix.a1, matrix.b1, matrix.c1, matrix.d1,
            matrix.a2, matrix.b2, matrix.c2, matrix.d2,
            matrix.a3, matrix.b3, matrix.c3, matrix.d3,
            matrix.a4, matrix.b4, matrix.c4, matrix.d4
        };
    }

// todo: maybe use this
//aiString fileBaseColor, fileMetallic, fileRoughness;
//material->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &fileBaseColor);
//material->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &fileMetallic);
//material->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &fileRoughness);
    std::string getTextureName(const aiMaterial &material, aiTextureType textureType)
    {
        if (material.GetTextureCount(textureType))
        {
            aiString textureFilename;
            material.GetTexture(textureType, 0, &textureFilename);
            return textureFilename.C_Str();
        }

        return {};
    }
}
