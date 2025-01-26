//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_INSTANCED_MESH_HPP
#define OPENGLRENDERINGENGINE_INSTANCED_MESH_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "../opengl/buffer.hpp"

// todo: add color vertex
class InstancedMesh
{
public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 texCoords;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;
    };

    struct InstanceData
    {
        glm::mat4 modelMatrix;
        glm::mat3 normalMatrix;
        uint32_t id;
        uint32_t materialIndex;
    };

public:
    InstancedMesh();
    InstancedMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    uint32_t addInstance(const glm::mat4& model, uint32_t id, uint32_t materialIndex);
    void updateInstance(uint32_t instanceID, const glm::mat4& model, uint32_t id, uint32_t materialIndex);
    void removeInstance(uint32_t instanceID);

private:
    void checkResize();
    uint32_t generateInstanceID();

    static VertexBufferLayout getVertexBufferLayout();
    static VertexBufferLayout getInstanceBufferLayout();

private:
    VertexArray mVertexArray;
    IndexBuffer mIndexBuffer;
    VertexBuffer mVertexBuffer;
    VertexBuffer mInstanceBuffer;

    uint32_t mInstanceCount;
    uint32_t mInstanceBufferCapacity;

    std::unordered_map<uint32_t, uint32_t> mInstanceIdToIndexMap;
    std::unordered_map<uint32_t, uint32_t> mInstanceIndexToIdMap;
};

#endif //OPENGLRENDERINGENGINE_INSTANCED_MESH_HPP
