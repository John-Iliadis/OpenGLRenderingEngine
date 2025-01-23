//
// Created by Gianni on 23/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_INSTANCED_MESH_HPP
#define OPENGLRENDERINGENGINE_INSTANCED_MESH_HPP

#include <glm/glm.hpp>
#include "../opengl/buffer.hpp"

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
        glm::mat4 normalMatrix;
        uint32_t id;
        uint32_t materialIndex;
    };

public:

private:
    VertexArray mVertexArray;
    IndexBuffer mIndexBuffer;
    VertexBuffer mVertexBuffer;
    VertexBuffer mInstanceBuffer;

    uint32_t
};

#endif //OPENGLRENDERINGENGINE_INSTANCED_MESH_HPP
