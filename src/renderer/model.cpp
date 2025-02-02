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
                materialID = defaultmaterialid;
        }
    }
}

std::optional<uuid64_t> Model::getMaterialID(const Model::Node &node) const
{
    if (node.materialName.has_value())
        return mMappedMaterials.at(node.materialName.value());
    return std::nullopt;
}
