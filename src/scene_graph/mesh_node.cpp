//
// Created by Gianni on 26/01/2025.
//

#include "mesh_node.hpp"

MeshNode::MeshNode()
    : SceneNode()
    , mMesh()
    , mMaterialIndex()
    , mInstanceID()
{
}

MeshNode::MeshNode(NodeType type, const std::string& name, SceneNode* parent, const glm::mat4& transformation,
                   uint32_t materialIndex, std::shared_ptr<InstancedMesh> mesh)
    : SceneNode(type, name, transformation, parent)
    , mMesh(mesh)
    , mMaterialIndex(materialIndex)
    , mInstanceID(mesh->addInstance(mGlobalTransform, mID, mMaterialIndex))
{
}

MeshNode::~MeshNode()
{
    mMesh->removeInstance(mInstanceID);
}

void MeshNode::notify(const Message &message)
{
    if (const auto m = message.getIf<Message::MaterialDeleted>())
    {
        auto updateMaterialIndex = [this](uint32_t newMaterialIndex) {
            mMaterialIndex = newMaterialIndex;
            mMesh->updateInstance(mInstanceID, mGlobalTransform, mID, mMaterialIndex);
        };

        if (mMaterialIndex == m->deletedIndex)
            updateMaterialIndex(m->defaultMaterialIndex);
        else if (m->movedMaterialIndex.has_value() && mMaterialIndex == m->movedMaterialIndex)
            updateMaterialIndex(m->deletedIndex);
    }
}

void MeshNode::updateGlobalTransform()
{
    if (mDirty)
    {
        if (mParent)
        {
            mGlobalTransform = mParent->globalTransform() * mLocalTransform;
        }
        else
        {
            mGlobalTransform = mLocalTransform;
        }

        mDirty = false;
        mMesh->updateInstance(mInstanceID, mGlobalTransform, mID, mMaterialIndex);
    }

    for (auto child : mChildren)
        child->updateGlobalTransform();
}

std::shared_ptr<InstancedMesh> MeshNode::mesh() const
{
    return mMesh;
}
