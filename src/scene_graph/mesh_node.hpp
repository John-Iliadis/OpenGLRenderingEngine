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
    MeshNode(NodeType type, const std::string& name, SceneNode* parent,
             uint32_t materialIndex, std::shared_ptr<InstancedMesh> mesh);
    ~MeshNode();

    void notify(const Message &message) override;
    void updateGlobalTransform() override;

    std::shared_ptr<InstancedMesh> mesh() const;

private:
    std::shared_ptr<InstancedMesh> mMesh;
    uint32_t mMaterialIndex;
    uint32_t mInstanceID;
};

#endif //OPENGLRENDERINGENGINE_MESH_NODE_HPP
