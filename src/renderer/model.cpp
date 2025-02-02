//
// Created by Gianni on 2/02/2025.
//
#include "model.hpp"

Model::Model()
    : SubscriberSNS({Topic::Type::SceneGraph})
    , root()
    , bb()
{
}

void Model::notify(const Message &message)
{
    if (const auto m = message.getIf<Message::MaterialDeleted>())
    {
        for (auto& [materialName, materialID] : mMappedMaterials)
        {
            if (materialID == m->materialID)
                materialID = UUIDRegistry::getDefMatID();
        }
    }
}

std::optional<uuid64_t> Model::getMaterialID(const Model::Node &node) const
{
    if (node.materialName.has_value())
        return mMappedMaterials.at(node.materialName.value());
    return std::nullopt;
}

static void getModelMeshesRecursive(const Model::Node& node, std::unordered_set<uuid64_t>& meshIDs)
{
    if (auto meshID = node.meshID)
        meshIDs.insert(*meshID);

    for (const auto& child : node.children)
        getModelMeshesRecursive(child, meshIDs);
}

std::unordered_set<uuid64_t> getModelMeshIDs(const Model& model)
{
    std::unordered_set<uuid64_t> modelMeshes;

    getModelMeshesRecursive(model.root, modelMeshes);

    return modelMeshes;
}