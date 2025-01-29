//
// Created by Gianni on 26/01/2025.
//

#include "mesh_node.hpp"

MeshNode::MeshNode()
    : SceneNode()
    , mMeshID()
    , mMaterialIndex()
    , mInstanceID()
    , mModifiedMaterial()
{
    subscribe(Topic::Type::ResourceManager);
}

MeshNode::MeshNode(NodeType type, const std::string& name, const glm::mat4& transformation, SceneNode* parent,
                   uint32_t meshID, uint32_t instanceID, uint32_t materialIndex)
    : SceneNode(type, name, transformation, parent)
    , mMeshID(meshID)
    , mInstanceID(instanceID)
    , mMaterialIndex(materialIndex)
    , mModifiedMaterial()
{
    subscribe(Topic::Type::ResourceManager);
}

MeshNode::~MeshNode()
{
    SNS::publishMessage(Topic::Type::SceneGraph, Message::create<Message::RemoveMeshInstance>(mMeshID, mInstanceID));
}

void MeshNode::notify(const Message &message)
{
    if (const auto m = message.getIf<Message::MaterialDeleted>())
    {
        if (m->removeIndex == mMaterialIndex)
            mMaterialIndex = 0;

        if (m->transferIndex.has_value() && m->transferIndex == mMaterialIndex)
            mMaterialIndex = m->removeIndex;
    }

    if (const auto m = message.getIf<Message::MaterialRemap>())
    {
        if (!mModifiedMaterial && mMeshID == m->meshID)
            mMaterialIndex = m->newMaterialIndex;
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

        SNS::publishMessage(Topic::Type::SceneGraph, Message::create<Message::MeshInstanceUpdate>(mMeshID, mID, mInstanceID, mMaterialIndex, mGlobalTransform));
    }

    for (auto child : mChildren)
        child->updateGlobalTransform();
}

uint32_t MeshNode::meshID() const
{
    return mMeshID;
}

uint32_t MeshNode::instanceID() const
{
    return mInstanceID;
}
