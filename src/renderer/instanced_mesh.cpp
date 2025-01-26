//
// Created by Gianni on 23/01/2025.
//

#include "instanced_mesh.hpp"

static constexpr uint32_t sVertexSize = sizeof(InstancedMesh::Vertex);
static constexpr uint32_t sInstanceSize = sizeof(InstancedMesh::InstanceData);
static constexpr uint32_t sInitialInstanceBufferCapacity = 32;

InstancedMesh::InstancedMesh()
    : mInstanceCount()
    , mInstanceBufferCapacity(sInitialInstanceBufferCapacity)
{
}

InstancedMesh::InstancedMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    : mIndexBuffer(indices.size(), indices.data())
    , mVertexBuffer(GL_STATIC_DRAW, vertices.size() * sVertexSize, vertices.data())
    , mInstanceBuffer(GL_DYNAMIC_DRAW, sInitialInstanceBufferCapacity * sInstanceSize, nullptr)
    , mInstanceCount()
    , mInstanceBufferCapacity(sInitialInstanceBufferCapacity)
{
    mVertexArray.attachIndexBuffer(mIndexBuffer);
    mVertexArray.attachVertexBuffer(mVertexBuffer, getVertexBufferLayout(), 0);
    mVertexArray.attachVertexBuffer(mInstanceBuffer, getInstanceBufferLayout(), 1);
}

uint32_t InstancedMesh::addInstance(const glm::mat4 &model, uint32_t id, uint32_t materialIndex)
{
    checkResize();

    ++mInstanceCount;

    uint32_t instanceID = generateInstanceID();
    uint32_t instanceIndex = mInstanceCount - 1;

    assert(mInstanceIdToIndexMap.emplace(instanceID, instanceIndex).second);
    assert(mInstanceIndexToIdMap.emplace(instanceIndex, instanceID).second);

    InstancedMesh::InstanceData instanceData {
        .modelMatrix = model,
        .normalMatrix = glm::inverseTranspose(glm::mat3(model)),
        .id = id,
        .materialIndex = materialIndex
    };

    mInstanceBuffer.update(instanceID * sInstanceSize, sInstanceSize, &instanceData);

    return instanceID;
}

void InstancedMesh::updateInstance(uint32_t instanceID, const glm::mat4 &model, uint32_t id, uint32_t materialIndex)
{
    uint32_t instanceIndex = mInstanceIdToIndexMap.at(instanceID);

    InstancedMesh::InstanceData instanceData {
        .modelMatrix = model,
        .normalMatrix = glm::inverseTranspose(glm::mat3(model)),
        .id = id,
        .materialIndex = materialIndex
    };

    mInstanceBuffer.update(instanceIndex * sInstanceSize, sInstanceSize, &instanceData);
}

void InstancedMesh::removeInstance(uint32_t instanceID)
{
    // edge case: Only one instance left or last instance in buffer
    uint32_t removeIndex = mInstanceIdToIndexMap.at(instanceID);
    mInstanceIdToIndexMap.erase(instanceID);

    if (mInstanceCount == 1 || removeIndex == mInstanceCount - 1)
    {
        --mInstanceCount;
        mInstanceIndexToIdMap.erase(removeIndex);
        return;
    }

    // regular case: Instance is in range [first, last)
    uint32_t lastIndex = mInstanceCount - 1;

    glCopyNamedBufferSubData(mInstanceBuffer.id(),
                             mInstanceBuffer.id(),
                             lastIndex * sInstanceSize,
                             removeIndex * sInstanceSize,
                             sInstanceSize);

    uint32_t lastIndexInstanceID = mInstanceIndexToIdMap.at(lastIndex);
    mInstanceIdToIndexMap.at(lastIndexInstanceID) = removeIndex;
    mInstanceIndexToIdMap.at(removeIndex) = lastIndexInstanceID;
}

void InstancedMesh::checkResize()
{
    if (mInstanceCount < mInstanceBufferCapacity)
        return;

    uint32_t newCapacity = sInstanceSize * glm::ceil(glm::log2(static_cast<float>(mInstanceCount)));
    VertexBuffer newInstanceBuffer(GL_DYNAMIC_DRAW, newCapacity, nullptr);

    glCopyNamedBufferSubData(mInstanceBuffer.id(),
                             newInstanceBuffer.id(),
                             0, 0, mInstanceBufferCapacity * sInstanceSize);

    mInstanceBuffer = std::move(newInstanceBuffer);
    mInstanceBufferCapacity = newCapacity;

    mVertexArray.attachVertexBuffer(mInstanceBuffer, getInstanceBufferLayout(), 1);
}

uint32_t InstancedMesh::generateInstanceID()
{
    static uint32_t counter = 0;
    return counter++;
}

VertexBufferLayout InstancedMesh::getVertexBufferLayout()
{
    VertexBufferLayout layout;

    layout.setStride(sVertexSize);
    layout.setStepRate(StepRate::Vertex);

    layout.addAttribute(0, 3, GL_FLOAT, offsetof(InstancedMesh::Vertex, position));
    layout.addAttribute(1, 2, GL_FLOAT, offsetof(InstancedMesh::Vertex, texCoords));
    layout.addAttribute(2, 3, GL_FLOAT, offsetof(InstancedMesh::Vertex, normal));
    layout.addAttribute(3, 3, GL_FLOAT, offsetof(InstancedMesh::Vertex, tangent));
    layout.addAttribute(4, 3, GL_FLOAT, offsetof(InstancedMesh::Vertex, bitangent));

    return layout;
}

VertexBufferLayout InstancedMesh::getInstanceBufferLayout()
{
    VertexBufferLayout layout;

    layout.setStride(sInstanceSize);
    layout.setStepRate(StepRate::Instance);

    layout.addAttribute(5, 4, GL_FLOAT, offsetof(InstancedMesh::InstanceData, modelMatrix));
    layout.addAttribute(6, 4, GL_FLOAT, offsetof(InstancedMesh::InstanceData, modelMatrix) + sizeof(glm::vec4));
    layout.addAttribute(7, 4, GL_FLOAT, offsetof(InstancedMesh::InstanceData, modelMatrix) + sizeof(glm::vec4) * 2);
    layout.addAttribute(8, 4, GL_FLOAT, offsetof(InstancedMesh::InstanceData, modelMatrix) + sizeof(glm::vec4) * 3);

    layout.addAttribute(9, 4, GL_FLOAT, offsetof(InstancedMesh::InstanceData, normalMatrix));
    layout.addAttribute(10, 4, GL_FLOAT, offsetof(InstancedMesh::InstanceData, normalMatrix) + sizeof(glm::vec4));
    layout.addAttribute(11, 4, GL_FLOAT, offsetof(InstancedMesh::InstanceData, normalMatrix) + sizeof(glm::vec4) * 2);

    layout.addAttribute(12, 1, GL_UNSIGNED_INT, offsetof(InstancedMesh::InstanceData, id));
    layout.addAttribute(13, 1, GL_UNSIGNED_INT, offsetof(InstancedMesh::InstanceData, materialIndex));

    return layout;
}
