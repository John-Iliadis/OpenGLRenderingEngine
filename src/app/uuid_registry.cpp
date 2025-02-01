//
// Created by Gianni on 2/02/2025.
//

#include "uuid_registry.hpp"

uuid64_t UUIDRegistry::generateID(ObjectType type)
{
    static uuid64_t counter = 10;
    mIdToType.emplace(counter, type);
    return counter++;
}

ObjectType UUIDRegistry::getObjectType(uuid64_t id)
{
    return mIdToType.at(id);
}

uuid64_t UUIDRegistry::generateModelID()
{
    return generateID(ObjectType::Model);
}

uuid64_t UUIDRegistry::generateTextureID()
{
    return generateID(ObjectType::Texture);
}

uuid64_t UUIDRegistry::generateMaterialID()
{
    return generateID(ObjectType::Material);
}

uuid64_t UUIDRegistry::generateMeshID()
{
    return generateID(ObjectType::Mesh);
}

uuid64_t UUIDRegistry::generateSceneNodeID()
{
    return generateID(ObjectType::SceneNode);
}
