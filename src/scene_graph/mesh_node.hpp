//
// Created by Gianni on 26/01/2025.
//

#ifndef OPENGLRENDERINGENGINE_MESH_NODE_HPP
#define OPENGLRENDERINGENGINE_MESH_NODE_HPP

#include "scene_node.hpp"

class MeshNode : public SceneNode
{
public:
    MeshNode();
    MeshNode(NodeType type, const std::string& name, const glm::mat4& transformation, SceneNode* parent,
             uint32_t meshID, uint32_t instanceID, uint32_t materialIndex);
    ~MeshNode();

    void notify(const Message &message) override;
    void updateGlobalTransform() override;

    uint32_t meshID() const;
    uint32_t instanceID() const;

private:
    uint32_t mMeshID;
    uint32_t mInstanceID;
    uint32_t mMaterialIndex;
    bool mModifiedMaterial;
};

#endif //OPENGLRENDERINGENGINE_MESH_NODE_HPP
