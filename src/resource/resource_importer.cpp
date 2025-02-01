//
// Created by Gianni on 23/01/2025.
//

#include "resource_importer.hpp"

// todo: handle multiple scenes
// todo: handle multiple sub-meshes
// todo: handle multiple primitive types
// todo: handle multiple tex coords
// todo: handle textures from extensions
// todo: handle missing textures
// todo: handle embedded image data
// todo: error handling
// todo: handle texture sampler and wrap
// todo: look for diffuse map if workflow is specular
// todo: handle texture loading fails
// todo: handle "Load error: No LoadImageData callback specified"
// todo: add support for separate metallic/roughness textures
// todo: add support for lights

namespace ResourceImporter
{
    std::future<std::shared_ptr<LoadedModelData>> loadModel(const std::filesystem::path &path, EnqueueCallback callback)
    {
        return std::async(std::launch::async, [path, callback] () -> std::shared_ptr<LoadedModelData> {
            std::shared_ptr<tinygltf::Model> gltfModel = loadGltfScene(path);
            std::shared_ptr<LoadedModelData> modelData = std::make_shared<LoadedModelData>();

            modelData->path = path;
            modelData->name = path.filename().string();
            modelData->root = createModelGraph(*gltfModel, gltfModel->nodes.at(gltfModel->scenes.at(0).nodes.at(0)));
            modelData->materials = loadMaterials(*gltfModel);
            modelData->indirectTextureMap = createIndirectTextureToImageMap(*gltfModel);

            // load mesh data
            std::vector<std::future<MeshData>> meshDataFutures;
            for (const auto& gltfMesh : gltfModel->meshes)
                meshDataFutures.push_back(createMeshData(*gltfModel, gltfMesh));

            // upload mesh data to opengl
            for (auto& meshDataFuture : meshDataFutures)
            {
                callback([modelData, meshData = meshDataFuture.get()] () {
                    modelData->meshes.push_back(createMesh(meshData));
                });
            }

            // load texture data
            std::filesystem::path directory = path.parent_path();
            std::vector<std::future<std::shared_ptr<LoadedImage>>> loadedImageFutures;
            for (const auto& image : gltfModel->images)
                loadedImageFutures.push_back(loadImageData(image, directory));

            // upload texture data to opengl
            for (auto& loadedImageFuture : loadedImageFutures)
            {
                callback([modelData, imageData = loadedImageFuture.get()] () {
                    modelData->textures.push_back(makeTexturePathPair(imageData));
                });
            }

            return modelData;
        });
    }

    std::shared_ptr<tinygltf::Model> loadGltfScene(const std::filesystem::path &path)
    {
        debugLog("ResourceImporter: Loading " + path.string());

        std::shared_ptr<tinygltf::Model> model = std::make_shared<tinygltf::Model>();
        tinygltf::TinyGLTF loader;
        std::string error;
        std::string warning;

        if (fileExtension(path) == ".gltf")
            loader.LoadASCIIFromFile(model.get(), &error, &warning, path.string());
        else if (fileExtension(path) == ".glb")
            loader.LoadBinaryFromFile(model.get(), &error, &warning, path.string());

        check(error.empty(), std::format("Failed to load model {}\nLoad error: {}", path.string(), error).c_str());

        if (!warning.empty())
            debugLog("ResourceImporter::loadGltfScene: Warning: " + warning);

        return model;
    }

    Model::Node createModelGraph(const tinygltf::Model& model, const tinygltf::Node& node)
    {
        Model::Node modelNode {
            .name = node.name,
            .transformation = getNodeTransformation(node)
        };

        if (node.mesh != -1)
            modelNode.mesh = node.mesh;

        for (uint32_t i = 0; i < node.children.size(); ++i)
            modelNode.children.push_back(createModelGraph(model, model.nodes.at(node.children.at(i))));

        return modelNode;
    }

    glm::mat4 getNodeTransformation(const tinygltf::Node& node)
    {
        if (!node.matrix.empty())
        {
            return glm::make_mat4(node.matrix.data());
        }

        glm::mat4 transformation = glm::identity<glm::mat4>();

        if (!node.translation.empty())
        {
            transformation = glm::translate(transformation, glm::make_vec3((float*)node.translation.data()));
        }

        if (!node.rotation.empty())
        {
            glm::quat q = glm::make_quat(node.rotation.data());
            transformation *= glm::mat4(q);
        }

        if (!node.scale.empty())
        {
            transformation = glm::scale(transformation, glm::make_vec3((float*)node.scale.data()));
        }

        return transformation;
    }

    LoadedModelData::Mesh createMesh(const MeshData &meshData)
    {
        return {
            .name = meshData.name,
            .mesh = std::make_shared<InstancedMesh>(meshData.vertices, meshData.indices),
            .materialIndex = meshData.materialIndex
        };
    }

    std::future<MeshData> createMeshData(const tinygltf::Model& model, const tinygltf::Mesh& mesh)
    {
        debugLog(std::format("ResourceImporter: Loading mesh {}", mesh.name));
        return std::async(std::launch::async, [&model, &mesh]() -> MeshData
        {
            return {
                .name = mesh.name,
                .vertices = loadMeshVertices(model, mesh),
                .indices = loadMeshIndices(model, mesh),
                .materialIndex = static_cast<uint32_t>(mesh.primitives.at(0).material)
            };
        });
    }

    std::vector<InstancedMesh::Vertex> loadMeshVertices(const tinygltf::Model& model, const tinygltf::Mesh& mesh)
    {
        std::vector<InstancedMesh::Vertex> vertices;

        const tinygltf::Primitive& primitive = mesh.primitives.at(0);

        size_t vertexCount = model.accessors.at(primitive.attributes.at("POSITION")).count;
        vertices.reserve(vertexCount);

        const float* positionBuffer = getBufferVertexData(model, primitive, "POSITION");
        const float* texCoordsBuffer = getBufferVertexData(model, primitive, "TEXCOORD_0");
        const float* normalsBuffer = getBufferVertexData(model, primitive, "NORMAL");
        const float* tangentBuffer = getBufferVertexData(model, primitive, "TANGENT");

        for (size_t j = 0; j < vertexCount; ++j)
        {
            InstancedMesh::Vertex vertex{};

            if (positionBuffer)
            {
                vertex.position = glm::make_vec3(&positionBuffer[j * 3]);
            }

            if (texCoordsBuffer)
            {
                vertex.texCoords = glm::make_vec2(&texCoordsBuffer[j * 2]);
            }

            if (normalsBuffer)
            {
                vertex.normal = glm::normalize(glm::make_vec3(&normalsBuffer[j * 3]));
            }

            if (tangentBuffer)
            {
                vertex.tangent = glm::normalize(glm::make_vec3(&tangentBuffer[j * 3]));
                vertex.bitangent = glm::normalize(glm::cross(vertex.tangent, vertex.normal));
            }

            vertices.push_back(vertex);
        }

        return vertices;
    }

    const float* getBufferVertexData(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const std::string& attribute)
    {
        const float* data = nullptr;

        if (primitive.attributes.contains(attribute))
        {
            const tinygltf::Accessor& accessor = model.accessors.at(primitive.attributes.at(attribute));
            const tinygltf::BufferView& bufferView = model.bufferViews.at(accessor.bufferView);
            const tinygltf::Buffer& buffer = model.buffers.at(bufferView.buffer);
            data = reinterpret_cast<const float*>(&buffer.data.at(bufferView.byteOffset + accessor.byteOffset));
        }

        return data;
    }

    std::vector<uint32_t> loadMeshIndices(const tinygltf::Model& model, const tinygltf::Mesh& mesh)
    {
        std::vector<uint32_t> indices;

        const tinygltf::Primitive& primitive = mesh.primitives.at(0);

        if (primitive.indices == -1)
            return indices;

        indices.reserve(model.accessors.at(primitive.indices).count);

        const tinygltf::Accessor& accessor = model.accessors.at(primitive.indices);
        const tinygltf::BufferView& bufferView = model.bufferViews.at(accessor.bufferView);
        const tinygltf::Buffer& buffer = model.buffers.at(bufferView.buffer);
        const uint8_t* data = &buffer.data.at(bufferView.byteOffset + accessor.byteOffset);

        for (size_t j = 0; j < accessor.count; ++j)
            indices.push_back(static_cast<uint32_t>(data[j]));

        return indices;
    }

    std::vector<LoadedModelData::Material> loadMaterials(const tinygltf::Model& model)
    {
        std::vector<LoadedModelData::Material> materials(model.materials.size());
        for (uint32_t i = 0; i < model.materials.size(); ++i)
        {
            const tinygltf::Material& gltfMaterial = model.materials.at(i);

            materials.at(i).name = gltfMaterial.name;

            int32_t baseColorTextureIndex = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
            int32_t metallicRoughnessTextureIndex = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
            int32_t normalTextureIndex = gltfMaterial.normalTexture.index;
            int32_t aoTextureIndex = gltfMaterial.occlusionTexture.index;
            int32_t emissionTextureIndex = gltfMaterial.emissiveTexture.index;

            if (baseColorTextureIndex != -1)
                materials.at(i).textures[BaseColor] = model.textures.at(baseColorTextureIndex).source;
            if (metallicRoughnessTextureIndex != -1)
                materials.at(i).textures[MetallicRoughness] = model.textures.at(metallicRoughnessTextureIndex).source;
            if (normalTextureIndex != -1)
                materials.at(i).textures[Normal] = model.textures.at(normalTextureIndex).source;
            if (aoTextureIndex != -1)
                materials.at(i).textures[Ao] = model.textures.at(aoTextureIndex).source;
            if (emissionTextureIndex != -1)
                materials.at(i).textures[Emission] = model.textures.at(emissionTextureIndex).source;

            materials.at(i).baseColorFactor = glm::make_vec4(gltfMaterial.pbrMetallicRoughness.baseColorFactor.data());
            materials.at(i).emissionFactor = glm::vec4(glm::make_vec3(gltfMaterial.emissiveFactor.data()), 0.f);
            materials.at(i).metallicFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
            materials.at(i).roughnessFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);
            materials.at(i).occlusionFactor = static_cast<float>(gltfMaterial.occlusionTexture.strength);
        }

        return materials;
    }

    std::unordered_map<int32_t, int32_t> createIndirectTextureToImageMap(const tinygltf::Model& model)
    {
        std::unordered_map<int32_t, int32_t> map;

        for (uint32_t i = 0; i < model.textures.size(); ++i)
            map.emplace(i, model.textures.at(i).source);

        return map;
    }

    std::future<std::shared_ptr<LoadedImage>> loadImageData(const tinygltf::Image& image, const std::filesystem::path& directory)
    {
        debugLog(std::format("ResourceImporter: Loading image {}", (directory / image.uri).string()));
        return std::async(std::launch::async, [&image, &directory] () -> std::shared_ptr<LoadedImage> {
            return std::make_shared<LoadedImage>(directory / image.uri);
        });
    }

    std::pair<std::shared_ptr<Texture2D>, std::filesystem::path> makeTexturePathPair(const std::shared_ptr<LoadedImage>& imageData)
    {
        TextureSpecification specification {
            .width = imageData->width(),
            .height = imageData->height(),
            .format = imageData->format(),
            .dataType = imageData->dataType(),
            .wrapMode = TextureWrap::Repeat,
            .filterMode = TextureFilter::Anisotropic,
            .generateMipMaps = true
        };

        return {std::make_shared<Texture2D>(specification, imageData->data()), imageData->path()};
    }
}
